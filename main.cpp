// main.cpp
#include "HttpServer.h"
#include "cc_muduo/EventLoop.h"
#include <iostream>
#include <signal.h>

int main(int argc, char *argv[])
{
    uint16_t port = 2000;
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
                   std::cout << "Received request for /" << std::endl;

                   // 构造包含大文字和玫瑰花的 HTML 响应
                   std::string body = R"(
        <html>
        <head>
            <meta charset="UTF-8">
            <style>
                body {
                    text-align: center;
                    font-family: Arial, sans-serif;
                    background-color: #f8f8f8;
                    padding: 50px;
                }
                h1 {
                    font-size: 80px;
                    color: #ff6347;
                    text-shadow: 2px 2px 5px rgba(0, 0, 0, 0.3);
                }
                .rose {
                    font-size: 50px;
                    color: red;
                }
                .message {
                    font-size: 40px;
                    color: #ff69b4;
                    margin-top: 20px;
                }
                .rose-icon {
                    font-size: 100px;
                    color: red;
                }
            </style>
        </head>
        <body>
            <h1>你是人机</h1>
            <div class='rose'>
                <span class="rose-icon">&#127801;</span> <!-- 玫瑰花图标 -->
            </div>

        </body>
        </html>
    )";

                   resp->setStatusCode(HttpResponse::k200Ok);
                   resp->setContentType("text/html");
                   resp->setBody(body); // 将构建的 HTML 作为响应体
               });

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
    server.get("/favicon.ico", [](const HttpRequest &req, HttpResponse *resp)
               {
                   // 返回一个空的 favicon.ico 或者一个图片文件
                   resp->setStatusCode(HttpResponse::k200Ok);
                   resp->setContentType("image/x-icon");
                   resp->setBody(""); // 或者发送一个实际的 favicon.ico 文件内容
               });

    // 启动服务器
    std::cout << "HTTP server started on port " << port << std::endl;
    server.start();

    // 运行事件循环
    loop.loop();

    return 0;
}
