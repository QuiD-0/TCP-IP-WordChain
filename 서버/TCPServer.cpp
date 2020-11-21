#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib,"ws2_32.lib")
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Windows.h>
#include <process.h>
#include <malloc.h>
#include <time.h>
#define BUF_SIZE 100
#define MAX_CLNT 256

DWORD WINAPI ClientRecv(LPVOID arg);//쓰레드 함수
void SendMsg(char* msg, int len);//메시지 보내는 함수
void ErrorHandling(char* msg);
bool linkedListFunction(int choice,const char* name);
void SetRandomCharacter();

int clientCount = 0;
SOCKET clientSocks[MAX_CLNT];//클라이언트 소켓 보관용 배열
CRITICAL_SECTION cs;

struct nodeType
{
    const char* name;
    nodeType* link;
};
nodeType* head = NULL;
nodeType* newNode;
nodeType* current;

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
char check; //마지막 단어 체크를 위한 변수

DWORD WINAPI ClientRecv(LPVOID arg) {
    SOCKET clientSock = *((SOCKET*)arg); //매개변수로받은 클라이언트 소켓을 전달
    int strLen = 0, i;
    char msg[BUF_SIZE];
    char temp[100];
    SetRandomCharacter();
    while ((strLen = recv(clientSock, msg, sizeof(msg), 0)) != 0) {
        FILE* fp = NULL;
        char buffer[200];

        SendMsg(msg, strLen);
        //클라이언트 순서대로 채팅 권한을 가짐 
        //1번 사용자가 방장. !start커맨드 입력하면 시작
        if (clientSocks[turn % clientCount] == clientSock ) {
            EnterCriticalSection(&cs);
            strcpy(temp, msg);
            const char* ptr= (const char*)malloc(20);
            ptr = strtok(temp, " ");    //첫번째 strtok 사용.
            ptr = strtok(NULL, "\n");     //자른 문자 다음부터 구분자 또 찾기
            LeaveCriticalSection(&cs);
            //start 입력 받았을 경우 게임 시작
            if (!strcmp(ptr, "!start") && turn==0) { 
                EnterCriticalSection(&cs);
                start = TRUE; 
                char notice[BUF_SIZE] = { 0 };
                sprintf(notice, "게임 시작 \n 1번 사용자 제시어 : [%c]\n", check);
                SendMsg(notice, 100);
                LeaveCriticalSection(&cs);
            }
            if (ptr[0] == check &&start) {
                EnterCriticalSection(&cs);
                    // 단어가 중복이 아닌경우
                    if (linkedListFunction(2, (const char*)ptr)) {
                        linkedListFunction(1, (const char*)ptr);
                        //단어 API 체크
                        //google dictinary API 사용 
                        char argv[BUF_SIZE] = { 0 };
                        sprintf(argv, "API.py %s", ptr);
                        system(argv);
                        // 파일을 통해 api.py와 통신
                        fopen_s(&fp, "./api.txt", "r");
                        fgets(buffer, sizeof(buffer), fp);
                        // 단어가 없는 경우
                        if (!strcmp(buffer, "404")) {
                            SendMsg("없는 단어입니다. 게임을 종료합니다.\n", 100);
                            head = NULL;
                            start = FALSE;
                            turn = 0;
                            SetRandomCharacter();
                        }
                        //단어가 있는 경우
                        else {
                            //단어의 뜻 출력
                            SendMsg(buffer, 100);
                            int a = strlen(ptr);
                            char s = ptr[a - 1];
                            check = s;
                            //다음 사람에게 턴 넘김
                            turn++;
                            //다음 사용자, 시작 문자 공지
                            char notice[100] = { 0 };
                            sprintf(notice, "######정답!##### \n%d번 사용자 시작단어 [%c]\n", turn % clientCount + 1, check);
                            SendMsg(notice, 100);
                        }
                        fclose(fp);
                        // 정답 단어 업데이트/// 정답자의 마지막 단어 -> check복사
                    
                    }
                    // 단어가 이미 사용되었으면
                    else {
                        char notice[100] = { 0 };
                        sprintf(notice, "이미 사용한 단어입니다.\n게임종료\n");
                        SendMsg(notice, 100); 
                        //게임 세팅 초기화 
                        head = NULL;
                        start = FALSE;
                        turn = 0;
                        SetRandomCharacter();
                    }
                     
                 //없는 단어일 경우
                    LeaveCriticalSection(&cs);
            }
            //첫글자가 이전 단어의 마지막과 다를경우
            else if(ptr[0] != check && start && turn !=0){
                EnterCriticalSection(&cs);
                char notice[100] = { 0 };
                sprintf(notice, "%d번 사용자가 틀렷습니다. 게임을 종료 합니다.\n", turn % clientCount+1);
                SendMsg(notice, 100);
                head = NULL;
                start = FALSE;
                turn = 0;
                //임시 -> 랜덤 으로 바꾸기 
                SetRandomCharacter();
                LeaveCriticalSection(&cs);
            }
            LeaveCriticalSection(&cs);
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


bool linkedListFunction(int choice, const char* in)
{
    const char* val = (const char*)malloc(20);
    if(val)strcpy((char*)val, in);
    switch (choice)
    {
    case 1:
        EnterCriticalSection(&cs);
        newNode = (nodeType*)malloc(sizeof(nodeType));
        if(newNode)newNode->name = val;
        if(newNode)newNode->link = NULL;
        if (head == NULL)
        {
            head = newNode;
        }
        else
        {
            if (newNode)newNode->link = head;
            head = newNode;
        }
        LeaveCriticalSection(&cs);
        break;
    case 2:
        EnterCriticalSection(&cs);
        current = head;
        while (current != NULL)
        {   
            if (strcmp(current->name, val)) {
                current = current->link;
            }
            else break;
        }
        if (current == NULL)
        {
            return true;

        }
        else
        {
            return false;

        }
        LeaveCriticalSection(&cs);
        break;
    }

}

void SetRandomCharacter()
{
    srand((unsigned int)time(0));
    check= 'a' + (rand() % 26);
}
