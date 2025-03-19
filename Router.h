// Router.h
#pragma once

#include "HttpRequest.h"
#include "HttpResponse.h"
#include <functional>
#include <map>
#include <string>

class Router
{
public:
    using HandlerCallback = std::function<void(const HttpRequest &, HttpResponse *)>;

    Router()
    {
        // 设置默认处理函数
        defaultHandler_ = [](const HttpRequest &, HttpResponse *resp)
        {
            resp->setStatusCode(HttpResponse::k404NotFound);
            resp->setContentType("text/plain");
            resp->setBody("404 Not Found");
        };
    }

    void addRoute(const std::string &method, const std::string &path, HandlerCallback handler)
    {
        routes_[method + path] = std::move(handler);
    }

    void get(const std::string &path, HandlerCallback handler)
    {
        addRoute("GET", path, std::move(handler));
    }

    void post(const std::string &path, HandlerCallback handler)
    {
        addRoute("POST", path, std::move(handler));
    }

    void put(const std::string &path, HandlerCallback handler)
    {
        addRoute("PUT", path, std::move(handler));
    }

    void del(const std::string &path, HandlerCallback handler)
    {
        addRoute("DELETE", path, std::move(handler));
    }

    void setDefaultHandler(HandlerCallback handler)
    {
        defaultHandler_ = std::move(handler);
    }

    void route(const HttpRequest &req, HttpResponse *resp)
    {
        std::string method = HttpRequest::methodToString(req.method());
        std::string path = req.path();
        std::string key = method + path;

        auto it = routes_.find(key);
        if (it != routes_.end())
        {
            // 找到匹配的路由
            it->second(req, resp);
        }
        else
        {
            // 没有找到匹配的路由，使用默认处理函数
            defaultHandler_(req, resp);
        }
    }

private:
    std::map<std::string, HandlerCallback> routes_;
    HandlerCallback defaultHandler_;
};
