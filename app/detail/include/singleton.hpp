#pragma once
#include <memory>
#include <mutex>

template <typename T>
class Singleton {
public:
    static std::shared_ptr<T> getInstance();
    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;
    ~Singleton() = default;
    

protected:
    Singleton() = default;
    inline static std::shared_ptr<T> instance_ = nullptr;
    
};

template <typename T>
std::shared_ptr<T> Singleton<T>::getInstance()
{
    static std::once_flag flag;
    std::call_once(flag, [&]() {
        instance_ = std::shared_ptr<T>(new T);
    });
    return instance_;
}


