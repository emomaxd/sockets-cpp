#pragma once

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <functional>
#include <atomic>
#include <cassert>
#include "ThreadPool.h"
#include "UDPUtils.h"  // Contains Request & Response structs using std::vector<uint8_t>

class UDPListener {
public:
    UDPListener(size_t numThreads)
        : m_Socket(CreateSocket()), 
          m_StopListening(false), 
          m_ThreadPool(numThreads) 
    {
        assert(m_Socket >= 0);
    }

    ~UDPListener() {
        StopListening();
    }

    bool Listen(uint16_t port, std::function<void(Request&, Response&)> onNewMessage = nullptr) {
        m_Address.sin_family = AF_INET;
        m_Address.sin_addr.s_addr = INADDR_ANY;  // Accept any client address
        m_Address.sin_port = htons(port);

        if (bind(m_Socket, reinterpret_cast<sockaddr*>(&m_Address), sizeof(m_Address)) < 0) {
            std::cerr << "Failed to bind socket" << std::endl;
            close(m_Socket);
            return false;
        }

        std::cout << "Listening on UDP port: " << port << std::endl;

        while (!m_StopListening) {
            Receive(onNewMessage);
        }
        return true;
    }

private:
    void Receive(std::function<void(Request&, Response&)> onNewMessage) {
        sockaddr_in clientAddress;
        socklen_t clientAddressLen = sizeof(clientAddress);
        std::vector<uint8_t> buffer(1024);  // Buffer to receive binary or any data

        ssize_t bytesRead = recvfrom(m_Socket, buffer.data(), buffer.size(), 0, 
                                     reinterpret_cast<sockaddr*>(&clientAddress), &clientAddressLen);

        if (bytesRead < 0) {
            if (m_StopListening) return;
            std::cerr << "Failed to receive message" << std::endl;
            return;
        }

        std::cout << "Received message from: " << inet_ntoa(clientAddress.sin_addr) << std::endl;

        // Resize buffer to actual received size
        buffer.resize(bytesRead);

        // Process the message in the thread pool
        m_ThreadPool.enqueue([this, clientAddress, clientAddressLen, buffer, onNewMessage]() {
            Request req(m_Socket, clientAddress, clientAddressLen);
            req.data = buffer;

            Response res(m_Socket, clientAddress, clientAddressLen);

            if (onNewMessage) {
                onNewMessage(req, res);
            }
        });
    }

    int CreateSocket() {
        int socketFD = socket(AF_INET, SOCK_DGRAM, 0);  // UDP socket
        if (socketFD < 0) {
            std::cerr << "Socket creation failed" << std::endl;
            return -1;
        }

        int opt = 1;
        if (setsockopt(socketFD, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
            std::cerr << "Failed to set socket options" << std::endl;
            close(socketFD);
            return -1;
        }

        return socketFD;
    }

    void StopListening() {
        if (m_Socket >= 0) {
            close(m_Socket);
        }

        m_StopListening = true;
    }

private:
    int m_Socket;
    sockaddr_in m_Address;
    std::atomic<bool> m_StopListening;
    ThreadPool m_ThreadPool;
};
