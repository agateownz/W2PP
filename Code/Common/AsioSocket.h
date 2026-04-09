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

// ASIO headers
#include <asio.hpp>
#include <asio/ip/tcp.hpp>

#include <atomic>
#include <mutex>

namespace W2PP {
namespace Network {

// Forward declaration
class AsioSocketManager;

// ASIO-based socket implementation
class AsioSocket : public ISocket
{
public:
    AsioSocket(asio::io_context& ioContext);
    AsioSocket(asio::io_context& ioContext, asio::ip::tcp::socket socket);
    ~AsioSocket() override;

    // ISocket interface implementation
    bool Initialize() override;
    void Close() override;
    bool Listen(const std::string& address, int port) override;
    bool Connect(const std::string& host, int port) override;
    std::unique_ptr<ISocket> Accept() override;
    int Send(const char* data, int size) override;
    int Receive(char* buffer, int maxSize) override;
    void SetEventCallback(SocketEventCallback callback) override;
    SocketState GetState() const override;
    unsigned int GetNativeSocket() override;
    void SetNativeSocket(unsigned int sock) override;
    bool IsValid() const override;
    bool GetLocalAddress(std::string& address, int& port) const override;
    bool GetRemoteAddress(std::string& address, int& port) const override;
    bool SetNonBlocking(bool nonBlocking) override;
    void AsyncAccept() override;
    void AsyncRead() override;
    void AsyncWrite(const char* data, int size) override;

    // Internal use - called by manager
    void OnAcceptComplete(const asio::error_code& error);
    void OnReadComplete(const asio::error_code& error, size_t bytesRead);
    void OnWriteComplete(const asio::error_code& error, size_t bytesWritten);

private:
    asio::io_context& m_ioContext;
    asio::ip::tcp::socket m_socket;
    asio::ip::tcp::acceptor m_acceptor;
    
    std::atomic<SocketState> m_state;
    SocketEventCallback m_callback;
    mutable std::mutex m_mutex;
    
    // Buffer for async operations
    std::vector<char> m_readBuffer;
    static constexpr size_t READ_BUFFER_SIZE = 8192;

    void HandleError(const asio::error_code& error, const char* operation);
    void NotifyEvent(SocketEventType event, int errorCode = 0);
};

// ASIO-based socket manager
class AsioSocketManager : public ISocketManager
{
public:
    AsioSocketManager();
    ~AsioSocketManager() override;

    // ISocketManager interface implementation
    bool Initialize() override;
    void Run() override;
    void Poll() override;
    void Stop() override;

    // Get the ASIO io_context
    asio::io_context& GetIoContext() { return m_ioContext; }

private:
    asio::io_context m_ioContext;
    std::atomic<bool> m_running;
    std::unique_ptr<asio::io_context::work> m_work;
};

} // namespace Network
} // namespace W2PP
