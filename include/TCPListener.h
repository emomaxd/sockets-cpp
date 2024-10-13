#pragma once

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <thread>
#include <vector>
#include <functional>
#include <atomic>
#include <cassert>
#include <string>

#include "ThreadPool.h"
#include "TCPUtils.h" /* Request & Response structs */

class TCPListener {
public:
    TCPListener(size_t numThreads)
        : m_Socket(CreateSocket()), 
          m_StopListening(false), 
          m_ThreadPool(numThreads) 
    {
        assert(m_Socket >= 0);
    }

    ~TCPListener() {
        StopListening();
    }

    bool Listen(uint16_t port, std::function<void(Request&, Response&)> onNewConnection = nullptr, uint32_t backlog = 50) {
        m_Address.sin_family = AF_INET;
        m_Address.sin_addr.s_addr = INADDR_ANY; // Accept any client address
        m_Address.sin_port = htons(port);

        if (bind(m_Socket, reinterpret_cast<sockaddr*>(&m_Address), sizeof(m_Address)) < 0) {
            std::cerr << "Failed to bind socket" << std::endl;
            close(m_Socket);
            return false;
        }

        if (listen(m_Socket, backlog) < 0) {
            std::cerr << "Failed to listen on socket" << std::endl;
            close(m_Socket);
            return false;
        }

        std::cout << "Listening on port: " << port << std::endl; 

        while (!m_StopListening) {
            Accept(onNewConnection);
        }
        return true;
    }

private:
    void Accept(std::function<void(Request&, Response&)> onNewConnection) {
        sockaddr_in clientAddress;
        socklen_t clientAddressLen = sizeof(clientAddress);
        int clientSocket = accept(m_Socket, reinterpret_cast<sockaddr*>(&clientAddress), &clientAddressLen);

        if (clientSocket < 0) {
            if (m_StopListening) return;
            std::cerr << "Failed to accept connection" << std::endl;
            return;
        }

        std::cout << "Client connected: " << inet_ntoa(clientAddress.sin_addr) << std::endl;

        m_ThreadPool.enqueue([clientSocket, onNewConnection]() {
            Request req(clientSocket);
            Response res(clientSocket);

            uint8_t buffer[1024];
            ssize_t bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);

            if (bytesRead > 0) {
                req.data = std::vector<uint8_t>(buffer, buffer + bytesRead);  // Fixed this line
                onNewConnection(req, res);
            } else if (bytesRead == 0) {
                std::cout << "Client disconnected" << std::endl;
            } else {
                std::cerr << "Error receiving data" << std::endl;
            }

        });
    }

    int CreateSocket() {
        int socketFD = socket(AF_INET, SOCK_STREAM, 0);
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

