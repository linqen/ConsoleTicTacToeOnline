// ChatClient.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

/*
Simple udp client
Silver Moon (m00n.silv3r@gmail.com)
*/

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include<stdio.h>
#include<winsock2.h>
#include<conio.h>
#include <iostream>
#include <string>
#pragma comment(lib,"ws2_32.lib") //Winsock Library

#define SERVER "127.0.0.1"  //ip address of udp server
#define PORT 8888   //The port on which to listen for incoming data
#define BUFLEN 512  //Max length of buffer
#define NAMEMAXLEN 10
#define SAYTEXTLEN 6
#define MESSAGELEN BUFLEN+SAYTEXTLEN

using namespace std;

int main(void)
{
	struct sockaddr_in si_other;
	int s, slen = sizeof(si_other);
	char buf[BUFLEN];
	char sendBuf[BUFLEN];
	char clientName[NAMEMAXLEN];
	WSADATA wsa;

	//Initialise winsock
	printf("\nInitialising Winsock...");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("Failed. Error Code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	printf("Initialised.\n");

	//create socket
	if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == SOCKET_ERROR)
	{
		printf("socket() failed with error code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}

	//setup address structure
	memset((char *)&si_other, 0, sizeof(si_other));
	si_other.sin_family = AF_INET;
	si_other.sin_port = htons(PORT);
	si_other.sin_addr.S_un.S_addr = inet_addr(SERVER);


	printf("Enter your name : ");
	gets_s(clientName, NAMEMAXLEN);

	//send the message
	if (sendto(s, clientName, strlen(clientName), 0, (struct sockaddr *) &si_other, slen) == SOCKET_ERROR)
	{
		printf("sendto() failed with error code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}

	//receive a reply and print it
	//clear the buffer by filling null, it might have previously received data
	//memset(buf, '\0', BUFLEN);
	////try to receive some data, this is a blocking call
	//if (recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen) == SOCKET_ERROR)
	//{
	//	printf("recvfrom() failed with error code : %d", WSAGetLastError());
	//	exit(EXIT_FAILURE);
	//}
	//
	//puts(buf);


	fd_set fds;
	timeval interval;
	interval.tv_sec = 0;
	interval.tv_usec = 30000;


	int sendMessageIndex = 0;
	memset(sendBuf, '\0', BUFLEN);
	//start communication
	while (1)
	{

		FD_ZERO(&fds);
		FD_SET(s, &fds);
		int j = select(s, &fds, NULL, NULL, &interval);

		if (j>0) {
			//receive a reply and print it
			//clear the buffer by filling null, it might have previously received data
			memset(buf, '\0', BUFLEN);
			//try to receive some data, this is a blocking call
			if (recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen) == SOCKET_ERROR) {
				printf("recvfrom() failed with error code : %d", WSAGetLastError());
				exit(EXIT_FAILURE);
			}
			int firstSpacePosition = 0;
			for (size_t i = 0; i < BUFLEN; i++)
			{
				//32 is spacebar char
				if (buf[i] == 32) {
					firstSpacePosition = i;
					i = BUFLEN;
				}
			}
			string strMessage;
			if (firstSpacePosition > 0) {
				//USAR ACA EN LUGAR DE BUFLEN +SAYTEXT EL TAMAÑO DE MI BUFFER MESSAGE Y HACERLE A BUF UN SUBSTRING
				//strcat_s(message, firstSpacePosition, buf);
				strMessage.append(buf, firstSpacePosition);
				
			}
			strMessage.append(" Say: ");
			strMessage.append(string(buf).substr(firstSpacePosition,BUFLEN));
			
			puts(strMessage.c_str());
		}
		else if (_kbhit()) {

			int ch = _getch();
			if (ch != 0) {
				//Enter is pressed
				if (ch == 13 && sendMessageIndex>0) {

					if (sendto(s, sendBuf, strlen(sendBuf), 0, (struct sockaddr *) &si_other, slen) == SOCKET_ERROR) {
						printf("sendto() failed with error code : %d", WSAGetLastError());
						exit(EXIT_FAILURE);
					}
					sendMessageIndex = 0;
					memset(sendBuf, '\0', BUFLEN);
					printf("\n");
					//Envío el mensaje
					//Limpio el index
					//Limpio el buffer
				}
				else if(ch!=13) {
					sendBuf[sendMessageIndex] = ch;
					printf((char*)&ch);
					sendMessageIndex++;
				}

				//printf("Enter message : ");
				//gets_s(message);

				//send the message

			}

		}
	}

	closesocket(s);
	WSACleanup();

	return 0;
}