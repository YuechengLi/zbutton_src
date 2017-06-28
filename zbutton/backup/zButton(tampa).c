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


	char first_loop = 1;

	char *Serial1_DEV= "/dev/ttyPS1";
	char *INT_DEV= "/dev/PL";
	char *MEM_DEV= "/dev/mem";

//	unsigned int MAP_SIZE_SRC = 2*m_image_x*m_image_y;
	unsigned int MAP_SIZE_JPG = 1024*1024;


    	int main (int argc, char* argv[])
    	{
	       FILE *fp=NULL;
		unsigned int num_loops=0;
		int i,j;
	       int fd_pl_int,fd_mem, fd_serial1;
	       unsigned char *start_src, *start_jpg;
	       short *start_motion;
	       unsigned int *start_baro;
	       unsigned int *start_jpg_size, *start_cfg, *tmp_int, configuration_int;
	       unsigned int jpg_size;
		unsigned char config_pl[8]={0x12,0x34,0,0,0,0,0,0};		

		char sys_cmd[MAXSTRING];
		char moDayDir[MAXSTRING];
		char hourDir[MAXSTRING];
		char hourDir_baro[MAXSTRING];
		char tm[MAXSTRING];
		char cmd[MAXSTRING];

		char FOLDER_NAME[MAXSTRING];
		char deviceID[5] = "0000";
		static Current_tm time_struct={"0000","000","00","00","00","00"};
		char imFile[MAXSTRING];
		char yrMonDayHrMinSec[MAXSTRING];
		char baroFile_old[MAXSTRING];
		char baroFile_new[MAXSTRING];
		char motionFile_new[MAXSTRING];
		unsigned char motion_fs = 80;

		char config_file[MAXSTRING];
		option_struct  *global_options;
		
		char rtc_cmd[MAXSTRING];
		char *deviceIDfile="/mnt/system/DeviceID";//"/root/DeviceID";//
		FILE *idFid;

		unsigned int cfg_tmp;
		int int_num_cur;
		int int_num_new;
		char src_en;
		char four_cam_mode;
		char jpg_en;
		char baro_en;
		char motion_en;
		char motion_sp;

		int image_width;
		int image_height;
		char quality_sl;
		char resolution_sl;

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
		//printf("inertial_ON is %d\n",motion_en);
		//printf("baro_ON is %d\n",baro_en);

		motion_sp = global_options->inertialNum;
		motion_fs =  (20*(motion_sp+1));

		//compression quality
		quality_sl = global_options->COMP_Q;

		resolution_sl = global_options->RESOLUTION;
		if(!resolution_sl)
		{
			image_width = 1280;
			image_height = 960;
		}
		else
		{
			image_width = 640;
			image_height = 480;			
		}

		//four camera loop mode
		four_cam_mode = global_options->FOUR_CAMERA_MODE;
		if(four_cam_mode)
		{
			resolution_sl = 1;//vga

			image_width = 640;
			image_height = 4*480;	//stitching		
		}

		src_size = (2*image_width*image_height);
		baro_size = (8*16);//8bytes for each sample
		motion_size = (12*(motion_fs));

		//loops before reboot
		num_loops = (unsigned int)(60*(global_options->reboot_interval_mins));

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
		////map physical memory for configuration 						
/*		start_cfg = (unsigned int *)mmap(NULL, 4, PROT_WRITE|MAP_LOCKED, MAP_PRIVATE, fd_mem, PHY_ADDR_CFG);
		if (start_cfg == 0)
		{
			printf("NULL pointer!\n");
			return -1;
		}*/

		fd_serial1 = open(Serial1_DEV, O_RDWR);
		if(fd_serial1<0)
		{			
			perror("cannot open serial port1!\n");
			return 1;
		}

				
		config_pl[2] = (unsigned char)((((unsigned int)motion_en)<<6)+(((unsigned int)baro_en)<<5)+(((unsigned int)quality_sl)<<3)+(((unsigned int)jpg_en)<<2)+(((unsigned int)resolution_sl)<<1)+(unsigned int)src_en);
		config_pl[3] = (unsigned char)motion_sp;

		for(i=0;i<8;i++)
			printf("%x\n",config_pl[i]);

/*		for(j=0;j<1000;j++)
		{
			for(i=0;i<8;i++)
			printf("%d",config_pl[i]);
		}
*/
		//////write 1MB to flush data out of cache
//		tmp_int = start_cfg;
		//for(i=0;i<256*1024;i++)	
		//{	
//			*tmp_int = 0x550000+cfg_tmp;
//			printf("configuration=%d\n",*tmp_int);
			//tmp_int++;
		//}

		///////////////////
		//__clear_cache(start_cfg,start_cfg+4);//disable cache
		//fflush(stdout);//flush out cache
		//////////////////

//		configuration_int = 0x550000+cfg_tmp;

//		printf("configuration=%d\n",configuration_int);
//		munmap(start_cfg, 4);
		

		////map physical memory for jpg file
		if(jpg_en)
		{			
			
			start_jpg_size = (unsigned int *)mmap(NULL, 4, PROT_READ|MAP_LOCKED, MAP_PRIVATE, fd_mem, PHY_ADDR_jpgSIZE);//0x0e000000);//MAP_PRIVATE
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
			start_baro = (unsigned int *)mmap(NULL, baro_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd_mem, PHY_ADDR_baro);
			if (start_baro == 0)
			{
				printf("NULL pointer!\n");
			}
		}
		//map physical memory for motion data
		if(motion_en)
		{			
			start_motion = (short *)mmap(NULL, motion_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd_mem, PHY_ADDR_motion);
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
		while(num_loops>1)//1)
		{
			num_loops--;

			////poll interruption from PL
			do
			{
				int_num_new = poll_int(fd_pl_int);
				for(i=0;i<100;i++)
					i=i;
			}while(int_num_cur==int_num_new);	      


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
				jpg_size = *start_jpg_size;
				//memcpy(jpg_size,start_jpg_size,4);
				
				//printf("jpg size = %d bytes, %d\n", jpg_size[0],jpg_size[0]%128);
				printf("jpg size = %d bytes, %d\n", jpg_size,jpg_size%128);
				
				//file name	
/*				get_time(&time_struct);
				sprintf(yrMonDayHrMinSec,"%s%s%02d%c%s%s%s",time_struct.time_year,time_struct.time_month,atoi(time_struct.time_day),'_',time_struct.time_hour,time_struct.time_min, time_struct.time_sec);
				//strcat(yrMonDayHrMinSec,".jpg");//".dat");//*/
				sprintf(imFile,"%s%s%c%s%s",hourDir,deviceID,'_',yrMonDayHrMinSec,jpgfileTail);

				jpg_save(imFile, start_jpg, jpg_size, quality_sl, image_width, image_height);				
			}

			//******************************************************* save barometer  ********************************************************************//
			if(baro_en)
			{				
				//sprintf(baroFile_new,"%s%s%c%s%s%s",hourDir_baro,deviceID,'_',"baro_",yrMonDayHrMinSec,".txt");

				sprintf(baroFile_new,"%s%s%c%s%s",hourDir,deviceID,'_',"baro", ".txt");

				BARO_save(baroFile_new,start_baro, first_loop);	
				//memcpy(baroFile_old, baroFile_new, MAXSTRING);
			}
			//******************************************************* save motion  ********************************************************************//
			if(motion_en)
			{				
				//sprintf(motionFile_new,"%s%s%c%s%s%s",hourDir,deviceID,'_',"motion_",yrMonDayHrMinSec, ".txt");
				sprintf(motionFile_new,"%s%s%c%s%s",hourDir,deviceID,'_',"motion", ".txt");

				MOTION_save(motionFile_new, motion_fs, start_motion, first_loop);	
				//memcpy(baroFile_old, baroFile_new, MAXSTRING);
			}			

			first_loop = 0;

			int_num_cur = int_num_new;
		
		}
		
		//////////////////////////////////////////////////////////////////////////
		if(jpg_en)
		{
			munmap(start_jpg_size, 4); 
			munmap(start_jpg, MAP_SIZE_JPG); //destroy map memory
		}
		if(src_en)
			munmap(start_src, baro_size); //destroy map memory
		if(baro_en)
			munmap(start_baro, baro_size); //destroy map memory
		if(motion_en)
			munmap(start_motion, motion_size); //destroy map memory

		close(fd_mem);  //close file

		close(fd_pl_int);

		strcpy(cmd,"reboot");//poweroff
		system(cmd);

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

	int BARO_save(char *imFile, unsigned int *start_baro, char header)
	{
		FILE *fp=NULL;
		float temperature;
		unsigned int pressure;
		int i;

		//create file
		fp=fopen(imFile, "a+");//"wb");
		if(NULL==fp)
		{
			return -1;
			printf("file fails!\n");
		}

		if(header)
			fprintf(fp, "pressure(hectopascal), temperature(Celsius Degree)\n");
		
		for(i = 0;i < 16; i++)
		{
			pressure = (unsigned int)(start_baro[2*i]);
			temperature = ((float)(start_baro[2*i+1]))/10.0;
	
			fprintf(fp, "%u,%5.2f\n", pressure, temperature);	

			if(i==0)printf("%u,%5.2f\n", pressure, temperature);				
		}

		//flush data
		fflush(fp);

		//close file
		fclose(fp);
		//printf("baro file is saved!\n");

		return 0;
	}

	int MOTION_save(char *imFile, unsigned char motion_fs, short *start_motion, char header)
	{
		FILE *fp=NULL;
		int i;
		char tmp_55;
		struct MPU9150_DATA MPU9150_DATA;

		//create file
		fp=fopen(imFile, "a+");//"wb");
		if(NULL==fp)
		{
			return -1;
			printf("file fails!\n");
		}

		if(header)
			fprintf(fp, "ACC_X,ACC_Y,ACC_Z,TEMP,GYRO_X,GYRO_Y,GYRO_Z,MAG_X,MAG_Y,MAG_Z,ASA_X,ASA_Y,ASA_Z\n");
		
		for(i = 0;i < motion_fs; i++)
		{
			MPU9150_DATA.ACC_X = *(start_motion+3);
			MPU9150_DATA.ACC_Y = *(start_motion+2);
			MPU9150_DATA.ACC_Z = *(start_motion+1);
			MPU9150_DATA.TEMP = ((float)(*(start_motion))/340.0+35);
			MPU9150_DATA.GYRO_X = *(start_motion+7);
			MPU9150_DATA.GYRO_Y = *(start_motion+6);
			MPU9150_DATA.GYRO_Z = *(start_motion+5);
			MPU9150_DATA.MAG_X = *(start_motion+4);
			MPU9150_DATA.MAG_Y = *(start_motion+11);
			MPU9150_DATA.MAG_Z = *(start_motion+10);
			MPU9150_DATA.ASA_X = (char)(((*(start_motion+9))>>8)&0xff);
			MPU9150_DATA.ASA_Y = (char)((*(start_motion+9))&0xff);
			MPU9150_DATA.ASA_Z = (char)(((*(start_motion+8))>>8)&0xff);
			////tmp_55 = (char)((*(start_motion+8))&0xff);
			
			start_motion = start_motion + 12;			
	
			fprintf(fp, "%d,%d,%d,%5.2f,%d,%d,%d,%d,%d,%d,%d,%d,%d\n", MPU9150_DATA.ACC_X, MPU9150_DATA.ACC_Y, MPU9150_DATA.ACC_Z, MPU9150_DATA.TEMP, MPU9150_DATA.GYRO_X, MPU9150_DATA.GYRO_Y, MPU9150_DATA.GYRO_Z, MPU9150_DATA.MAG_X, MPU9150_DATA.MAG_Y, MPU9150_DATA.MAG_Z, MPU9150_DATA.ASA_X, MPU9150_DATA.ASA_Y, MPU9150_DATA.ASA_Z);	

			if(i==0)printf("%d,%d,%d,%5.2f,%d,%d,%d,%d,%d,%d,%d,%d,%d\n", MPU9150_DATA.ACC_X, MPU9150_DATA.ACC_Y, MPU9150_DATA.ACC_Z, MPU9150_DATA.TEMP, MPU9150_DATA.GYRO_X, MPU9150_DATA.GYRO_Y, MPU9150_DATA.GYRO_Z, MPU9150_DATA.MAG_X, MPU9150_DATA.MAG_Y, MPU9150_DATA.MAG_Z, MPU9150_DATA.ASA_X, MPU9150_DATA.ASA_Y, MPU9150_DATA.ASA_Z);
				
		}

		//flush data
		fflush(fp);

		//close file
		fclose(fp);
		//printf("baro file is saved!\n");

		return 0;
	}


