#pragma comment(lib,"ws2_32.lib")
#include <winsock2.h>
#include <ws2tcpip.h>
#include <bits/stdc++.h>
using namespace std;

int main(int argc, char* argv[]){

	WORD sockVersion=MAKEWORD(2, 2);
	WSADATA wsdata;
	if (WSAStartup(sockVersion, &wsdata) != 0){
		return 1;
	}

	while (1) {
		SOCKET clientSocket=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
		if (clientSocket==INVALID_SOCKET){
			printf("Socket error\n");
			return 1;
		}
		sockaddr_in sock_in;
		sock_in.sin_family=AF_INET;
		sock_in.sin_port=htons(12131);
		sock_in.sin_addr.S_un.S_addr=inet_addr("127.0.0.1");
		if (connect(clientSocket,(sockaddr*) &sock_in,sizeof(sock_in))==SOCKET_ERROR){
			printf("Connect error\n");
			return 1;
		}
		while(1){
            printf("Client say: ");
            string data;
            getline(cin, data);
            const char* msg;
            msg=data.c_str();
            send(clientSocket,msg,strlen(msg),0);

            char revdata[100];
            int num=recv(clientSocket,revdata,100,0);
            if (num>0) {
                revdata[num]='\0';
                printf("Sever say: %s\n",revdata);
            }
		}

		closesocket(clientSocket);

	}

	WSACleanup();

	return 0;
}
