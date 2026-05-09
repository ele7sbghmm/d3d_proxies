#pragma once

#include <fstream>
#include <sstream>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>

#include "resource.h"

class server
{
    void serve_html(SOCKET client);
    void run();
};

