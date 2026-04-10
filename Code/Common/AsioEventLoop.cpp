/*
*   Copyright (C) {2015}  {VK, Charles TheHouse}
*
*   This program is free software: you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation, either version 3 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program.  If not, see [http://www.gnu.org/licenses/].
*
*   Contact at:
*/

#include "AsioEventLoop.h"
#include "Logger.h"

#include <iostream>
#include <mutex>

namespace W2PP {
namespace Network {

#ifdef _WIN32
std::atomic<AsioEventLoop*> AsioEventLoop::s_instance{nullptr};
#endif

// AsioTimer implementation

AsioTimer::AsioTimer(asio::io_context& ioContext)
    : m_ioContext(ioContext)
    , m_timer(ioContext)
    , m_interval(0)
    , m_running(false)
{
}

AsioTimer::~AsioTimer()
{
    Stop();
}

void AsioTimer::Start(int intervalMs, TimerCallback callback)
{
    m_interval = std::chrono::milliseconds(intervalMs);
    m_callback = std::move(callback);
    m_running = true;

    ScheduleNext();
}

void AsioTimer::ScheduleNext()
{
    m_timer.expires_after(m_interval);
    
    // Use shared_ptr to keep this alive during async operation
    auto self = shared_from_this();
    m_timer.async_wait([self](const asio::error_code& error)
    {
        self->OnTimer(error);
    });
}

void AsioTimer::Stop()
{
    m_running = false;
    asio::error_code ec;
    m_timer.cancel(ec);
}

void AsioTimer::OnTimer(const asio::error_code& error)
{
    if (error)
    {
        if (error != asio::error::operation_aborted)
        {
            LOG_ERROR("Timer error: {}", error.message());
        }
        return;
    }

    if (!m_running)
    {
        return;
    }

    // Execute callback (may modify m_running)
    if (m_callback)
    {
        m_callback();
    }

    // Re-check m_running AFTER callback - the callback may have called Stop()
    if (!m_running)
    {
        return;
    }

    // Safe to reschedule
    ScheduleNext();
}

// AsioEventLoop implementation

AsioEventLoop::AsioEventLoop()
#ifndef _WIN32
    : m_signals(m_ioContext)
#endif
{
#ifdef _WIN32
    s_instance.store(this, std::memory_order_release);
#endif
}

AsioEventLoop::~AsioEventLoop()
{
    Stop();
#ifdef _WIN32
    s_instance.store(nullptr, std::memory_order_release);
#endif
}

AsioEventLoop& AsioEventLoop::GetInstance()
{
    static AsioEventLoop instance;
    return instance;
}

bool AsioEventLoop::Initialize()
{
    LOG_INFO("Initializing ASIO event loop...");

    // Create work guard to keep io_context running (modern ASIO API)
    m_workGuard = std::make_unique<WorkGuard>(m_ioContext.get_executor());

    // Setup signal handling
#ifdef _WIN32
    // On Windows, use SetConsoleCtrlHandler for Ctrl+C
    if (!SetConsoleCtrlHandler(ConsoleHandler, TRUE))
    {
        LOG_ERROR("Failed to set console control handler");
        return false;
    }
    LOG_INFO("Console control handler registered (Windows)");
#else
    // On Linux/Unix, use ASIO signal_set
    m_signals.add(SIGINT);
    m_signals.add(SIGTERM);
    m_signals.async_wait([this](const asio::error_code& error, int signal)
    {
        OnSignal(error, signal);
    });
    LOG_INFO("Signal handlers registered (POSIX)");
#endif

    LOG_INFO("ASIO event loop initialized");
    return true;
}

void AsioEventLoop::Run()
{
    if (m_running.exchange(true))
    {
        LOG_WARN("Event loop is already running");
        return;
    }

    m_stopRequested = false;

    LOG_INFO("Starting ASIO event loop...");

    // Scope guard to ensure m_running is reset on exit
    auto cleanup = [this]() {
        m_running = false;
        LOG_INFO("ASIO event loop stopped");
    };

    while (!m_stopRequested)
    {
        try
        {
            // Run the io_context until stop is requested
            m_ioContext.run();
            break;  // Normal exit (no work remaining)
        }
        catch (const std::runtime_error& e)
        {
            // Recoverable runtime errors - log and continue
            LOG_ERROR("Runtime error in event loop: {}", e.what());
        }
        catch (const std::exception& e)
        {
            // Other exceptions - log as fatal and continue with caution
            LOG_ERROR("Fatal exception in event loop: {}", e.what());
            // Continue running as the event loop should be resilient
        }

        // If stop wasn't requested but io_context stopped (e.g., no work),
        // restart it
        if (!m_stopRequested)
        {
            m_ioContext.restart();
            
            // Small sleep to prevent CPU spinning
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

    cleanup();
}

void AsioEventLoop::Stop()
{
    if (!m_running)
    {
        return;
    }

    LOG_INFO("Stopping ASIO event loop...");

    m_stopRequested = true;
    m_workGuard.reset();  // Allow io_context to run out of work

    // Stop all timers
    StopAllTimers();

    // Stop the io_context
    m_ioContext.stop();
}

void AsioEventLoop::StartTimer(TimerType type, int intervalMs, TimerCallback callback)
{
    std::scoped_lock lock(m_timerMutex);

    // Stop existing timer if present
    auto it = m_timers.find(type);
    if (it != m_timers.end())
    {
        it->second->Stop();
        m_timers.erase(it);
    }

    // Create and start new timer using shared_ptr for lifetime safety
    auto timer = std::make_shared<AsioTimer>(m_ioContext);
    timer->Start(intervalMs, std::move(callback));
    m_timers[type] = std::move(timer);

    LOG_INFO("Started timer type {} with interval {}ms", static_cast<int>(type), intervalMs);
}

void AsioEventLoop::StopTimer(TimerType type)
{
    std::scoped_lock lock(m_timerMutex);

    auto it = m_timers.find(type);
    if (it != m_timers.end())
    {
        it->second->Stop();
        m_timers.erase(it);
        LOG_INFO("Stopped timer type {}", static_cast<int>(type));
    }
}

void AsioEventLoop::StopAllTimers()
{
    std::scoped_lock lock(m_timerMutex);

    for (auto& [type, timer] : m_timers)
    {
        timer->Stop();
    }
    m_timers.clear();

    LOG_INFO("All timers stopped");
}

void AsioEventLoop::OnSignal(const asio::error_code& error, int signalNumber)
{
    if (error)
    {
        LOG_ERROR("Signal handler error: {}", error.message());
        return;
    }

    LOG_INFO("Received signal {}, stopping event loop...", signalNumber);
    Stop();
}

#ifdef _WIN32
BOOL WINAPI AsioEventLoop::ConsoleHandler(DWORD signal)
{
    if (signal == CTRL_C_EVENT || signal == CTRL_BREAK_EVENT)
    {
        LOG_INFO("Ctrl+C or Ctrl+Break received, stopping event loop...");
        // Use atomic load with acquire semantics for thread safety
        if (AsioEventLoop* instance = s_instance.load(std::memory_order_acquire))
        {
            instance->Stop();
        }
        return TRUE;
    }
    return FALSE;
}
#endif

} // namespace Network
} // namespace W2PP

// C-style compatibility functions

bool EventLoop_Init()
{
    return W2PP::Network::AsioEventLoop::GetInstance().Initialize();
}

void EventLoop_Run()
{
    W2PP::Network::AsioEventLoop::GetInstance().Run();
}

void EventLoop_Stop()
{
    W2PP::Network::AsioEventLoop::GetInstance().Stop();
}

void EventLoop_StartTimer(int timerId, int intervalMs, void (*callback)())
{
    // Safety check: callback must not be null
    if (callback == nullptr)
    {
        return;
    }
    
    auto timerType = static_cast<W2PP::Network::TimerType>(timerId);
    W2PP::Network::AsioEventLoop::GetInstance().StartTimer(
        timerType, 
        intervalMs, 
        [callback]() { callback(); }
    );
}

void EventLoop_StopTimer(int timerId)
{
    auto timerType = static_cast<W2PP::Network::TimerType>(timerId);
    W2PP::Network::AsioEventLoop::GetInstance().StopTimer(timerType);
}
