#include "UChatService.h"
#include "HttpRequest.h"
#include "HttpResponse.h"

#include <fcntl.h>
#include <functional>
#include <string>
#include <unistd.h>


UChatService& UChatService::instance() {
    static UChatService service;
    return service;
}

const UChatService::HttpHandler& UChatService::getHttpHandler(const std::string &key) const {
    auto it = httpHandlerMap_.find(key);
    if(it == httpHandlerMap_.end())
        return httpHandlerMap_.at("/html");
    else
        return it->second;
}

const std::string& UChatService::getMimetype(const std::string &key) const {
    auto it = mimetypeMap_.find(key);
    if(it == mimetypeMap_.end())
        return mimetypeMap_.at("default");
    else
        return it->second;
}

void UChatService::handleRegister(const TcpConnectionPtr &conn, const HttpRequest &req, HttpResponse &resp) {

}

void UChatService::handleLogin(const TcpConnectionPtr &conn, const HttpRequest &req, HttpResponse &resp) {

}

void UChatService::handlePoll(const TcpConnectionPtr &conn, const HttpRequest &req, HttpResponse &resp) {

}

void UChatService::handleSend(const TcpConnectionPtr &conn, const HttpRequest &req, HttpResponse &resp) {

}

void UChatService::handleHtml(const TcpConnectionPtr &conn, const HttpRequest &req, HttpResponse &resp) {
    if(req.getMethod() != "GET") {
        handleError(conn, req, resp);
        return ;
    }

    std::string path = "web";
    if(req.getPath()  == "/")
        path += "/uchatclient.html";
    else
        path += req.getPath();
    int fd = open(path.c_str(), O_RDONLY, 0);
    if(fd == -1) {
        resp.setStatusCode("404");
        resp.setStatusMessage("Not Found");
        handleError(conn, req, resp);
        return ;
    }

    resp.setStatusCode("200");
    resp.setStatusMessage("OK");

    size_t pos = path.find('.');
    std::string dot = path.substr(pos);
    resp.addHeader("Content-Type", getMimetype(dot));
    resp.addHeader("Connection", "keep-alive");
    resp.setKeepAlive(true);

    resp.getBody().resize(1024 * 1024 * 4);
    int rnd = read(fd, &*(resp.getBody().begin()), 1024 * 1024 * 4);
    resp.getBody().resize(rnd);
    close(fd);
}


void UChatService::handleError(const TcpConnectionPtr &conn, const HttpRequest &req, HttpResponse &resp) {
    resp.addHeader("Content-Type", "text/html; charset=utf-8");
    resp.addHeader("Connection", "close");
    resp.setKeepAlive(false);

    std::string body = "<html><title>Error</title><body bgcolor=\"ffffff\">";
    body += resp.getStatusCode() + " " + resp.getStatusMessage();
    body += "<hr><em>shiroOmO's Server</em></body></html>";
    resp.setBody(body);
}

UChatService::UChatService() {
    httpHandlerMap_.insert({"/register", std::bind(&UChatService::handleRegister, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)});
    httpHandlerMap_.insert({"/login", std::bind(&UChatService::handleLogin, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)});
    httpHandlerMap_.insert({"/poll", std::bind(&UChatService::handlePoll, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)});
    httpHandlerMap_.insert({"/send", std::bind(&UChatService::handleSend, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)});
    httpHandlerMap_.insert({"/html", std::bind(&UChatService::handleHtml, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)});
    httpHandlerMap_.insert({"/error", std::bind(&UChatService::handleError, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)});

    mimetypeMap_.insert({".html", "text/html; charset=utf-8"});
    mimetypeMap_.insert({".avi", "video/x-msvideo"});
    mimetypeMap_.insert({".bmp", "image/bmp"});
    mimetypeMap_.insert({".c", "text/plain"});
    mimetypeMap_.insert({".doc", "application/msword"});
    mimetypeMap_.insert({".gif", "image/gif"});
    mimetypeMap_.insert({".gz", "application/x-gzip"});
    mimetypeMap_.insert({".htm", "text/html"});
    mimetypeMap_.insert({".ico", "application/x-ico"});
    mimetypeMap_.insert({".jpg", "image/jpeg"});
    mimetypeMap_.insert({".png", "image/png; charset=utf-8"});
    mimetypeMap_.insert({".txt", "text/plain"});
    mimetypeMap_.insert({".mp3", "audio/mp3"});
    mimetypeMap_.insert({"default", "text/html"}); 
}

