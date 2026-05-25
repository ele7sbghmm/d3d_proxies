#pragma once

#include <string>

#include <winsock2.h>
#include <Ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

#include "www.hpp"

std::string header(int code, const char* mime, int len) {
	std::string http =
		"HTTP/1.1 " + std::to_string(code) + " OK\r\n"
		"Content-Type: " + mime + "\r\n"
		"Content-Length: " + std::to_string(len) + "\r\n"
		"\r\n";
	return http;
}

void serve_html(SOCKET client) {
	std::string html((const char*)index_html, index_html_size);
	std::string http = header(200, "text/html", index_html_size);
	send(client, http.c_str(), http.size(), 0);
	send(client, html.c_str(), html.size(), 0);
}

void serve_js(SOCKET client) {
	std::string js((const char*)index_js, index_js_size);
	std::string http = header(200, "application/javascript", index_js_size);
	send(client, http.c_str(), http.size(), 0);
	send(client, js.c_str(), js.size(), 0);
}

void handle_client(SOCKET client) {
	char buf[4096]{};
	int len = recv(client, buf, sizeof(buf), 0);

	std::string str(buf, len);
	if (str.contains("GET /index.js"))
		serve_js(client);
	else if (str.contains("GET /"))
		serve_html(client);
	closesocket(client);
}

void run() {
	WSADATA wsaData{};
	(void)WSAStartup(MAKEWORD(2, 2), &wsaData);

	SOCKET server = socket(AF_INET, SOCK_STREAM, 0);

	sockaddr_in addr{};
	addr.sin_family = AF_INET;
	addr.sin_port = htons(9001);
	addr.sin_addr.s_addr = INADDR_ANY;

	bind(server, (sockaddr*)&addr, sizeof(addr));
	listen(server, 1);

	while (true) {
		SOCKET client = accept(server, nullptr, nullptr);
		handle_client(client);
	}

	WSACleanup();
}