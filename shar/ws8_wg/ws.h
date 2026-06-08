#pragma once

 
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

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

	void fetch_vertices(std::vector<Vtx>& vertices) {
		auto* rm = shar::RenderManager::GetInstance();
		if (!rm || !rm->wrl_ || !rm->wrl_->mStaticLoadLists.mUseSize) return;

		shar::SwapArray<shar::FenceEntityDSG*> fenceArray
			= rm->wrl_->mStaticLoadLists.mpData->mFenceElems;
		for (int i = fenceArray.mUseSize; 0 < i--;) {
			auto* fence = fenceArray.mpData[i];
			D3DXVECTOR3& s = fence->mStartPoint;
			D3DXVECTOR3& e = fence->mEndPoint;
			vertices.push_back({ { s.x, 0, s.z }, 1});
			vertices.push_back({ { e.x, 0, e.z }, 1});
		}
	}

	void handle_client(SOCKET client) {
		char buf[4096]{};
		int received = recv(client, buf, sizeof(buf), 0);
		if (received <= 0) return;

		std::string str{ buf, (std::size_t)received };
		

		if (str.contains("GET /fences")) {
			std::vector<Vtx> vertices;
			fetch_vertices(vertices);
			std::size_t byteLength = vertices.size() * sizeof(Vtx);
			if (!byteLength) return;

			std::string http = header("200 OK", "application/octet-stream", byteLength);
			send(client, http.c_str(), http.size(), 0);
			send(client, (const char*)vertices.data(), byteLength, 0);
		}
		else if (str.contains("GET /index.js")) {
			std::string http = header("200 OK", "application/javascript", index_js_len);
			send(client, http.c_str(), http.size(), 0);
			send(client, (const char*)index_js, index_js_len, 0);
		}
		else if (str.contains("GET /")) {
			std::string http = header("200 OK", "text/html", index_html_len);
			send(client, http.c_str(), http.size(), 0);
			send(client, (const char*)index_html, index_html_len, 0);
		}
	}

	void run() {
		WSADATA wsadata{};
		(void)WSAStartup(MAKEWORD(2, 2), &wsadata);

		SOCKET server = socket(AF_INET, SOCK_STREAM, 0);

		sockaddr_in addr{};
		addr.sin_family = AF_INET;
		addr.sin_port = htons(9001);
		addr.sin_addr.s_addr = INADDR_ANY;


		bind(server, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
		listen(server, 100);

		while (true) {
			SOCKET client = accept(server, nullptr, nullptr);
			handle_client(client);
			closesocket(client);
		}

		WSACleanup();
	}
}
