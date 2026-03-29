#pragma once

#include <string>


class WebSocketHandler {
public:
    WebSocketHandler();
    ~WebSocketHandler();

    static bool isWebSocketUpgrade(const std::string &request);
    static std::string generateHandshakeResponse(const std::string &request);
    static bool parseFrame(const std::string &data, std::string &outMessage);
    static std::string encodeFrame(const std::string &message);

private:
    static std::string extractSecWebSocketKey(const std::string &request);
    static std::string computeAcceptKey(const std::string &key);
};

