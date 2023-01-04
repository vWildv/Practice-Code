#pragma comment(lib,"ws2_32.lib")
#include <winsock2.h>
#include <bits/stdc++.h>
using namespace std;

int main(int argc, char* argv[]) {

	WORD sockVersion=MAKEWORD(2, 2);
	WSADATA wsdata;
	if (WSAStartup(sockVersion, &wsdata) != 0){
		return 1;
	}

	SOCKET serverSocket=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
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

	SOCKET clientSocket;
	sockaddr_in client_sin;
	char msg[100];
	int flag=0;
	int len=sizeof(client_sin);
	while (1){
        int ff=recvfrom(serverSocket,msg,100,0,(sockaddr*) &client_sin,&len);
        if(ff<0){
            printf("Recv error\n");
        }
        msg[ff]='\0';
        printf("Client say: %s\n",msg);
        printf("Sever say: ");
        string data;
        getline(cin, data);
        const char* sendData;
        sendData=data.c_str();
        int llen=data.length();
        sendto(serverSocket,sendData,llen,0,(sockaddr*) &client_sin,len);
	}

	closesocket(serverSocket);
	WSACleanup();

	return 0;
}

