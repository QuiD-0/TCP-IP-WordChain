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

    if (bind(serverSock, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
        ErrorHandling("bind() error");
    if (listen(serverSock, 5) == SOCKET_ERROR)
        ErrorHandling("listen() error");

    InitializeCriticalSection(&cs);
    while (1) {
        clientAddrSize = sizeof(clientAddr);
        clientSock = accept(serverSock, (SOCKADDR*)&clientAddr, &clientAddrSize);//서버에게 전달된 클라이언트 소켓을 clientSock에 전달
        EnterCriticalSection(&cs);
        clientSocks[clientCount++] = clientSock;//클라이언트 소켓배열에 방금 가져온 소켓 주소를 전달
        LeaveCriticalSection(&cs);
        hThread = CreateThread(NULL, 0, ClientRecv, &clientSock, 0, NULL);
        printf("Connected Client IP : %s\n", inet_ntoa(clientAddr.sin_addr));
    }
    DeleteCriticalSection(&cs);
    closesocket(serverSock);
    WSACleanup();//윈도우 소켓을 종료
    return 0;
}

int turn = 0;//클라이언트 순서 제어
bool start = FALSE;
char check; //마지막 단어 체크를 위한 변수
int gameCount = 0;//게임인원
SOCKET gameSocks[MAX_CLNT];//게임 참가자 소켓

DWORD WINAPI ClientRecv(LPVOID arg) {
    SOCKET clientSock = *((SOCKET*)arg); 
    int strLen = 0, i;
    char msg[BUF_SIZE];
    char temp[100];
    char checkmsg[100];
    SetRandomCharacter();
    while ((strLen = recv(clientSock, msg, sizeof(msg), 0)) != 0) {
        FILE* fp = NULL;
        char buffer[200];
        SendMsg(msg, strLen);
        strcpy(checkmsg, msg);
        const char* ptr = (const char*)malloc(20);
        const char* ptr2 = (const char*)malloc(20);
        const char* username = (const char*)malloc(20);
        // 첫 ptr은 이름
        EnterCriticalSection(&cs);
        username = strtok(checkmsg, " ");
        // 두번째 ptr은 단어
        ptr2 = strtok(NULL, "\n");
        if (!strcmp(ptr2, "!ready")) {
            gameSocks[gameCount] = clientSock;
            gameCount++;
            char notice[BUF_SIZE] = { 0 };
            sprintf(notice, "%s 님은 %d번 사용자 입니다.\n", username, gameCount);
            SendMsg(notice, 100);
        }
        LeaveCriticalSection(&cs);
        //클라이언트 순서대로 채팅 권한을 가짐 
        //1번 사용자가 방장. !start커맨드 입력하면 시작
        if (gameCount > 0 && gameSocks[turn % gameCount] == clientSock ) {
            EnterCriticalSection(&cs);
            strcpy(temp, msg);
            const char* ptr= (const char*)malloc(20);
            // 첫 ptr은 이름
            ptr = strtok(temp, " ");    
            // 두번째 ptr은 단어
            ptr = strtok(NULL, "\n");    
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
                        //exe 바로가기 경로
                        sprintf(argv, "API.exe %s", ptr);
                        //파이썬 실행 -> API response를 받아서 파일에 작성 
                        system(argv);
                        // 파일을 통해 값을 읽어옴
                        fopen_s(&fp, "api.txt", "r");
                        fgets(buffer, sizeof(buffer), fp);
                        // 단어가 없는 경우 404 리턴
                        if (!strcmp(buffer, "404")) {
                            SendMsg("없는 단어입니다. 게임을 종료합니다.\n", 100);
                            //게임 초기화
                            head = NULL;
                            start = FALSE;
                            turn = 0;
                            SOCKET gameSocks[MAX_CLNT];
                            gameCount = 0;
                            //랜덤 제시단어 변경
                            SetRandomCharacter();
                        }
                        //단어가 있는 경우
                        else {
                            //단어의 뜻 출력 
                            SendMsg(buffer, 100);
                            //제시단어 바꿔주기
                            int a = strlen(ptr);
                            char s = ptr[a - 1];
                            check = s;
                            //다음 사람에게 턴 넘김
                            turn++;
                            //다음 사용자, 시작 문자 공지
                            char notice[100] = { 0 };
                            sprintf(notice, "-------정답!------- \n [%d]번 사용자 시작단어 [%c]\n", turn % gameCount + 1, check);
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
                        //게임 초기화 
                        head = NULL;
                        start = FALSE;
                        turn = 0;
                        SOCKET gameSocks[MAX_CLNT];
                        gameCount = 0;
                        //랜덤 제시단어 변경
                        SetRandomCharacter();
                    }
                    LeaveCriticalSection(&cs);
            }
            //첫글자가 이전 단어의 마지막과 다를경우
            else if(ptr[0] != check && start && turn !=0){
                EnterCriticalSection(&cs);
                char notice[100] = { 0 };
                sprintf(notice, "%d번 사용자가 틀렷습니다. 게임을 종료 합니다.\n", turn % gameCount +1);
                SendMsg(notice, 100);
                //게임 초기화
                head = NULL;
                start = FALSE;
                turn = 0;
                SOCKET gameSocks[MAX_CLNT];
                gameCount = 0;
                //제시단어 랜덤 초기화
                SetRandomCharacter();
                LeaveCriticalSection(&cs);
            }
            LeaveCriticalSection(&cs);
        }
        
    } 
        
    //클라이언트가 나갔을때 
    EnterCriticalSection(&cs);
    for (i = 0; i < clientCount; i++) {
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

// 단어 중복체크를 위한 링크드리스트
bool linkedListFunction(int choice, const char* in)
{
    const char* val = (const char*)malloc(20);
    if(val)strcpy((char*)val, in);
    switch (choice)
    {
        //연결리스트에 추가
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
        // 검색 - 단어가있으면 false 없으면 true 리턴
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
// 제시어 랜덤 설정
void SetRandomCharacter()
{
    //시간에따라 시드값 변경
    srand((unsigned int)time(0));
    //값설정 a~z 
    check= 'a' + (rand() % 26);
}
