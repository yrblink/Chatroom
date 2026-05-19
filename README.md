# Ifeanyi Ugwuoke
# CS402 ChatRoom

A multi-threaded TCP chat application built in C++ using Winsock.

## Features
- Multi-client chat over TCP sockets
- Broadcast messages to all connected users
- Private DMs 
- User listing 
- Clean disconnect 

## Quick Start (Windows, pre-built binary)

1. Open three terminal tabs in your project folder
2. In each tab, run:

       .\Chatroom\chatroom.exe
   (The compiled exe file might be included in the folder (chatroom.exe), if so just click on it)

4. In the first tab, press `1` to start the server
5. In the others, press `2` to connect as a client

> Note: Windows Defender may flag the unsigned `.exe`. Click "More info" - "Run anyway."

## Build from Source (Windows)

Requires Visual Studio Build Tools with the "Desktop development with C++" workload.
From a Developer Command Prompt:

    cl /EHsc /std:c++17 main.cpp Server.cpp Client.cpp /Fe:chatroom.exe

## Architecture
- **TCP** chosen for guaranteed delivery and ordered messages, critical for chat
- One thread per connected client on the server
- Mutex-protected client registry for safe concurrent access
- Atomic flag for clean shutdown of the receive thread on the client

## Commands
| Command | Description |
|---------|-------------|
| (any text) | Broadcast to all users |
| `/dm <user> <msg>` | Private message to a user |
| `/list` | Show connected users |
| `/quit` | Disconnect from server |

