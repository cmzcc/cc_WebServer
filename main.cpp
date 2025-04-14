// main.cpp
#include "HttpServer.h"
#include "cc_muduo/EventLoop.h"
#include "PerformanceMonitor.h"
#include <iostream>
#include <signal.h>

// 全局服务器指针，用于信号处理
HttpServer* g_server = nullptr;

void signalHandler(int signum) {
    std::cout << "收到信号 " << signum << "，正在关闭服务器..." << std::endl;
    
    if (g_server) {
        // 在关闭前输出性能报告
        std::cout << g_server->getPerformanceReport() << std::endl;
        g_server->writePerformanceReport("performance_report.txt");
    }
    
    exit(signum);
}

int main(int argc, char *argv[])
{
    uint16_t port = 2000;
    if (argc > 1)
    {
        port = static_cast<uint16_t>(std::stoi(argv[1]));
    }

    // 注册信号处理
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    // 创建事件循环
    EventLoop loop;

    // 创建监听地址
    InetAddress listenAddr(port);

    // 创建HTTP服务器
    HttpServer server(&loop, listenAddr);
    g_server = &server;

    // 设置线程数
    server.setThreadNum(4);

    // 启用性能监控
    server.enablePerformanceMonitoring(true);

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
            <div class='message'>
                欢迎使用 cc_WebServer
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

    // 性能监控路由
    server.get("/monitor", [](const HttpRequest &req, HttpResponse *resp)
               {
                   if (g_server) {
                       std::string report = g_server->getPerformanceReport();
                       resp->setStatusCode(HttpResponse::k200Ok);
                       resp->setContentType("text/plain");
                       resp->setBody(report);
                   } else {
                       resp->setStatusCode(HttpResponse::k500InternalServerError);
                       resp->setContentType("application/json");
                       resp->setBody("{\"status\":\"error\",\"message\":\"Server not initialized\"}");
                   }
               });

    // 启动服务器
    std::cout << "HTTP server started on port " << port << std::endl;
    std::cout << "Performance monitoring enabled. Visit /monitor to see statistics." << std::endl;
    server.start();

    // 运行事件循环
    loop.loop();

    return 0;
}