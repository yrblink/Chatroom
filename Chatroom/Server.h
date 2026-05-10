#ifndef Server_h
#define Server_h
#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <unordered_map>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")
using namespace std;

class Server
{
private:
    SOCKET listenSock;          // socket the server listens on.
    int port;                   // tcp port number.
    bool running;               // server loop flag.

    mutex clientsLock;          // protects the clients map below.
    unordered_map<SOCKET, string> clients;  // socket to username.

    // helpers.
    string getTime();                                       // current time as HH:MM:SS.
    string trim(const string& s);                           // strip whitespace.
    bool sendMsg(SOCKET sock, const string& msg);           // safe send wrapper.
    void broadcast(const string& msg, SOCKET exclude);      // send to all except one.
    SOCKET findUser(const string& name);                    // lookup by username.
    string listUsers();                                     // for the /list command.

    // per-client thread function.
    void handleClient(SOCKET sock, string addr);

public:
    Server();                   // initialize Winsock.
    ~Server();                  // cleanup.
    void start();               // bind, listen, accept loop.
};

#endif /* Server_h */
