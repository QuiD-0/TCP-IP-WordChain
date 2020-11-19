#include "stdio.h"
#include "malloc.h"
#include "string.h"

typedef struct nodeType
{
	const char* name;
	nodeType* link;

};
nodeType* newNode;
nodeType* head = NULL;
nodeType* current;
nodeType* previousNode;

void main()
{
	void linkedListFunction(int choice, const char* name);

	linkedListFunction(1,"hello");
	linkedListFunction(1, "hello2");
	linkedListFunction(2, "hellodx");
	linkedListFunction(4, "hello");

}

void linkedListFunction(int choice,const char* name)
{
	
	printf("\n insert=1, search=2");
	switch (choice)
	{
	case 1:
		printf(" Type insert name : %s",name);

		newNode = (nodeType*)malloc(sizeof(nodeType));
		newNode->name = name;
		newNode->link = NULL;
		if (head == NULL)
		{
			head = newNode;

		}
		else
		{
			current = head;
			while (current != NULL)
			{
				previousNode = current;
				current = current->link;

			};
			previousNode->link = newNode;

		}

		break;
	case 2:
		printf(" Type search name : %s", name);

		current = head;
		while (current != NULL)
		{
			if (strcmp(current->name, name))
				current = current->link;
			else break;

		}
		if (current == NULL)
		{
			printf("\n Have no node \n");

		}
		else
		{
			printf("\n search name : %s\n", current->name);

		}
		break;
	}
}
