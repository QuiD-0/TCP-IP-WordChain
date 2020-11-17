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

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        ErrorHandling("WSAStartup() error!");

    serverSock = socket(AF_INET, SOCK_STREAM, 0); //소켓을 생성

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(atoi("9000"));

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
        char notice[BUF_SIZE] = { 0 };
        sprintf(notice, "%d번 사용자입장.\n",clientCount);
        SendMsg(notice, 100);
    }
    DeleteCriticalSection(&cs);
    closesocket(serverSock);//생성한 소켓을 끈다.
    WSACleanup();//윈도우 소켓을 종료하겠다는 사실을 운영체제에 전달
    return 0;
}


int turn = 0;//클라이언트 순서 제어
bool start = FALSE;
char check = 't'; //마지막 단어 체크

DWORD WINAPI ClientRecv(LPVOID arg) {
    SOCKET clientSock = *((SOCKET*)arg); //매개변수로받은 클라이언트 소켓을 전달
    int strLen = 0, i;
    char msg[BUF_SIZE];
    char temp[BUF_SIZE];
    
    while ((strLen = recv(clientSock, msg, sizeof(msg), 0)) != 0) {
        SendMsg(msg, strLen);
        //클라이언트 순서대로 채팅 권한을 가짐 
        if (clientSocks[turn % clientCount] == clientSock) {
            strcpy(temp, msg);
            char* ptr = strtok(temp, " ");    //첫번째 strtok 사용.
            ptr = strtok(NULL, "\n");     //자른 문자 다음부터 구분자 또 찾기

            if (!strcmp(ptr, "start") && turn==0) { 
                start = TRUE; 
                SendMsg("게임 시작 \n 0번 사용자 제시어 : [t]\n", 100);
            }
            if (ptr[0] == check &&start) {
                // 사용 단어 리스트 만들기 
                //연결리스트?

                EnterCriticalSection(&cs);
                // 정답 단어 업데이트/// 정답자의 마지막 단어 -> check복사
                int a = strlen(ptr);
                char s = ptr[a - 1];
                check = s;
                turn++;

                //다음 사용자, 시작 문자 공지
                char notice[BUF_SIZE] = { 0 };
                sprintf(notice, " 정답! \n %d번 사용자 시작단어 [%c]\n", turn % clientCount+1, check);
                SendMsg(notice, 100);
                LeaveCriticalSection(&cs);
            }else if(ptr[0] != check && start && turn !=0){
                char notice[BUF_SIZE] = { 0 };
                sprintf(notice, "%d번 사용자가 틀렷습니다. 게임을 종료 합니다.\n", turn % clientCount+1);
                SendMsg(notice, 100);
                start = FALSE;
                turn = 0;
            }
           
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