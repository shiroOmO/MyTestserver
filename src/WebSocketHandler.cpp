#include "WebSocketHandler.h"

#include <openssl/sha.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <cstring>
#include <sstream>


const std::string WEBSOCKET_MAGIC = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

WebSocketHandler::WebSocketHandler() {
}

WebSocketHandler::~WebSocketHandler() {
}

bool WebSocketHandler::isWebSocketUpgrade(const std::string &request) {
    if(request.find("Upgrade: websocket") == std::string::npos) {
        return false;
    }
    if(request.find("Connection: Upgrade") == std::string::npos) {
        return false;
    }
    if(request.find("Sec-WebSocket-Key: ") == std::string::npos) {
        return false;
    }
    return true;
}

std::string WebSocketHandler::extractSecWebSocketKey(const std::string &request) {
    size_t pos = request.find("Sec-WebSocket-Key: ");
    if(pos == std::string::npos) {
        return "";
    }
    pos += 19;
    size_t end = request.find("\r\n", pos);
    if(end == std::string::npos) {
        end = request.find("\n", pos);
    }
    return request.substr(pos, end - pos);
}

std::string base64Encode(const unsigned char *input, int length) {
    BIO *bio = BIO_new(BIO_s_mem());
    BIO *b64 = BIO_new(BIO_f_base64());
    BIO_push(b64, bio);
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    BIO_write(b64, input, length);
    BIO_flush(b64);
    char *out;
    long outLen = BIO_get_mem_data(bio, &out);
    std::string result(out, outLen);
    BIO_free_all(b64);
    return result;
}

std::string WebSocketHandler::computeAcceptKey(const std::string &key) {
    std::string combined = key + WEBSOCKET_MAGIC;
    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA1(reinterpret_cast<const unsigned char*>(combined.c_str()), combined.size(), hash);
    return base64Encode(hash, SHA_DIGEST_LENGTH);
}

std::string WebSocketHandler::generateHandshakeResponse(const std::string &request) {
    std::string key = extractSecWebSocketKey(request);
    std::string acceptKey = computeAcceptKey(key);

    std::ostringstream response;
    response << "HTTP/1.1 101 Switching Protocols\r\n"
             << "Upgrade: websocket\r\n"
             << "Connection: Upgrade\r\n"
             << "Sec-WebSocket-Accept: " << acceptKey << "\r\n"
             << "\r\n";
    return response.str();
}

bool WebSocketHandler::parseFrame(const std::string &data, std::string &outMessage) {
    size_t frameLen;
    unsigned char opcode;
    return parseFrame(data.c_str(), data.size(), outMessage, frameLen, opcode);
}

bool WebSocketHandler::parseFrame(const char *data, size_t dataLen, std::string &outMessage, size_t &frameLen, unsigned char &outOpcode) {
    if(dataLen < 2) {
        return false;
    }

    const unsigned char *bytes = reinterpret_cast<const unsigned char*>(data);
    size_t pos = 0;

    // First byte: FIN and Opcode
    unsigned char firstByte = bytes[pos++];
    bool fin = (firstByte & 0x80) != 0;
    unsigned char opcode = firstByte & 0x0F;
    outOpcode = opcode;

    // We only support text frames and close frames
    if(opcode != 0x1 && opcode != 0x8) { // Text or Close
        return false;
    }

    // Second byte: MASK and Payload length
    unsigned char secondByte = bytes[pos++];
    bool masked = (secondByte & 0x80) != 0;
    uint64_t payloadLen = secondByte & 0x7F;

    // Handle extended payload length
    if(payloadLen == 126) {
        if(dataLen < pos + 2)
            return false;
        payloadLen = (bytes[pos] << 8) | bytes[pos + 1];
        pos += 2;
    }
    else if(payloadLen == 127) {
        if(dataLen < pos + 8)
            return false;
        payloadLen = 0;
        for(int i = 0; i < 8; i++) {
            payloadLen = (payloadLen << 8) | bytes[pos + i];
        }
        pos += 8;
    }

    // Get masking key
    unsigned char maskingKey[4];
    if(masked) {
        if(dataLen < pos + 4)
            return false;
        for(int i = 0; i < 4; i++) {
            maskingKey[i] = bytes[pos + i];
        }
        pos += 4;
    }

    // Check we have all payload
    if(dataLen < pos + payloadLen) {
        return false;
    }

    // Calculate total frame length
    frameLen = pos + payloadLen;

    // Unmask payload
    outMessage.clear();
    outMessage.reserve(payloadLen);
    for(uint64_t i = 0; i < payloadLen; i++) {
        unsigned char decoded = bytes[pos + i] ^ maskingKey[i % 4];
        outMessage += static_cast<char>(decoded);
    }

    return fin;
}

std::string WebSocketHandler::encodeFrame(const std::string &message) {
    std::string frame;
    frame.reserve(2 + 8 + message.size());

    // First byte: FIN=1, Opcode=0x1 (text)
    unsigned char firstByte = 0x80 | 0x01;
    frame += static_cast<char>(firstByte);

    // Second byte: MASK=0 (server doesn't mask), Payload length
    size_t len = message.size();
    if(len <= 125) {
        unsigned char secondByte = static_cast<unsigned char>(len);
        frame += static_cast<char>(secondByte);
    }
    else if(len <= 65535) {
        frame += static_cast<char>(126);
        frame += static_cast<char>((len >> 8) & 0xFF);
        frame += static_cast<char>(len & 0xFF);
    }
    else {
        frame += static_cast<char>(127);
        for(int i = 7; i >= 0; i--) {
            frame += static_cast<char>((len >> (i * 8)) & 0xFF);
        }
    }

    // Add payload (no masking for server)
    frame += message;
    return frame;
}
