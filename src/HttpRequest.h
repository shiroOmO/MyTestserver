#pragma once

#include <string>
#include <unordered_map>


class Buffer;

class HttpRequest {
public:
    void setMethod(const char *start, const char *end);
    void setVersion(const char *start, const char *end);
    void setPath(const char *start, const char *end);
    void setQuery(const char *start, const char *end);
    void setBody(const char *start, const char *end);

    const std::string& getMethod() const ;
    const std::string& getVersion() const;
    const std::string& getPath() const;
    const std::string& getQuery() const;
    const std::string& getBody() const;

    void addHeader(const char *start, const char *colon, const char *end);
    std::string getHeader(const std::string &key) const;
    const std::unordered_map<std::string, std::string>& getHeaders() const;

private:
    std::string method_;
    std::string version_;
    std::string path_;
    std::string query_;
    std::string body_;
    std::unordered_map<std::string, std::string> headers_;
};


class HttpParser {
public:
    enum HttpRequestParseState {
        kExpectRequestLine,
        kExpectHeaders,
        kExpectBody,
        kGetAll
    };

    HttpParser();
    
    bool parseRequest(Buffer *buf);
    const HttpRequest& getRequest() const;

private:
    bool processRequestLine(const char *begin, const char *end);

private:
    HttpRequestParseState state_;
    HttpRequest request_;
};





