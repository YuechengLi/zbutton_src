#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#define LENGTH 100
main()
{
	int fd, len;
	char str[LENGTH];

 	fd = open("hello.txt", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR); /*���������ļ�*/
 	if (fd)
 	{
 		write(fd, "Hello World", 100); /*д���ַ���*/

 		close(fd);
 	}

 	fd = open("hello.txt", O_RDWR);
 	len = read(fd, str, LENGTH); /* ��ȡ�ļ�����*/
 	str[len] = '\0';
 	printf("%s\n", str);
 	close(fd);
 }
