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

struct Client {
	char _name[NAMEMAXLENGHT];
	struct sockaddr_in _sockaddr_in;
public:
	int InRoomNumber = 0;
	Client() {}
	Client(char* name, struct sockaddr_in addres) {
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

class Room {
private:
public:
	Client* _client1;
	Client* _client2;
	int PeopleInRoom = 0;
	int RoomNumber = 0;
	Room() {}
	void Initialize(Client* client1, Client* client2, int iRoomNumber) {
		_client1 = client1;
		_client2 = client2;
		PeopleInRoom = 2;
		RoomNumber = iRoomNumber;
		_client1->InRoomNumber = RoomNumber;
		_client2->InRoomNumber = RoomNumber;
	}
	bool AddClient(Client* client) {
		if (PeopleInRoom > 1)return false;
		if (PeopleInRoom == 0)
			_client1 = client;
		else if (PeopleInRoom == 1)
			_client2 = client;
		PeopleInRoom++;
		return true;
	}
	bool IsRoomFull() {
		if (PeopleInRoom == 2)
			return true;
		return false;
	}
};

//Return true if clients are the same
bool CompareClients(Client* client1, Client* client2) {
	//bool compareNames = 0 == strcmp(client1->_name,client2->_name);
	bool compareIp = client1->_sockaddr_in.sin_addr.s_addr == client2->_sockaddr_in.sin_addr.s_addr;
	bool comparePort = client1->_sockaddr_in.sin_port == client2->_sockaddr_in.sin_port;
	if (compareIp && comparePort)
		return true;
	return false;
}
using namespace std;

Room* GetRoomByNumber(int RoomNumber, vector<Room> _rooms) {
	for (int i = 0; i < _rooms.size(); i++)
	{
		if (_rooms[i].RoomNumber == RoomNumber)
			return &_rooms[i];
	}
	return NULL;
}

int main()
{
	SOCKET s;
	struct sockaddr_in server, si_other;
	int slen, recv_len;
	char buf[BUFLEN];
	WSADATA wsa;
	vector<Client*> _clients;
	vector<Room> _rooms;
	Room mainRoom = Room();
	Client* _actualClient = new Client();
	int RoomCount = 100;

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
					_actualClient = _clients[i];
					//_actualClient->SetName(_clients[i]->_name);
					//_actualClient->InRoomNumber = _clients[i]->InRoomNumber;
					clientAlreadyExist = true;
					printf("Client Already Exist \n");
				}
			}
			//Si es un nuevo player
			if (!clientAlreadyExist) {
				_actualClient->SetName(buf);
				_clients.push_back(new Client(_actualClient->_name, _actualClient->_sockaddr_in));
				printf("New User Connected \n");

				if (mainRoom.PeopleInRoom == 1) {
					_rooms.push_back(Room());
					_rooms.back().Initialize(mainRoom._client1, _actualClient, RoomCount);
					RoomCount++;
					//StartGame(_rooms.back());
					mainRoom = Room();
				}
				else {
					mainRoom.AddClient(_actualClient);
					//SendLobbyInfo(mainRoom);
				}
			}//Si ya está
			else {
				Room* room;
				if (_actualClient->InRoomNumber == 0)
					room = &mainRoom;
				else
					room = GetRoomByNumber(_actualClient->InRoomNumber, _rooms);

				memset(message, '\0', BUFLEN);
				strcat_s(message, _actualClient->_name);
				strcat_s(message, " ");
				strcat_s(message, (BUFLEN - strlen(_actualClient->_name) - 1) * sizeof(char), buf);
				printf(message);

				//Le mando al cliente 1
				if (room->_client1 != NULL) {
					if (sendto(s, message, recv_len + strlen(_actualClient->_name) + 1, 0, (struct sockaddr*) &room->_client1->_sockaddr_in, slen) == SOCKET_ERROR)
					{
						printf("sendto() failed with error code : %d", WSAGetLastError());
						exit(EXIT_FAILURE);
					}
				}
				//Le mando al cliente 2
				if (room->_client2 != NULL) {
					if (sendto(s, message, recv_len + strlen(_actualClient->_name) + 1, 0, (struct sockaddr*) &room->_client2->_sockaddr_in, slen) == SOCKET_ERROR)
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


