// HttpRequest.h
#pragma once

#include <string>
#include <map>

class HttpRequest
{
public:
    enum Method
    {
        kInvalid,
        kGet,
        kPost,
        kPut,
        kDelete,
        kHead
    };

    HttpRequest() : method_(kInvalid), version_("HTTP/1.1") {}

    void setMethod(Method method) { method_ = method; }
    Method method() const { return method_; }

    void setPath(const std::string &path) { path_ = path; }
    const std::string &path() const { return path_; }

    void setVersion(const std::string &version) { version_ = version; }
    const std::string &version() const { return version_; }

    void setBody(const std::string &body) { body_ = body; }
    const std::string &body() const { return body_; }

    void addHeader(const std::string &key, const std::string &value)
    {
        headers_[key] = value;
    }

    std::string getHeader(const std::string &key) const
    {
        auto it = headers_.find(key);
        return it != headers_.end() ? it->second : "";
    }

    const std::map<std::string, std::string> &headers() const { return headers_; }

    // 将方法字符串转换为Method枚举
    static Method stringToMethod(const std::string &methodStr)
    {
        if (methodStr == "GET")
            return kGet;
        if (methodStr == "POST")
            return kPost;
        if (methodStr == "PUT")
            return kPut;
        if (methodStr == "DELETE")
            return kDelete;
        if (methodStr == "HEAD")
            return kHead;
        return kInvalid;
    }

    // 将Method枚举转换为字符串
    static std::string methodToString(Method method)
    {
        switch (method)
        {
        case kGet:
            return "GET";
        case kPost:
            return "POST";
        case kPut:
            return "PUT";
        case kDelete:
            return "DELETE";
        case kHead:
            return "HEAD";
        default:
            return "UNKNOWN";
        }
    }

    void reset()
    {
        method_ = kInvalid;
        path_ = "";
        version_ = "HTTP/1.1";
        body_ = "";
        headers_.clear();
    }

private:
    Method method_;
    std::string path_;
    std::string version_;
    std::string body_;
    std::map<std::string, std::string> headers_;
};

