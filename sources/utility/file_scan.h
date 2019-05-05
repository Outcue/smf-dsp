#pragma once
#include "portfts.h"
#include <list>
#include <vector>
#include <string>
#include <memory>
#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

class File_Scan {
public:
    explicit File_Scan(
        std::string path,
        std::function<bool (const std::string &)> file_filter);
    ~File_Scan();

    size_t file_count() const;
    bool files_empty() const { return file_count() == 0; }
    std::string file_name(size_t index) const;

    bool is_complete() const;
    void wait_for_completion();
    void wait_for_file_count(size_t count);

private:
    void scan_in_thread();

private:
    struct Entry {
        Entry(size_t parent, std::string name)
            : parent(parent), name(std::move(name)) {}
        size_t parent = ~size_t(0);
        std::string name;
    };

private:
    const std::string path_;
    std::function<bool (const std::string &)> file_filter_;
    std::vector<Entry> files_;
    std::vector<Entry> dirs_;
    size_t file_count_ = 0;

    std::thread thread_;
    mutable std::mutex mutex_;
    std::atomic_bool cancel_{false};

    std::condition_variable cond_;
    bool complete_ = false;
};
