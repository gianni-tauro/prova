/*
 * server-tcp.c
 *
 *  Created on: 3 dic 2020
 *      Author: Gianni
 */

//inclusioni per rendere il codice portabile
#if defined WIN32
#include <winsock.h>
#else
#define closesocket close
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define QLEN 1 // numero max di connessioni in entrata
#define SERVER_PORT 53781
#define SERVER_ADDRESS "127.0.0.1"

#define BUFFERSIZE 256

//struct in cui verranno memorizzati i numeri inviati dal client
typedef struct {
	int a;
	int b;
} Msgstruct;

//funzione che stampa i messaggi di errore
void ErrorHandler(char *errorMessage) {
	printf("Errore: %s", errorMessage);
}

void ClearWinSock() {
#if defined WIN32
	WSACleanup();
#endif
}

//Ripulisce le stringhe
void cleanString(char *buffer) {
	memset(buffer, '\0', strlen(buffer));
}

int main() {

#if defined WIN32
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		ErrorHandler("Errore nella funzione WSAStartup()\n");
		return -1;
	}
#endif

	//Creazione socket del server
	int S_Socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (S_Socket < 0) {
		ErrorHandler("Creazione socket fallita.\n");
		ClearWinSock();
		return -1;
		closesocket(S_Socket);
	}

	//Assegnazione indirizzo alla socket
	struct sockaddr_in serverAddress;
	memset(&serverAddress, 0, sizeof(serverAddress)); //si assicura che i Byte extra contengano 0
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = inet_addr(SERVER_ADDRESS);
	serverAddress.sin_port = htons(SERVER_PORT); //se necessario, converte dall'ordine di Byte dell'host a quello della rete(Big Endian)

	//associo alla socket un indirizzo in modo da essere contattata dai client
	int funzioneBind = bind(S_Socket, (struct sockaddr*) &serverAddress, sizeof(serverAddress));
	if (funzioneBind < 0) {
		ErrorHandler("Operazione bind() fallita.\n");
		closesocket(S_Socket);
		ClearWinSock();
		return -1;
	}

	//Settaggio della socket all'ascolto
	if (listen(S_Socket, QLEN) < 0) {
		ErrorHandler("Operazione listen() fallita.\n");
		closesocket(S_Socket);
		ClearWinSock();
		return -1;
	}

	struct sockaddr_in clientAddress; // struttura dell'indirizzo del client
	int clientSocket; // descrittore socket per il client
	int clientLen; // dimensione dell'indirizzo del client

	while (1) { //Quando un client viene chiuso, il server torna disponibile alla connessione con un altro client
		printf("In attesa di una nuova connessione...\n");

		clientLen = sizeof(clientAddress); // setta la dimensione dell'indirizzo del client
		//accetta la connessione del client e indica che la comunicazione tra i due puo avvenire
		clientSocket = accept(S_Socket, (struct sockaddr *) &clientAddress,&clientLen);
		if (clientSocket < 0) {
			ErrorHandler("Operazione accept() fallita.\n");
			closesocket(S_Socket);			// Chiusura della connessione
			ClearWinSock();
			return 0;
		}
		printf("Client %s:%d connesso.\n", inet_ntoa(clientAddress.sin_addr), clientAddress.sin_port);

		//Invio stringa di avvenuta connessione
		char inputString[BUFFERSIZE] = "Connessione avvenuta"; // Stringa da inviare al client
		int byteTrasmessi = send(clientSocket, inputString, strlen(inputString), 0);
		if (byteTrasmessi != strlen(inputString)) {
			ErrorHandler("La funzione send() ha inviato un numero di Byte differente rispetto a quanto previsto.");
			system("pause");
			closesocket(clientSocket);
			ClearWinSock();
			return -1;
		}

		Msgstruct msg;
		char sbuf[BUFFERSIZE];
		int rcvdbytes;
		char operation;
		int risultato;
		char* sendline;
		//riceve la lettera dal client
		if((rcvdbytes = recv(clientSocket,sbuf,BUFFERSIZE-1,0)) <=0){
			printf("Error at recv()\n");
			closesocket(S_Socket);
			ClearWinSock();
			return -1;
		}
		sbuf[rcvdbytes] = '\0';
		operation = sbuf[0];

		if ((strcmp(&operation, "a") == 0) || (strcmp(&operation, "A") == 0)) {
			sendline = "ADDIZIONE";
			printf("\n%s\n",sendline);
			byteTrasmessi = send(clientSocket, sendline, strlen(sendline),0);
			if (byteTrasmessi != strlen(sendline)) {
				ErrorHandler("La funzione send() ha inviato un numero di Byte differente rispetto a quanto previsto.");
				system("pause");
				closesocket(clientSocket);
				ClearWinSock();
				return -1;
			}

			//ricevo due interi dal client
			if((rcvdbytes = recv(clientSocket, &msg, sizeof(Msgstruct), 0)) <=0){
				printf("Error at recv()\n");
				closesocket(S_Socket);
				ClearWinSock();
				return -1;
			}
			//riconversione numeri
			msg.a = ntohl(msg.a);
			msg.b = ntohl(msg.b);
			printf("Primo numero = %d\nSecondo Numero = %d",msg.a,msg.b);
			risultato = msg.a + msg.b;

			sprintf(sbuf, "%d", risultato);

			byteTrasmessi =send(clientSocket, sbuf, strlen(sbuf), 0); // invia il risultato al client
			if (byteTrasmessi != strlen(sbuf)) {
				ErrorHandler("La funzione send() ha inviato un numero di Byte differente rispetto a quanto previsto.");
				system("pause");
				closesocket(clientSocket);
				ClearWinSock();
				return -1;
			}

		}
		else if ((strcmp(&operation, "s") == 0) ||(strcmp(&operation, "S")== 0)) {

			sendline = "SOTTRAZIONE";
			printf("\n%s\n",sendline);
			byteTrasmessi = send(clientSocket, sendline, strlen(sendline), 0);
			if (byteTrasmessi != strlen(sendline)) {
				ErrorHandler("La funzione send() ha inviato un numero di Byte differente rispetto a quanto previsto.");
				system("pause");
				closesocket(clientSocket);
				ClearWinSock();
				return -1;
			}

			//ricevo due interi dal client
			if((rcvdbytes = recv(clientSocket, &msg, sizeof(Msgstruct), 0)) <=0){
				printf("Error at recv()\n");
				closesocket(S_Socket);
				ClearWinSock();
				return -1;
			}
			msg.a = ntohl(msg.a);
			msg.b = ntohl(msg.b);
			printf("Primo numero = %d\nSecondo Numero = %d",msg.a,msg.b);
			risultato = msg.a - msg.b;

			sprintf(sbuf, "%d", risultato);
			byteTrasmessi =send(clientSocket, sbuf, strlen(sbuf), 0); // invia il risultato al client
			if (byteTrasmessi != strlen(sbuf)) {
				ErrorHandler("La funzione send() ha inviato un numero di Byte differente rispetto a quanto previsto.");
				system("pause");
				closesocket(clientSocket);
				ClearWinSock();
				return -1;
			}

		}
		else if ((strcmp(&operation, "m") == 0) ||(strcmp(&operation, "M") == 0)) {

			sendline = "MOLTIPLICAZIONE";
			printf("\n%s\n",sendline);
			byteTrasmessi = send(clientSocket, sendline, strlen(sendline), 0);
			if (byteTrasmessi != strlen(sendline)) {
				ErrorHandler("La funzione send() ha inviato un numero di Byte differente rispetto a quanto previsto.");
				system("pause");
				closesocket(clientSocket);
				ClearWinSock();
				return -1;
			}

			//ricevo due interi dal client
			if((rcvdbytes = recv(clientSocket, &msg, sizeof(Msgstruct), 0)) <=0){
				printf("Error at recv()\n");
				closesocket(S_Socket);
				ClearWinSock();
				return -1;
			}
			msg.a = ntohl(msg.a);
			msg.b = ntohl(msg.b);
			printf("Primo numero = %d\nSecondo Numero = %d",msg.a,msg.b);
			risultato = msg.a * msg.b;

			sprintf(sbuf, "%d", risultato);

			byteTrasmessi= send(clientSocket, sbuf, strlen(sbuf), 0); // invia il risultato al client
			if (byteTrasmessi != strlen(sbuf)) {
				ErrorHandler("La funzione send() ha inviato un numero di Byte differente rispetto a quanto previsto.");
				system("pause");
				closesocket(clientSocket);
				ClearWinSock();
				return -1;
			}

		}
		else if ((strcmp(&operation, "d") == 0) ||(strcmp(&operation, "D") == 0)) {

			sendline = "DIVISIONE";
			printf("\n%s\n",sendline);
			byteTrasmessi = send(clientSocket, sendline, strlen(sendline),0);
			if (byteTrasmessi != strlen(sendline)) {
				ErrorHandler("La funzione send() ha inviato un numero di Byte differente rispetto a quanto previsto.");
				system("pause");
				closesocket(clientSocket);
				ClearWinSock();
				return -1;
			}

			//ricevo due interi dal client
			if((rcvdbytes = recv(clientSocket, &msg, sizeof(Msgstruct), 0)) <=0){
				printf("Error at recv()\n");
				closesocket(S_Socket);
				ClearWinSock();
				return -1;
			}
			msg.a = ntohl(msg.a);
			msg.b = ntohl(msg.b);
			if(msg.b == 0){//Controllo divisione per 0
				sendline = "Divisione per 0 impossibile!\nTerminazione processo.\n";
				byteTrasmessi = send(clientSocket, sendline, strlen(sendline),0);
				if (byteTrasmessi != strlen(sendline)) {
					ErrorHandler("La funzione send() ha inviato un numero di Byte differente rispetto a quanto previsto.");
					system("pause");
					closesocket(clientSocket);
					printf("\nClient disconnesso.\n");
					printf("------------------------------------------------------------------------------------------\n");
				}
				printf("%s\n",sendline);
				cleanString(&operation);
				closesocket(clientSocket);
				printf("\nClient disconnesso.\n\n\n");
				printf("------------------------------------------------------------------------------------------\n");
				continue;
			}
			printf("Primo numero = %d\nSecondo Numero = %d",msg.a,msg.b);
			risultato = msg.a / msg.b;

			sprintf(sbuf, "%d", risultato);

			byteTrasmessi= send(clientSocket, sbuf, strlen(sbuf), 0); // invia il risultato al client
			if (byteTrasmessi != strlen(sbuf)) {
				ErrorHandler("La funzione send() ha inviato un numero di Byte differente rispetto a quanto previsto.");
				system("pause");
				closesocket(clientSocket);
				ClearWinSock();
				return -1;
			}

		} else //inserito un carattere non corrispondente a quelli definiti dalle operazioni possibili
		{

			sendline = "TERMINE PROCESSO CLIENT";
			printf("%s\n\n\n",sendline);
			byteTrasmessi = send(clientSocket, sendline, strlen(sendline),0);
			if (byteTrasmessi != strlen(sendline)) {
				ErrorHandler("La funzione send() ha inviato un numero di Byte differente rispetto a quanto previsto.");
				system("pause");
				closesocket(clientSocket);
				ClearWinSock();
				return -1;
			}
		}
		if(strcmp(sendline,"TERMINE PROCESSO CLIENT") != 0){//Controlla che ci sia un risultato
			printf("\nRisultato = %d \n",risultato);
		}
		cleanString(&operation);
		closesocket(clientSocket);
		printf("\nClient disconnesso.\n");
		printf("------------------------------------------------------------------------------------------\n");

	}
	closesocket(S_Socket);
	ClearWinSock();
	system("pause");
	return 0;
}
