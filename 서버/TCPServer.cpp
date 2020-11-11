#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib,"ws2_32.lib")
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Windows.h>
#include <process.h>

#define BUF_SIZE 100
#define MAX_CLNT 256

DWORD WINAPI ClientRecv(LPVOID arg);//������ �Լ�
void SendMsg(char* msg, int len);//�޽��� ������ �Լ�
void ErrorHandling(char* msg);

int clientCount = 0;
SOCKET clientSocks[MAX_CLNT];//Ŭ���̾�Ʈ ���� ������ �迭
CRITICAL_SECTION cs;


int main(int argc, char* argv[]) {
    WSADATA wsaData;
    SOCKET serverSock, clientSock;
    SOCKADDR_IN serverAddr, clientAddr;
    int clientAddrSize;
    HANDLE hThread;
    if (argc != 2) {
        printf("Usage : %s <port>\n", argv[0]);
        exit(1);
    }
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        ErrorHandling("WSAStartup() error!");

    serverSock = socket(PF_INET, SOCK_STREAM, 0); //������ ����

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(atoi(argv[1]));

    if (bind(serverSock, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) //������ ������ ��ġ�Ѵ�.
        ErrorHandling("bind() error");
    if (listen(serverSock, 5) == SOCKET_ERROR)//������ �غ���¿� �д�.
        ErrorHandling("listen() error");
    InitializeCriticalSection(&cs);
    while (1) {
        clientAddrSize = sizeof(clientAddr);
        clientSock = accept(serverSock, (SOCKADDR*)&clientAddr, &clientAddrSize);//�������� ���޵� Ŭ���̾�Ʈ ������ clientSock�� ����
        EnterCriticalSection(&cs);
        clientSocks[clientCount++] = clientSock;//Ŭ���̾�Ʈ ���Ϲ迭�� ��� ������ ���� �ּҸ� ����
        LeaveCriticalSection(&cs);
        hThread = CreateThread(NULL, 0, ClientRecv, &clientSock, 0, NULL);//Client ������ ����, clientSock�� �Ű������� ����
        printf("Connected Client IP : %s\n", inet_ntoa(clientAddr.sin_addr));
    }
    DeleteCriticalSection(&cs);
    closesocket(serverSock);//������ ������ ����.
    WSACleanup();//������ ������ �����ϰڴٴ� ����� �ü���� ����
    return 0;
}
int turn = 0;
DWORD WINAPI ClientRecv(LPVOID arg) {
    SOCKET clientSock = *((SOCKET*)arg); //�Ű������ι��� Ŭ���̾�Ʈ ������ ����
    int strLen = 0, i;
    char msg[BUF_SIZE];

    while ((strLen = recv(clientSock, msg, sizeof(msg), 0)) != 0) {
        //Ŭ���̾�Ʈ ������� ä�� ������ ���� 
        if (clientSocks[turn % clientCount] == clientSock && clientCount!=0) {
            //�ܾ� üũ �߰��ϱ�
            if (strLen == 11) {
                SendMsg(msg, strLen);//SendMsg�� ���� �޽����� �����Ѵ�.
            }
            else {
                SendMsg("2���ڰ� �ƴմϴ�.\n",100);//SendMsg�� ���� �޽����� �����Ѵ�.
            }
            
            turn++;
        }
    } 
        
    //Ŭ���̾�Ʈ�� �������� 
    EnterCriticalSection(&cs);
    for (i = 0; i < clientCount; i++) {//�迭�� ������ŭ
        if (clientSock == clientSocks[i]) {//���� ���� clientSock���� �迭�� ���� ���ٸ�
            while (i++ < clientCount - 1)//Ŭ���̾�Ʈ ���� ��ŭ
                clientSocks[i] = clientSocks[i + 1];//������ �����.
            break;
        }
    }
    clientCount--;//Ŭ���̾�Ʈ ���� �ϳ� ����
    LeaveCriticalSection(&cs);
    closesocket(clientSock);//������ �����Ѵ�.
    return 0;
}

void SendMsg(char* msg, int len) { //�޽����� ��� Ŭ���̾�Ʈ���� ������.
    int i;
    EnterCriticalSection(&cs);
    for (i = 0; i < clientCount; i++)//Ŭ���̾�Ʈ ������ŭ
        send(clientSocks[i], msg, len, 0);//Ŭ���̾�Ʈ�鿡�� �޽����� �����Ѵ�.
    LeaveCriticalSection(&cs);
}
void ErrorHandling(char* msg) {
    fputs(msg, stderr);
    fputc('\n', stderr);
    exit(1);
}