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

    if (bind(serverSock, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
        ErrorHandling("bind() error");
    if (listen(serverSock, 5) == SOCKET_ERROR)
        ErrorHandling("listen() error");

    InitializeCriticalSection(&cs);
    while (1) {
        clientAddrSize = sizeof(clientAddr);
        clientSock = accept(serverSock, (SOCKADDR*)&clientAddr, &clientAddrSize);//�������� ���޵� Ŭ���̾�Ʈ ������ clientSock�� ����
        EnterCriticalSection(&cs);
        clientSocks[clientCount++] = clientSock;//Ŭ���̾�Ʈ ���Ϲ迭�� ��� ������ ���� �ּҸ� ����
        LeaveCriticalSection(&cs);
        hThread = CreateThread(NULL, 0, ClientRecv, &clientSock, 0, NULL);
        printf("Connected Client IP : %s\n", inet_ntoa(clientAddr.sin_addr));
    }
    DeleteCriticalSection(&cs);
    closesocket(serverSock);
    WSACleanup();//������ ������ ����
    return 0;
}

int turn = 0;//Ŭ���̾�Ʈ ���� ����
bool start = FALSE;
char check; //������ �ܾ� üũ�� ���� ����
int gameCount = 0;//�����ο�
SOCKET gameSocks[MAX_CLNT];//���� ������ ����

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
        // ù ptr�� �̸�
        EnterCriticalSection(&cs);
        username = strtok(checkmsg, " ");
        // �ι�° ptr�� �ܾ�
        ptr2 = strtok(NULL, "\n");
        if (!strcmp(ptr2, "!ready")) {
            gameSocks[gameCount] = clientSock;
            gameCount++;
            char notice[BUF_SIZE] = { 0 };
            sprintf(notice, "%s ���� %d�� ����� �Դϴ�.\n", username, gameCount);
            SendMsg(notice, 100);
        }
        LeaveCriticalSection(&cs);
        //Ŭ���̾�Ʈ ������� ä�� ������ ���� 
        //1�� ����ڰ� ����. !startĿ�ǵ� �Է��ϸ� ����
        if (gameCount > 0 && gameSocks[turn % gameCount] == clientSock ) {
            EnterCriticalSection(&cs);
            strcpy(temp, msg);
            const char* ptr= (const char*)malloc(20);
            // ù ptr�� �̸�
            ptr = strtok(temp, " ");    
            // �ι�° ptr�� �ܾ�
            ptr = strtok(NULL, "\n");    
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
                        //exe �ٷΰ��� ���
                        sprintf(argv, "API.exe %s", ptr);
                        //���̽� ���� -> API response�� �޾Ƽ� ���Ͽ� �ۼ� 
                        system(argv);
                        // ������ ���� ���� �о��
                        fopen_s(&fp, "api.txt", "r");
                        fgets(buffer, sizeof(buffer), fp);
                        // �ܾ ���� ��� 404 ����
                        if (!strcmp(buffer, "404")) {
                            SendMsg("���� �ܾ��Դϴ�. ������ �����մϴ�.\n", 100);
                            //���� �ʱ�ȭ
                            head = NULL;
                            start = FALSE;
                            turn = 0;
                            SOCKET gameSocks[MAX_CLNT];
                            gameCount = 0;
                            //���� ���ôܾ� ����
                            SetRandomCharacter();
                        }
                        //�ܾ �ִ� ���
                        else {
                            //�ܾ��� �� ��� 
                            SendMsg(buffer, 100);
                            //���ôܾ� �ٲ��ֱ�
                            int a = strlen(ptr);
                            char s = ptr[a - 1];
                            check = s;
                            //���� ������� �� �ѱ�
                            turn++;
                            //���� �����, ���� ���� ����
                            char notice[100] = { 0 };
                            sprintf(notice, "-------����!------- \n [%d]�� ����� ���۴ܾ� [%c]\n", turn % gameCount + 1, check);
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
                        //���� �ʱ�ȭ 
                        head = NULL;
                        start = FALSE;
                        turn = 0;
                        SOCKET gameSocks[MAX_CLNT];
                        gameCount = 0;
                        //���� ���ôܾ� ����
                        SetRandomCharacter();
                    }
                    LeaveCriticalSection(&cs);
            }
            //ù���ڰ� ���� �ܾ��� �������� �ٸ����
            else if(ptr[0] != check && start && turn !=0){
                EnterCriticalSection(&cs);
                char notice[100] = { 0 };
                sprintf(notice, "%d�� ����ڰ� Ʋ�ǽ��ϴ�. ������ ���� �մϴ�.\n", turn % gameCount +1);
                SendMsg(notice, 100);
                //���� �ʱ�ȭ
                head = NULL;
                start = FALSE;
                turn = 0;
                SOCKET gameSocks[MAX_CLNT];
                gameCount = 0;
                //���ôܾ� ���� �ʱ�ȭ
                SetRandomCharacter();
                LeaveCriticalSection(&cs);
            }
            LeaveCriticalSection(&cs);
        }
        
    } 
        
    //Ŭ���̾�Ʈ�� �������� 
    EnterCriticalSection(&cs);
    for (i = 0; i < clientCount; i++) {
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

// �ܾ� �ߺ�üũ�� ���� ��ũ�帮��Ʈ
bool linkedListFunction(int choice, const char* in)
{
    const char* val = (const char*)malloc(20);
    if(val)strcpy((char*)val, in);
    switch (choice)
    {
        //���Ḯ��Ʈ�� �߰�
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
        // �˻� - �ܾ������ false ������ true ����
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
// ���þ� ���� ����
void SetRandomCharacter()
{
    //�ð������� �õ尪 ����
    srand((unsigned int)time(0));
    //������ a~z 
    check= 'a' + (rand() % 26);
}
