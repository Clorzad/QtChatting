#pragma once



template <typename T>
class Singleton {
public:
    static T& get_instance();
    virtual ~Singleton();

protected:
    Singleton();
};

template <typename T>
T& Singleton<T>::get_instance()
{
    static T instance_;
    return instance_;
}
