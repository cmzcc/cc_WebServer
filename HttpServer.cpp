#include "HttpServer.h"
#include "HttpRequest.h"
#include "HttpRequestParser.h"
#include "PerformanceMonitor.h"
#include <iostream>
#include <random>

HttpServer::HttpServer(EventLoop *loop,
                       const InetAddress &listenAddr,
                       const std::string &name)
    : server_(loop, listenAddr, name), performanceMonitoringEnabled_(false)
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
        
        // 如果启用了性能监控，记录连接增加
        if (performanceMonitoringEnabled_) {
            PerformanceMonitor::getInstance().incrementConnections();
        }
    }
    else
    {
        std::cout << "Connection closed: " << conn->peerAddress().toIpPort() << std::endl;
        
        // 如果启用了性能监控，记录连接减少
        if (performanceMonitoringEnabled_) {
            PerformanceMonitor::getInstance().decrementConnections();
        }
    }
}

void HttpServer::onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp receiveTime)
{
    // 生成请求ID用于性能监控
    std::string requestId = generateRequestId();
    
    // 如果启用了性能监控，记录请求开始
    if (performanceMonitoringEnabled_) {
        PerformanceMonitor::getInstance().startRequest(requestId);
    }
    
    // 获取连接的解析器
    HttpRequestParser &parser = conn->getContext<HttpRequestParser>();

    // 解析请求
    const char *data = buf->peek();
    size_t len = buf->readableBytes();

    HttpRequestParser::HttpRequestParseResult result =
        parser.parse(data, data + len);

    bool requestSuccess = false;  // 用于记录请求是否成功

    if (result == HttpRequestParser::kOk)
    {
        // 解析成功，处理请求
        const HttpRequest &request = parser.request();
        HttpResponse response;

        std::cout << "Parsed request: " << std::endl;
        std::cout << "Method: " << request.method() << std::endl;
        std::cout << "Path: " << request.path() << std::endl;

        // 如果启用了性能监控，记录请求路径
        if (performanceMonitoringEnabled_) {
            PerformanceMonitor::getInstance().recordPath(request.path());
        }

        // 调用请求处理函数
        if (getRequestHandler())
        {
            // 调用注册的请求处理器
            std::cout << "Calling request handler..." << std::endl;
            getRequestHandler()(request, &response);
            requestSuccess = true;  // 请求处理成功
        }
        else
        {
            // 没有设置处理函数，返回404
            std::cout << "No request handler set, returning 404" << std::endl;
            response.setStatusCode(HttpResponse::k404NotFound);
            response.setContentType("text/plain");
            response.setBody("404 Not Found");
        }

        // 发送响应
        std::string responseStr = response.toString();
        std::cout << "Sending response: " << responseStr << std::endl;
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
        std::cout << "Bad request, sending 400 response." << std::endl;
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
    
    // 如果启用了性能监控，记录请求结束
    if (performanceMonitoringEnabled_) {
        PerformanceMonitor::getInstance().endRequest(requestId, requestSuccess);
    }
}

// 添加性能监控相关方法的实现
void HttpServer::enablePerformanceMonitoring(bool enable) {
    performanceMonitoringEnabled_ = enable;
    if (enable) {
        PerformanceMonitor::getInstance().startPeriodicReporting(60); // 每60秒输出一次报告
    } else {
        PerformanceMonitor::getInstance().stopPeriodicReporting();
    }
}

void HttpServer::writePerformanceReport(const std::string& filename) {
    PerformanceMonitor::getInstance().writeStatisticsToFile(filename);
}

std::string HttpServer::getPerformanceReport() {
    return PerformanceMonitor::getInstance().getStatisticsReport();
}

void HttpServer::resetPerformanceStatistics() {
    PerformanceMonitor::getInstance().resetStatistics();
}

// 删除这个函数，因为它已经在 HttpServer.h 中定义了
// std::string HttpServer::generateRequestId() {
//     static std::random_device rd;
//     static std::mt19937 gen(rd());
//     static std::uniform_int_distribution<> dis(0, 15);
//     static const char* digits = "0123456789abcdef";
//     
//     std::string uuid;
//     uuid.reserve(36);
//
//     for (int i = 0; i < 36; i++) {
//         if (i == 8 || i == 13 || i == 18 || i == 23) {
//             uuid += '-';
//         } else {
//             uuid += digits[dis(gen)];
//         }
//     }
//     
//     return uuid;
// }