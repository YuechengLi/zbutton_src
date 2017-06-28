#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#define LENGTH 100
main()
{
	int fd, len;
	char str[LENGTH];

 	fd = open("hello.txt", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR); /*创建并打开文件*/
 	if (fd)
 	{
 		write(fd, "Hello World", 100); /*写入字符串*/

 		close(fd);
 	}

 	fd = open("hello.txt", O_RDWR);
 	len = read(fd, str, LENGTH); /* 读取文件内容*/
 	str[len] = '\0';
 	printf("%s\n", str);
 	close(fd);
 }
