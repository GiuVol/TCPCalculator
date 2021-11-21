/*
 ============================================================================
 Name        : EsoneroTCP_Server.c
 Author      : GiuseppeVolpe
 Version     :
 Copyright   : Your copyright notice
 Description : Calculator Server
 ============================================================================
 */

#if defined WIN32
#include <winsock.h>
#else
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#define closesocket close
#endif

#include <stdio.h>
#include <math.h>
#include "ClientServerAgreement.h"

#define ENDEDWITHERROR -1

void clearwinsock();
int startedCorrectly();
int main(int argc, char *argv[]);
void handleclientconnection(int clientSocket, int* exit);
void executeOperation(char operator, int firstOperand, int secondOperand, struct serverAnswer* result);
void add(int firstOperand, int secondOperand, struct serverAnswer* result);
void mult(int firstOperand, int secondOperand, struct serverAnswer* result);
void sub(int firstOperand, int secondOperand, struct serverAnswer* result);
void division(int firstOperand, int secondOperand, struct serverAnswer* result);

void clearwinsock() {
#if defined WIN32
	WSACleanup();
#endif
}

int startedCorrectly(){
	//...
#if defined WIN32
	// Initialize Winsock
	WSADATA wsa_data;
	int result = WSAStartup(MAKEWORD(2,2), &wsa_data);
	if (result != 0) {
		return 0;
	}
#endif

	return 1;
}

int main(int argc, char *argv[]) {

	setvbuf(stdout, NULL, _IONBF, 0);

	if(startedCorrectly() == 0){
		printf("Startup Failed.\n");
		return ENDEDWITHERROR;
	}

	//Defining Server Port

	int definedPort = DEFINEDSERVERPORT;

	if(argc > 1){
		definedPort = atoi(argv[1]);
	}

	//

	int welcomeSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	if(welcomeSocket < 0){
		printf("Socket Creation Failed.\n");
		closesocket(welcomeSocket);
		clearwinsock();
		return ENDEDWITHERROR;
	}

	struct sockaddr_in welcomeSocketAddress;
	memset(&welcomeSocketAddress, 0, sizeof(welcomeSocketAddress));

	welcomeSocketAddress.sin_family = AF_INET;
	welcomeSocketAddress.sin_addr.s_addr = inet_addr(SERVERIPADDRESS);
	welcomeSocketAddress.sin_port = htons(definedPort);

	if(bind(welcomeSocket, (struct sockaddr*) &welcomeSocketAddress, sizeof(welcomeSocketAddress)) < 0){
		printf("Binding Failed.\n");
		closesocket(welcomeSocket);
		clearwinsock();
		return ENDEDWITHERROR;
	}

	if(listen(welcomeSocket, QLEN) < 0){
		printf("Listening Failed.\n");
		closesocket(welcomeSocket);
		clearwinsock();
		return ENDEDWITHERROR;
	}

	struct sockaddr_in clientSocketAddress;
	int clientSocket, clientAddressLength;	//This is the socket that will comunicate with the client

	while (1) {
		printf("Waiting For Client Connection...\n");

		memset(&clientSocketAddress, 0, sizeof(clientSocketAddress));

		clientAddressLength = sizeof(clientSocketAddress);

		clientSocket = accept(welcomeSocket, (struct sockaddr *) &clientSocketAddress, &clientAddressLength);

		if (clientSocket < 0){
			printf("Accept Failed.\n");
			closesocket(clientSocket);
			continue;
		}

		printf("Connection established with %s:%d\n", inet_ntoa(clientSocketAddress.sin_addr) , ntohs(clientSocketAddress.sin_port));

		int exit = 0;

		do{
			handleclientconnection(clientSocket, &exit);
		} while(!exit);

		printf("Connection with %s:%d ended.\n", inet_ntoa(clientSocketAddress.sin_addr) , ntohs(clientSocketAddress.sin_port));
	}

	return 0;
}

void handleclientconnection(int clientSocket, int* exit){
	struct clientRequest request;
	memset(&request, 0, sizeof(request));

	int recievedBytes = recv(clientSocket, (char*) &request, sizeof(request), 0);

	if(recievedBytes != sizeof(request)){
		printf("Recieving Failed.\n");
		closesocket(clientSocket);
		*exit = 1;
		return;
	}

	if(request.operator == ENDOPERATOR){
		closesocket(clientSocket);
		*exit = 1;
		return;
	}

	struct serverAnswer result;
	memset(&result, 0, sizeof(result));

	char operator = request.operator;
	int firstOperand = ntohl(request.firstOperand);
	int secondOperand = ntohl(request.secondOperand);

	executeOperation(operator, firstOperand, secondOperand, &result);

	int sentBytes = send(clientSocket, (char*) &result, sizeof(result), 0);

	if(sentBytes != sizeof(result)){
		printf("Sending Failed.\n");
		return;
	}
}

void executeOperation(char operator, int firstOperand, int secondOperand, struct serverAnswer* result){

	result->success = htonl(1);
	strcpy(result->message, "");

	switch(operator){
		case ADDOPERATOR:
			add(firstOperand, secondOperand, result);
			break;
		case MULTOPERATOR:
			mult(firstOperand, secondOperand, result);
			break;
		case SUBOPERATOR:
			sub(firstOperand, secondOperand, result);
			break;
		case DIVOPERATOR:
			division(firstOperand, secondOperand, result);
			break;
		default:
			result->success = htonl(0);
			strcpy(result->message, "Wrong operator inserted.");
			break;
	}
}

void add(int firstOperand, int secondOperand, struct serverAnswer* result){
	result->result = htonl(firstOperand + secondOperand);
	result->success = htonl(1);
}

void mult(int firstOperand, int secondOperand, struct serverAnswer* result){
	result->result = htonl(firstOperand * secondOperand);
	result->success = htonl(1);
}

void sub(int firstOperand, int secondOperand, struct serverAnswer* result){
	result->result = htonl(firstOperand - secondOperand);
	result->success = htonl(1);
}

void division(int firstOperand, int secondOperand, struct serverAnswer* result){
	if(secondOperand == 0){
		result->success = htonl(0);
		strcpy(result->message, "Cannot divide by 0.");
	} else {
		float decimalValue = firstOperand / secondOperand;
		int integralValue = (int) decimalValue;

		result->result = htonl(integralValue);
		result->success = htonl(1);
		strcpy(result->message, "This result might be truncated, because this calculator only calculates the integral part.");
	}
}
