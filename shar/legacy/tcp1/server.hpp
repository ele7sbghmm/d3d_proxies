#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <wincrypt.h>
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "crypt32.lib")

#include <shellapi.h>
#include <string>
#include <regex>

#include "shar.hpp"

namespace server {
    std::string Base64Encode(const unsigned char* data, size_t len) {
        DWORD outLen = 0;
        CryptBinaryToStringA(data, len, CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, nullptr, &outLen);
        std::string out(outLen, 0);
        CryptBinaryToStringA(data, len, CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, out.data(), &outLen);
        out.resize(outLen);
        return out;
    }

    std::string MakeAcceptKey(const std::string& clientKey) {
        std::string magic = clientKey + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

        HCRYPTPROV hProv = 0;
        HCRYPTHASH hHash = 0;

        if (!CryptAcquireContext(&hProv, nullptr, nullptr, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) {
            printf("CryptAcquireContext failed: %lu\n", GetLastError());
            return {};
        }
        if (!CryptCreateHash(hProv, CALG_SHA1, 0, 0, &hHash)) {
            printf("CryptCreateHash failed: %lu\n", GetLastError());
            CryptReleaseContext(hProv, 0);
            return {};
        }
        if (!CryptHashData(hHash, (BYTE*)magic.data(), (DWORD)magic.size(), 0)) {
            printf("CryptHashData failed: %lu\n", GetLastError());
            CryptDestroyHash(hHash);
            CryptReleaseContext(hProv, 0);
            return {};
        }

        unsigned char hash[20];
        DWORD hashLen = 20;
        if (!CryptGetHashParam(hHash, HP_HASHVAL, hash, &hashLen, 0)) {
            printf("CryptGetHashParam failed: %lu\n", GetLastError());
            CryptDestroyHash(hHash);
            CryptReleaseContext(hProv, 0);
            return {};
        }

        CryptDestroyHash(hHash);
        CryptReleaseContext(hProv, 0);
        return Base64Encode(hash, 20);
    }

    void DoHandshake(SOCKET client, const std::string& request) {
        std::smatch m;
        std::regex keyRe("Sec-WebSocket-Key: ([^\r\n]+)");
        if (!std::regex_search(request, m, keyRe)) {
            printf("DoHandshake: no Sec-WebSocket-Key found\n");
            return;
        }

        std::string accept = MakeAcceptKey(m[1].str());
        if (accept.empty()) {
            printf("DoHandshake: MakeAcceptKey failed\n");
            return;
        }

        printf("DoHandshake: key=%s accept=%s\n", m[1].str().c_str(), accept.c_str());

        std::string response =
            "HTTP/1.1 101 Switching Protocols\r\n"
            "Upgrade: websocket\r\n"
            "Connection: Upgrade\r\n"
            "Sec-WebSocket-Accept: " + accept + "\r\n\r\n";

        int r = send(client, response.c_str(), (int)response.size(), 0);
        if (r == SOCKET_ERROR) {
            printf("DoHandshake: send failed: %d\n", WSAGetLastError());
            return;
        }

        printf("DoHandshake: handshake complete, sent %d bytes\n", r);
    }

    void SendWebSocketBinary(SOCKET client, const void* data, size_t len) {
        if (client == INVALID_SOCKET) return;

        unsigned char header[4];
        int headerLen = 0;

        header[0] = 0x82;
        if (len <= 125) {
            header[1] = (unsigned char)len;
            headerLen = 2;
        }
        else if (len <= 65535) {
            header[1] = 126;
            header[2] = (len >> 8) & 0xFF;
            header[3] = len & 0xFF;
            headerLen = 4;
        }
        else {
            printf("SendWebSocketBinary: payload too large\n");
            return;
        }

        if (send(client, (char*)header, headerLen, 0) == SOCKET_ERROR) {
            printf("SendWebSocketBinary: header send failed: %d\n", WSAGetLastError());
            return;
        }
        if (send(client, (char*)data, (int)len, 0) == SOCKET_ERROR) {
            printf("SendWebSocketBinary: data send failed: %d\n", WSAGetLastError());
            return;
        }
    }

    SOCKET client = INVALID_SOCKET;

    void Run() {
        WSADATA wsa;
        (void)WSAStartup(MAKEWORD(2, 2), &wsa);

        SOCKET server = socket(AF_INET, SOCK_STREAM, 0);
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(9000);
        addr.sin_addr.s_addr = INADDR_ANY;

        bind(server, (sockaddr*)&addr, sizeof(addr));
        listen(server, 1);

        while (true) {
            client = accept(server, nullptr, nullptr);

            char buf[4096]{};
            int r = recv(client, buf, sizeof(buf) - 1, 0);
            printf("recv %d bytes:\n%s\n", r, buf);

            std::string request(buf);

            if (request.find("Upgrade: websocket") != std::string::npos) {
                DoHandshake(client, request);
                // now client is a valid WebSocket connection
                // use SendFence() but wrap data in WebSocket frame

                char frame[256];
                while (recv(client, frame, sizeof(frame), 0) > 0) { }
            }
            else if (request.find("GET /") != std::string::npos) {
                const char* body = R"(
<!DOCTYPE html>
<html>
<body>
<canvas id="c" width="800" height="600"></canvas>
<script>
    const ws = new WebSocket('ws://localhost:9000');
    ws.binaryType = 'arraybuffer';
    ws.onopen = () => console.log('connected');
    ws.onmessage = e => {
        console.log('got data', e.data.byteLength, 'bytes');
    };
    ws.onerror = e => console.error('ws error', e);
</script>
</body>
</html>
)";

                std::string response =
                    "HTTP/1.1 200 OK\r\n"
                    "Content-Type: text/html\r\n"
                    "Connection: close\r\n"
                    "Content-Length: " + std::to_string(strlen(body)) + "\r\n"
                    "\r\n" + body;

                send(client, response.c_str(), (int)response.size(), 0);

                closesocket(client);
                client = INVALID_SOCKET;
            }
        }
    }

#pragma pack(push, 1)
    struct Fence {
        float fence_start[3];
        float fence_end[3];
        float fence_normal[3];
        float position[3];
        float range;
        float distance;
        float radius;
        std::uint32_t collision_area_index;
    };
#pragma pack(pop)
    void SendFence(Fence fence) {
        static std::size_t count = 0;
        
        if (client == INVALID_SOCKET) return;
        
        count++;
        if (count % 10000) return;

        //send(client, reinterpret_cast<char*>(&fence), sizeof(Fence), 0);
        
        SendWebSocketBinary(client, &fence, sizeof(Fence));
    }
}
