#include <iostream>
#include <ws2tcpip.h>

#pragma comment (lib, "ws2_32.lib") 
using namespace std;



void main()
{
	//initialise winsoc
	WSADATA wsData;
	WORD ver = MAKEWORD(2, 2);

	int wsOk = WSAStartup(ver, &wsData);
	if (wsOk != 0)
	{
		cerr << "cant initialise winsoc! Quitting." << endl;
		return;
	}
	//create a socket
	SOCKET listening = socket(AF_INET, SOCK_STREAM, 0);
	if (listening == INVALID_SOCKET)
	{
		cerr << "cant create socket! Quitting." << endl;
		return;
	}

	//bind socket to an IP address and port
	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(54000);
	hint.sin_addr.S_un.S_addr = INADDR_ANY;

	bind(listening, (sockaddr*)&hint, sizeof(hint));

	// Tell Winsoc the socket is for listening 
	listen(listening, SOMAXCONN);

	fd_set master;
    FD_ZERO(&master);

	FD_SET(listening, &master);

	while (true)
	{
		fd_set copy = master;

		int socketCount = select(0,&copy ,nullptr,nullptr,nullptr);

		for (int i = 0; i < socketCount; i++)
		{
			SOCKET sock = copy.fd_array[i];
			if (sock == listening)
			{
				//accept new connection
				SOCKET client = accept(listening,nullptr, nullptr);
				//add new connection to list of connected clients
				FD_SET(client, &master);
				//send welcome message
				string welcomeMsg = "welcome to the chat server";
				send(client, welcomeMsg.c_str(), welcomeMsg.size() + 1, 0 );
				//TODO: Broad cast we have a new connection
			}
			else 
			{
				char buf[4096];
				ZeroMemory(buf, 4096);
				//accept a new message
				int bytesIn = recv(sock, buf ,4096 , 0);
				cout << string(buf, 0, bytesIn) << endl;
				if (bytesIn <= 0)
				{
                    //drop client from list
					closesocket(sock);
					FD_CLR(sock, &master);

				}
				else
				{
					//send message to other clients and NOT the listening socket

					for (int i = 0; i < master.fd_count; i++)
					{
						SOCKET outSock = master.fd_array[i];
						if(outSock != listening && outSock != sock)
						{
							send(outSock, buf, bytesIn, 0);
						}

					}
				}
				
			}
		}
	}

	//close socket
	//closesocket(clientSocket);

	//Shutdown winsoc
	WSACleanup();
}


