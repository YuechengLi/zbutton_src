	#include <stdio.h>	
	#include <stdlib.h>
	#include <string.h>
    	#include <fcntl.h>
    	#include <sys/mman.h>//mmap head file
	#include "jpg_header.h"
	#include "get_time.h"
	#include "global_def.h"

    	int main ()//int argc, char* argv[])
    	{
	       int i,j;
	       int fd;
	       char *start;
	       char *buf;// = "butterfly!";
	       FILE *fp=NULL;
		int file_size;
		static Current_tm time_struct={"0000","000","00","00","00","00"};
		char imFile[MAXSTRING];
		char yrMonDayHrMinSec[MAXSTRING];

		char jpg_en=1;

	//	char *filename;

		while(1)
		{
	       
			if(1)//write source file
			{
				file_size = 2*m_image_x*m_image_y;

				//open /dev/mem with read and write mode
			       fd = open ("/dev/mem", O_RDWR);
			       if (fd < 0)
			       {
				   printf("cannot open /dev/mem.");
				   return -1;
			       }
	
			       //map physical memory 
			       start = mmap(NULL, file_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0x0c000000);
				if (start == 0)
				{
					printf("NULL pointer!\n");
				 }

				get_time(&time_struct);
				sprintf(yrMonDayHrMinSec,"%s%s%02d%c%s%s%s",time_struct.time_year,time_struct.time_month,atoi(time_struct.time_day),'_',time_struct.time_hour,time_struct.time_min, time_struct.time_sec);
				strcat(yrMonDayHrMinSec,".dat");
				fp=fopen(yrMonDayHrMinSec, "wb");
				if(NULL==fp)
				{
					return -1;
			 		printf("file fails!\n");
				}

				//source image data
			       	fwrite(start, file_size, 1, fp);
			   	fclose(fp);

			 	printf("source image is saved!\n");

			       munmap(start, file_size); //destroy map memory
			       close(fd);  //close file
			}
		
			if(jpg_en)//write jpg file
			{
				file_size = m_image_x*(m_image_y);

				//open /dev/mem with read and write mode
			       fd = open ("/dev/mem", O_RDWR);
			       if (fd < 0)
			       {
				   printf("cannot open /dev/mem.");
				   return -1;
			       }
	
			       //map physical memory 
			       start = mmap(NULL, file_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0x0f000000);
				if (start == 0)
				{
					printf("NULL pointer!\n");
				 }

				get_time(&time_struct);
				sprintf(yrMonDayHrMinSec,"%s%s%02d%c%s%s%s",time_struct.time_year,time_struct.time_month,atoi(time_struct.time_day),'_',time_struct.time_hour,time_struct.time_min, time_struct.time_sec);
				strcat(yrMonDayHrMinSec,".jpg");
				fp=fopen(yrMonDayHrMinSec, "wb");
				//fp=fopen("/mnt/zButton/pic.jpg", "wb");
				if(NULL==fp)
				{
					return -1;
			 		printf("file fails!\n");
				}

				//jpg header
				fwrite(HEADER, 623, sizeof(unsigned char), fp);

				//jpg stream
			       	fwrite(start, file_size, 1, fp);
			   	fclose(fp);

				////clear buffer
				memset(start,0x00,file_size);

			 	printf("jpg file is saved!\n");

			       munmap(start, file_size); //destroy map memory
			       close(fd);  //close file

				jpg_en = 0;
			}

			for(j=0;j<30000000;j++)
			{
				j=j;
			}
		}
	       return 0;
    	}

