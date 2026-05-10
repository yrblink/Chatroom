#include "Client.h"
using namespace std;

// constructor.
Client::Client()
{
    host = "127.0.0.1";
    port = 5555;
    sock = INVALID_SOCKET;
    stopFlag = false;

    // initialize Winsock.
    WSADATA wsa;
    int result = WSAStartup(MAKEWORD(2, 2), &wsa);
    if (result != 0)
    {
        cout << "WSAStartup failed: " << result << endl;
    }
}

// destructor.
Client::~Client()
{
    if (sock != INVALID_SOCKET)
    {
        closesocket(sock);
    }
    WSACleanup();
}

// runs in its own thread. just reads the socket and prints to the screen.
// this lets the main thread block on cin without missing incoming messages.
void Client::receiveLoop()
{
    char buffer[2048];
    while (!stopFlag)
    {
        int bytes = recv(sock, buffer, sizeof(buffer) - 1, 0);
        if (bytes <= 0)
        {
            if (!stopFlag)
            {
                cout << "\n[CLIENT] Lost connection to server.\n";
            }
            break;
        }
        buffer[bytes] = '\0';
        cout << buffer << flush;
    }
    stopFlag = true;
}

// main client routine. connects, spawns the receive thread, and reads input.
void Client::run()
{
    // ask which server to connect to.
    cout << "Enter server IP (press enter for localhost): ";
    string input;
    getline(cin, input);
    if (!input.empty())
    {
        host = input;
    }

    // create the socket.
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET)
    {
        cout << "Failed to create socket. \n";
        return;
    }

    // build the server address struct.
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    if (inet_pton(AF_INET, host.c_str(), &serverAddr.sin_addr) <= 0)
    {
        cout << "Invalid server address: " << host << endl;
        closesocket(sock);
        return;
    }

    // try to connect.
    if (connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        cout << "Could not connect to " << host << ":" << port
             << ". Is the server running? \n";
        closesocket(sock);
        return;
    }

    cout << "Connected to " << host << ":" << port << "\n";

    // start the receive thread.
    thread receiver(&Client::receiveLoop, this);

    // main thread reads input and forwards it to the server.
    string line;
    while (!stopFlag && getline(cin, line))
    {
        int sent = send(sock, line.c_str(), (int)line.size(), 0);
        if (sent == SOCKET_ERROR)
        {
            cout << "Failed to send. \n";
            break;
        }
        if (line == "/quit")
        {
            // give the server a moment to send its goodbye.
            this_thread::sleep_for(chrono::milliseconds(500));
            break;
        }
    }

    // shut down the socket so the receive thread's recv() unblocks.
    stopFlag = true;
    shutdown(sock, SD_BOTH);
    closesocket(sock);
    sock = INVALID_SOCKET;

    if (receiver.joinable())
    {
        receiver.join();
    }

    cout << "Disconnected. \n";
}
