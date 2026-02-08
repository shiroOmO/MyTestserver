#include "HttpRequest.h"
#include "Buffer.h"

#include <algorithm>
#include <cstddef>
#include <string>
#include <unordered_map>


void HttpRequest::setMethod(const char *start, const char *end) {
    method_.assign(start, end);
}

void HttpRequest::setVersion(const char *start, const char *end) {
    version_.assign(start, end);
}

void HttpRequest::setPath(const char *start, const char *end) {
    path_.assign(start, end);
}

void HttpRequest::setQuery(const char *start, const char *end) {
    query_.assign(start, end);
}

void HttpRequest::setBody(const char *start, const char *end) {
    body_.assign(start, end);
}

const std::string& HttpRequest::getMethod() const {
    return method_;
}

const std::string& HttpRequest::getVersion() const {
    return version_;
}

const std::string& HttpRequest::getPath() const {
    return path_;
}

const std::string& HttpRequest::getQuery() const {
    return query_;
}

const std::string& HttpRequest::getBody() const {
    return body_;
}

void HttpRequest::addHeader(const char *start, const char *colon, const char *end) {
    std::string key(start, colon);
    if(key.empty())
        return ;

    ++colon;
    while(colon < end && *colon == ' ')
        ++colon;
    while(colon < end && *(end - 1) == ' ')
        --end;

    std::string value(colon, end);
    if(value.empty())
        return ;

    headers_[key] = value;
}

const std::unordered_map<std::string, std::string>& HttpRequest::getHeaders() const {
    return headers_;
}

std::string HttpRequest::getHeader(const std::string &key) const {
    std::string result;
    std::unordered_map<std::string, std::string>::const_iterator it = headers_.find(key);
    if(it != headers_.end())
        result = it->second;

    return result;
}

HttpParser::HttpParser(): state_(kExpectRequestLine) {}

bool HttpParser::parseRequest(Buffer *buf) {
    const char *crlf, *colon;

    while(true) {
        switch(state_) {
            case kExpectRequestLine:
                crlf = buf->findCRLF();
                if(!crlf)
                    return false;

                if(!processRequestLine(buf->peek(), crlf))
                    return false;

                buf->retrieve(crlf + 2 - buf->peek());
                state_ = kExpectHeaders;
                break;

            case kExpectHeaders:
                crlf = buf->findCRLF();
                if(!crlf)
                    return false;

                colon = std::find(buf->peek(), crlf, ':');
                if(colon != crlf)
                    request_.addHeader(buf->peek(), colon, crlf);
                else {
                    if(request_.getMethod() == "POST")
                        state_ = kExpectBody;
                    else
                        state_ = kGetAll;
                }
                buf->retrieve(crlf + 2 - buf->peek());
                break;

            case kExpectBody: {
                std::string contentLengthStr = request_.getHeader("Content-Length");
                if(contentLengthStr.empty())
                    state_ = kGetAll;
                else {
                    size_t contentLength = static_cast<size_t>(std::stoi(contentLengthStr));
                    if(buf->readableBytes() >= contentLength) {
                        request_.setBody(buf->peek(), buf->peek() + contentLength);
                        buf->retrieve(contentLength);
                        state_ = kGetAll;
                    }
                    else
                        return false;
                }
                break;
            }

            case kGetAll:
                return true;
            default:
                return false;
        }
    }
}

const HttpRequest& HttpParser::getRequest() const {
    return request_;
}

bool HttpParser::processRequestLine(const char *begin, const char *end) {
    const char *now = begin;

    const char *space = std::find(now, end, ' ');
    if(space == end || space == now)
        return false;
    request_.setMethod(now, space);
    now = space + 1;
    if(request_.getMethod() == "GET");
    else if(request_.getMethod() == "POST");
    else return false;

    space = std::find(now, end, ' ');
    if(space == end || space == now)
        return false;
    const char *question = std::find(now, space, '?');
    if(now == question)
        return false;
    request_.setPath(now, question);
    request_.setQuery(question, space);
    now = space + 1;

    space = std::find(now, end, ' ');
    if(space != end)
        return false;
    request_.setVersion(now, space);

    return true;
}



