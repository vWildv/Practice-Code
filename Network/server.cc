#pragma comment(lib,"ws2_32.lib")
#include <winsock2.h>
#include <ws2tcpip.h>
#include <bits/stdc++.h>
using namespace std;


int main(int argc, char* argv[]) {

	WORD sockVersion=MAKEWORD(2, 2);
	WSADATA wsdata;
	if (WSAStartup(sockVersion, &wsdata) != 0){
		return 1;
	}

	SOCKET serverSocket=socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (serverSocket==INVALID_SOCKET){
		printf("Socket error\n");
		return 1;
	}

	sockaddr_in sockAddr;
	sockAddr.sin_family=AF_INET;
	sockAddr.sin_port=htons(12131);
	sockAddr.sin_addr.S_un.S_addr=INADDR_ANY;

	if (bind(serverSocket,(sockaddr*) &sockAddr,sizeof(sockAddr))==SOCKET_ERROR){
		printf("Bind error\n");
		return 1;
	}

	if (listen(serverSocket,10)==SOCKET_ERROR){
		printf("Listen error\n");
		return 1;
	}


	SOCKET clientSocket;
	sockaddr_in client_sin;
	char msg[100];
	int flag=0;
	int len=sizeof(client_sin);
	while (1) {
		if (!flag) printf("waiting for connect\n");
		clientSocket=accept(serverSocket,(sockaddr*) &client_sin,&len);
		if (clientSocket == INVALID_SOCKET){
			printf("Accept error\n");
			flag=0;
			return 1;
		}
		char IPdotdec[20];
		if (!flag) printf("connect build from %s\n",inet_ntoa(client_sin.sin_addr));
		while(1){
            int num = recv(clientSocket,msg,100,0);
            if (num>0){
                msg[num]='\0';
                printf("Client say: %s\n",msg);
            }
            if(strcmp(msg,"exit")==0){
                break;
            }
            printf("Sever say: ");
            string data;
            getline(cin, data);
            const char* sendData;
            sendData=data.c_str();
            send(clientSocket, sendData, strlen(sendData),0);
		}
		flag=0;
		closesocket(clientSocket);
	}

	closesocket(serverSocket);
	WSACleanup();

	return 0;
}
