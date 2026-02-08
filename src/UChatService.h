#pragma once

#include "Callbacks.h"

#include <functional>
#include <string>
#include <unordered_map>


class HttpRequest;
class HttpResponse;

class UChatService {
public:
    using HttpHandler = std::function<void(const TcpConnectionPtr &conn, const HttpRequest &req, HttpResponse &resp)>;

    static UChatService& instance();

    const HttpHandler& getHttpHandler(const std::string &key) const;
    const std::string& getMimetype(const std::string &key) const;

    void handleRegister(const TcpConnectionPtr &conn, const HttpRequest &req, HttpResponse &resp);
    void handleLogin(const TcpConnectionPtr &conn, const HttpRequest &req, HttpResponse &resp);

    void handlePoll(const TcpConnectionPtr &conn, const HttpRequest &req, HttpResponse &resp);
    void handleSend(const TcpConnectionPtr &conn, const HttpRequest &req, HttpResponse &resp);

    void handleHtml(const TcpConnectionPtr &conn, const HttpRequest &req, HttpResponse &resp);
    void handleError(const TcpConnectionPtr &conn, const HttpRequest &req, HttpResponse &resp);

private:
    UChatService();

private:
    std::unordered_map<std::string, HttpHandler> httpHandlerMap_;
    std::unordered_map<std::string, std::string> mimetypeMap_;
};



