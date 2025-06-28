//
// Created by Markus on 05.04.21.
//

#ifndef LLSERVER_ECUI_HOUBOLT_SINGLETON_H
#define LLSERVER_ECUI_HOUBOLT_SINGLETON_H

template <class T>
class Singleton
{
protected:
    //TODO: MP make friend for all template classes here
    // template <class> friend class Singleton; // this does not compile when destructor is private of child class for some reason

    static T *instance;    // elements

    Singleton(const Singleton& copy) {};
    Singleton() {};

    virtual ~Singleton() {};
public:

    static T* Instance() {
        if (instance == nullptr)
        {
            instance = new T();
        }
        return instance;
    }

    static void Destroy() {
        if (instance != nullptr)
        {
            delete instance;
            instance = nullptr;
        }
    }
    static void SetInstance(T* new_instance) {
        instance = new_instance;
    }

};

template<class T>
T *Singleton<T>::instance = nullptr;

#endif //LLSERVER_ECUI_HOUBOLT_SINGLETON_H
