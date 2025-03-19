#pragma once

#include <chrono>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>
#include <atomic>
#include <iostream>
#include <fstream>
#include <thread>
#include <functional>

class PerformanceMonitor {
public:
    static PerformanceMonitor& getInstance() {
        static PerformanceMonitor instance;
        return instance;
    }

    // 开始记录请求处理时间
    void startRequest(const std::string& requestId) {
        std::lock_guard<std::mutex> lock(mutex_);
        requestStartTimes_[requestId] = std::chrono::high_resolution_clock::now();
    }

    // 结束记录请求处理时间
    void endRequest(const std::string& requestId, bool success = true) {
        auto endTime = std::chrono::high_resolution_clock::now();
        
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = requestStartTimes_.find(requestId);
        if (it != requestStartTimes_.end()) {
            auto startTime = it->second;
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
            
            totalRequests_++;
            if (success) {
                successfulRequests_++;
            }
            
            totalProcessingTime_ += duration;
            
            // 更新最大和最小处理时间
            if (duration > maxProcessingTime_) {
                maxProcessingTime_ = duration;
            }
            if (minProcessingTime_ == 0 || duration < minProcessingTime_) {
                minProcessingTime_ = duration;
            }
            
            // 记录处理时间以计算平均值和中位数
            processingTimes_.push_back(duration);
            
            requestStartTimes_.erase(it);
        }
    }

    // 增加当前连接数
    void incrementConnections() {
        currentConnections_++;
        if (currentConnections_ > peakConnections_) {
            peakConnections_ = currentConnections_.load();
        }
    }

    // 减少当前连接数
    void decrementConnections() {
        currentConnections_--;
    }

    // 记录请求的路径统计
    void recordPath(const std::string& path) {
        std::lock_guard<std::mutex> lock(mutex_);
        pathCounts_[path]++;
    }

    // 获取性能统计报告
    std::string getStatisticsReport() {
        std::lock_guard<std::mutex> lock(mutex_);
        
        std::string report = "===== 服务器性能统计 =====\n";
        
        // 基本统计信息
        report += "总请求数: " + std::to_string(totalRequests_) + "\n";
        report += "成功请求数: " + std::to_string(successfulRequests_) + "\n";
        
        double successRate = totalRequests_ > 0 ? 
            (double)successfulRequests_ / totalRequests_ * 100.0 : 0.0;
        report += "请求成功率: " + std::to_string(successRate) + "%\n";
        
        // 连接统计
        report += "当前连接数: " + std::to_string(currentConnections_) + "\n";
        report += "峰值连接数: " + std::to_string(peakConnections_) + "\n";
        
        // 处理时间统计
        double avgProcessingTime = totalRequests_ > 0 ? 
            (double)totalProcessingTime_ / totalRequests_ : 0.0;
        report += "平均处理时间: " + std::to_string(avgProcessingTime) + " ms\n";
        report += "最大处理时间: " + std::to_string(maxProcessingTime_) + " ms\n";
        report += "最小处理时间: " + std::to_string(minProcessingTime_) + " ms\n";
        
        // 路径统计（前5个最常访问的路径）
        report += "\n最常访问的路径:\n";
        std::vector<std::pair<std::string, int>> pathStats(pathCounts_.begin(), pathCounts_.end());
        std::sort(pathStats.begin(), pathStats.end(), 
                 [](const auto& a, const auto& b) { return a.second > b.second; });
        
        int count = 0;
        for (const auto& path : pathStats) {
            report += path.first + ": " + std::to_string(path.second) + " 次\n";
            if (++count >= 5) break;
        }
        
        return report;
    }

    // 将统计信息写入文件
    void writeStatisticsToFile(const std::string& filename) {
        std::ofstream file(filename);
        if (file.is_open()) {
            file << getStatisticsReport();
            file.close();
        }
    }

    // 启动定期统计报告（每隔指定秒数输出一次报告）
    void startPeriodicReporting(int intervalSeconds) {
        if (reportingThread_.joinable()) {
            stopReporting_ = true;
            reportingThread_.join();
        }
        
        stopReporting_ = false;
        reportingThread_ = std::thread([this, intervalSeconds]() {
            while (!stopReporting_) {
                std::this_thread::sleep_for(std::chrono::seconds(intervalSeconds));
                if (!stopReporting_) {
                    std::cout << getStatisticsReport() << std::endl;
                }
            }
        });
    }

    // 停止定期报告
    void stopPeriodicReporting() {
        if (reportingThread_.joinable()) {
            stopReporting_ = true;
            reportingThread_.join();
        }
    }

    // 重置所有统计数据
    void resetStatistics() {
        std::lock_guard<std::mutex> lock(mutex_);
        totalRequests_ = 0;
        successfulRequests_ = 0;
        totalProcessingTime_ = 0;
        maxProcessingTime_ = 0;
        minProcessingTime_ = 0;
        processingTimes_.clear();
        pathCounts_.clear();
        // 不重置当前连接数，只重置峰值
        peakConnections_ = currentConnections_.load();
    }

    ~PerformanceMonitor() {
        stopPeriodicReporting();
    }

private:
    PerformanceMonitor() : 
        totalRequests_(0), 
        successfulRequests_(0),
        totalProcessingTime_(0),
        maxProcessingTime_(0),
        minProcessingTime_(0),
        currentConnections_(0),
        peakConnections_(0),
        stopReporting_(false) {}
    
    // 禁止复制和赋值
    PerformanceMonitor(const PerformanceMonitor&) = delete;
    PerformanceMonitor& operator=(const PerformanceMonitor&) = delete;

    std::mutex mutex_;
    std::unordered_map<std::string, std::chrono::time_point<std::chrono::high_resolution_clock>> requestStartTimes_;
    std::unordered_map<std::string, int> pathCounts_;
    std::vector<long long> processingTimes_;
    
    long long totalRequests_;
    long long successfulRequests_;
    long long totalProcessingTime_;
    long long maxProcessingTime_;
    long long minProcessingTime_;
    
    std::atomic<int> currentConnections_;
    std::atomic<int> peakConnections_;
    
    std::thread reportingThread_;
    std::atomic<bool> stopReporting_;
};