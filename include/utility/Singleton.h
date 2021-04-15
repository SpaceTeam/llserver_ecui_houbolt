//
// Created by Markus on 05.04.21.
//

#ifndef LLSERVER_ECUI_HOUBOLT_SINGLETON_H
#define LLSERVER_ECUI_HOUBOLT_SINGLETON_H

template <class T>
class Singleton
{
protected:
    static T *instance;    // elements

    Singleton(const Singleton& copy) {};
    Singleton() {};
public:

    virtual ~Singleton()
    {
        if (instance != nullptr)
        {
            delete instance;
            instance = nullptr;
        }
    }

    static T* Instance() {
        if (instance == nullptr)
        {
            instance = new T();
        }
        return instance;
    }

};

template<class T>
T *Singleton<T>::instance = nullptr;

#endif //LLSERVER_ECUI_HOUBOLT_SINGLETON_H
