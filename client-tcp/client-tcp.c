#include <stdio.h>
#include <stdlib.h>
#include <winsock.h>

#define SERVER_PORT 53781
#define SERVER_ADDRESS "127.0.0.1"
#define BUFFERSIZE 256

//struct che deve contenere i numeri da inviare al server per poter esguire l'operazione
typedef struct{
	int a;
	int b;
}Msgstruct;

void ClearWinSock(){
#if defined Win32
	WSACleanup();
	system("pause");
#endif
}

int main(void) {
	//Inizializzazione WinSock
	WSADATA wsaData;
	struct sockaddr_in sad;//riferimento socket server
	int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
	if(iResult != 0){
		printf("Error at WSAStartup \n");
		return -1;
	}

	//Creazione socket client
	int C_Socket;
	C_Socket = socket(PF_INET,SOCK_STREAM,IPPROTO_TCP);
	if(C_Socket <0){
		printf("Error at creating Socket\n");
		return -1;
	}

	//Costruzione socket server
	memset(&sad,0,sizeof(sad));
	sad.sin_family = AF_INET;
	sad.sin_addr.S_un.S_addr= inet_addr(SERVER_ADDRESS);
	sad.sin_port = htons(SERVER_PORT);

	//Connessione al Server
	if(connect(C_Socket,(struct sockaddr*) &sad,sizeof(sad))<0){
		printf("Failed to connect\n");
		ClearWinSock();
		return 0;
	}

	//Ricevuta messaggio connessione stabilita
	char buf[BUFFERSIZE];
	int bytesRcvd;
	printf("Received: ");
	if((bytesRcvd = recv(C_Socket,buf,BUFFERSIZE -1,0))<=0){
		printf("recv() failed\n");
		closesocket(C_Socket);
		ClearWinSock();
	}
	buf[bytesRcvd] = '\0';//Fa capire al printf quando fermarsi
	printf("%s\n",buf);

	//Inserimento operazione desiderata
	printf("Indicare Operazione Desiderata : \nA = Addizione \nS = Sottrazione \nM = Moltiplicazione \nD = Divisione \n");
	char operation;
	scanf("%s",&operation);
	int strLen = strlen(&operation);

	//Send messaggio operazione
	if((send(C_Socket,&operation,strLen,0)) != strLen){
		printf("Error! send() sent a different number of bytes than expected");
		closesocket(C_Socket);
		ClearWinSock();
		return -1;
	}

	//Ricevuta messaggio conferma operazione
	if((bytesRcvd = recv(C_Socket,buf,BUFFERSIZE -1,0))<=0){
		printf("recv() failed\n");
		closesocket(C_Socket);
		ClearWinSock();
	}
	buf[bytesRcvd] = '\0';
	printf("\n%s\n",buf);

	//Controllo inserimento sbagliato
	if(strcmp(buf,"TERMINE PROCESSO CLIENT")== 0){
		printf("Hai inserito una lettera errata!Chiusura connessione\n");
		closesocket(C_Socket);
		ClearWinSock();
		system("pause");
		return -1;
	}

	//Inserimento dei 2 numeri e conversione per rete
	Msgstruct msg;
	printf("\nInserire il primo numero : ");
	scanf("%d",&msg.a);
	msg.a = htonl(msg.a);
	printf("\nInserire il secondo numero : ");
	scanf("%d",&msg.b);
	msg.b = htonl(msg.b);

	//Invio numeri al server
	if((send(C_Socket, &msg, sizeof(Msgstruct), 0)) != sizeof(Msgstruct)){
		printf("Error! send() sent a different number of bytes than expected");
		closesocket(C_Socket);
		ClearWinSock();
		return -1;
	}

	//Ricevuta risultato
	if((bytesRcvd = recv(C_Socket,buf,BUFFERSIZE - 1,0))<=0){
		printf("recv() failed\n");
		closesocket(C_Socket);
		ClearWinSock();
	}
	buf[bytesRcvd] = '\0';

	//Output risultato
	printf("\nRisultato = %s \n",buf);

	closesocket(C_Socket);
	ClearWinSock();
	system("pause");
	return 0;
}
