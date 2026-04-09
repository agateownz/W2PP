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

// Include winsock2.h BEFORE any windows.h to avoid conflicts
#ifdef _WIN32
#include <winsock2.h>
#endif

#include "AsioSocket.h"
#include "Logger.h"

#include <cstring>

namespace W2PP {
namespace Network {

// Global manager instance for factory methods
static AsioSocketManager* g_manager = nullptr;

// AsioSocket implementation

AsioSocket::AsioSocket(asio::io_context& ioContext)
    : m_ioContext(ioContext)
    , m_socket(ioContext)
    , m_acceptor(ioContext)
    , m_state(SocketState::Disconnected)
    , m_readBuffer(READ_BUFFER_SIZE)
{
}

AsioSocket::AsioSocket(asio::io_context& ioContext, asio::ip::tcp::socket socket)
    : m_ioContext(ioContext)
    , m_socket(std::move(socket))
    , m_acceptor(ioContext)
    , m_state(SocketState::Connected)
    , m_readBuffer(READ_BUFFER_SIZE)
{
}

AsioSocket::~AsioSocket()
{
    Close();
}

bool AsioSocket::Initialize()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (m_state != SocketState::Disconnected)
    {
        return false;
    }
    
    // Socket is created on demand
    return true;
}

void AsioSocket::Close()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    
    m_state = SocketState::Disconnected;
    
    asio::error_code ec;
    
    if (m_acceptor.is_open())
    {
        m_acceptor.close(ec);
    }
    
    if (m_socket.is_open())
    {
        m_socket.close(ec);
    }
}

bool AsioSocket::Listen(const std::string& address, int port)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    
    try
    {
        asio::ip::tcp::endpoint endpoint;
        
        if (address.empty() || address == "0.0.0.0")
        {
            endpoint = asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port);
        }
        else
        {
            asio::ip::address addr = asio::ip::make_address(address);
            endpoint = asio::ip::tcp::endpoint(addr, port);
        }
        
        m_acceptor.open(endpoint.protocol());
        m_acceptor.set_option(asio::ip::tcp::acceptor::reuse_address(true));
        m_acceptor.bind(endpoint);
        m_acceptor.listen(asio::socket_base::max_listen_connections);
        
        m_state = SocketState::Listening;
        return true;
    }
    catch (const std::exception& e)
    {
        LOG_ERROR("AsioSocket::Listen failed: {}", e.what());
        m_state = SocketState::Error;
        return false;
    }
}

bool AsioSocket::Connect(const std::string& host, int port)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    
    try
    {
        asio::ip::tcp::resolver resolver(m_ioContext);
        auto endpoints = resolver.resolve(host, std::to_string(port));
        
        m_state = SocketState::Connecting;
        
        asio::error_code ec;
        asio::connect(m_socket, endpoints, ec);
        
        if (ec)
        {
            HandleError(ec, "Connect");
            m_state = SocketState::Error;
            return false;
        }
        
        m_state = SocketState::Connected;
        return true;
    }
    catch (const std::exception& e)
    {
        LOG_ERROR("AsioSocket::Connect failed: {}", e.what());
        m_state = SocketState::Error;
        return false;
    }
}

std::unique_ptr<ISocket> AsioSocket::Accept()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (m_state != SocketState::Listening)
    {
        return nullptr;
    }
    
    try
    {
        asio::ip::tcp::socket peerSocket(m_ioContext);
        m_acceptor.accept(peerSocket);
        
        return std::make_unique<AsioSocket>(m_ioContext, std::move(peerSocket));
    }
    catch (const std::exception& e)
    {
        LOG_ERROR("AsioSocket::Accept failed: {}", e.what());
        return nullptr;
    }
}

int AsioSocket::Send(const char* data, int size)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (!m_socket.is_open() || m_state != SocketState::Connected)
    {
        return -1;
    }
    
    try
    {
        asio::error_code ec;
        size_t bytesSent = m_socket.write_some(asio::buffer(data, size), ec);
        
        if (ec)
        {
            if (ec == asio::error::would_block)
            {
                return 0; // Would block in non-blocking mode
            }
            HandleError(ec, "Send");
            return -1;
        }
        
        return static_cast<int>(bytesSent);
    }
    catch (const std::exception& e)
    {
        LOG_ERROR("AsioSocket::Send failed: {}", e.what());
        return -1;
    }
}

int AsioSocket::Receive(char* buffer, int maxSize)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (!m_socket.is_open())
    {
        return -1;
    }
    
    try
    {
        asio::error_code ec;
        size_t bytesRead = m_socket.read_some(asio::buffer(buffer, maxSize), ec);
        
        if (ec)
        {
            if (ec == asio::error::would_block)
            {
                return 0; // No data available in non-blocking mode
            }
            if (ec == asio::error::eof)
            {
                m_state = SocketState::Disconnected;
                return -1; // Connection closed
            }
            HandleError(ec, "Receive");
            return -1;
        }
        
        return static_cast<int>(bytesRead);
    }
    catch (const std::exception& e)
    {
        LOG_ERROR("AsioSocket::Receive failed: {}", e.what());
        return -1;
    }
}

void AsioSocket::SetEventCallback(SocketEventCallback callback)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_callback = callback;
}

SocketState AsioSocket::GetState() const
{
    return m_state.load();
}

unsigned int AsioSocket::GetNativeSocket()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    
#ifdef _WIN32
    return static_cast<unsigned int>(m_socket.native_handle());
#else
    return static_cast<unsigned int>(m_socket.native_handle());
#endif
}

void AsioSocket::SetNativeSocket(unsigned int sock)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // This is a compatibility method - on ASIO we can't easily set native socket
    // after creation. This would need platform-specific code.
    LOG_WARN("AsioSocket::SetNativeSocket called - limited support");
}

bool AsioSocket::IsValid() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_socket.is_open();
}

bool AsioSocket::GetLocalAddress(std::string& address, int& port) const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (!m_socket.is_open())
    {
        return false;
    }
    
    try
    {
        asio::error_code ec;
        auto endpoint = m_socket.local_endpoint(ec);
        
        if (ec)
        {
            return false;
        }
        
        address = endpoint.address().to_string();
        port = endpoint.port();
        return true;
    }
    catch (...)
    {
        return false;
    }
}

bool AsioSocket::GetRemoteAddress(std::string& address, int& port) const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (!m_socket.is_open())
    {
        return false;
    }
    
    try
    {
        asio::error_code ec;
        auto endpoint = m_socket.remote_endpoint(ec);
        
        if (ec)
        {
            return false;
        }
        
        address = endpoint.address().to_string();
        port = endpoint.port();
        return true;
    }
    catch (...)
    {
        return false;
    }
}

bool AsioSocket::SetNonBlocking(bool nonBlocking)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (!m_socket.is_open())
    {
        return false;
    }
    
    try
    {
        asio::error_code ec;
        m_socket.non_blocking(nonBlocking, ec);
        return !ec;
    }
    catch (...)
    {
        return false;
    }
}

void AsioSocket::AsyncAccept()
{
    if (m_state != SocketState::Listening)
    {
        return;
    }
    
    // Create a new socket for the incoming connection
    auto newSocket = std::make_shared<asio::ip::tcp::socket>(m_ioContext);
    
    m_acceptor.async_accept(*newSocket,
        [this, newSocket](const asio::error_code& error)
    {
        OnAcceptComplete(error);
    });
}

void AsioSocket::AsyncRead()
{
    if (!m_socket.is_open())
    {
        return;
    }
    
    m_socket.async_read_some(
        asio::buffer(m_readBuffer),
        [this](const asio::error_code& error, size_t bytesRead)
    {
        OnReadComplete(error, bytesRead);
    });
}

void AsioSocket::AsyncWrite(const char* data, int size)
{
    if (!m_socket.is_open())
    {
        return;
    }
    
    auto buffer = std::make_shared<std::vector<char>>(data, data + size);
    
    m_socket.async_write_some(
        asio::buffer(*buffer),
        [this, buffer](const asio::error_code& error, size_t bytesWritten)
    {
        OnWriteComplete(error, bytesWritten);
    });
}

void AsioSocket::OnAcceptComplete(const asio::error_code& error)
{
    if (error)
    {
        HandleError(error, "AsyncAccept");
        NotifyEvent(SocketEventType::Error);
    }
    else
    {
        NotifyEvent(SocketEventType::Accept);
    }
}

void AsioSocket::OnReadComplete(const asio::error_code& error, size_t bytesRead)
{
    if (error)
    {
        if (error == asio::error::eof)
        {
            m_state = SocketState::Disconnected;
            NotifyEvent(SocketEventType::Close);
        }
        else
        {
            HandleError(error, "AsyncRead");
            NotifyEvent(SocketEventType::Error);
        }
    }
    else
    {
        NotifyEvent(SocketEventType::Read);
    }
}

void AsioSocket::OnWriteComplete(const asio::error_code& error, size_t bytesWritten)
{
    if (error)
    {
        HandleError(error, "AsyncWrite");
        NotifyEvent(SocketEventType::Error);
    }
    else
    {
        NotifyEvent(SocketEventType::Write);
    }
}

void AsioSocket::HandleError(const asio::error_code& error, const char* operation)
{
    if (error != asio::error::operation_aborted)
    {
        LOG_ERROR("AsioSocket {} error: {}", operation, error.message());
    }
    
    if (error == asio::error::connection_reset ||
        error == asio::error::connection_aborted ||
        error == asio::error::connection_refused)
    {
        m_state = SocketState::Disconnected;
    }
}

void AsioSocket::NotifyEvent(SocketEventType event, int errorCode)
{
    SocketEventCallback callback;
    
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        callback = m_callback;
    }
    
    if (callback)
    {
        callback(event, errorCode);
    }
}

// AsioSocketManager implementation

AsioSocketManager::AsioSocketManager()
    : m_running(false)
{
}

AsioSocketManager::~AsioSocketManager()
{
    Stop();
}

bool AsioSocketManager::Initialize()
{
    m_work = std::make_unique<asio::io_context::work>(m_ioContext);
    return true;
}

void AsioSocketManager::Run()
{
    m_running = true;
    
    while (m_running)
    {
        try
        {
            m_ioContext.run();
        }
        catch (const std::exception& e)
        {
            LOG_ERROR("AsioSocketManager::Run exception: {}", e.what());
        }
        
        if (m_running)
        {
            // Restart if stopped due to no work
            m_ioContext.restart();
        }
    }
}

void AsioSocketManager::Poll()
{
    try
    {
        m_ioContext.poll();
    }
    catch (const std::exception& e)
    {
        LOG_ERROR("AsioSocketManager::Poll exception: {}", e.what());
    }
}

void AsioSocketManager::Stop()
{
    m_running = false;
    m_work.reset();
    m_ioContext.stop();
}

} // namespace Network
} // namespace W2PP

// Factory implementations

std::unique_ptr<ISocket> ISocket::Create()
{
    // This should be provided with an io_context from the manager
    // For now, we need to ensure the manager is created first
    if (!W2PP::Network::g_manager)
    {
        return nullptr;
    }
    
    return std::make_unique<W2PP::Network::AsioSocket>(
        W2PP::Network::g_manager->GetIoContext());
}

std::unique_ptr<ISocketManager> ISocketManager::Create()
{
    auto manager = std::make_unique<W2PP::Network::AsioSocketManager>();
    W2PP::Network::g_manager = manager.get();
    return manager;
}
