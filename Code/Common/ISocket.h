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

#include <functional>
#include <memory>
#include <string>
#include <vector>

// Socket event types for async operations
enum class SocketEventType
{
    Accept,     // New connection accepted
    Read,       // Data available to read
    Write,      // Ready to write
    Close,      // Connection closed
    Error       // Error occurred
};

// Socket event structure
struct SocketEvent
{
    SocketEventType type;
    int errorCode;
    std::string errorMessage;
};

// Socket state
enum class SocketState
{
    Disconnected,
    Connecting,
    Connected,
    Listening,
    Error
};

// Forward declaration
class ISocket;

// Socket event callback type
using SocketEventCallback = std::function<void(SocketEventType event, int errorCode)>;

// Socket interface for cross-platform abstraction
class ISocket
{
public:
    virtual ~ISocket() = default;

    // Initialize the socket (create socket, setup etc)
    virtual bool Initialize() = 0;

    // Close the socket
    virtual void Close() = 0;

    // Start listening for connections
    virtual bool Listen(const std::string& address, int port) = 0;

    // Connect to a remote host
    virtual bool Connect(const std::string& host, int port) = 0;

    // Accept a new connection (returns a new ISocket for the accepted connection)
    virtual std::unique_ptr<ISocket> Accept() = 0;

    // Send data
    virtual int Send(const char* data, int size) = 0;

    // Receive data
    virtual int Receive(char* buffer, int maxSize) = 0;

    // Set event callback for async operations
    virtual void SetEventCallback(SocketEventCallback callback) = 0;

    // Get current socket state
    virtual SocketState GetState() const = 0;

    // Get native socket handle (for compatibility during transition)
    virtual unsigned int GetNativeSocket() = 0;

    // Set native socket (for compatibility during transition)
    virtual void SetNativeSocket(unsigned int sock) = 0;

    // Check if socket is valid
    virtual bool IsValid() const = 0;

    // Get local address/port
    virtual bool GetLocalAddress(std::string& address, int& port) const = 0;

    // Get remote address/port
    virtual bool GetRemoteAddress(std::string& address, int& port) const = 0;

    // Enable/disable non-blocking mode
    virtual bool SetNonBlocking(bool nonBlocking) = 0;

    // Start async accept operation
    virtual void AsyncAccept() = 0;

    // Start async read operation
    virtual void AsyncRead() = 0;

    // Start async write operation
    virtual void AsyncWrite(const char* data, int size) = 0;

    // Factory method to create a socket implementation
    static std::unique_ptr<ISocket> Create();
};

// Socket manager for running the event loop
class ISocketManager
{
public:
    virtual ~ISocketManager() = default;

    // Initialize the manager
    virtual bool Initialize() = 0;

    // Run the event loop (blocking)
    virtual void Run() = 0;

    // Run one iteration of the event loop (non-blocking)
    virtual void Poll() = 0;

    // Stop the event loop
    virtual void Stop() = 0;

    // Factory method to create a manager
    static std::unique_ptr<ISocketManager> Create();
};
