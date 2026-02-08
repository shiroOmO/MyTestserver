#pragma once

#include <string>
#include <unordered_map>


class Buffer;

class HttpResponse {
public:
    HttpResponse();

    void setStatusCode(const std::string &code);
    void setStatusMessage(const std::string &message);
    void addHeader(const std::string &key, const std::string &value);
    void setBody(const std::string &body);

    const std::string& getStatusCode() const;
    const std::string& getStatusMessage() const;

    void setKeepAlive(bool yon);
    bool getKeepAlive() const;

    std::string& getBody();
    void appendToBuffer(Buffer *output);

private:
    std::string statusCode_;
    std::string statusMessage_;
    std::unordered_map<std::string, std::string> headers_;
    std::string body_;
    bool keepAlive_;
};



