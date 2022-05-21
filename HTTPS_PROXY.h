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
    template <typename F>
    void Server(F f);
    void ProxyDriver(int maxThreadsCount);
    void RunWithoutPreAllocatingThreads();
private:
    size_t mDataUsages[USERSCOUNT] = {};
    SOCKET data_fd, dataC_fd;
    std::mutex tMutex;

    unsigned long mOn = 1;
    /*
    void initDataUsageServer()
    {
        struct sockaddr_in data_addr = initAddr("127.0.0.1", 4399);
        data_fd = initSocket();
        setsockopt(data_fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&mOn, sizeof(char));
        listenSocket(data_fd, &data_addr);
        ioctlsocket(data_fd, FIONBIO, &mOn);
    }

    void dataUsageServerTryAccept()
    {
        if ((dataC_fd = accept(data_fd, nullptr, nullptr)) != (SOCKET)-1)
        {
            recv()
        }
    }

    void closeDataUsageServer()
    {
        closesocket(data_fd);
    }*/
};