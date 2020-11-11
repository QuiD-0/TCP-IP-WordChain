#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib,"ws2_32.lib")
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Windows.h>
#include <process.h>

#define BUF_SIZE 100
#define MAX_CLNT 256

DWORD WINAPI ClientRecv(LPVOID arg);//쓰레드 함수
void SendMsg(char* msg, int len);//메시지 보내는 함수
void ErrorHandling(char* msg);

int clientCount = 0;
SOCKET clientSocks[MAX_CLNT];//클라이언트 소켓 보관용 배열
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

    serverSock = socket(PF_INET, SOCK_STREAM, 0); //소켓을 생성

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(atoi(argv[1]));

    if (bind(serverSock, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) //생성한 소켓을 배치한다.
        ErrorHandling("bind() error");
    if (listen(serverSock, 5) == SOCKET_ERROR)//소켓을 준비상태에 둔다.
        ErrorHandling("listen() error");
    InitializeCriticalSection(&cs);
    while (1) {
        clientAddrSize = sizeof(clientAddr);
        clientSock = accept(serverSock, (SOCKADDR*)&clientAddr, &clientAddrSize);//서버에게 전달된 클라이언트 소켓을 clientSock에 전달
        EnterCriticalSection(&cs);
        clientSocks[clientCount++] = clientSock;//클라이언트 소켓배열에 방금 가져온 소켓 주소를 전달
        LeaveCriticalSection(&cs);
        hThread = CreateThread(NULL, 0, ClientRecv, &clientSock, 0, NULL);//Client 쓰레드 실행, clientSock을 매개변수로 전달
        printf("Connected Client IP : %s\n", inet_ntoa(clientAddr.sin_addr));
    }
    DeleteCriticalSection(&cs);
    closesocket(serverSock);//생성한 소켓을 끈다.
    WSACleanup();//윈도우 소켓을 종료하겠다는 사실을 운영체제에 전달
    return 0;
}
int turn = 0;
DWORD WINAPI ClientRecv(LPVOID arg) {
    SOCKET clientSock = *((SOCKET*)arg); //매개변수로받은 클라이언트 소켓을 전달
    int strLen = 0, i;
    char msg[BUF_SIZE];

    while ((strLen = recv(clientSock, msg, sizeof(msg), 0)) != 0) {
        //클라이언트 순서대로 채팅 권한을 가짐 
        if (clientSocks[turn % clientCount] == clientSock && clientCount!=0) {
            //단어 체크 추가하기
            if (strLen == 11) {
                SendMsg(msg, strLen);//SendMsg에 받은 메시지를 전달한다.
            }
            else {
                SendMsg("2글자가 아닙니다.\n",100);//SendMsg에 받은 메시지를 전달한다.
            }
            
            turn++;
        }
    } 
        
    //클라이언트가 나갔을때 
    EnterCriticalSection(&cs);
    for (i = 0; i < clientCount; i++) {//배열의 갯수만큼
        if (clientSock == clientSocks[i]) {//만약 현재 clientSock값이 배열의 값과 같다면
            while (i++ < clientCount - 1)//클라이언트 개수 만큼
                clientSocks[i] = clientSocks[i + 1];//앞으로 땡긴다.
            break;
        }
    }
    clientCount--;//클라이언트 개수 하나 감소
    LeaveCriticalSection(&cs);
    closesocket(clientSock);//소켓을 종료한다.
    return 0;
}

void SendMsg(char* msg, int len) { //메시지를 모든 클라이언트에게 보낸다.
    int i;
    EnterCriticalSection(&cs);
    for (i = 0; i < clientCount; i++)//클라이언트 개수만큼
        send(clientSocks[i], msg, len, 0);//클라이언트들에게 메시지를 전달한다.
    LeaveCriticalSection(&cs);
}
void ErrorHandling(char* msg) {
    fputs(msg, stderr);
    fputc('\n', stderr);
    exit(1);
}