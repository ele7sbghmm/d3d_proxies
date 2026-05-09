#pragma once

#include <fstream>
#include <sstream>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#include "www.h"
//#include <App.h>

namespace server {
    void handle_client(SOCKET client) {
        printf("handle_client\n");

        std::string body = INDEX_HTML;
        std::string response = "HTTP/1.1 200 OK\r\n"
                               "Content-Type: text/html\r\n"
                               "Content-Length: " + std::to_string(body.size()) + "\r\n"
                               "Connection: close\r\n\r\n"
                               + body;

        send(client, response.c_str(), (int)response.size(), 0);
        closesocket(client);
    }

    void run() {
        WSADATA wsa;
        (void)WSAStartup(MAKEWORD(2, 2), &wsa);

        SOCKET server = socket(AF_INET, SOCK_STREAM, 0);

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(9000);
        addr.sin_addr.s_addr = INADDR_ANY;

        bind(server, (sockaddr*)&addr, sizeof(addr));
        listen(server, 10);

        while (true) {
            SOCKET client = accept(server, nullptr, nullptr);
            handle_client(client);
        }

        WSACleanup();
    }
}