	#include <stdio.h>	
	#include <stdlib.h>
	#include <string.h>
    	#include <fcntl.h>
    	#include <sys/mman.h>//mmap head file

	#include "jpg_header.h"
	#include "get_time.h"
	#include "global_def.h"
	#include "global_api.h"
	#include "bmp_ops.h"


	char *INT_DEV= "/dev/PL";
	char *MEM_DEV= "/dev/mem";

//	unsigned int MAP_SIZE_SRC = 2*m_image_x*m_image_y;
	unsigned int MAP_SIZE_JPG = 1024*1024;


    	int main (int argc, char* argv[])
    	{
	       FILE *fp=NULL;
		int i,j;
	       int fd_pl_int,fd_mem;
	       unsigned char *start_src, *start_jpg, *start_motion;
	       unsigned char *start_baro;
	       unsigned char *start_jpg_size;
	       unsigned int jpg_size[1]={0};

		char sys_cmd[MAXSTRING];
		char moDayDir[MAXSTRING];
		char hourDir[MAXSTRING];
		char tm[MAXSTRING];
		char cmd[MAXSTRING];

		char FOLDER_NAME[MAXSTRING];
		char deviceID[5] = "0000";
		static Current_tm time_struct={"0000","000","00","00","00","00"};
		char imFile[MAXSTRING];
		char yrMonDayHrMinSec[MAXSTRING];
		char baroFile_old[MAXSTRING];
		char baroFile_new[MAXSTRING];

		char config_file[MAXSTRING];
		option_struct  *global_options;
		
		char rtc_cmd[MAXSTRING];
		char *deviceIDfile="/mnt/system/DeviceID";//"/root/DeviceID";//
		FILE *idFid;

		int int_num_cur;
		int int_num_new;
		char src_en;
		char jpg_en;
		char baro_en;
		char motion_en;

		int image_width;
		int image_height;
		unsigned char quality_sl;

		int src_size;
		int baro_size;
		int motion_size;

		idFid=fopen(deviceIDfile,"r");
		if (idFid!=NULL)
		{
			fgets(deviceID, 4, idFid);
			fclose(idFid);
		}
		printf("/**************************************************************************/\n");
		printf("DeviceID = %s\n", deviceID);

		//************************************************** Get configuration infos *************************************************************************//
		if (( global_options = (option_struct *) malloc( sizeof(option_struct) ) ) == NULL) {
				printf("ERROR ALLOCATING global_options\n");
				exit(1);
		}
		/* Initialize global options */
		initialize_global(global_options);
		/* Parse the command options */
		cmd_proc(config_file,argc,argv);
		/* Read the configuration file to set the global options */
		get_param(global_options, config_file);

		//*************************************************** Set configuration directives *******************************************************************//
		if (global_options -> RTC_UPDATE)
		{
			sprintf(rtc_cmd,"%s%c%s","date",' ', global_options->RTC_TIME);
			system(rtc_cmd);
			system(hwclock_rtc);
		}

		//capture source image
		src_en = global_options->SRC_EN;
		//capture jpg image
		jpg_en = global_options->JPG_EN;
		
		baro_en = global_options->BAROMETER_ON;
		motion_en = global_options->INERTIAL_ON;

		//compression quality
		quality_sl = global_options->COMP_Q;

		if(global_options->RESOLUTION)
		{
			image_width = 1280;
			image_height = 960;
		}
		else
		{
			image_width = 640;
			image_height = 480;			
		}

		src_size = 2*image_width*image_height;
		baro_size = 8*16;//8bytes for each sample
		motion_size = 24*(global_options->inertialNum);

		//*******************************************************Open device********************************************************************//
		////open interruption
		fd_pl_int = open(INT_DEV,O_RDWR);
		if(fd_pl_int<0)
		{			
			perror("cannot open zbutton int\n");
			return 1;
		}

		////Open mem device
		//open /dev/mem with read and write mode
		fd_mem = open (MEM_DEV, O_RDWR|O_SYNC);//O_RDONLY);//
		if (fd_mem < 0)
		{
			printf("cannot open /dev/mem.");
			return -1;
		}

		//******************************************************** MMAP physical memory ***************************************************************//
		////map physical memory for jpg file
		if(jpg_en)
		{			
			
			start_jpg_size = (unsigned char *)mmap(NULL, 4, PROT_READ|MAP_LOCKED, MAP_PRIVATE, fd_mem, PHY_ADDR_jpgSIZE);//0x0e000000);//MAP_PRIVATE
			if (start_jpg_size == 0)
			{
				printf("NULL pointer!\n");
			}

			start_jpg = (unsigned char *)mmap(NULL, MAP_SIZE_JPG, PROT_READ|MAP_LOCKED, MAP_SHARED, fd_mem, PHY_ADDR_JPG);//0x0f000000);
			//start = mmap(NULL, file_size_jpg, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0x0f000000);
			if (start_jpg == 0)
			{
				printf("NULL pointer!\n");
			}
		}
		//map physical memory for source image
		if(src_en)
		{			
			start_src = (unsigned char *)mmap(NULL, src_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd_mem, PHY_ADDR_SRC);//0x0c000000);//MAP_PRIVATE,MAP_SHARED
			if (start_src == 0)
			{
				printf("NULL pointer!\n");
			}
		}
		//map physical memory for barometer data
		if(baro_en)
		{			
			start_baro = (unsigned char *)mmap(NULL, baro_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd_mem, PHY_ADDR_baro);
			if (start_baro == 0)
			{
				printf("NULL pointer!\n");
			}
		}
		//map physical memory for barometer data
		if(motion_en)
		{			
			start_motion = (unsigned char *)mmap(NULL, motion_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd_mem, PHY_ADDR_motion);
			if (start_motion == 0)
			{
				printf("NULL pointer!\n");
			}
		}



		//***************************************************************************************************************************//

		printf("/**************************************************************************/\n");

		int_num_cur=0;
		int_num_new=0;
		int_num_cur = poll_int(fd_pl_int);
		while(1)
		{
			////poll interruption from PL
			do
			{
				int_num_new = poll_int(fd_pl_int);
				for(i=0;i<100;i++)
					i=i;
			}while(int_num_cur==int_num_new);
	       
/*			printf("int_cur: %d\n", int_num_cur);
			printf("int_new: %d\n", int_num_new);

			printf("Taking image\n");

			for(i=0;i<100;i++)
				i=i;*/


			//************************************************* Path generation **************************************************************************//
			get_time(&time_struct);

			//printf("store data to %s\n",PATH); //PATH="/sdcard/"
			strcpy(moDayDir,PATH);
			//printf("moDayDir is %s\n",moDayDir);

			sprintf(FOLDER_NAME,"%s%s%c%s%c%02d","ID",deviceID, '_',time_struct.time_month,'.',atoi(time_struct.time_day));	 //month.day

			strcat(moDayDir,FOLDER_NAME);
			//printf("folder is %s\n",FOLDER_NAME);
			//printf("moDayDir is %s\n",moDayDir); 
			strcpy(sys_cmd,"mkdir -p ");
			strcat(sys_cmd,moDayDir);
			system(sys_cmd);

			sprintf(moDayDir,"%s%c",moDayDir,'/'); //moDayDir: sdcard/month.day/

			get_time(&time_struct); //update time

			/// create a folder named as the current hour 
			sprintf(hourDir,"%s%d%c",moDayDir,atoi(time_struct.time_hour),'/');// moDayDir: sdard/month.day/; folderHead: hour
			//printf("hourDir is %s\n",hourDir);
			strcpy(cmd,"mkdir -p ");
			strcat(cmd,hourDir);
			system(cmd);
			//******************************************************* end ********************************************************************//

			get_time(&time_struct);
				sprintf(yrMonDayHrMinSec,"%s%s%02d%c%s%s%s",time_struct.time_year,time_struct.time_month,atoi(time_struct.time_day),'_',time_struct.time_hour,time_struct.time_min, time_struct.time_sec);
				//strcat(yrMonDayHrMinSec,".dat");

			//******************************************************* save src  ********************************************************************//
			if(src_en)//read source file
			{

/*				get_time(&time_struct);
				sprintf(yrMonDayHrMinSec,"%s%s%02d%c%s%s%s",time_struct.time_year,time_struct.time_month,atoi(time_struct.time_day),'_',time_struct.time_hour,time_struct.time_min, time_struct.time_sec);
				//strcat(yrMonDayHrMinSec,".dat");*/
				sprintf(imFile,"%s%s%c%s%s",hourDir,deviceID,'_',yrMonDayHrMinSec,srcfileTail);

				src_save(imFile, start_src, image_width, image_height);
			}
		
			//******************************************************* save jpg  ********************************************************************//
			if(jpg_en)//read jpg file
			{
				//jpg_size = *start_jpg_size;
				memcpy(jpg_size,start_jpg_size,4);
				
				printf("jpg size = %d bytes, %d\n", jpg_size[0],jpg_size[0]%128);
				
				//file name	
/*				get_time(&time_struct);
				sprintf(yrMonDayHrMinSec,"%s%s%02d%c%s%s%s",time_struct.time_year,time_struct.time_month,atoi(time_struct.time_day),'_',time_struct.time_hour,time_struct.time_min, time_struct.time_sec);
				//strcat(yrMonDayHrMinSec,".jpg");//".dat");//*/
				sprintf(imFile,"%s%s%c%s%s",hourDir,deviceID,'_',yrMonDayHrMinSec,jpgfileTail);

				jpg_save(imFile, start_jpg, jpg_size[0], quality_sl, image_width, image_height);				
			}

			//******************************************************* save barometer  ********************************************************************//
			if(baro_en)
			{
				
				sprintf(baroFile_new,"%s%s%c%s%s%s",hourDir,deviceID,'_',"baro",yrMonDayHrMinSec,".txt");
/*				if(baroFile_old != baroFile_new)
				{				
					fp=fopen(baroFile_new, "wb");
					if(NULL==fp)
					{
						return -1;
						printf("file fails!\n");
					}

					fprintf(fp, "pressure(hectopascal), temperature(Celsius Degree)\n");
					fclose(fp);
				}*/

				BARO_save(baroFile_new,start_baro);	
				memcpy(baroFile_old, baroFile_new, MAXSTRING);
			}

			int_num_cur = int_num_new;
		
		}
		
		//////////////////////////////////////////////////////////////////////////
		if(jpg_en)
		{
			munmap(start_jpg_size, 4); 
			munmap(start_jpg, MAP_SIZE_JPG); //destroy map memory
		}
		if(src_en)
			munmap(start_src, 2*image_width*image_height); //destroy map memory



		close(fd_mem);  //close file

		close(fd_pl_int);

	       return 0;
    	}

	int poll_int(int fd)
	{
	
		int int_num;
		char buff;

		buff = read(fd, &int_num, sizeof(int));
		
		return int_num;

	}

	int jpg_save(char *imFile, unsigned char *start_jpgstream, unsigned int jpg_size, unsigned char quality_sl, int image_width, int image_height)
	{
		unsigned char image_size[4];
		FILE *fp=NULL;

		//create file
		fp=fopen(imFile, "wb");
		//fp=fopen("/mnt/zButton/pic.jpg", "wb");
		if(NULL==fp)
		{
			return -1;
			printf("file fails!\n");
		}

		//jpg header
		////fwrite(HEADER, 623, sizeof(unsigned char), fp);
		fwrite(&header_part1[quality_sl], 163, sizeof(unsigned char), fp);//header
		image_size[0] = (uint8)(image_height>>8);
		image_size[1] = (uint8)(image_height & 0xFF);
		image_size[2] = (uint8)(image_width>>8);
		image_size[3] = (uint8)(image_width & 0xFF);
		fwrite(image_size, 4, sizeof(unsigned char), fp);//image size
		fwrite(&header_part2[quality_sl], 456, sizeof(unsigned char), fp);//header

		//jpg stream
		fwrite(start_jpgstream, jpg_size, 1, fp);//

		///clear buffer
		//memset(start,0x00,file_size);

		fflush(fp);

		//close file
		fclose(fp);
		printf("jpg file is saved!\n");

		return 0;
	}

	int src_save(char * imFile, unsigned char *start_src, int image_width, int image_height)
	{
		FILE *fp=NULL;
		unsigned char *RGB;

		/*fp=fopen(imFile, "wb");
		if(NULL==fp)
		{
			return -1;
			printf("file fails!\n");
		}

		//source image data
		fwrite(start_src, src_size, 1, fp);

		fflush(fp);
		fclose(fp);*/

		RGB = (unsigned char *)(malloc(sizeof(unsigned char) * image_height * image_width * 3 ));
		YCBCR422_RGB(start_src, RGB, image_width, image_height);
		BMP_save(imFile, RGB, image_width, image_height);

		printf("source image is saved!\n");

		return 0;
	}

	int BARO_save(char *imFile, unsigned char *start_baro)
	{
		FILE *fp=NULL;
		unsigned char baro_data[4*2*16];
		float temperature;
		unsigned int pressure;
		int i;

		//create file
		fp=fopen(imFile, "wb");
		if(NULL==fp)
		{
			return -1;
			printf("file fails!\n");
		}

		fseek(fp,0, SEEK_END);

		memcpy(baro_data, start_baro, 4*32);
		
		for(i = 0;i < 16; i++)
		{
			pressure = (unsigned int)((baro_data[8*i+3]<<24)+(baro_data[8*i+2]<<16)+(baro_data[8*i+1]<<8)+baro_data[8*i]);
			temperature = ((float)((baro_data[8*i+7]<<24)+(baro_data[8*i+6]<<16)+(baro_data[8*i+5]<<8)+baro_data[8*i+4]))/10.0;

			//fprintf(fp, "%9f,", temperature);
			//fprintf(fp, "%9f\n", pressure);	
			fprintf(fp, "%u,%5.2f\n", pressure, temperature);	
			printf("%u,%5.2f\n", pressure, temperature);				
		}

		//flush data
		fflush(fp);

		//close file
		fclose(fp);
		//printf("baro file is saved!\n");

		return 0;
	}




