#include "include/UDPListener.h"
#include <iostream>

int main() {
    UDPListener listener(40);
    listener.Listen(8080, [](Request& req, Response& res) {
        std::cout << "Message from client: " << req.DataAsString() << std::endl;

        // Send a response back to the client
        std::string response = "Hello from server!";
        res.SendString(response);
    });

    return 0;
}
