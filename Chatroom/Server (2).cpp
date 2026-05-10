#include "Server.h"
#include <ctime>
#include <iomanip>
#include <sstream>
using namespace std;

// constructor.
Server::Server()
{
    port = 5555;
    running = false;
    listenSock = INVALID_SOCKET;

    // initialize Winsock (required on Windows before using sockets).
    WSADATA wsa;
    int result = WSAStartup(MAKEWORD(2, 2), &wsa);
    if (result != 0)
    {
        cout << "WSAStartup failed: " << result << endl;
    }
}

// destructor.
Server::~Server()
{
    cout << "Shutting down server. \n";
    if (listenSock != INVALID_SOCKET)
    {
        closesocket(listenSock);
    }
    WSACleanup();
}

// get current time as HH:MM:SS.
string Server::getTime()
{
    time_t t = time(nullptr);
    tm local;
    localtime_s(&local, &t);
    ostringstream out;
    out << setfill('0')
        << setw(2) << local.tm_hour << ":"
        << setw(2) << local.tm_min  << ":"
        << setw(2) << local.tm_sec;
    return out.str();
}

// remove leading and trailing whitespace.
string Server::trim(const string& s)
{
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == string::npos)
    {
        return "";
    }
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

// send a string to one socket. returns false if it fails.
bool Server::sendMsg(SOCKET sock, const string& msg)
{
    int sent = send(sock, msg.c_str(), (int)msg.size(), 0);
    if (sent == SOCKET_ERROR)
    {
        return false;
    }
    return true;
}

// send a message to every connected client except one.
void Server::broadcast(const string& msg, SOCKET exclude)
{
    lock_guard<mutex> lock(clientsLock);
    for (auto& pair : clients)
    {
        SOCKET sock = pair.first;
        if (sock != exclude)
        {
            sendMsg(sock, msg);
        }
    }
}

// find a connected user by name. returns INVALID_SOCKET if not found.
// caller must already hold clientsLock.
SOCKET Server::findUser(const string& name)
{
    for (auto& pair : clients)
    {
        if (pair.second == name)
        {
            return pair.first;
        }
    }
    return INVALID_SOCKET;
}

// build the user list string for /list command.
string Server::listUsers()
{
    lock_guard<mutex> lock(clientsLock);
    if (clients.empty())
    {
        return "  (no users connected)\n";
    }
    string out;
    for (auto& pair : clients)
    {
        out += "  - " + pair.second + "\n";
    }
    return out;
}

// per-client thread. handles one connection from start to finish.
void Server::handleClient(SOCKET sock, string addr)
{
    cout << "[" << getTime() << "] New connection from " << addr << endl;

    char buffer[2048];

    // step 1: get a username.
    sendMsg(sock, "Enter your username: ");
    int bytes = recv(sock, buffer, sizeof(buffer) - 1, 0);
    if (bytes <= 0)
    {
        closesocket(sock);
        return;
    }
    buffer[bytes] = '\0';
    string username = trim(buffer);

    if (username.empty() || username.size() > 20)
    {
        sendMsg(sock, "[SERVER] Invalid username. Disconnecting.\n");
        closesocket(sock);
        return;
    }

    // reject duplicate usernames.
    {
        lock_guard<mutex> lock(clientsLock);
        if (findUser(username) != INVALID_SOCKET)
        {
            sendMsg(sock, "[SERVER] Username already taken.\n");
            closesocket(sock);
            return;
        }
        clients[sock] = username;
    }

    // step 2: welcome banner.
    string welcome =
        "\n--------------------------------------------------\n"
        "  Welcome to ChatRoom, " + username + "!\n"
        "  Commands:\n"
        "    /dm <user> <msg>  - private message\n"
        "    /list             - show connected users\n"
        "    /quit             - disconnect\n"
        "--------------------------------------------------\n";
    sendMsg(sock, welcome);
    broadcast("\n[SERVER] " + username + " joined the chat.\n", sock);
    cout << "[" << getTime() << "] " << username << " joined.\n";

    // step 3: message loop. read whatever the client sends and route it.
    while (true)
    {
        bytes = recv(sock, buffer, sizeof(buffer) - 1, 0);
        if (bytes <= 0)
        {
            break;  // client disconnected.
        }
        buffer[bytes] = '\0';

        string msg = trim(buffer);
        if (msg.empty())
        {
            continue;
        }

        // /quit - client wants to disconnect.
        if (msg == "/quit")
        {
            sendMsg(sock, "[SERVER] Goodbye!\n");
            break;
        }
        // /list - send back a list of connected users.
        else if (msg == "/list")
        {
            sendMsg(sock, "\n[Connected users]\n" + listUsers() + "\n");
        }
        // /dm <user> <message> - private message.
        else if (msg.rfind("/dm ", 0) == 0)
        {
            // parse the target username and message body.
            size_t firstSpace = msg.find(' ', 4);
            if (firstSpace == string::npos)
            {
                sendMsg(sock, "[SERVER] Usage: /dm <username> <message>\n");
                continue;
            }
            string target = msg.substr(4, firstSpace - 4);
            string body = msg.substr(firstSpace + 1);

            lock_guard<mutex> lock(clientsLock);
            SOCKET targetSock = findUser(target);
            if (targetSock == INVALID_SOCKET)
            {
                sendMsg(sock, "[SERVER] User '" + target + "' not found.\n");
            }
            else if (targetSock == sock)
            {
                sendMsg(sock, "[SERVER] You can't DM yourself.\n");
            }
            else
            {
                sendMsg(targetSock, "\n[DM from " + username + "]: " + body + "\n\n");
                sendMsg(sock, "[DM to " + target + "]: " + body + "\n");
                cout << "[" << getTime() << "] DM: " << username << " -> " << target << endl;
            }
        }
        // unknown slash command.
        else if (msg[0] == '/')
        {
            sendMsg(sock, "[SERVER] Unknown command. Try /list or /quit.\n");
        }
        // regular broadcast message.
        else
        {
            string out = "[" + getTime() + "] " + username + ": " + msg + "\n";
            broadcast(out, sock);
            cout << "[" << getTime() << "] " << username << ": " << msg << endl;
        }
    }

    // step 4: cleanup. remove from registry and notify everyone.
    {
        lock_guard<mutex> lock(clientsLock);
        clients.erase(sock);
    }
    closesocket(sock);
    broadcast("\n[SERVER] " + username + " left the chat.\n", INVALID_SOCKET);
    cout << "[" << getTime() << "] " << username << " disconnected.\n";
}

// main server loop. binds the port, listens, and spawns a thread per client.
void Server::start()
{
    listenSock = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSock == INVALID_SOCKET)
    {
        cout << "Failed to create socket. \n";
        return;
    }

    // let us reuse the port if the server restarts quickly.
    int opt = 1;
    setsockopt(listenSock, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));

    // bind to the port on all interfaces.
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(listenSock, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
    {
        cout << "Bind failed. Error: " << WSAGetLastError() << endl;
        closesocket(listenSock);
        return;
    }

    if (listen(listenSock, SOMAXCONN) == SOCKET_ERROR)
    {
        cout << "Listen failed. Error: " << WSAGetLastError() << endl;
        closesocket(listenSock);
        return;
    }

    running = true;
    cout << "[" << getTime() << "] Server listening on port " << port << endl;
    cout << "Press Ctrl+C in this window to shut down. \n";

    // accept loop. one thread per incoming client.
    while (running)
    {
        sockaddr_in clientAddr;
        int clientLen = sizeof(clientAddr);
        SOCKET clientSock = accept(listenSock, (sockaddr*)&clientAddr, &clientLen);

        if (clientSock == INVALID_SOCKET)
        {
            cout << "Accept failed. \n";
            continue;
        }

        // format the client's IP and port for logging.
        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &clientAddr.sin_addr, ip, sizeof(ip));
        string addrStr = string(ip) + ":" + to_string(ntohs(clientAddr.sin_port));

        // detach the thread so it cleans itself up when handleClient returns.
        thread t(&Server::handleClient, this, clientSock, addrStr);
        t.detach();
    }
}
