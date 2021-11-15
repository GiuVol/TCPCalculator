/*
 ============================================================================
 Name        : EsoneroTCP_Client.c
 Author      : GiuseppeVolpe
 Version     :
 Copyright   : Your copyright notice
 Description : Calculator Client
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
#include "ClientServerAgreement.h"

#define MAXCOMMANDSIZE 150
#define ENDEDWITHERROR -1

void clearwinsock();
int startedCorrectly();
int main(int argc, char *argv[]);
void handleServerConnection(int clientSocket, int* exit);
int composeRequest(struct clientRequest* request);

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

	//Defining Server Address & Port

	char serverAddress[40];	//Larger for Safety
	strcpy(serverAddress, SERVERIPADDRESS);
	int serverPort = DEFINEDSERVERPORT;

	if(argc > 2){
		strcpy(serverAddress, argv[1]);
		serverPort = atoi(argv[2]);
	}

	//

	int clientSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	if(clientSocket < 0){
		printf("Socket Creation Failed.\n");
		closesocket(clientSocket);
		clearwinsock();
		return ENDEDWITHERROR;
	}

	struct sockaddr_in destinationSocketAddress;
	memset(&destinationSocketAddress, 0, sizeof(destinationSocketAddress));

	destinationSocketAddress.sin_family = AF_INET;
	destinationSocketAddress.sin_addr.s_addr = inet_addr(serverAddress);
	destinationSocketAddress.sin_port = htons(serverPort);

	int connectionResult = connect(clientSocket, (struct sockaddr*) &destinationSocketAddress, sizeof(destinationSocketAddress));

	if(connectionResult < 0){
		printf("Connection Failed.\n");
		closesocket(clientSocket);
		clearwinsock();
		return ENDEDWITHERROR;
	}

	int exit = 0;

	do{
		handleServerConnection(clientSocket, &exit);
	} while(!exit);

	closesocket(clientSocket);
	clearwinsock();
	printf("Goodbye!\n");
	system("pause");

	return 0;
}

void handleServerConnection(int clientSocket, int* exit){

	struct clientRequest request;
	memset(&request, 0, sizeof(request));

	int correctInsertion = 0;

	do{
		correctInsertion = composeRequest(&request);
	} while(!correctInsertion);

	int sentBytes = send(clientSocket, (char*) &request, sizeof(request), 0);

	if(sentBytes != sizeof(request)){
		printf("Sending Failed.\n");
		return;
	}

	if(request.operator == ENDOPERATOR){
		*exit = 1;
		return;
	}

	struct serverAnswer result;
	memset(&result, 0, sizeof(result));

	int recievedBytes = recv(clientSocket, (char*) &result, sizeof(result), 0);

	if(recievedBytes != sizeof(result)){
		printf("Recieving Failed.\n");
		return;
	}

	result.success = ntohl(result.success);
	result.result = ntohl(result.result);

	if(result.success == 0){
		printf("Operation failed...\n");
		printf("%s\n", result.message);
	} else {
		printf("Operation succeeded!\n");
		printf("The result is: %d\n", result.result);

		if(strcmp(result.message, "") != 0){
			printf("%s\n", result.message);
		}
	}
}

int composeRequest(struct clientRequest* request){

	char command[MAXCOMMANDSIZE];

	printf("Enter the operation you want to execute using prefix notation: ");
	fgets(command, MAXCOMMANDSIZE - 1, stdin);
	fflush(stdin);

	if(command[0] == ENDOPERATOR){
		request->operator = ENDOPERATOR;
		request->firstOperand = htonl(0);
		request->secondOperand = htonl(0);
		return 1;
	}

	char operator;
	int firstOperand, secondOperand;

	int valuesEntered = sscanf(command, "%c %d %d", &operator, &firstOperand, &secondOperand);
	fflush(stdin);

	int error = 0;

	if(valuesEntered != 3){
		printf("Wrong Insertion Format.\n");
		error = 1;
	}

	switch(operator){
		case ADDOPERATOR:
		case MULTOPERATOR:
		case SUBOPERATOR:
		case DIVOPERATOR:
			break;
		default:
			printf("Wrong Operator Inserted.\n");
			error = 1;
			break;
	}

	if(error){
		printf("Retry.\n");
		return 0;
	}

	request->operator = operator;
	request->firstOperand = htonl(firstOperand);
	request->secondOperand = htonl(secondOperand);

	return 1;
}
