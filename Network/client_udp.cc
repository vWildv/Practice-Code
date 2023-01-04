#pragma comment(lib,"ws2_32.lib")
#include <winsock2.h>
#include <bits/stdc++.h>
using namespace std;

int main(int argc, char* argv[]){

	WORD sockVersion=MAKEWORD(2, 2);
	WSADATA wsdata;
	if (WSAStartup(sockVersion, &wsdata) != 0){
		return 1;
	}

	SOCKET clientSocket=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (clientSocket==INVALID_SOCKET){
		printf("Socket error\n");
		return 1;
	}

	sockaddr_in sockAddr;
	sockAddr.sin_family=AF_INET;
	sockAddr.sin_port=htons(12131);
	sockAddr.sin_addr.S_un.S_addr=inet_addr("127.0.0.1");

	SOCKET serverSocket;
	sockaddr_in server_sin;
	char msg[100];
	int len=sizeof(sockAddr);
	while(1){
        printf("Client say: ");
        string data;
        getline(cin, data);
        const char* sendData;
        sendData=data.c_str();
        sendto(clientSocket,sendData,data.length(),0,(sockaddr*) &sockAddr,len);
        int ff=recvfrom(clientSocket,msg,100,0,(sockaddr*) &sockAddr,&len);
        if(ff<0){
            printf("Recv error\n");
        }
        msg[ff]='\0';
        printf("Server say: %s\n",msg);
	}

	closesocket(serverSocket);
	WSACleanup();

	return 0;
}

