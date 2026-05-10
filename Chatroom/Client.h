#ifndef Client_h
#define Client_h
#include <iostream>
#include <string>
#include <thread>
#include <atomic>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")
using namespace std;

class Client
{
private:
    SOCKET sock;                    // connection to the server.
    string host;                    // server address.
    int port;                       // server port.
    atomic<bool> stopFlag;          // tells both threads to shut down.

    // thread function that prints whatever the server sends.
    void receiveLoop();

public:
    Client();                       // initialize Winsock.
    ~Client();                      // cleanup.
    void run();                     // connect, then read/send messages.
};

#endif /* Client_h */
