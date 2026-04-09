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

#include "SocketEventHandler.h"
#include "Logger.h"

namespace W2PP {
namespace Network {

// SocketEventHandler implementation

SocketEventHandler::SocketEventHandler()
    : m_running(false)
#ifdef _WIN32
    , m_hWnd(nullptr)
#else
    , m_hWnd(nullptr)
#endif
{
}

SocketEventHandler::~SocketEventHandler()
{
    Shutdown();
}

SocketEventHandler& SocketEventHandler::GetInstance()
{
    static SocketEventHandler instance;
    return instance;
}

bool SocketEventHandler::Initialize()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (m_manager)
    {
        return true; // Already initialized
    }
    
    m_manager = ISocketManager::Create();
    if (!m_manager)
    {
        LOG_ERROR("Failed to create socket manager");
        return false;
    }
    
    if (!m_manager->Initialize())
    {
        LOG_ERROR("Failed to initialize socket manager");
        m_manager.reset();
        return false;
    }
    
    m_running = true;
    
    // Start event processing thread
    m_eventThread = std::thread(&SocketEventHandler::EventLoopThread, this);
    
    LOG_INFO("SocketEventHandler initialized");
    return true;
}

void SocketEventHandler::Shutdown()
{
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_running = false;
    }
    
    if (m_eventThread.joinable())
    {
        m_eventThread.join();
    }
    
    if (m_manager)
    {
        m_manager->Stop();
        m_manager.reset();
    }
    
    std::lock_guard<std::mutex> lock(m_mutex);
    m_registrations.clear();
    
    LOG_INFO("SocketEventHandler shutdown complete");
}

bool SocketEventHandler::RegisterSocket(unsigned int socket, int wsaMsg, EventHandler callback)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (m_registrations.find(socket) != m_registrations.end())
    {
        // Already registered - update it
        m_registrations[socket]->wsaMsg = wsaMsg;
        m_registrations[socket]->callback = callback;
        return true;
    }
    
    auto reg = std::make_unique<SocketRegistration>();
    reg->socket = socket;
    reg->wsaMsg = wsaMsg;
    reg->callback = callback;
    reg->hasPendingAccept = false;
    reg->hasPendingRead = false;
    reg->hasPendingWrite = false;
    
    m_registrations[socket] = std::move(reg);
    
    LOG_DEBUG("Socket {} registered for events (msg={})", socket, wsaMsg);
    return true;
}

void SocketEventHandler::UnregisterSocket(unsigned int socket)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    
    auto it = m_registrations.find(socket);
    if (it != m_registrations.end())
    {
        m_registrations.erase(it);
        LOG_DEBUG("Socket {} unregistered", socket);
    }
}

bool SocketEventHandler::UpdateSocketEvents(unsigned int socket, int wsaMsg)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    
    auto it = m_registrations.find(socket);
    if (it == m_registrations.end())
    {
        return false;
    }
    
    it->second->wsaMsg = wsaMsg;
    return true;
}

void SocketEventHandler::ProcessEvents(int timeoutMs)
{
    if (!m_manager)
    {
        return;
    }
    
    // Poll for events
    m_manager->Poll();
}

void SocketEventHandler::RunEventLoop()
{
    if (!m_manager)
    {
        return;
    }
    
    LOG_INFO("Starting socket event loop");
    m_manager->Run();
}

void SocketEventHandler::StopEventLoop()
{
    if (!m_manager)
    {
        return;
    }
    
    LOG_INFO("Stopping socket event loop");
    m_manager->Stop();
}

void SocketEventHandler::EventLoopThread()
{
    LOG_INFO("Event loop thread started");
    
    while (m_running)
    {
        ProcessEvents(10); // 10ms timeout
        
        // Small sleep to prevent CPU spinning
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    
    LOG_INFO("Event loop thread stopped");
}

void SocketEventHandler::ProcessSocketEvents(SocketRegistration* reg)
{
    // This would be called when we have actual ASIO socket implementation
    // For now, it's a placeholder
}

void SocketEventHandler::OnSocketEvent(unsigned int socket, SocketEventType event, int errorCode)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    
    auto it = m_registrations.find(socket);
    if (it == m_registrations.end())
    {
        return;
    }
    
    auto& reg = it->second;
    
    // Map event type to WSA event code
    int wsaEvent = 0;
    switch (event)
    {
        case SocketEventType::Read:
            wsaEvent = WSA_EVENT_READ;
            break;
        case SocketEventType::Write:
            wsaEvent = WSA_EVENT_WRITE;
            break;
        case SocketEventType::Accept:
            wsaEvent = WSA_EVENT_ACCEPT;
            break;
        case SocketEventType::Close:
            wsaEvent = WSA_EVENT_CLOSE;
            break;
        case SocketEventType::Error:
            wsaEvent = WSA_EVENT_CLOSE;
            break;
    }
    
    // Call direct callback if provided
    if (reg->callback)
    {
        reg->callback(socket, wsaEvent, errorCode);
    }
    
#ifdef _WIN32
    // Post to window if available
    if (m_hWnd)
    {
        PostWindowMessage(socket, reg->wsaMsg, wsaEvent, errorCode);
    }
#endif
}

#ifdef _WIN32
bool SocketEventHandler::PostWindowMessage(unsigned int socket, int msg, int eventCode, int errorCode)
{
    if (!m_hWnd)
    {
        return false;
    }
    
    // Pack event code and error into lParam (similar to WSAAsyncSelect)
    LPARAM lParam = MAKELPARAM(eventCode, errorCode);
    
    return PostMessage(m_hWnd, msg, (WPARAM)socket, lParam) != 0;
}
#endif

// SocketEventListener implementation

SocketEventListener::SocketEventListener()
    : m_running(false)
    , m_readBuffer(8192)
{
}

SocketEventListener::~SocketEventListener()
{
    Stop();
}

void SocketEventListener::SetAcceptCallback(AcceptCallback callback)
{
    m_acceptCallback = callback;
}

void SocketEventListener::SetReadCallback(ReadCallback callback)
{
    m_readCallback = callback;
}

void SocketEventListener::SetCloseCallback(CloseCallback callback)
{
    m_closeCallback = callback;
}

void SocketEventListener::SetErrorCallback(ErrorCallback callback)
{
    m_errorCallback = callback;
}

void SocketEventListener::Attach(std::shared_ptr<ISocket> socket)
{
    Detach();
    m_socket = socket;
}

void SocketEventListener::Detach()
{
    Stop();
    m_socket.reset();
}

void SocketEventListener::Start()
{
    if (!m_socket || m_running)
    {
        return;
    }
    
    m_running = true;
    
    // Set up event callback
    m_socket->SetEventCallback(
        [this](SocketEventType event, int errorCode)
    {
        OnSocketEvent(event, errorCode);
    });
    
    // Start async operations based on socket state
    if (m_socket->GetState() == SocketState::Listening)
    {
        m_socket->AsyncAccept();
    }
    else if (m_socket->GetState() == SocketState::Connected)
    {
        m_socket->AsyncRead();
    }
}

void SocketEventListener::Stop()
{
    m_running = false;
}

void SocketEventListener::OnSocketEvent(SocketEventType event, int errorCode)
{
    if (!m_running)
    {
        return;
    }
    
    switch (event)
    {
        case SocketEventType::Accept:
            HandleAccept();
            break;
            
        case SocketEventType::Read:
            HandleRead();
            break;
            
        case SocketEventType::Write:
            // Write ready - not typically needed with our model
            break;
            
        case SocketEventType::Close:
            if (m_closeCallback)
            {
                m_closeCallback();
            }
            break;
            
        case SocketEventType::Error:
            if (m_errorCallback)
            {
                m_errorCallback(errorCode, "Socket error");
            }
            break;
    }
}

void SocketEventListener::HandleRead()
{
    if (!m_socket || !m_readCallback)
    {
        return;
    }
    
    // Read available data
    int bytesRead = m_socket->Receive(m_readBuffer.data(), static_cast<int>(m_readBuffer.size()));
    
    if (bytesRead > 0)
    {
        m_readCallback(m_readBuffer.data(), bytesRead);
        
        // Continue reading
        if (m_running)
        {
            m_socket->AsyncRead();
        }
    }
    else if (bytesRead < 0)
    {
        // Error or disconnect
        if (m_closeCallback)
        {
            m_closeCallback();
        }
    }
    else
    {
        // Would block - continue reading
        if (m_running)
        {
            m_socket->AsyncRead();
        }
    }
}

void SocketEventListener::HandleAccept()
{
    if (!m_socket || !m_acceptCallback)
    {
        return;
    }
    
    // Accept the connection
    auto newSocket = m_socket->Accept();
    
    if (newSocket)
    {
        std::string address;
        int port = 0;
        newSocket->GetRemoteAddress(address, port);
        
        m_acceptCallback(std::shared_ptr<ISocket>(newSocket.release()), address, port);
    }
    
    // Continue accepting
    if (m_running)
    {
        m_socket->AsyncAccept();
    }
}

} // namespace Network
} // namespace W2PP

// C-style compatibility functions

bool SocketEvent_Init()
{
    return W2PP::Network::SocketEventHandler::GetInstance().Initialize();
}

void SocketEvent_Shutdown()
{
    W2PP::Network::SocketEventHandler::GetInstance().Shutdown();
}

#ifdef _WIN32
bool SocketEvent_AsyncSelect(unsigned int socket, HWND hWnd, int wsaMsg, int events)
{
    // Store the window handle for posting messages
    auto& handler = W2PP::Network::SocketEventHandler::GetInstance();
    
    // Register the socket
    return handler.RegisterSocket(socket, wsaMsg, nullptr);
}
#else
bool SocketEvent_AsyncSelect(unsigned int socket, void* context, int wsaMsg, int events)
{
    // On Linux, we use direct callbacks instead of window messages
    auto& handler = W2PP::Network::SocketEventHandler::GetInstance();
    
    // Register the socket with a callback
    return handler.RegisterSocket(socket, wsaMsg, nullptr);
}
#endif

void SocketEvent_Process()
{
    W2PP::Network::SocketEventHandler::GetInstance().ProcessEvents(0);
}

void SocketEvent_Run()
{
    W2PP::Network::SocketEventHandler::GetInstance().RunEventLoop();
}

void SocketEvent_Stop()
{
    W2PP::Network::SocketEventHandler::GetInstance().StopEventLoop();
}
