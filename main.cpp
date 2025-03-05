// main.cpp
#include "HttpServer.h"
#include "cc_muduo/EventLoop.h"
#include <iostream>
#include <signal.h>

int main(int argc, char *argv[])
{
    uint16_t port = 8080;
    if (argc > 1)
    {
        port = static_cast<uint16_t>(std::stoi(argv[1]));
    }

    // 创建事件循环
    EventLoop loop;

    // 创建监听地址
     InetAddress listenAddr(port);

    // 创建HTTP服务器
    HttpServer server(&loop, listenAddr);

    // 设置线程数
    server.setThreadNum(4);

    // 添加路由
    server.get("/", [](const HttpRequest &req, HttpResponse *resp)
               {
        resp->setStatusCode(HttpResponse::k200Ok);
        resp->setContentType("text/html");
        resp->setBody("<html><body><h1>Welcome to HTTP Server</h1></body></html>"); });

    server.get("/hello", [](const HttpRequest &req, HttpResponse *resp)
               {
        resp->setStatusCode(HttpResponse::k200Ok);
        resp->setContentType("text/plain");
        resp->setBody("Hello, World!"); });

    server.post("/echo", [](const HttpRequest &req, HttpResponse *resp)
                {
        resp->setStatusCode(HttpResponse::k200Ok);
        resp->setContentType("text/plain");
        resp->setBody("You sent: " + req.body()); });

    // 启动服务器
    std::cout << "HTTP server started on port " << port << std::endl;
    server.start();

    // 运行事件循环
    loop.loop();

    return 0;
}
