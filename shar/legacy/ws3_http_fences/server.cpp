#include "pch.h"

#include <string>
#include "server.h"
#include "hooks.h"

void server::serve_not_ready(SOCKET client)
{
    const char* res =
        "HTTP/1.1 503 Service Unavailable\r\n"
        "Retry-After: 1\r\n"
        "Content-Length: 0\r\n\r\n";

    send(client, res, strlen(res), 0);
}

void server::serve_html(SOCKET client)
{
    std::string html =
        "<!doctype html>\r\n<html>\r\n<head>\r\n"
        "    <title>workin'</title>\r\n"
        "</head>\r\n<body style=\"background: black;\">\r\n"
        "    <h1 id=\"h1\">...</h1>\r\n"
        "    <canvas id=\"cv\" width=\"1000\" height=\"1000\"></canvas>\r\n"
        "    <script src=\"/index.js\"></script>\r\n"
        "</body>\r\n</html>\r\n";
    std::string res =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: " + std::to_string(html.size()) + "\r\n"
        "Connection: close\r\n\r\n"
        + html;

    send(client, res.c_str(), (int)res.size(), 0);
}

void server::serve_js(SOCKET client)
{
#include "index_js.h"
    std::string res =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: application/javascript\r\n"
        "Content-Length: " + std::to_string(index_js_len) + "\r\n"
        "Connection: close\r\n\r\n"
        + std::string((char*)index_js, index_js_len);

    send(client, res.c_str(), (int)res.size(), 0);
}

void server::serve_fences(SOCKET client)
{
    std::string fences_json = hooks::stringify_fences();
    if (fences_json.empty()) {
        serve_not_ready(client);
        return;
    }

    std::string fence_headers =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: " + std::to_string(fences_json.size()) + "\r\n\r\n"
        + fences_json;

    send(client, fence_headers.c_str(), (int)fence_headers.size(), 0);
}

void server::handle_client(SOCKET client) {
    char buf[4096]{};
    recv(client, buf, sizeof(buf), 0);

    if (strstr(buf, "GET /fence"))
        serve_fences(client);
    else if (strstr(buf, "GET /index.js"))
        serve_js(client);
    else if (strstr(buf, "GET /"))
        serve_html(client);

    closesocket(client);
}

void server::run()
{
    WSADATA wsa;
    (void)WSAStartup(MAKEWORD(2, 2), &wsa);

    SOCKET s = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(9001);
    addr.sin_addr.s_addr = INADDR_ANY;

    bind(s, (sockaddr*)&addr, sizeof(addr));
    listen(s, 10);

    while (true)
    {
        SOCKET client = accept(s, nullptr, nullptr);
        server::handle_client(client);
    }

    WSACleanup();
}
