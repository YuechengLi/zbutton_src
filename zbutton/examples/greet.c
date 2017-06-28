#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int display1(char *string);
int display2(char *string1);

int main()
{
	char string[]="ebutton!";
	display1(string);
	display2(string);	
}

int display1(char *string)
{
	printf("The original string is %s \n", string);
}

int display2(char *string1)
{
	char *string2;
	int size,i;
	size = strlen (string1);

	string2 = (char *) malloc(size+1);

	for (i=0;i<size;i++)
	{
		string2[i] = string1[size-1-i];	
	}
	string2[size]=' ';
	printf("The string afterward is %s \n", string2);
}
