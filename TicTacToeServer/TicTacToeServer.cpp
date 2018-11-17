// ChatServer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

/*
Simple UDP Server
*/

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include<stdio.h>
#include<winsock2.h>
#include <vector>

#pragma comment(lib,"ws2_32.lib") //Winsock Library

#define BUFLEN 512  //Max length of buffer
#define PORT 8888   //The port on which to listen for incoming data
#define NAMEMAXLENGHT 10

struct ChatClient {
	char _name[NAMEMAXLENGHT];
	struct sockaddr_in _sockaddr_in;
public:
	ChatClient() {}
	ChatClient(char* name, struct sockaddr_in addres) {
		for (int i = 0; i < NAMEMAXLENGHT; i++) {
			if (strcmp(&name[i], " ") != 0) {
				_name[i] = name[i];
			}
			else {
				i = NAMEMAXLENGHT;
			}
		}
		_sockaddr_in = addres;
	}
	void SetName(char* name) {
		for (int i = 0; i < NAMEMAXLENGHT; i++) {
			if (strcmp(&name[i], " ") != 0) {
				_name[i] = name[i];
			}
			else {
				i = NAMEMAXLENGHT;
			}
		}
	}
};

//Return true if clients are the same
bool CompareClients(ChatClient* client1, ChatClient* client2) {
	//bool compareNames = 0 == strcmp(client1->_name,client2->_name);
	bool compareIp = client1->_sockaddr_in.sin_addr.s_addr == client2->_sockaddr_in.sin_addr.s_addr;
	bool comparePort = client1->_sockaddr_in.sin_port == client2->_sockaddr_in.sin_port;
	if (compareIp && comparePort)
		return true;
	return false;
}

using namespace std;
int main()
{
	SOCKET s;
	struct sockaddr_in server, si_other;
	int slen, recv_len;
	char buf[BUFLEN];
	WSADATA wsa;
	vector<ChatClient*> _clients;
	ChatClient* _actualClient = new ChatClient();

	slen = sizeof(si_other);

	//Initialise winsock
	printf("\nInitialising Winsock...");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("Failed. Error Code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	printf("Initialised.\n");

	//Create a socket
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
	{
		printf("Could not create socket : %d", WSAGetLastError());
	}
	printf("Socket created.\n");

	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(PORT);

	//Bind
	if (bind(s, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR)
	{
		printf("Bind failed with error code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	puts("Bind done");
	fd_set fds;
	timeval interval;
	interval.tv_sec = 0;
	interval.tv_usec = 300000;
	//keep listening for data
	char message[BUFLEN];
	int text = 0;
	while (1)
	{
		if (text == 0) {
			printf("\rWaiting for data...");
			text = 1;
		}
		else if (text == 1) {
			printf("\r                   ");
			text = 0;
		}
		fflush(stdout);

		FD_ZERO(&fds);
		FD_SET(s, &fds);
		int j = select(s, &fds, NULL, NULL, &interval);

		if (j>0) {
			//clear the buffer by filling null, it might have previously received data
			memset(buf, '\0', BUFLEN);

			//try to receive some data, this is a blocking call
			if ((recv_len = recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen)) == SOCKET_ERROR)
			{
				printf("recvfrom() failed with error code : %d", WSAGetLastError());
				exit(EXIT_FAILURE);
			}
			_actualClient->_sockaddr_in = si_other;
			bool clientAlreadyExist = false;
			for (int i = 0; i < _clients.size(); i++) {
				if (CompareClients(_actualClient, _clients[i])) {
					_actualClient->SetName(_clients[i]->_name);
					clientAlreadyExist = true;
					printf("Client Already Exist \n");
				}
			}

			if (!clientAlreadyExist) {
				_actualClient->SetName(buf);
				_clients.push_back(new ChatClient(_actualClient->_name, _actualClient->_sockaddr_in));
				printf("New User Connected \n");
			}
			
			//print details of the client/peer and the data received
			//printf("Received packet from %s:%d\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
			//printf("%s Say: %s\n",_actualClient->_name,buf);
			//string broadcastMessage;
			memset(message, '\0', BUFLEN);
			strcat_s(message, _actualClient->_name);
			strcat_s(message, " ");
			strcat_s(message, (BUFLEN-strlen(_actualClient->_name)-1) * sizeof(char), buf);

				//= ("%s Say: %s\n", _actualClient->_name, buf);
			printf(message);
			//printf("Data: %s\n" , buf);

			//now reply the client with the same data
			for (int k = 0; k < _clients.size(); ++k) {
				//Compare Clients to dont sent the message to the sender
				if (!CompareClients(_actualClient, _clients[k])) {
					if (sendto(s, message, recv_len+strlen(_actualClient->_name)+1, 0, (struct sockaddr*) &_clients[k]->_sockaddr_in, slen) == SOCKET_ERROR)
					{
						printf("sendto() failed with error code : %d", WSAGetLastError());
						exit(EXIT_FAILURE);
					}
				}
			}
		}
		else {

		}
	}

	closesocket(s);
	WSACleanup();

	return 0;
}


