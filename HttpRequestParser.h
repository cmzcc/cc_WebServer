// HttpRequestParser.h
#pragma once

#include "HttpRequest.h"
#include <string>

class HttpRequestParser
{
public:
    enum HttpRequestParseResult
    {
        kOk,         // 解析成功
        kBadRequest, // 请求格式错误
        kNotComplete // 请求不完整
    };

    HttpRequestParser() : state_(kRequestLine) {}

    HttpRequestParseResult parse(const char *begin, const char *end)
    {
        const char *start = begin;
        const char *lineEnd;

        while (start < end)
        {
            if (state_ == kRequestLine)
            {
                lineEnd = std::find(start, end, '\n');
                if (lineEnd == end)
                {
                    return kNotComplete;
                }

                if (!parseRequestLine(start, lineEnd))
                {
                    return kBadRequest;
                }

                start = lineEnd + 1;
                state_ = kHeaders;
            }
            else if (state_ == kHeaders)
            {
                lineEnd = std::find(start, end, '\n');
                if (lineEnd == end)
                {
                    return kNotComplete;
                }

                if (!parseHeader(start, lineEnd))
                {
                    // 空行，表示头部结束
                    if (start == lineEnd - 1 && *start == '\r')
                    {
                        start = lineEnd + 1;

                        // 检查Content-Length头部
                        std::string contentLengthStr = request_.getHeader("Content-Length");
                        if (!contentLengthStr.empty())
                        {
                            contentLength_ = std::stoi(contentLengthStr);
                            state_ = kBody;
                        }
                        else
                        {
                            // 没有请求体，解析完成
                            return kOk;
                        }
                    }
                    else
                    {
                        // 头部格式错误
                        return kBadRequest;
                    }
                }
                else
                {
                    // 继续解析下一个头部
                    start = lineEnd + 1;
                }
            }
            else if (state_ == kBody)
            {
                // 解析请求体
                if (end - start < contentLength_)
                {
                    // 请求体不完整
                    return kNotComplete;
                }

                request_.setBody(std::string(start, start + contentLength_));

                // 解析完成
                return kOk;
            }
        }

        // 请求不完整
        return kNotComplete;
    }

    const HttpRequest &request() const { return request_; }

    void reset()
    {
        request_.reset();
        state_ = kRequestLine;
        contentLength_ = 0;
    }

private:
    enum ParseState
    {
        kRequestLine,
        kHeaders,
        kBody
    };

    HttpRequest request_;
    ParseState state_;
    int contentLength_;

    bool parseRequestLine(const char *begin, const char *end)
    {
        const char *space = std::find(begin, end, ' ');
        if (space == end)
        {
            return false;
        }

        // 解析方法
        std::string methodStr(begin, space);
        HttpRequest::Method method = HttpRequest::stringToMethod(methodStr);
        if (method == HttpRequest::kInvalid)
        {
            return false;
        }
        request_.setMethod(method);

        // 解析路径
        begin = space + 1;
        space = std::find(begin, end, ' ');
        if (space == end)
        {
            return false;
        }

        request_.setPath(std::string(begin, space));

        // 解析版本
        begin = space + 1;
        std::string version(begin, end);
        if (version.length() >= 2 && version[version.length() - 2] == '\r')
        {
            version = version.substr(0, version.length() - 2);
        }
        else if (version.length() >= 1 && version[version.length() - 1] == '\n')
        {
            version = version.substr(0, version.length() - 1);
        }

        if (version.empty() || version.substr(0, 5) != "HTTP/")
        {
            return false;
        }

        request_.setVersion(version);

        return true;
    }

    bool parseHeader(const char *begin, const char *end)
    {
        // 检查是否为空行
        if (begin == end - 1 && *begin == '\r')
        {
            return false;
        }

        const char *colon = std::find(begin, end, ':');
        if (colon == end)
        {
            return false;
        }

        std::string key(begin, colon);
        ++colon;

        // 跳过空格
        while (colon < end && isspace(*colon))
        {
            ++colon;
        }

        std::string value(colon, end);
        if (value.length() >= 2 && value[value.length() - 2] == '\r')
        {
            value = value.substr(0, value.length() - 2);
        }
        else if (value.length() >= 1 && value[value.length() - 1] == '\n')
        {
            value = value.substr(0, value.length() - 1);
        }

        request_.addHeader(key, value);

        return true;
    }
};
