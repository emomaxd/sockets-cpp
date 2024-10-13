#pragma once

#include <vector>
#include <cstdint>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <string>

struct Request {
    int clientSocket;                  // The server socket used for communication
    std::vector<uint8_t> data;         // To handle binary or any data type
    sockaddr_in clientAddress;         // The address of the client
    socklen_t clientAddressLen;

    Request(int socket, sockaddr_in addr, socklen_t addrLen)
        : clientSocket(socket), clientAddress(addr), clientAddressLen(addrLen) {}

    std::string DataAsString() const {
        return std::string(data.begin(), data.end());  // Convert binary data to string
    }
};

struct Response {
    int serverSocket;                  // The socket used for sending data
    sockaddr_in clientAddress;         // The client address to send the response to
    socklen_t clientAddressLen;

    Response(int socket, sockaddr_in addr, socklen_t addrLen)
        : serverSocket(socket), clientAddress(addr), clientAddressLen(addrLen) {}

    void Send(const std::vector<uint8_t>& message) {
        sendto(serverSocket, message.data(), message.size(), 0,
               reinterpret_cast<sockaddr*>(&clientAddress), clientAddressLen);
    }

    // Convenience function for sending string data
    void SendString(const std::string& message) {
        std::vector<uint8_t> data(message.begin(), message.end());
        Send(data);
    }
};
