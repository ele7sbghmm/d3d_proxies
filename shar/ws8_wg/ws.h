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
		return "HTTP/1.1 " + code + "\r\n" +
			"Content-Type: " + mime + "\r\n" +
			"Content-Length: " + std::to_string(length) + "\r\n\r\n";
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

	int recv_all(SOCKET client, void* buf, int len) {
		int total = 0;
		char* ptr = reinterpret_cast<char*>(buf);
		while (total < len) {
			int r = recv(client, ptr + total, len - total, 0);
			if (r <= 0) return r;
			total += r;
		}
		return total;
	}

	std::string read_frame(SOCKET client) {
		std::uint8_t header[2]{};
		if (recv_all(client, header, 2) != 2) return {};

		std::uint8_t opcode = header[0] & 0xf;
		bool masked = (header[1] & 0x80) != 0;
		std::uint64_t plen = header[1] & 0x7f;

		if (plen == 126) {
			std::uint8_t ext[2]{};
			recv_all(client, ext, 2);
			plen = (static_cast<std::uint64_t>(ext[0]) << 8) | ext[1];
		}
		else if (plen == 127) {
			std::uint8_t ext[2]{};
			recv_all(client, ext, 8);
			plen = 0;
			for (int i = 0; i < 8; ++i)
				plen = (plen << 8) | ext[i];
		}

		std::uint8_t mask[4]{};
		if (masked) recv_all(client, mask, 4);

		std::vector<std::uint8_t> payload(static_cast<std::size_t>(plen));
		recv_all(client, payload.data(), static_cast<int>(plen));
		if (masked)
			for (std::size_t i = 0; i < payload.size(); ++i)
				payload[i] ^= mask[i % 4];

		if (opcode == 8) return {};
		return std::string(payload.begin(), payload.end());
	}

	std::vector<std::uint8_t> build_frame(const std::string& msg) {
		std::vector<unsigned char> frame;
		frame.push_back(0x81);

		std::size_t len = msg.size();
		if (len <= 125)
			frame.push_back(static_cast<std::uint8_t>(len));
		else if (len <= 65535) {
			frame.push_back(126);
			frame.push_back(static_cast<std::uint8_t>((len >> 8) & 0xff));
			frame.push_back(static_cast<unsigned char>(len & 0xff));
		}
		else {
			frame.push_back(127);
			for (int i = 8; 0 < i--;)
				frame.push_back(static_cast<unsigned char>((len >> (8 * i)) & 0xff));
		}

		frame.insert(frame.end(), msg.begin(), msg.end());
		return frame;
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
			if (client == INVALID_SOCKET) continue;

			std::jthread worker(handle_client, client).detach();
		}

		WSACleanup();
	}
}
