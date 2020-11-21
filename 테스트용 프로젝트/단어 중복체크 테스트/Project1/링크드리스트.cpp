#include "stdio.h"
#include "malloc.h"
#include "string.h"

typedef struct nodeType
{
	const char* name;
	nodeType* link;

};

void main()
{
	void linkedListFunction(int choice, const char* name);

	linkedListFunction(1,"hello");
	linkedListFunction(4, "hello");
	linkedListFunction(1, "hello2");
	linkedListFunction(4, "hello");
	linkedListFunction(1, "hello3");
	linkedListFunction(4, "hello");
	linkedListFunction(2, "hello");
}
nodeType* head = NULL;
nodeType* newNode;
nodeType* current;
nodeType* previousNode;
void linkedListFunction(int choice, const char* val)
{

    switch (choice)
    {
    case 1:
        newNode = (nodeType*)malloc(sizeof(nodeType));
        if (newNode)newNode->name = val;
        if (newNode)newNode->link = NULL;
        if (head == NULL)
        {
            head = newNode;
        }
        else
        {
            newNode->link = head;
            head = newNode;
        }
        break;
    case 2:
        current = head;
        while (current != NULL)
        {
            if (strcmp(current->name, val)) {
                printf("\n search name : %s %p %p\n", current->name, current, current->link);
                current = current->link;
            }
            else break;
        }
        if (current == NULL)
        {
            printf("\n Have no node \n");

        }
        else
        {
            printf("\n search name : %s %p %p\n", current->name, current, current->link);

        }
        break;
    case 4:
        if (head == NULL)
        {
            printf("\n Have no node \n");

        }
        else
        {
            current = head;
            while (current != NULL)
            {
                printf("\n %s %p, %p", current->name, current, current->link);
                current = current->link;

            }
            printf("\n");

        }
        break;
    }

}
