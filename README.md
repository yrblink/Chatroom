\# CS402 ChatRoom



A multi-threaded TCP chat application built in C++ using Winsock.



\## Features

\- Multi-client chat over TCP sockets

\- Broadcast messages to all connected users

\- Private DMs with `/dm <user> <message>`

\- User listing with `/list`

\- Clean disconnect with `/quit`



\## Build (Windows, MSVC)



&#x20;   cl /EHsc /std:c++17 main.cpp Server.cpp Client.cpp /Fe:chatroom.exe



\## Run



Open three terminals. In each:



&#x20;   .\\chatroom.exe



In one, choose option 1 (server). In the others, option 2 (client).





Ifeanyi Ugwuoke

