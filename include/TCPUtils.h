#pragma once

#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>



struct Request {
    int clientSocket;
    std::vector<uint8_t> data;

    explicit Request(int socket) : clientSocket(socket) {}
};

struct Response {
    int clientSocket;

    explicit Response(int socket) : clientSocket(socket) {}

    ~Response() {
        if (clientSocket >= 0)
            close(clientSocket);
    }


    void Send(const std::string& message) {
        send(clientSocket, message.c_str(), message.size(), 0);
    }
};


