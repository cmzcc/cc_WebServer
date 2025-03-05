// HttpServer.cpp
#include "HttpServer.h"
#include "HttpRequest.h"
#include "HttpRequestParser.h"
#include <iostream>

HttpServer::HttpServer(EventLoop *loop,
                       const InetAddress &listenAddr,
                       const std::string &name)
    : server_(loop, listenAddr, name)
{

    // 设置默认的请求处理函数，使用路由器处理请求
    requestHandler_ = [this](const HttpRequest &req, HttpResponse *resp)
    {
        router_.route(req, resp);
    };

    // 设置连接回调
    server_.setConnectionCallback(
        std::bind(&HttpServer::onConnection, this, std::placeholders::_1));

    // 设置消息回调
    server_.setMessageCallback(
        std::bind(&HttpServer::onMessage, this, std::placeholders::_1,
                  std::placeholders::_2, std::placeholders::_3));
}
void HttpServer::onConnection(const TcpConnectionPtr &conn)
{
    if (conn->connected())
    {
        std::cout << "New connection: " << conn->peerAddress().toIpPort() << std::endl;
        conn->setContext(HttpRequestParser()); // 设置上下文
    }
    else
    {
        std::cout << "Connection closed: " << conn->peerAddress().toIpPort() << std::endl;
    }
}

void HttpServer::onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp receiveTime)
{
    // 获取连接的解析器
    HttpRequestParser &parser = conn->getContext<HttpRequestParser>();

    // 解析请求
    const char *data = buf->peek();
    size_t len = buf->readableBytes();

    HttpRequestParser::HttpRequestParseResult result =
        parser.parse(data, data + len);

    if (result == HttpRequestParser::kOk)
    {
        // 解析成功，处理请求
        const HttpRequest &request = parser.request();
        HttpResponse response;

        if (getRequestHandler())
        {
            HttpRequest req;
            HttpResponse resp;
            getRequestHandler()(req, &resp);
        }

        else
        {
            // 没有设置处理函数，返回404
            response.setStatusCode(HttpResponse::k404NotFound);
            response.setContentType("text/plain");
            response.setBody("404 Not Found");
        }

        // 发送响应
        std::string responseStr = response.toString();
        conn->send(responseStr);

        // 清空已处理的数据
        buf->retrieve(len);

        // 重置解析器状态，准备处理下一个请求
        parser.reset();
    }
    else if (result == HttpRequestParser::kBadRequest)
    {
        // 请求格式错误，返回400
        HttpResponse response;
        response.setStatusCode(HttpResponse::k400BadRequest);
        response.setContentType("text/plain");
        response.setBody("400 Bad Request");

        std::string responseStr = response.toString();
        conn->send(responseStr);

        // 清空缓冲区
        buf->retrieveAll();

        // 重置解析器状态
        parser.reset();
    }
    else
    {
        // 请求不完整，等待更多数据
        return;
    }
}
