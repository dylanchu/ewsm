#include "timer_manager.hpp"

namespace utils
{

    TimerManager::TimerManager() :
        m_nextTimerId(1)
    {
    }

    TimerManager::~TimerManager()
    {
        StopAllTimers();
    }

    int TimerManager::AddTimer(int ms, std::function<void()> callback)
    {
        // std::lock_guard<std::mutex> lock(m_mutex);
        int timerId = m_nextTimerId++;
        ManagedTimer* timer = new ManagedTimer(this, timerId, callback);
        m_timers[timerId] = timer;
        timer->Start(ms, true);
        return timerId;
    }

    int TimerManager::AddRepeatTimer(int ms, std::function<void()> callback, bool run_once_right_now)
    {
        // std::lock_guard<std::mutex> lock(m_mutex);
        int timerId = m_nextTimerId++;
        ManagedTimer* timer = new ManagedTimer(this, timerId, callback);
        m_timers[timerId] = timer;
        timer->Start(ms, false);
        if (run_once_right_now) {
            callback();
        }
        return timerId;
    }

    void TimerManager::RemoveTimer(int timerId)
    {
        // std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_timers.find(timerId);
        if (it != m_timers.end()) {
            it->second->Stop();
            // delete it->second;
            m_timers.erase(it);
        }
    }

    void TimerManager::StopAllTimers()
    {
        // std::lock_guard<std::mutex> lock(m_mutex);
        for (auto& [timerId, timer] : m_timers) {
            timer->Stop();
            // delete timer;
        }
        m_timers.clear();
    }

} // namespace utils
