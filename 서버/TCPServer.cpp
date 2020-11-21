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

DWORD WINAPI ClientRecv(LPVOID arg);//������ �Լ�
void SendMsg(char* msg, int len);//�޽��� ������ �Լ�
void ErrorHandling(char* msg);
bool linkedListFunction(int choice,const char* name);
void SetRandomCharacter();

int clientCount = 0;
SOCKET clientSocks[MAX_CLNT];//Ŭ���̾�Ʈ ���� ������ �迭
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
char check; //������ �ܾ� üũ�� ���� ����

DWORD WINAPI ClientRecv(LPVOID arg) {
    SOCKET clientSock = *((SOCKET*)arg); //�Ű������ι��� Ŭ���̾�Ʈ ������ ����
    int strLen = 0, i;
    char msg[BUF_SIZE];
    char temp[100];
    SetRandomCharacter();
    while ((strLen = recv(clientSock, msg, sizeof(msg), 0)) != 0) {
        FILE* fp = NULL;
        char buffer[200];

        SendMsg(msg, strLen);
        //Ŭ���̾�Ʈ ������� ä�� ������ ���� 
        //1�� ����ڰ� ����. !startĿ�ǵ� �Է��ϸ� ����
        if (clientSocks[turn % clientCount] == clientSock ) {
            EnterCriticalSection(&cs);
            strcpy(temp, msg);
            const char* ptr= (const char*)malloc(20);
            ptr = strtok(temp, " ");    //ù��° strtok ���.
            ptr = strtok(NULL, "\n");     //�ڸ� ���� �������� ������ �� ã��
            LeaveCriticalSection(&cs);
            //start �Է� �޾��� ��� ���� ����
            if (!strcmp(ptr, "!start") && turn==0) { 
                EnterCriticalSection(&cs);
                start = TRUE; 
                char notice[BUF_SIZE] = { 0 };
                sprintf(notice, "���� ���� \n 1�� ����� ���þ� : [%c]\n", check);
                SendMsg(notice, 100);
                LeaveCriticalSection(&cs);
            }
            if (ptr[0] == check &&start) {
                EnterCriticalSection(&cs);
                    // �ܾ �ߺ��� �ƴѰ��
                    if (linkedListFunction(2, (const char*)ptr)) {
                        linkedListFunction(1, (const char*)ptr);
                        //�ܾ� API üũ
                        //google dictinary API ��� 
                        char argv[BUF_SIZE] = { 0 };
                        sprintf(argv, "API.py %s", ptr);
                        system(argv);
                        // ������ ���� api.py�� ���
                        fopen_s(&fp, "./api.txt", "r");
                        fgets(buffer, sizeof(buffer), fp);
                        // �ܾ ���� ���
                        if (!strcmp(buffer, "404")) {
                            SendMsg("���� �ܾ��Դϴ�. ������ �����մϴ�.\n", 100);
                            head = NULL;
                            start = FALSE;
                            turn = 0;
                            SetRandomCharacter();
                        }
                        //�ܾ �ִ� ���
                        else {
                            //�ܾ��� �� ���
                            SendMsg(buffer, 100);
                            int a = strlen(ptr);
                            char s = ptr[a - 1];
                            check = s;
                            //���� ������� �� �ѱ�
                            turn++;
                            //���� �����, ���� ���� ����
                            char notice[100] = { 0 };
                            sprintf(notice, "######����!##### \n%d�� ����� ���۴ܾ� [%c]\n", turn % clientCount + 1, check);
                            SendMsg(notice, 100);
                        }
                        fclose(fp);
                        // ���� �ܾ� ������Ʈ/// �������� ������ �ܾ� -> check����
                    
                    }
                    // �ܾ �̹� ���Ǿ�����
                    else {
                        char notice[100] = { 0 };
                        sprintf(notice, "�̹� ����� �ܾ��Դϴ�.\n��������\n");
                        SendMsg(notice, 100); 
                        //���� ���� �ʱ�ȭ 
                        head = NULL;
                        start = FALSE;
                        turn = 0;
                        SetRandomCharacter();
                    }
                     
                 //���� �ܾ��� ���
                    LeaveCriticalSection(&cs);
            }
            //ù���ڰ� ���� �ܾ��� �������� �ٸ����
            else if(ptr[0] != check && start && turn !=0){
                EnterCriticalSection(&cs);
                char notice[100] = { 0 };
                sprintf(notice, "%d�� ����ڰ� Ʋ�ǽ��ϴ�. ������ ���� �մϴ�.\n", turn % clientCount+1);
                SendMsg(notice, 100);
                head = NULL;
                start = FALSE;
                turn = 0;
                //�ӽ� -> ���� ���� �ٲٱ� 
                SetRandomCharacter();
                LeaveCriticalSection(&cs);
            }
            LeaveCriticalSection(&cs);
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
