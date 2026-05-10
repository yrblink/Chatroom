// ifeanyi ugwuoke
// Network Project
// ChatRoom
#include "Server.h"
#include "Client.h"
using namespace std;

int main()
{
    int choice = 0;
    do
    {
        cout << "Welcome to ChatRoom! \n";
        cout << "Enter 1 to start a server. \n";
        cout << "Enter 2 to connect as a client. \n";
        cout << "Enter 3 to exit. \n";
        cin >> choice;
        cin.ignore();

        if (choice == 1)
        {
            Server s;
            s.start();
        }
        else if (choice == 2)
        {
            Client c;
            c.run();
        }
        else if (choice == 3)
        {
            cout << "Exiting program... \n";
        }
        else
        {
            cout << "Invalid option. Try again. \n";
        }
    } while (choice != 3);

    return 0;
}
