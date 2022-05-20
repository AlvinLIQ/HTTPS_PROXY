#include "socket.h"

class HTTPS_PROXY
{
public:
    HTTPS_PROXY()
    {
#ifdef _WIN32
    initWinSock();
#endif
    }
    ~HTTPS_PROXY()
    {

    }
    int proxyAuth(std::string proxyAuthStr);
    void proxy(SOCKET c_fd);
    template <typename F>
    void server(F f);
    void proxyDriver(int maxThreadsCount);
    void runWithoutPreAllocatingThreads();
};