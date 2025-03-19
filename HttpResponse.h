// HttpResponse.h
#pragma once

#include <string>
#include <map>
#include <sstream>

class HttpResponse
{
public:
    enum HttpStatusCode
    {
        k200Ok = 200,
        k201Created=201,
        k301MovedPermanently = 301,
        k302Found = 302,
        k400BadRequest = 400,
        k401Unauthorized = 401,
        k403Forbidden = 403,
        k404NotFound = 404,
        k500InternalServerError = 500
    };

    HttpResponse() : statusCode_(k200Ok), version_("HTTP/1.1") {}

    void setStatusCode(HttpStatusCode code) { statusCode_ = code; }
    HttpStatusCode statusCode() const { return statusCode_; }

    void setVersion(const std::string &version) { version_ = version; }
    const std::string &version() const { return version_; }

    void setBody(const std::string &body)
    { 
        body_ = body;
        std::cout << "Response body: " << body << std::endl;
    }
    const std::string &body() const { return body_; }

    void addHeader(const std::string &key, const std::string &value)
    {
        headers_[key] = value;
    }

    void setContentType(const std::string &contentType)
    {
        addHeader("Content-Type", contentType);
    }

    std::string toString() const
    {
        std::ostringstream oss;

        // 状态行
        oss << version_ << " " << statusCode_ << " " << statusCodeToString(statusCode_) << "\r\n";

        // 响应头
        for (const auto &header : headers_)
        {
            oss << header.first << ": " << header.second << "\r\n";
        }

        // 如果没有Content-Length头，添加一个
        if (headers_.find("Content-Length") == headers_.end())
        {
            oss << "Content-Length: " << body_.size() << "\r\n";
        }

        // 空行
        oss << "\r\n";

        // 响应体
        oss << body_;

        return oss.str();
    }

    // 将状态码转换为状态短语
    static std::string statusCodeToString(HttpStatusCode code)
    {
        switch (code)
        {
        case k200Ok:
            return "OK";
        case k301MovedPermanently:
            return "Moved Permanently";
        case k302Found:
            return "Found";
        case k400BadRequest:
            return "Bad Request";
        case k401Unauthorized:
            return "Unauthorized";
        case k403Forbidden:
            return "Forbidden";
        case k404NotFound:
            return "Not Found";
        case k500InternalServerError:
            return "Internal Server Error";
        default:
            return "Unknown";
        }
    }

private:
    HttpStatusCode statusCode_;
    std::string version_;
    std::string body_;
    std::map<std::string, std::string> headers_;
};
