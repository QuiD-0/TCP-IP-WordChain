#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib,"ws2_32.lib")
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <WinSock2.h>
#include <Windows.h>
#include <process.h>

#define BUF_SIZE 100
#define NAME_SIZE 20

DWORD WINAPI SendMsg(LPVOID arg);//쓰레드 전송함수
DWORD WINAPI RecvMsg(LPVOID arg);//쓰레드 수신함수
void ErrorHandling(char* msg);

char name[NAME_SIZE] = "[DEFAULT]";
char msg[BUF_SIZE];
char temp[BUF_SIZE];
int main(int argc, char* argv[]) {
    WSADATA wsaData;
    SOCKET sock;
    SOCKADDR_IN serverAddr;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)// 윈도우 소켓을 사용한다고 운영체제에 알림
        ErrorHandling("WSAStartup() error!");
    printf("닉네임 입력:");
    scanf("%s",temp);
    sprintf_s(name, "[%s]",temp);

    sock = socket(AF_INET, SOCK_STREAM, 0);//소켓을 하나 생성한다.

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverAddr.sin_port = htons(atoi("9000"));

    if (connect(sock, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)//서버에 접속한다.
        ErrorHandling("connect() error");

    //접속에 성공하면 이 줄 아래가 실행된다.
    HANDLE hthread[2];
    hthread[0] = CreateThread(NULL, 0, SendMsg, &sock, 0, NULL);
    hthread[1] = CreateThread(NULL, 0, RecvMsg, &sock, 0, NULL);//메시지 수신용 쓰레드가 실행된다.
    WaitForMultipleObjects(2, hthread, TRUE, INFINITE);//전송용 쓰레드가 중지될때까지 기다린다./

    //클라이언트가 종료를 시도한다면 이줄 아래가 실행된다.
    closesocket(sock);//소켓을 종료한다.
    WSACleanup();//윈도우 소켓 사용중지를 운영체제에 알린다.
    return 0;
}

DWORD WINAPI SendMsg(LPVOID arg) {//전송용 쓰레드함수
    SOCKET sock = *((SOCKET*)arg);//서버용 소켓을 전달한다.
    char nameMsg[NAME_SIZE + BUF_SIZE];
    while (1) {//반복
        fgets(msg, BUF_SIZE, stdin);//입력을 받는다.
        if (!strcmp(msg, "q\n") || !strcmp(msg, "Q\n")) {//q를 입력하면 종료한다.
            closesocket(sock);
            exit(0);
        }
        sprintf_s(nameMsg, "%s %s", name, msg);//nameMsg에 메시지를 전달한다.
        send(sock, nameMsg, strlen(nameMsg), 0);//nameMsg를 서버에게 전송한다.
    }
    return 0;
}

DWORD WINAPI RecvMsg(LPVOID arg) {
    SOCKET sock = *((SOCKET*)arg);//서버용 소켓을 전달한다.
    char nameMsg[NAME_SIZE + BUF_SIZE];
    int strLen;
    while (1) {//반복
        strLen = recv(sock, nameMsg, NAME_SIZE + BUF_SIZE - 1, 0);//서버로부터 메시지를 수신한다.
        if (strLen == -1)
            return -1;
        nameMsg[strLen] = 0;//문자열의 끝을 알리기 위해 설정
        fputs(nameMsg, stdout);//자신의 콘솔에 받은 메시지를 출력한다.
    }
    return 0;
}

void ErrorHandling(char* msg) {
    fputs(msg, stderr);
    fputc('\n', stderr);
    exit(1);
}
