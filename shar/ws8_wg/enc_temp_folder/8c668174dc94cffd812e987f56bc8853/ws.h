#pragma once

 
#include <winsock2.h>
#include <ws2tcpip.h>
#include <bcrypt.h>
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "Bcrypt.lib")

#include <string>
#include <vector>

#include <d3dx8.h>
#include "www/www.h"
#include "shar.h"

struct Vtx {
	D3DXVECTOR3 xyz;
	D3DCOLOR w;
};



namespace svr {
	std::string header(std::string code, std::string mime, int length) {
		return	"HTTP/1.1 " + code + "\r\n" +
				"Content-Type: " + mime + "\r\n" +
				"Content-Length: " + std::to_string(length) + "\r\n" +
				"\r\n";
	}

	std::string sha1_base64(const std::string& input) {
		BCRYPT_ALG_HANDLE alg = nullptr;
		BCRYPT_HASH_HANDLE hash = nullptr;
		static constexpr int digest_len = 20;
		BYTE digest[digest_len] = {};

		(void)BCryptOpenAlgorithmProvider(&alg, BCRYPT_SHA1_ALGORITHM, nullptr, 0);
		(void)BCryptCreateHash(alg, &hash, nullptr, 0, nullptr, 0, 0);
		(void)BCryptHashData(hash,
			reinterpret_cast<PUCHAR>(const_cast<char*>(input.data())),
			static_cast<ULONG>(input.size()), 0);
		(void)BCryptFinishHash(hash, digest, 20, 0);
		BCryptDestroyHash(hash);
		BCryptCloseAlgorithmProvider(alg, 0);

		static constexpr char B64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
		std::string out;
		for (int i = 0; i < digest_len; i += 3) {
			unsigned char b0 = digest[i];
			unsigned char b1 = (i + 1 < digest_len) ? digest[i + 1] : 0;
			unsigned char b2 = (i + 2 < digest_len) ? digest[i + 2] : 0;
			out += B64[b0 >> 2];
			out += B64[((b0 & 3) << 4) | (b1 >> 4)];
			out += (i + 1 < digest_len) ? B64[((b1 & 0xf) << 2) | (b2 >> 6)] : '=';
			out += (i + 2 < digest_len) ? B64[b2 & 0x3f] : '=';
		}

		return out;
	}

	bool handle_client(SOCKET client) {
		char buf[4096]{};
		int received = recv(client, buf, sizeof(buf), 0);
		if (received <= 0) return false;

		std::string request{ buf, (std::size_t)received };

		if (request.contains("GET /ws")) {
			const std::string guid = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

			const std::string needle = "Sec-WebSocket-Key: ";
			auto pos = request.find(needle);
			if (pos == std::string::npos) return false;
			pos += needle.size();
			auto end = request.find("\r\n", pos);
			std::string key = request.substr(pos, end - pos);

			std::string accept_key = sha1_base64(key + guid);
			std::string response =
				"HTTP/1.1 101 Switching Protocols\r\n"
				"Upgrade: websocket\r\n"
				"Connection: Upgrade\r\n"
				"Sec-WebSocket-Accept: " + accept_key + "\r\n\r\n";

			send(client, response.c_str(), response.size(), 0);

			return true;
		}

		else if (request.contains("GET /index.js")) {
			std::string response = header("200 OK", "application/javascript", index_js_len);
			send(client, response.c_str(), response.size(), 0);
			send(client, (const char*)index_js, index_js_len, 0);
		}
		else if (request.contains("GET /")) {
			std::string http = header("200 OK", "text/html", index_html_len);
			send(client, http.c_str(), http.size(), 0);
			send(client, (const char*)index_html, index_html_len, 0);
		}

		return false;
	}

	void run() {
		WSADATA wsadata{};
		(void)WSAStartup(MAKEWORD(2, 2), &wsadata);

		SOCKET server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

		int opt = 1;
		setsockopt(server, SOL_SOCKET, SO_REUSEADDR,
			reinterpret_cast<const char*>(&opt), sizeof(opt));

		sockaddr_in addr{};
		addr.sin_family = AF_INET;
		addr.sin_port = htons(9001);
		addr.sin_addr.s_addr = INADDR_ANY;

		bind(server, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
		listen(server, 100);

		while (true) {
			SOCKET client = accept(server, nullptr, nullptr); 
			bool keep_alive = handle_client(client);
			if (!keep_alive) closesocket(client);
		}

		WSACleanup();
	}
}
