#pragma once
#include "common.h"
#include "Singleton.hpp"
#include <boost/asio/io_context.hpp>
#include <cstddef>
#include <memory>


class IOcontextPool : public Singleton<IOcontextPool> {
    friend class Singleton<IOcontextPool>;
    using Work = asio::executor_work_guard<asio::io_context::executor_type>;
public:
    ~IOcontextPool();
    asio::io_context& getIOcontext();
    void stop();

private:
    IOcontextPool(size_t size = 5 /*std::thread::hardward_concurrency()*/);
    std::vector<asio::io_context> contexts_;
    std::vector<std::unique_ptr<Work>> works_;
    std::vector<std::thread> threads_;
    std::size_t next_iocontext;
};