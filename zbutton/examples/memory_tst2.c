#include <stdio.h>
    #include <fcntl.h>
    #include <sys/mman.h>//mmap head file
    int main (void)
    {
       int i;
       int fd;
       char *start;
       char *buf = "butterfly!";
       FILE *fp=NULL;
		

       //open /dev/mem with read and write mode
       fd = open ("/dev/mem", O_RDWR);
       if (fd < 0)
       {
           printf("cannot open /dev/mem.");
           return -1;
       }

	

       //map physical memory 0-10 bytes 
       start = (char *)mmap(0, 10, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
       if(start < 0)
       {
          printf("mmap failed.");
          return -1;
       }

	for (i = 0; i < 10; i++)
       {
           start[i] = buf[i];
       }

       //open file and write data
       fp=fopen("/mnt/butterfly.raw", "wb");
	if(NULL==fp)
	{
		return -1;
	}
       fwrite(start, 10, 1, fp);
   	fclose(fp);
 	
       munmap(start, 10); //destroy map memory
       close(fd);  //close file
       return 0;
    }

