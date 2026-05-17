#include "pch.h"

#include "server.h"

namespace sv {
inline void serve_html(SOCKET cs) {
  std::string html = "<!DOCTYPE html>\r\n<html>\r\n<head>\r\n"
                     "<title>workin'</title>\r\n"
                     "</head><body style=\"background: black;\">\r\n"
                     "<h1>workin'</h1>\r\n"
                     "<canvas id=\"cv\" width=1000 height=1000></canvas>\r\n"
                     "<script>\r\n"
                     " const cv = document.getElementById('cv');\r\n"
                     " const ctx = cv.getContext('2d');\r\n"
                     " setInterval(() = > { await fetchVertices(); }, 1000);\r\n"

                     "  async function fetchVertices() {\r\n"
                     "   const res = await fetch('/api/vertices');\r\n"
                     "   const buffer = await res.arrayBuffer();\r\n"

                     "   const stride = 16;\r\n"
                     "   const view = new DataView(buffer);\r\n"
                     "   const count = buffer.byteLength / stride;\r\n"

                     "   for (let i = 0; i < count; i++) {\r\n"
                     "   let offset = i * stride;\r\n"
                     "   const x = view.getFloat32(offset, true);\r\n"
                     "   const y = view.getFloat32(offset + 4, true);\r\n"
                     "   const z = view.getFloat32(offset + 8, true);\r\n"
                     "   const c = view.getUint32(offset + 12, true);\r\n"
                     "   const cv = document.getElementById('cv');\r\n"
                     "   const ctx = cv.getContext('2d');\r\n"

                     "   const b = (c & 0xff);\r\n"
                     "   const g = (c >> 8) & 0xff;\r\n"
                     "   const r = (c >> 16) & 0xff;\r\n"
                     "   const a = ((c >> 24) & 0xff) / 255;\r\n"
                     "   ctx.fillStyle = `rgba(${r}, ${g}, ${b}, ${a})`;\r\n"

                     "   ctx.fillRect(x + 500, y + 500, 2, 2);\r\n"
                     "  }\r\n"
                     " }\r\n"
                     "</script>\r\n</body>\r\n</html>\r\n";
  std::string http = "HTTP/1.1 200 OK\r\n"
                     "Content-Type: text/html\r\n"
                     "Content-Length: " +
                     std::to_string(html.size()) + "Connection: close\r\n\r\n" +
                     html;

  send(cs, http.c_str(), http.size(), 0);
}

inline void serve_vb(SOCKET cs) {
  Vtx *data = g_ctx.b.data;
  size_t len = g_ctx.b.vtx_n * sizeof(Vtx);
  std::string http = "HTTP/1.1 200 OK\r\n"
                     "Content-Type: application/octet-stream\r\n"
                     "Content-Length: " +
                     std::to_string(len) +
                     "\r\n"
                     "Connection: close\r\n"
                     "\r\n";
  send(cs, http.c_str(), (int)http.size(), 0);
  send(cs, (char *)data, (int)len, 0);
}

inline void handle_client(SOCKET cs) {
  char buf[1024];
  recv(cs, buf, sizeof(buf), 0);

  if (strstr(buf, "GET /api/vertices"))
    serve_vb(cs);
  else if (strstr(buf, "GET /"))
    serve_html(cs);

  closesocket(cs);
}

inline void run() {
  WSADATA wsa{};
  (void)WSAStartup(MAKEWORD(2, 2), &wsa);

  SOCKET ss = socket(AF_INET, SOCK_STREAM, 0);

  sockaddr_in addr{};
  addr.sin_family = AF_INET;
  addr.sin_port = htons(9001);
  addr.sin_addr.s_addr = INADDR_ANY;

  bind(ss, reinterpret_cast<sockaddr *>(&addr), sizeof(addr));
  listen(ss, 100);

  while (true) {
    int len = sizeof(addr);
    SOCKET cs = accept(ss, reinterpret_cast<sockaddr *>(&addr), &len);

    handle_client(cs);
  }

  WSACleanup();
}
} // namespace sv
