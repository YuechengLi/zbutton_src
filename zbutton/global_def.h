/*
 * global_def.h
 *
 *  Created on: April 4, 2011
 *      Author: Yaofeng Yue
 */

#ifndef GLOBAL_DEF_H_
#define GLOBAL_DEF_H_

//#define MAXSTRING 100

char *jpgfileTail=".JPG";
char *srcfileTail = ".bmp";
char *hwclock_rtc="hwclock -w";

#define PATH "/mnt/Data/"
#define CONFIG_FILE_NAME "config"

//#define FOLDER_NAME "lcn_obesity"

#define MAX_PIC 40000
#define FOLDER_PIC 4000

#define PHY_ADDR_CFG      0x0E0f0000
#define PHY_ADDR_SRC      0x0C000000
#define PHY_ADDR_jpgSIZE  0x0E000000
#define PHY_ADDR_JPG      0x0F000000
#define PHY_ADDR_xadc     0x0fd00000
#define PHY_ADDR_baro     0x0fe00000
#define PHY_ADDR_motion   0x0ff00000


struct MPU9150_DATA
{
	float ACC_X;
	float ACC_Y;
	float ACC_Z;
	float TEMP;
	float GYRO_X;
	float GYRO_Y;
	float GYRO_Z;
	float MAG_X;
	float MAG_Y;
	float MAG_Z;
	char ASA_X;
	char ASA_Y;
	char ASA_Z;
	float ACTUAL_ACC_X;
	float ACTUAL_ACC_Y;
	float ACTUAL_ACC_Z;

	};


int poll_int(int fd);
int jpg_save(char *imFile, unsigned char *start_jpgstream, unsigned int jpg_size, unsigned char quality_sl, int image_width, int image_height);
int src_save(char * imFile, unsigned char *start_src, int image_width, int image_height);
int XADC_save(char *imFile, unsigned int xadc_fs, unsigned short *start_xadc, char header);
int BARO_save(unsigned char *gpio, char *imFile, unsigned char baro_fs, unsigned int *start_baro, char header);

int MOTION_save(char *imFile, unsigned short motion_fs, short *start_motion, char header);
int MOTION_save_sixaxis(char *imFile, unsigned short motion_fs, short *start_motion, char header);

void getGravity();

#endif /* GLOBAL_DEF_H_ */

