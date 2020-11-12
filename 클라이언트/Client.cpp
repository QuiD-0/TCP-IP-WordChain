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

DWORD WINAPI SendMsg(LPVOID arg);//������ �����Լ�
DWORD WINAPI RecvMsg(LPVOID arg);//������ �����Լ�
void ErrorHandling(char* msg);

char name[NAME_SIZE] = "[DEFAULT]";
char msg[BUF_SIZE];
char temp[BUF_SIZE];
int main(int argc, char* argv[]) {
    WSADATA wsaData;
    SOCKET sock;
    SOCKADDR_IN serverAddr;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)// ������ ������ ����Ѵٰ� �ü���� �˸�
        ErrorHandling("WSAStartup() error!");
    printf("�г��� �Է�:");
    scanf("%s",temp);
    sprintf_s(name, "[%s]",temp);

    sock = socket(AF_INET, SOCK_STREAM, 0);//������ �ϳ� �����Ѵ�.

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverAddr.sin_port = htons(atoi("9000"));

    if (connect(sock, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)//������ �����Ѵ�.
        ErrorHandling("connect() error");

    //���ӿ� �����ϸ� �� �� �Ʒ��� ����ȴ�.
    HANDLE hthread[2];
    hthread[0] = CreateThread(NULL, 0, SendMsg, &sock, 0, NULL);
    hthread[1] = CreateThread(NULL, 0, RecvMsg, &sock, 0, NULL);//�޽��� ���ſ� �����尡 ����ȴ�.
    WaitForMultipleObjects(2, hthread, TRUE, INFINITE);//���ۿ� �����尡 �����ɶ����� ��ٸ���./

    //Ŭ���̾�Ʈ�� ���Ḧ �õ��Ѵٸ� ���� �Ʒ��� ����ȴ�.
    closesocket(sock);//������ �����Ѵ�.
    WSACleanup();//������ ���� ��������� �ü���� �˸���.
    return 0;
}

DWORD WINAPI SendMsg(LPVOID arg) {//���ۿ� �������Լ�
    SOCKET sock = *((SOCKET*)arg);//������ ������ �����Ѵ�.
    char nameMsg[NAME_SIZE + BUF_SIZE];
    while (1) {//�ݺ�
        fgets(msg, BUF_SIZE, stdin);//�Է��� �޴´�.
        if (!strcmp(msg, "q\n") || !strcmp(msg, "Q\n")) {//q�� �Է��ϸ� �����Ѵ�.
            closesocket(sock);
            exit(0);
        }
        sprintf_s(nameMsg, "%s %s", name, msg);//nameMsg�� �޽����� �����Ѵ�.
        send(sock, nameMsg, strlen(nameMsg), 0);//nameMsg�� �������� �����Ѵ�.
    }
    return 0;
}

DWORD WINAPI RecvMsg(LPVOID arg) {
    SOCKET sock = *((SOCKET*)arg);//������ ������ �����Ѵ�.
    char nameMsg[NAME_SIZE + BUF_SIZE];
    int strLen;
    while (1) {//�ݺ�
        strLen = recv(sock, nameMsg, NAME_SIZE + BUF_SIZE - 1, 0);//�����κ��� �޽����� �����Ѵ�.
        if (strLen == -1)
            return -1;
        nameMsg[strLen] = 0;//���ڿ��� ���� �˸��� ���� ����
        fputs(nameMsg, stdout);//�ڽ��� �ֿܼ� ���� �޽����� ����Ѵ�.
    }
    return 0;
}

void ErrorHandling(char* msg) {
    fputs(msg, stderr);
    fputc('\n', stderr);
    exit(1);
}
