#include <thread>
#include <signal.h>

#include "HTTPS_PROXY.h"
#include "base64.h"

short isRuning = 1;
int clients_count = 0;

using namespace std;

#define USERSCOUNT 2
string users[USERSCOUNT] = {"TEA:ASDAS", "MR.ROBOT:000"};
size_t dataUsages[USERSCOUNT] = {};

int HTTPS_PROXY::proxyAuth(std::string proxyAuthStr)
{
    proxyAuthStr = base64_decrypt((uchar*)proxyAuthStr.substr(5).c_str());
    int i;
    for (i = 0; i < USERSCOUNT; i++)
    {
        if (users[i] == proxyAuthStr)
        {
            break;
        }
    }
    return i;
}

void HTTPS_PROXY::proxy(SOCKET c_fd)
{
    clients_count++;
    cout << c_fd << ", Connected\n";

    SOCKET sc_fd = 0;
    char rBuf[1025] = "", srBuf[1025] = "";
    string headerStr = "", hostStr = "", proxyAuthStr = "";
    int rLen = 0, srLen, curUser = 0;
    unsigned long mOn = 1;
    ioctlsocket(c_fd, FIONBIO, &mOn);
    sockaddr_in sc_Addr;
    while (isRuning)
    {
        if (c_fd <= 0)
            break;
        rLen = recv(c_fd, rBuf, 1024, 0);
        if (rLen < 0 && !sc_fd)
            continue;
        else if (!rLen)
            break;

        if (!sc_fd)
        {
            rBuf[rLen] = '\0';
            headerStr += rBuf;
            if (rLen < 1024)
            {
                /*
                proxyAuthStr = httpGetHeaderContent(headerStr, "Proxy-Authorization");
                if (proxyAuthStr == "")
                {
                    const char authReq[] = "HTTP/1.1 407 Proxy Authentication Required\r\n"
                                         "Proxy-Authenticate:Basic realm=\"hello mf\"\r\n\r\n";
                    send(c_fd, authReq, strlen(authReq), 0);
                    break;
                }
                else if ((curUser = proxyAuth(proxyAuthStr)) >= USERSCOUNT)
                {
                    send(c_fd, "HTTP/1.1 403 No Fucking Way\r\n\r\n", 31, 0);
                    cout<<"OHNONONONO"<<endl;
                    break;
                }
                dataUsages[curUser] += headerStr.length();
                */
                hostStr = httpGetHeaderContent(headerStr, "Host").c_str();
                if (hostStr != "")
                {
                    sc_fd = initSocket();
//                    if (hostStr.find("github.com") == -1)
                    sc_Addr = initAddr(hostStr.c_str());
//                    else
//                        sc_Addr = initAddr("140.82.114.4", 443);
                    if (sockConn(&sc_fd, &sc_Addr) < 0)
                    {
                        break;//FUCK!!!
                    }
                    cout << c_fd << ", " << hostStr << endl;
                    ioctlsocket(sc_fd, FIONBIO, &mOn);
                }

                cout <<headerStr<<endl;
            }
        }
        
        if (headerStr.find("CONNECT"))
        {
            if (rLen > 0)
                if (send(sc_fd, rBuf, rLen, 0) == -1)
                {
                    break;//FUCK!!!
                }
            dataUsages[curUser] += rLen;
            while ((srLen = recv(sc_fd, srBuf, 1024, 0)) > 0 && isRuning)
            {
                srBuf[srLen] = 0;
                send(c_fd, srBuf, srLen, 0);
                dataUsages[curUser] += srLen;
            }
            if (!srLen)
                break;
            
        }
        else
        {
            send(c_fd, "HTTP/1.1 200 Connection established\r\n\r\n", 40, 0);
            dataUsages[curUser] += 40;
            if (httpGetHeaderContent(headerStr, "Connection") == "close" || httpGetHeaderContent(headerStr, "Proxy-Connection") == "close")
                break;
            headerStr = "";
            continue;
        }

        if (httpGetHeaderContent(headerStr, "Connection") == "close" || httpGetHeaderContent(headerStr, "Proxy-Connection") == "close")
            break;
    }
    closesocket(sc_fd);
    closesocket(c_fd);
    clients_count--;
    cout << c_fd << ", closed\n";
}

template <typename F>
void HTTPS_PROXY::server(F f)
{
    SOCKET s_fd, c_fd;
    struct sockaddr_in cli_addr = {}, srv_addr = initAddr("0.0.0.0", 4399);
    socklen_t cli_len;
    unsigned long mOn = 1;
    while (isRuning)
    {
        s_fd = initSocket();
        setsockopt(s_fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&mOn, sizeof(char));
        printf("Listening...\n");
        if (listenSocket(s_fd, &srv_addr) == (SOCKET)-1)
            continue;
        ioctlsocket(s_fd, FIONBIO, &mOn);
        while (isRuning)
        {
            c_fd = accept(s_fd, (struct sockaddr*)&cli_addr, &cli_len);
            if (c_fd > 0 && c_fd != (SOCKET)-1)
            {
    //            while (clients_count > 12);
                f(c_fd);
    //            thread(proxy, c_fd).detach();
                break;
            }
        }
        closesocket(s_fd);
    }
    while (clients_count);
    closeSocket(&s_fd);
}

void HTTPS_PROXY::proxyDriver(int maxThreadsCount)
{
    std::thread threads[maxThreadsCount];
    SOCKET c_fds[maxThreadsCount];
    memset(c_fds, -1, sizeof (SOCKET) * maxThreadsCount);
    for (int i = 0; i < maxThreadsCount; i++)
    {
        threads[i] = thread([pC_fd = &c_fds[i], this]()
        {
            while(isRuning)
            {
                while(isRuning && (*pC_fd == (SOCKET)-1 || !*pC_fd));
                proxy(*pC_fd);
                *pC_fd = (SOCKET)-1;
            }
        });
        threads[i].detach();
    }
    server([maxThreadsCount, c_fds=&c_fds[0]](SOCKET c_fd)
    {
        if (c_fd <= 0)
            return;

        int i;
        while (isRuning)
        {
            for (i = 0; i < maxThreadsCount; i++)
            {
                if (c_fds[i] == (SOCKET)-1)
                {
                    c_fds[i] = c_fd;
                    return;
                }
            }
//            cout << "clients count:" << clients_count <<endl;
        };
    });
}

void HTTPS_PROXY::runWithoutPreAllocatingThreads()
{
#ifndef _WIN32
 //   if(!fork())
 //   {
#endif

        server([this](SOCKET c_fd)
        {
            std::thread([this, c_fd]()
            {
                this->proxy(c_fd);
            }).detach();
        });
        
//    proxyDriver(12);
#ifndef _WIN32
//    }
#endif
}

void onStop(int argc)
{
    isRuning = 0;
}

int main()
{
    signal(SIGINT, onStop);

    HTTPS_PROXY* httpProxy = new HTTPS_PROXY();
    httpProxy->runWithoutPreAllocatingThreads();
    delete httpProxy;
    return 0;
}
