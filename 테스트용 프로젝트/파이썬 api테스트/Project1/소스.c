#include<stdio.h>
int main() {
    FILE* fp = NULL;
    char buffer[200];    // ������ ���� �� ����� �ӽ� ����
    system("test.py hello");
    fopen_s(&fp,"./test.txt", "r");  
    fgets(buffer, sizeof(buffer), fp); 
    if (!strcmp(buffer, "404")) {
        printf("���� �ܾ��Դϴ�.");
    }
    else {
        printf("�� : %s\n", buffer);
    }

    fclose(fp);    
}