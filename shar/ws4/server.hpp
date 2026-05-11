#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <iostream>

#include <winsock2.h>
#include <ws2tcpip.h>
#include <bcrypt.h>
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "Bcrypt.lib")

class Server
{
public:
    static constexpr char B64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    static std::string base64_encode(const unsigned char* data, int len)
    {
        std::string out;
        for (int i = 0; i < len; i += 3)
        {
            unsigned char b0 = data[i];
            unsigned char b1 = (i + 1 < len) ? data[i + 1] : 0;
            unsigned char b2 = (i + 2 < len) ? data[i + 2] : 0;
            out += B64[b0 >> 2];
            out += B64[((b0 & 3) << 4) | (b1 >> 4)];
            out += (i + 1 < len) ? B64[((b1 & 0xf) << 2) | (b2 >> 6)] : '=';
            out += (i + 2 < len) ? B64[b2 & 0x3f] : '=';
        }
        return out;
    }

    static std::string sha1_base64(const std::string& input)
    { 
        BCRYPT_ALG_HANDLE alg = nullptr;
        BCRYPT_HASH_HANDLE hash = nullptr;
        BYTE digest[20] = {};

        (void)BCryptOpenAlgorithmProvider(&alg, BCRYPT_SHA1_ALGORITHM, nullptr, 0);
        (void)BCryptCreateHash(alg, &hash, nullptr, 0, nullptr, 0, 0);
        (void)BCryptHashData(hash,
            reinterpret_cast<PUCHAR>(const_cast<char*>(input.data())),
            static_cast<ULONG>(input.size()), 0);
        (void)BCryptFinishHash(hash, digest, 20, 0);
        BCryptDestroyHash(hash);
        BCryptCloseAlgorithmProvider(alg, 0);

        return base64_encode(digest, 20);
    }

    static std::string compute_accept_key(const std::string& client_key)
    {
        return sha1_base64(client_key + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
    }

    static bool do_handshake(SOCKET cfd)
    {
        char buf[4096] = {};
        int n = recv(cfd, buf, sizeof(buf) - 1, 0);
        if (n <= 0) return false;

        std::string request(buf, n);
        const std::string needle = "Sec-WebSocket-Key: ";
        auto pos = request.find(needle);
        if (pos == std::string::npos) return false;
        pos += needle.size();
        auto end = request.find("\r\n", pos);
        std::string key = request.substr(pos, end - pos);

        std::string accept = compute_accept_key(key);
        std::string res =
            "HTTP/1.1 101 Switching Protocols\r\n"
            "Upgrade: websocket\r\n"
            "Connection: Upgrade\r\n"
            "Sec-WebSocket-Accept: " + accept + "\r\n\r\n";

        send(cfd, res.c_str(), static_cast<int>(res.size()), 0);
        return true;
    }

    static int recv_all(SOCKET fd, void* buf, int len)
    {
        int total = 0;
        char* ptr = reinterpret_cast<char*>(buf);
        while (total < len)
        {
            int r = recv(fd, ptr + total, len - total, 0);
            if (r <= 0) return r;
            total += r;
        }
        return total;
    }

    static std::string read_frame(SOCKET fd)
    {
        std::uint8_t header[2] = {};
        if (recv_all(fd, header, 2) != 2) return {};

        std::uint8_t opcode = header[0] & 0x0f;
        bool masked = (header[1] & 0x80) != 0;
        std::uint64_t plen = header[1] & 0x7f;

        if (plen == 126)
        {
            std::uint8_t ext[2] = {};
            recv_all(fd, ext, 2);
            plen = (static_cast<std::uint64_t>(ext[0]) << 8) | ext[1];
        }
        else if (plen == 127)
        {
            std::uint8_t ext[8] = {};
            recv_all(fd, ext, 8);
            plen = 0;
            for (int i = 0; i < 8; ++i)
            {
                plen = (plen << 8) | ext[i];
            }
        }

        std::uint8_t mask[4] = {};
        if (masked) recv_all(fd, mask, 4);

        std::vector<std::uint8_t> payload(static_cast<std::size_t>(plen));
        recv_all(fd, payload.data(), static_cast<int>(plen));
        if (masked)
            for (std::size_t i = 0; i < payload.size(); ++i)
            {
                payload[i] ^= mask[i % 4];
            }
        
        if (opcode == 0x8) return {};
        return std::string(payload.begin(), payload.end());
    }

    static std::vector<std::uint8_t> build_frame(const std::string& msg)
    {
        std::vector<std::uint8_t> frame;
        frame.push_back(0x81);

        std::size_t len = msg.size();
        if (len <= 125)
        {
            frame.push_back(static_cast<std::uint8_t>(len));
        }
        else if (len <= 65535)
        {
            frame.push_back(126);
            frame.push_back(static_cast<std::uint8_t>((len >> 8) & 0xff));
            frame.push_back(static_cast<std::uint8_t>(len & 0xff));
        }
        else
        {
            frame.push_back(127);
            for (int i = 7; i >= 0; --i)
            {
                frame.push_back(static_cast<std::uint8_t>((len >> (8 * i)) & 0xff));
            }
        }
        frame.insert(frame.end(), msg.begin(), msg.end());
        return frame;
    }

    static void send_text(SOCKET fd, const std::string& msg)
    {
        auto frame = build_frame(msg);
        send(fd,
            reinterpret_cast<char*>(frame.data()),
            static_cast<int>(frame.size()), 0);
    }

    static void serve_html(SOCKET cli)
    {
        std::string html =
            "<!DOCTYPE html>\r\n<html>\r\n<head>\r\n"
            "<title>workin'</title>\r\n"
            "</head><body>\r\n"
            "<h1>workin'</h1>\r\n"
            "<script>\r\n"
            "console.log('workin\\'');\r\n"
            "</script>\r\n</body></html>";
        std::string http =
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: " + std::to_string(html.size()) + "\r\n\r\n"
            + html;

        send(cli, html.c_str(), static_cast<int>(http.size()), 0);
    }

    static void run()
    {
        WSADATA wsa{};
        if (WSAStartup(MAKEWORD(2, 2), &wsa))
        {
            std::cerr << "WSAStartup failed\n";
            return;
        }

        SOCKET srv = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (srv == INVALID_SOCKET)
        {
            std::cerr << "socket() failed: " << WSAGetLastError() << "\n";
            WSACleanup();
            return;
        }

        int opt = 1;
        setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&opt), sizeof(opt));

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(static_cast<u_short>(9001));
        addr.sin_addr.s_addr = INADDR_ANY;

        if (bind(srv, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == SOCKET_ERROR)
        {
            std::cerr << "bind() failed: " << WSAGetLastError() << "\n";
            closesocket(srv);
            WSACleanup();
            return;
        }

        listen(srv, 5);
        std::cout << "WebSocket server listening on ws://localhost:9001\n";

        while (true)
        {
            sockaddr_in cli{};
            int cli_len = sizeof(cli);
            SOCKET cfd = accept(srv, reinterpret_cast<sockaddr*>(&cli), &cli_len);
            if (cfd == INVALID_SOCKET) continue;

            char ip[INET_ADDRSTRLEN] = {};
            inet_ntop(AF_INET, &cli.sin_addr, ip, sizeof(ip));
            std::cout << "Client connected: " << ip << "\n";

            if (!do_handshake(cfd))
            {
                closesocket(cfd); continue;
            }

            send_text(cfd, "Hello from C++ WebSocket server!");

            while (true)
            {
                std::string msg = read_frame(cfd);
                if (msg.empty())
                {
                    std::cout << "Client disconnected\n";
                    break;
                }
                std::cout << "Received: " << msg << "\n";
                send_text(cfd, "Echo: " + msg);
            }
            closesocket(cfd);
        }
    }
};