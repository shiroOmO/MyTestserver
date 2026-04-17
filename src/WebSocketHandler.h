#pragma once

#include <string>


class WebSocketHandler {
public:
    WebSocketHandler();
    ~WebSocketHandler();

    static bool isWebSocketUpgrade(const std::string &request);
    static std::string generateHandshakeResponse(const std::string &request);
    // Legacy: parse from string
    static bool parseFrame(const std::string &data, std::string &outMessage);
    // New: parse from buffer, returns whether a complete frame was parsed
    // on success, frameLen is set to the total bytes consumed (header + payload)
    // opcode is output to indicate frame type (0x1 = text, 0x8 = close)
    static bool parseFrame(const char *data, size_t dataLen, std::string &outMessage, size_t &frameLen, unsigned char &opcode);
    static std::string encodeFrame(const std::string &message);

private:
    static std::string extractSecWebSocketKey(const std::string &request);
    static std::string computeAcceptKey(const std::string &key);
};

