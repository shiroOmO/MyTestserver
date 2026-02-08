#include "HttpResponse.h"
#include "Buffer.h"

#include <cstdio>
#include <cstring>
#include <string>


HttpResponse::HttpResponse(): statusCode_("400"), statusMessage_("Bad Request"), keepAlive_(false) {}

void HttpResponse::setStatusCode(const std::string &code) {
    statusCode_ = code;
}

void HttpResponse::setStatusMessage(const std::string &message) {
    statusMessage_ = message;
}

void HttpResponse::addHeader(const std::string &key, const std::string &value) {
    headers_[key] = value;
}

void HttpResponse::setBody(const std::string &body) {
    body_ = body;
}

const std::string& HttpResponse::getStatusCode() const {
    return statusCode_;
}

const std::string& HttpResponse::getStatusMessage() const {
    return statusMessage_;
}

void HttpResponse::setKeepAlive(bool yon) {
    keepAlive_ = yon;
}

bool HttpResponse::getKeepAlive() const {
    return keepAlive_;
}

std::string& HttpResponse::getBody() {
    return body_;
}

void HttpResponse::appendToBuffer(Buffer *output) {
    output->append("HTTP/1.1 ", 9);
    output->append(statusCode_.c_str(), statusCode_.size());
    output->append(" ", 1);
    output->append(statusMessage_.c_str(), statusMessage_.size());
    output->append("\r\n", 2);

    if(!body_.empty())
        addHeader("Content-Length", std::to_string(body_.size()));

    for(const auto &header: headers_) {
        output->append(header.first.c_str(), header.first.size());
        output->append(": ", 2);
        output->append(header.second.c_str(), header.second.size());
        output->append("\r\n", 2);
    }

    output->append("\r\n", 2);
    output->append(body_.c_str(), body_.size());
}



