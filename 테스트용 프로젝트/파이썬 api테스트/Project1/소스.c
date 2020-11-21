#include<stdio.h>
int main() {
    FILE* fp = NULL;
    char buffer[200];    // 파일을 읽을 때 사용할 임시 공간
    system("test.py hello");
    fopen_s(&fp,"./test.txt", "r");  
    fgets(buffer, sizeof(buffer), fp); 
    if (!strcmp(buffer, "404")) {
        printf("없는 단어입니다.");
    }
    else {
        printf("뜻 : %s\n", buffer);
    }

    fclose(fp);    
}