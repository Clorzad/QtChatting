#include "IOcontextPool.h"
#include <boost/asio/executor_work_guard.hpp>
#include <boost/asio/io_context.hpp>
#include <memory>

IOcontextPool::IOcontextPool(size_t size) : contexts_(size), works_(size), next_iocontext(0)
{
    for (int i = 0; i < size; ++i) {
        works_[i] = std::unique_ptr<Work>(new Work(contexts_[i].get_executor()));
    }
    for (int i = 0; i < size; ++i) {
        threads_.emplace_back([this, i]() {
            contexts_[i].run();
        });
    }
}

IOcontextPool::~IOcontextPool()
{
    stop();
}

asio::io_context& IOcontextPool::getIOcontext()
{
    auto current = next_iocontext;
    next_iocontext = next_iocontext < 4 ? (next_iocontext + 1) : 0;
    return contexts_[current];
}

void IOcontextPool::stop()
{
    for (auto& work : works_) {
        work->reset();
    }
    for (auto& iocontext : contexts_) {
        iocontext.stop();
    }
    for (auto& thread : threads_) {
        thread.join();
    }
}
