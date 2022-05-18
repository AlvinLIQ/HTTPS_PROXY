#include <thread>

#include "header.h"

short isRuning = 1;
int clients_count = 0;

using namespace std;

void proxy(SOCKET c_fd)
{
    cout << c_fd << ", Connected\n";

    SOCKET sc_fd = 0;
    char rBuf[1025] = "", srBuf[1025] = "";
    string headerStr = "", hostStr = "";
    int rLen = 0, srLen;
    unsigned long mOn = 1;
    ioctlsocket(c_fd, FIONBIO, &mOn);
    sockaddr_in sc_Addr;
    while (isRuning)
    {
        rLen = recv(c_fd, rBuf, 1024, 0);
        if (rLen < 0 && !sc_fd)
            continue;
        else if (!rLen)
            break;

        if (!sc_fd)
        {
            rBuf[rLen] = '\0';
            headerStr = rBuf;
            if (rLen < 1024)
            {
                hostStr = httpGetHeaderContent(headerStr, "Host").c_str();
                if (hostStr != "")
                {
                    sc_fd = initSocket();
                    sc_Addr = initAddr(hostStr.c_str());
                    if (sockConn(&sc_fd, &sc_Addr) < 0)
                    {
                        break;//FUCK!!!
                    }
                    cout << c_fd << ", " << hostStr << endl;
                    ioctlsocket(sc_fd, FIONBIO, &mOn);
                }
            }
        }
        
        if (headerStr.find("CONNECT"))
        {
            if (rLen > 0)
                if (send(sc_fd, rBuf, rLen, 0) == -1)
                {
                    break;//FUCK!!!
                }

            while ((srLen = recv(sc_fd, srBuf, 1024, 0)) > 0)
            {
                srBuf[srLen] = 0;
                send(c_fd, srBuf, srLen, 0);
            }
            if (!srLen)
                break;
        }
        else
        {
            send(c_fd, "HTTP/1.1 200 Connection established\r\n\r\n", 40, 0);
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

void server(void (*f)(SOCKET))
{
    SOCKET s_fd, c_fd;
    int mOn = 1;
    while (isRuning)
    {
        s_fd = initSocket();
        setsockopt(s_fd, SOL_SOCKET, SO_REUSEADDR, &mOn, sizeof(mOn));
        printf("Listening...\n");
        c_fd = listenSocket(s_fd, 4399);
        if (c_fd != (unsigned int)-1)
        {
            clients_count++;
    //        while (clients_count > 8);
            f(c_fd);
//            thread(proxy, c_fd).detach();
        }
        close(s_fd);
    }
    while (clients_count);
    closeSocket(&s_fd);
}

SOCKET conn_fd = 0;
void proxyDriver(int maxThreadsCount)
{
    std::thread threads[maxThreadsCount];
    for (int i = 0; i < maxThreadsCount; i++)
    {
        threads[i] = thread([]()
        {
            while(isRuning)
            {
                while(!conn_fd);
                proxy(conn_fd);
                conn_fd = 0;
            }
        });
        threads[i].detach();
    }
    server([](SOCKET c_fd)
    {
        conn_fd = c_fd;
    });
}

int main()
{
#ifdef _WIN32
    initWinSock();
#endif
/*
    server([](SOCKET c_fd)
    {
        thread(proxy, c_fd);
    });*/
    proxyDriver(8);
    return 0;
}
