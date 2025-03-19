// HttpServer.h
#pragma once

#include "cc_muduo/TcpServer.h"
#include "cc_muduo/EventLoop.h"
#include "cc_muduo/InetAddress.h"
#include "cc_muduo/TcpConnection.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "Router.h"

#include <functional>
#include <string>
#include <memory>

// 在文件顶部添加包含
#include "PerformanceMonitor.h"
#include <random>

class HttpServer
{
public:
    using RequestHandler = std::function<void(const HttpRequest &, HttpResponse *)>;

    // 构造函数，指定监听地址和端口
    HttpServer(EventLoop *loop, const InetAddress &listenAddr,
               const std::string &name = "HttpServer");

    // 设置线程数
    void setThreadNum(int numThreads)
    {
        server_.setThreadNum(numThreads);
    }

    // 设置请求处理函数
    void setRequestHandler(RequestHandler handler)
    {
        requestHandler_ = std::move(handler);
    }

    // 添加路由规则
    void addRoute(const std::string &method, const std::string &path, Router::HandlerCallback handler)
    {
        router_.addRoute(method, path, std::move(handler));
    }

    // GET方法的路由添加
    void get(const std::string &path, Router::HandlerCallback handler)
    {
        router_.get(path, std::move(handler));
    }

    // POST方法的路由添加
    void post(const std::string &path, Router::HandlerCallback handler)
    {
        router_.post(path, std::move(handler));
    }

    // PUT方法的路由添加
    void put(const std::string &path, Router::HandlerCallback handler)
    {
        router_.put(path, std::move(handler));
    }

    // DELETE方法的路由添加
    void del(const std::string &path, Router::HandlerCallback handler)
    {
        router_.del(path, std::move(handler));
    }

    // 启动服务器
    void start()
    {
        server_.start();
    }
    
    std::function<void(const HttpRequest &, HttpResponse *)> getRequestHandler()
    {
        return requestHandler_;
    }

    // 添加性能监控相关方法
    void enablePerformanceMonitoring(bool enable = true);
    void writePerformanceReport(const std::string& filename);
    std::string getPerformanceReport();
    void resetPerformanceStatistics();

private:
    TcpServer server_;
    
    Router router_;
    std::function<void(const HttpRequest &, HttpResponse *)> requestHandler_;
    // 处理新连接
    void onConnection(const std::shared_ptr<TcpConnection> &conn);
    void onMessage(const std::shared_ptr<TcpConnection> &conn, Buffer *buf, Timestamp receiveTime);
    
    // 添加性能监控标志
    bool performanceMonitoringEnabled_ = false;
    
    // 生成唯一的请求ID
    std::string generateRequestId() {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_int_distribution<> dis(0, 15);
        static const char* digits = "0123456789abcdef";
        
        std::string uuid;
        uuid.reserve(36);

        for (int i = 0; i < 36; i++) {
            if (i == 8 || i == 13 || i == 18 || i == 23) {
                uuid += '-';
            } else {
                uuid += digits[dis(gen)];
            }
        }
        
        return uuid;
    }
};
