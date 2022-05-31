#include "socket.h"
#include <mutex>

#define USERSCOUNT 2

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
    int ProxyAuth(std::string proxyAuthStr);
    void Proxy(SOCKET c_fd);
    void ProxyDriver(int maxThreadsCount);
    void RunWithoutPreAllocatingThreads();
private:
    size_t mDataUsages[USERSCOUNT] = {};
    std::mutex tMutex;
};