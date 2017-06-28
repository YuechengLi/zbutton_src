#include <stdio.h>
#include <fcntl.h>

int main(void)
{
        int fd;
        char *rdbuf;
        char *wrbuf = "abcdefghjk";
        int i;
        fd = open("/dev/mem",O_RDWR);
        if(fd < 0)
        {
            printf("open /dev/mem failed.");
        }
        read(fd,rdbuf,10);

        for(i = 0;i < 10;i++)
        {
            printf("old mem[%d]:%c\n",i,*(rdbuf + i));
        }
        lseek(fd,5,0);
        write(fd,wrbuf,10);
        lseek(fd,0,0);//move f_ops to the front
        read(fd,rdbuf,10);
        for(i = 0;i < 10;i++)
        {
            printf("new mem[%d]:%c\n",i,*(rdbuf + i));
        }

        return 0;
}

