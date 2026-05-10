#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

class server
{
public:
    static void serve_not_ready(SOCKET client);
    static void serve_html(SOCKET client);
    static void serve_js(SOCKET client);
    static void serve_fences(SOCKET client);
    static void handle_client(SOCKET client);
    static void run();
};
