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

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        ErrorHandling("WSAStartup() error!");

    serverSock = socket(AF_INET, SOCK_STREAM, 0); //������ ����

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(atoi("9000"));

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
        char notice[BUF_SIZE] = { 0 };
        sprintf(notice, "%d�� ���������.\n",clientCount);
        SendMsg(notice, 100);
    }
    DeleteCriticalSection(&cs);
    closesocket(serverSock);//������ ������ ����.
    WSACleanup();//������ ������ �����ϰڴٴ� ����� �ü���� ����
    return 0;
}


int turn = 0;//Ŭ���̾�Ʈ ���� ����
bool start = FALSE;
char check = 't'; //������ �ܾ� üũ

DWORD WINAPI ClientRecv(LPVOID arg) {
    SOCKET clientSock = *((SOCKET*)arg); //�Ű������ι��� Ŭ���̾�Ʈ ������ ����
    int strLen = 0, i;
    char msg[BUF_SIZE];
    char temp[BUF_SIZE];
    
    while ((strLen = recv(clientSock, msg, sizeof(msg), 0)) != 0) {
        SendMsg(msg, strLen);
        //Ŭ���̾�Ʈ ������� ä�� ������ ���� 
        if (clientSocks[turn % clientCount] == clientSock) {
            strcpy(temp, msg);
            char* ptr = strtok(temp, " ");    //ù��° strtok ���.
            ptr = strtok(NULL, "\n");     //�ڸ� ���� �������� ������ �� ã��

            if (!strcmp(ptr, "start") && turn==0) { 
                start = TRUE; 
                SendMsg("���� ���� \n 0�� ����� ���þ� : [t]\n", 100);
            }
            if (ptr[0] == check &&start) {
                // ��� �ܾ� ����Ʈ ����� 
                //���Ḯ��Ʈ?

                EnterCriticalSection(&cs);
                // ���� �ܾ� ������Ʈ/// �������� ������ �ܾ� -> check����
                int a = strlen(ptr);
                char s = ptr[a - 1];
                check = s;
                turn++;

                //���� �����, ���� ���� ����
                char notice[BUF_SIZE] = { 0 };
                sprintf(notice, " ����! \n %d�� ����� ���۴ܾ� [%c]\n", turn % clientCount+1, check);
                SendMsg(notice, 100);
                LeaveCriticalSection(&cs);
            }else if(ptr[0] != check && start && turn !=0){
                char notice[BUF_SIZE] = { 0 };
                sprintf(notice, "%d�� ����ڰ� Ʋ�ǽ��ϴ�. ������ ���� �մϴ�.\n", turn % clientCount+1);
                SendMsg(notice, 100);
                start = FALSE;
                turn = 0;
            }
           
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