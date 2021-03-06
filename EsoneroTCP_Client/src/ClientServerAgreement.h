/*
 * ClientServerAgreement.h
 *
 *  Created on: 12 nov 2021
 *      Author: giuse
 */

#ifndef CLIENTSERVERAGREEMENT_H_
#define CLIENTSERVERAGREEMENT_H_

#define SERVERIPADDRESS "127.0.0.1"
#define DEFINEDSERVERPORT 27105
#define QLEN 5
#define MESSAGELEN 150
#define ADDOPERATOR '+'
#define MULTOPERATOR 'x'
#define SUBOPERATOR '-'
#define DIVOPERATOR '/'
#define ENDOPERATOR '='

struct clientRequest{
	char operator;
	int firstOperand;
	int secondOperand;
};

struct serverAnswer{
	int result;
	int success;
	char message[MESSAGELEN];
};

#endif /* CLIENTSERVERAGREEMENT_H_ */
