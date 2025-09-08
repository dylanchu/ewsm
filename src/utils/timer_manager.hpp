#pragma once

#include <functional>
#include <unordered_map>
#include <wx/timer.h>
#include "wx/event.h"

namespace utils
{

    class ManagedTimer;
    class TimerManager;

    class TimerManager : public wxEvtHandler
    {
    public:
        static TimerManager& Get()
        {
            static TimerManager instance;
            return instance;
        }

        int AddTimer(int ms, std::function<void()> callback);
        int AddRepeatTimer(int ms, std::function<void()> callback, bool run_once_right_now = false);
        void RemoveTimer(int timerId);
        void StopAllTimers();

        TimerManager(const TimerManager&) = delete;
        TimerManager& operator=(const TimerManager&) = delete;

    private:
        TimerManager();
        ~TimerManager();

        int m_nextTimerId;
        std::unordered_map<int, ManagedTimer*> m_timers;
        // std::mutex m_mutex;
    };


    class ManagedTimer : public wxTimer
    {
    public:
        ManagedTimer(TimerManager* manager, int id, std::function<void()> callback) :
            wxTimer(manager, id), m_callback(callback) {}

        void Notify() override
        {
            if (m_callback) {
                m_callback();
            }
        }

    private:
        std::function<void()> m_callback;
    };

} // namespace utils
