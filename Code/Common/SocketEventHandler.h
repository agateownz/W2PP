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

// Compatibility with Windows-based code
#ifdef _WIN32
#include <Windows.h>
#endif

#include <atomic>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

namespace W2PP {
namespace Network {

// Socket event handler that bridges ASIO events to the old Windows message-based system
// This allows gradual migration from WSAAsyncSelect to ASIO
class SocketEventHandler
{
public:
    // Event handler callback type - matches the old Windows message style
    using EventHandler = std::function<void(unsigned int socket, int eventType, int errorCode)>;

    SocketEventHandler();
    ~SocketEventHandler();

    // Initialize the handler
    bool Initialize();

    // Shutdown the handler
    void Shutdown();

    // Register a socket for event handling
    // hWnd: Window to post messages to (Windows only, can be NULL for direct callback mode)
    // wsaMsg: Message ID to use (e.g., WSA_READ, WSA_ACCEPT)
    // callback: Direct callback function (used when hWnd is NULL)
    bool RegisterSocket(unsigned int socket, int wsaMsg, EventHandler callback = nullptr);

    // Unregister a socket
    void UnregisterSocket(unsigned int socket);

    // Update socket event registration (change message type)
    bool UpdateSocketEvents(unsigned int socket, int wsaMsg);

    // Process pending events - call this in your main loop
    // timeoutMs: Maximum time to wait for events (0 = non-blocking)
    void ProcessEvents(int timeoutMs = 0);

    // Run the event loop (blocking) - for console mode
    void RunEventLoop();

    // Stop the event loop
    void StopEventLoop();

    // Check if running
    bool IsRunning() const { return m_running; }

    // Get singleton instance
    static SocketEventHandler& GetInstance();

private:
    struct SocketRegistration
    {
        unsigned int socket;
        int wsaMsg;
        EventHandler callback;
        std::shared_ptr<ISocket> socketImpl;
        bool hasPendingAccept;
        bool hasPendingRead;
        bool hasPendingWrite;
    };

    std::unique_ptr<ISocketManager> m_manager;
    std::map<unsigned int, std::unique_ptr<SocketRegistration>> m_registrations;
    std::mutex m_mutex;
    std::atomic<bool> m_running;
    std::thread m_eventThread;

    // Event processing
    void ProcessSocketEvents(SocketRegistration* reg);
    void OnSocketEvent(unsigned int socket, SocketEventType event, int errorCode);

    // Thread function for event loop
    void EventLoopThread();

    // Windows-specific: Post message to window
#ifdef _WIN32
    HWND m_hWnd;
    bool PostWindowMessage(unsigned int socket, int msg, int eventCode, int errorCode);
#else
    void* m_hWnd; // Placeholder for non-Windows
#endif
};

// Compatibility macros for old WSA event types
enum WSAEventType
{
    WSA_EVENT_READ = 1,
    WSA_EVENT_WRITE = 2,
    WSA_EVENT_ACCEPT = 8,
    WSA_EVENT_CONNECT = 16,
    WSA_EVENT_CLOSE = 32
};

// Helper class to manage socket events in a more modern way
class SocketEventListener
{
public:
    using AcceptCallback = std::function<void(std::shared_ptr<ISocket> newSocket, std::string address, int port)>;
    using ReadCallback = std::function<void(const char* data, int size)>;
    using CloseCallback = std::function<void()>;
    using ErrorCallback = std::function<void(int errorCode, const std::string& message)>;

    SocketEventListener();
    ~SocketEventListener();

    // Set callbacks
    void SetAcceptCallback(AcceptCallback callback);
    void SetReadCallback(ReadCallback callback);
    void SetCloseCallback(CloseCallback callback);
    void SetErrorCallback(ErrorCallback callback);

    // Attach to a socket
    void Attach(std::shared_ptr<ISocket> socket);

    // Detach from socket
    void Detach();

    // Start listening for events
    void Start();

    // Stop listening
    void Stop();

private:
    std::shared_ptr<ISocket> m_socket;
    AcceptCallback m_acceptCallback;
    ReadCallback m_readCallback;
    CloseCallback m_closeCallback;
    ErrorCallback m_errorCallback;

    std::atomic<bool> m_running;
    std::vector<char> m_readBuffer;

    void OnSocketEvent(SocketEventType event, int errorCode);
    void HandleRead();
    void HandleAccept();
};

} // namespace Network
} // namespace W2PP

// C-style compatibility functions for minimal code changes
extern "C" {

// Initialize the socket event system
bool SocketEvent_Init();

// Shutdown the socket event system
void SocketEvent_Shutdown();

// Register a socket for async events (compatibility with WSAAsyncSelect)
// On Windows, posts messages to hWnd. On Linux, uses callback.
#ifdef _WIN32
bool SocketEvent_AsyncSelect(unsigned int socket, HWND hWnd, int wsaMsg, int events);
#else
bool SocketEvent_AsyncSelect(unsigned int socket, void* context, int wsaMsg, int events);
#endif

// Process pending socket events (call in main loop)
void SocketEvent_Process();

// Run the socket event loop (blocking)
void SocketEvent_Run();

// Stop the socket event loop
void SocketEvent_Stop();

} // extern "C"
