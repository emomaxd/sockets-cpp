#include "include/TCPListener.h"
#include <iostream>

int main() {

    constexpr size_t THREAD_COUNT = 40;

    TCPListener listener(THREAD_COUNT); 

    listener.Listen(8080, [](Request& req, Response& res) {
        std::cout << "Received request: " << req.data << std::endl;

        std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nHello, World!\n";
        res.Send(response);
        
    });

    return 0;
}
