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

    // 改进的 getParam 方法实现
    std::string getParam(const std::string &paramName) const
    {
        // 首先检查查询参数
        auto queryIt = queryParams_.find(paramName);
        if (queryIt != queryParams_.end())
        {
            return queryIt->second;
        }

        // 然后检查表单参数
        auto formIt = formParams_.find(paramName);
        if (formIt != formParams_.end())
        {
            return formIt->second;
        }

        // 最后检查路径参数
        std::string path = getPath();
        size_t pos = path.find(paramName);
        if (pos != std::string::npos)
        {
            size_t start = path.find_last_of('/') + 1;
            if (start != std::string::npos)
            {
                return path.substr(start);
            }
        }
        return "";
    }

    // 在 reset 方法中也需要清理参数映射
    void reset()
    {
        method_ = kInvalid;
        path_ = "";
        version_ = "HTTP/1.1";
        body_ = "你好";
        headers_.clear();
        queryParams_.clear(); // 清理查询参数
        formParams_.clear();  // 清理表单参数
    }

    std::string getPath() const
    {
        return path_;
    }
    // 添加解析查询参数和表单参数的方法
    void parseQueryParam(const std::string& param) {
    size_t pos = param.find('=');
    if (pos != std::string::npos) {
        std::string key = param.substr(0, pos);
        std::string value = param.substr(pos + 1);
        queryParams_[key] = value;
    }
}

std::string getQueryParam(const std::string& name) const {
    auto it = queryParams_.find(name);
    return it != queryParams_.end() ? it->second : "";
}


void parseFormParam(const std::string& param) {
    size_t pos = param.find('=');
    if (pos != std::string::npos) {
        std::string key = param.substr(0, pos);
        std::string value = param.substr(pos + 1);
        formParams_[key] = value;
    }
}

std::string getFormParam(const std::string& name) const {
    auto it = formParams_.find(name);
    return it != formParams_.end() ? it->second : "";
}

private:
Method method_;
std::string path_;
std::string version_;
std::string body_;
std::map<std::string, std::string> headers_;
std::map<std::string, std::string> queryParams_; // 添加查询参数映射
std::map<std::string, std::string> formParams_;  // 添加表单参数映射
};

