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

#pragma once

#include "ISocket.h"
#include "AsioSocket.h"

#include <asio.hpp>
#include <asio/steady_timer.hpp>
#include <functional>
#include <map>
#include <atomic>
#include <thread>
#include <chrono>
#include <mutex>

namespace W2PP {
namespace Network {

// Timer callback type
using TimerCallback = std::function<void()>;

// Timer types matching the original Windows timer IDs
enum class TimerType
{
    SecTimer,
    MinTimer
};

// ASIO-based timer wrapper
class AsioTimer : public std::enable_shared_from_this<AsioTimer>
{
public:
    AsioTimer(asio::io_context& ioContext);
    ~AsioTimer();

    // Non-copyable, non-movable
    AsioTimer(const AsioTimer&) = delete;
    AsioTimer& operator=(const AsioTimer&) = delete;
    AsioTimer(AsioTimer&&) = delete;
    AsioTimer& operator=(AsioTimer&&) = delete;

    // Start a repeating timer
    void Start(int intervalMs, TimerCallback callback);

    // Stop the timer
    void Stop();

    // Check if timer is running
    bool IsRunning() const noexcept { return m_running; }

private:
    asio::io_context& m_ioContext;
    asio::steady_timer m_timer;
    std::chrono::milliseconds m_interval;
    TimerCallback m_callback;
    std::atomic<bool> m_running;

    void OnTimer(const asio::error_code& error);
    void ScheduleNext();
};

// ASIO-based event loop replacing Windows message pump
class AsioEventLoop
{
public:
    AsioEventLoop();
    ~AsioEventLoop();

    // Initialize the event loop
    bool Initialize();

    // Run the event loop (blocking until Stop() is called)
    void Run();

    // Stop the event loop
    void Stop();

    // Check if running
    bool IsRunning() const noexcept { return m_running; }

    // Get the io_context for socket operations
    asio::io_context& GetIoContext() { return m_ioContext; }
    const asio::io_context& GetIoContext() const { return m_ioContext; }

    // Create and start a timer
    void StartTimer(TimerType type, int intervalMs, TimerCallback callback);

    // Stop a timer
    void StopTimer(TimerType type);

    // Stop all timers
    void StopAllTimers();

    // Get singleton instance
    static AsioEventLoop& GetInstance();

private:
    asio::io_context m_ioContext;
    using WorkGuard = asio::executor_work_guard<asio::io_context::executor_type>;
    std::unique_ptr<WorkGuard> m_workGuard;
    std::atomic<bool> m_running{false};
    std::atomic<bool> m_stopRequested{false};

    // Signal handling
#ifdef _WIN32
    // On Windows, we handle Ctrl+C via SetConsoleCtrlHandler
#else
    asio::signal_set m_signals;
#endif

    // Timers - using shared_ptr for lifetime safety
    std::map<TimerType, std::shared_ptr<AsioTimer>> m_timers;
    std::mutex m_timerMutex;

    // Signal handler callback
    void OnSignal(const asio::error_code& error, int signalNumber);

    // Windows console handler (static for Windows API callback)
#ifdef _WIN32
    static BOOL WINAPI ConsoleHandler(DWORD signal);
    static std::atomic<AsioEventLoop*> s_instance;
#endif
};

} // namespace Network
} // namespace W2PP

// C-style compatibility functions
// Note: These functions are provided for C interoperability.
// The callback parameter must not be null for StartTimer.
extern "C" {

// Initialize and run the ASIO event loop
bool EventLoop_Init();

// Run the event loop (blocking)
void EventLoop_Run();

// Stop the event loop
void EventLoop_Stop();

// Start a timer. callback must not be null.
void EventLoop_StartTimer(int timerId, int intervalMs, void (*callback)());

// Stop a timer
void EventLoop_StopTimer(int timerId);

} // extern "C"
