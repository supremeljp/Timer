#ifndef TIMER_HPP
#define TIMER_HPP

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <thread>

class Timer
{
public:
    Timer() :m_bExpired(true), m_bTryToExpired(false) {}
    Timer(const Timer& other)
    {
        m_bExpired = other.m_bExpired.load();
        m_bTryToExpired = other.m_bTryToExpired.load();
    }
    ~Timer()
    {
        Stop();
    }
    template<class F, class... Args>
    void Start(int interval, F&& f, Args... args)
    {
        if(!m_bExpired)
        {
            return;
        }
        m_bExpired = false;
        using RetType = decltype(f(args...));
        std::function<RetType()> task = (std::bind(std::forward<F>(f), std::forward<Args>(args)...));
        std::thread([this, interval, task](){
            while(!m_bTryToExpired)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(interval));
                task();
            }
            {
                std::lock_guard<std::mutex> lock(m_Mutex);
                m_bExpired = true;
                m_CV.notify_one();
            }
        }).detach();
    }

    void Stop()
    {
        if(m_bExpired)
        {
            return;
        }
        if(m_bTryToExpired)
        {
            return;
        }
        m_bTryToExpired = true;
        {
            std::unique_lock<std::mutex> lock(m_Mutex);
            m_CV.wait(lock, [this]() {return m_bExpired == true;});
            if(m_bExpired)
            {
                m_bTryToExpired = false;
            }
        }
    }

private:
    std::atomic_bool m_bExpired;        // 已经失效
    std::atomic_bool m_bTryToExpired;   // 尝试失效
    std::mutex m_Mutex;
    std::condition_variable m_CV;
};

#endif