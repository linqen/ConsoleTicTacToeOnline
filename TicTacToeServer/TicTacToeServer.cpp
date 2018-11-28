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
	char Mark = 'X';
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
	void StartBoard() {
		square[1] = '1';
		square[2] = '2';
		square[3] = '3';
		square[4] = '4';
		square[5] = '5';
		square[6] = '6';
		square[7] = '7';
		square[8] = '8';
		square[9] = '9';

		int randomPos = rand() % 8 + 1;
		square[randomPos] = _client1->Mark;
		nextClientToPlay = _client2;
	}
public:
	char square[10] = { 'o','1','2','3','4','5','6','7','8','9' };
	Client* _client1=NULL;
	Client* _client2=NULL;
	int PeopleInRoom = 0;
	int RoomNumber = 0;
	Client* nextClientToPlay;
	Room() {}
	void Initialize(Client* client1, Client* client2, int iRoomNumber) {
		_client1 = client1;
		_client2 = client2;
		PeopleInRoom = 2;
		RoomNumber = iRoomNumber;
		_client1->InRoomNumber = RoomNumber;
		_client2->InRoomNumber = RoomNumber;
		_client1->Mark = 'X';
		_client2->Mark = 'O';
		StartBoard();
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
	bool DoBoardMove(int choice) {
		if (choice == 1 && square[1] == '1')
			square[1] = nextClientToPlay->Mark;
		else if (choice == 2 && square[2] == '2')
			square[2] = nextClientToPlay->Mark;
		else if (choice == 3 && square[3] == '3')
			square[3] = nextClientToPlay->Mark;
		else if (choice == 4 && square[4] == '4')
			square[4] = nextClientToPlay->Mark;
		else if (choice == 5 && square[5] == '5')
			square[5] = nextClientToPlay->Mark;
		else if (choice == 6 && square[6] == '6')
			square[6] = nextClientToPlay->Mark;
		else if (choice == 7 && square[7] == '7')
			square[7] = nextClientToPlay->Mark;
		else if (choice == 8 && square[8] == '8')
			square[8] = nextClientToPlay->Mark;
		else if (choice == 9 && square[9] == '9')
			square[9] = nextClientToPlay->Mark;
		else
		{
			return false;
		}
		nextClientToPlay = nextClientToPlay == _client1 ? _client2 : _client1;
		return true;
	}
	
};


/*********************************************

	FUNCTION TO RETURN GAME STATUS
	1 FOR GAME IS OVER WITH RESULT
	-1 FOR GAME IS IN PROGRESS
	O GAME IS OVER AND NO RESULT
**********************************************/

int checkwin(char square[10])
{
	if (square[1] == square[2] && square[2] == square[3])

		return 1;
	else if (square[4] == square[5] && square[5] == square[6])

		return 1;
	else if (square[7] == square[8] && square[8] == square[9])

		return 1;
	else if (square[1] == square[4] && square[4] == square[7])

		return 1;
	else if (square[2] == square[5] && square[5] == square[8])

		return 1;
	else if (square[3] == square[6] && square[6] == square[9])

		return 1;
	else if (square[1] == square[5] && square[5] == square[9])

		return 1;
	else if (square[3] == square[5] && square[5] == square[7])

		return 1;
	else if (square[1] != '1' && square[2] != '2' && square[3] != '3'
		&& square[4] != '4' && square[5] != '5' && square[6] != '6'
		&& square[7] != '7' && square[8] != '8' && square[9] != '9')

		return 0;
	else
		return -1;
}


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

Room* GetRoomByNumber(int RoomNumber, vector<Room>* _rooms) {
	for (int i = 0; i < _rooms->size(); i++)
	{
		if (_rooms->at(i).RoomNumber == RoomNumber)
			return &_rooms->at(i);
	}
	return NULL;
}
void appendChar(char* s, char c) {
	int len = strlen(s);
	s[len] = c;
	s[len + 1] = '\0';
}



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
fd_set fds;
timeval interval;
char message[BUFLEN];
int text = 0;

void InitialiseWinSock();
void CreateSocket();
void SetSockAddr();
void Bind();
void ShowWaitingForDataText();

int main()
{
	slen = sizeof(si_other);

	//Initialise winsock
	InitialiseWinSock();
	//Create a socket
	CreateSocket();
	//Prepare the sockaddr_in structure
	SetSockAddr();
	//Bind
	Bind();

	interval.tv_sec = 0;
	interval.tv_usec = 300000;

	//keep listening for data
	while (1)
	{
		ShowWaitingForDataText();

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
					//_actualClient = _clients[i];
					_actualClient->SetName(_clients[i]->_name);
					_actualClient->InRoomNumber = _clients[i]->InRoomNumber;
					clientAlreadyExist = true;
					printf("Client Already Exist \n");
				}
			}
			//Si es un nuevo player
			if (!clientAlreadyExist) {
				_actualClient->SetName(buf);
				Client* newClient = new Client(_actualClient->_name, _actualClient->_sockaddr_in);
				_clients.push_back(newClient);
				printf("New User Connected \n");

				if (mainRoom.PeopleInRoom == 1) {
					_rooms.push_back(Room());
					_rooms.back().Initialize(mainRoom._client1, newClient, RoomCount);
					RoomCount++;

					//Envío el tablero con el estado inicial y los nombres de contra quien juegan

					mainRoom = Room();
				}
				else {
					mainRoom.AddClient(newClient);
					//SendLobbyInfo(mainRoom);
				}
			}//Si ya está
			else if(_actualClient->InRoomNumber != 0) {
				Room* room;
				room = GetRoomByNumber(_actualClient->InRoomNumber, &_rooms);
				//Chequeo buf
				memset(message, '\0', BUFLEN);

				//Si arranca con # && es el turno de ese player
				if (recv_len > 1 && buf[0] == '#' && CompareClients(_actualClient,room->nextClientToPlay)) {
					//Envío acción al tablero
					//room.board.DoMove(buf[1]);
					int choice = buf[1] - '0';
					if (room->DoBoardMove(choice)) {
						//Chequeo victoria
						int matchStatus = checkwin(room->square);
						//int win = room.board.checkwin();
						//Le envío a los 2 jugadores el estado resultante del tablero

						strcat_s(message, "\n\n\tTic Tac Toe\n\n");
						strcat_s(message, room->_client1->_name);
						strcat_s(message, " (X)  -  ");
						strcat_s(message, room->_client2->_name);
						strcat_s(message, " (O) \n\n");
						strcat_s(message, "     |     |     \n");
						strcat_s(message, "  "); appendChar(message, room->square[1]); strcat_s(message, "  |  "); appendChar(message, room->square[2]); strcat_s(message, "  |  "); appendChar(message, room->square[3]); strcat_s(message, "\n");
						strcat_s(message, "_____|_____|_____\n");
						strcat_s(message, "     |     |     \n");
						strcat_s(message, "  "); appendChar(message, room->square[4]); strcat_s(message, "  |  "); appendChar(message, room->square[5]); strcat_s(message, "  |  "); appendChar(message, room->square[6]); strcat_s(message, "\n");
						strcat_s(message, "_____|_____|_____\n");
						strcat_s(message, "     |     |     \n");
						strcat_s(message, "  "); appendChar(message, room->square[7]); strcat_s(message, "  |  "); appendChar(message, room->square[8]); strcat_s(message, "  |  "); appendChar(message, room->square[9]); strcat_s(message, "\n");
						strcat_s(message, "     |     |     \n");

						//match in progress
						if (matchStatus == -1) {
							strcat_s(message, "Now is turn of ");
							strcat_s(message, room->nextClientToPlay->_name);
						}
						else if (matchStatus == 0) {
							//if game get stuck
						}
						else if (matchStatus == 1) {
							//Someone win
						}


						//Le mando al cliente 1
						if (sendto(s, message, strlen(message), 0, (struct sockaddr*) &room->_client1->_sockaddr_in, slen) == SOCKET_ERROR)
						{
							printf("sendto() failed with error code : %d", WSAGetLastError());
							exit(EXIT_FAILURE);
						}
						//Le mando al cliente 2
						if (sendto(s, message, strlen(message), 0, (struct sockaddr*) &room->_client2->_sockaddr_in, slen) == SOCKET_ERROR)
						{
							printf("sendto() failed with error code : %d", WSAGetLastError());
							exit(EXIT_FAILURE);
						}


						//Si se termino indico ganador y perdedor
						//Ofrezco comenzar denuevo o salir

					}
					else {
						//Invalid move
						//Notify client

						strcat_s(message, "Invalid Move, retry");
						if (sendto(s, message, strlen(message), 0, (struct sockaddr*) &room->nextClientToPlay->_sockaddr_in, slen) == SOCKET_ERROR)
						{
							printf("sendto() failed with error code : %d", WSAGetLastError());
							exit(EXIT_FAILURE);
						}
					}

				}
				//Si no arranca con # o si no es el turno de ese player
				else {
					//Envío el mensaje a la sala
					strcat_s(message, _actualClient->_name);
					strcat_s(message, " ");
					const int whiteSpaceLengt = 1;
					strcat_s(message, (BUFLEN - strlen(_actualClient->_name) - whiteSpaceLengt), buf);
					printf(message);

					//Le mando al cliente 1
					if (sendto(s, message, recv_len + strlen(_actualClient->_name) + 1, 0, (struct sockaddr*) &room->_client1->_sockaddr_in, slen) == SOCKET_ERROR)
					{
						printf("sendto() failed with error code : %d", WSAGetLastError());
						exit(EXIT_FAILURE);
					}
					//Le mando al cliente 2
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

void CreateSocket() {
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
	{
		printf("Could not create socket : %d", WSAGetLastError());
	}
	printf("Socket created.\n");
}

void InitialiseWinSock() {
	printf("\nInitialising Winsock...");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("Failed. Error Code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	printf("Initialised.\n");
}
void SetSockAddr() {
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(PORT);
}
void Bind() {
	if (bind(s, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR)
	{
		printf("Bind failed with error code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	puts("Bind done");
}
void ShowWaitingForDataText() {
	if (text == 0) {
		printf("\rWaiting for data...");
		text = 1;
	}
	else if (text == 1) {
		printf("\r                   ");
		text = 0;
	}
	fflush(stdout);
}