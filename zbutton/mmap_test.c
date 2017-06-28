#include <stdio.h>
#include <stdlib.h>
#include <string.h>
    #include <fcntl.h>
    #include <sys/mman.h>//mmap head file
    int main ()//int argc, char* argv[])
    {
       int i,j;
       int fd;
       char *start;
       char *buf;// = "butterfly!";
       FILE *fp=NULL;
	int width = 64;
	int height = 64;
//	char *filename;

       //test image
	buf = (char *)malloc (sizeof(char)*width*height); 
	for (i=0;i<width*height;i++)
	{
		buf[i] = i;
	}

       //open /dev/mem with read and write mode
       fd = open ("/dev/mem", O_RDWR);
       if (fd < 0)
       {
           printf("cannot open /dev/mem.");
           return -1;
       }
	
       //map physical memory 
       start = mmap(NULL, 0xffff, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0x08000000);
	if (start == 0)
	{
		printf("NULL pointer!\n");
	 }
	 else
	 {
		printf("Successfull!\n");
	 }

	fp=fopen("/mnt/jpg.raw", "wb");
	if(NULL==fp)
	{
		return -1;
 		printf("file fails!\n");
	}
       fwrite(start, width*height, 1, fp);
   	fclose(fp);

	for (i = 0; i < width*height; i++)
       {
           start[i] = buf[i];
       }
	fp=fopen("/mnt/demoimage.raw", "wb");
	if(NULL==fp)
	{
		return -1;
 		printf("file fails!\n");
	}
       fwrite(start, width*height, 1, fp);
   	fclose(fp);
	
 printf("done!\n");

       munmap(start, width*height); //destroy map memory
       close(fd);  //close file
       return 0;
    }

