	#ifndef BMP_OPS_H
	#define BMP_OPS_H


	typedef signed char   int8;
	typedef signed short   int16;
	typedef signed int     int32;
	typedef unsigned char  uint8;
	typedef unsigned short uint16;
	typedef unsigned int   uint32;

	#pragma pack(1)

	typedef struct BMP_FILE_HEADER
	{
		uint16 bType;
		uint32 bSize;
		uint16 bReserved1;
		uint16 bReserved2;
		uint32 bOffset;
	} BMPFILEHEADER;

	typedef struct BMP_INFO
	{
		uint32 bInfoSize;
		uint32 bWidth;
		uint32 bHeight;
		uint16 bPlanes;
		uint16 bBitCount;
		uint32 bCompression;
		uint32 bmpImageSize;
		uint32 bXPelsPerMeter;
		uint32 bYPelsPerMeter;
		uint32 bClrUsed;
		uint32 bClrImportant;
	} BMPINF;

	typedef struct RGB_QUAD
	{
		uint8 rgbBlue;
		uint8 rgbGreen;
		uint8 rgbRed;
		uint8 rgbReversed;
	} RGBQUAD;
	#pragma pack()
	

	void RGB_to_YCC(uint8 *pDst_Y,uint8 *pDst_U,uint8 *pDst_V, uint8 *pSrc, int num_pixels);
	void YCBCR422_RGB(unsigned char *ycbcr422, unsigned char *g_paa, int w, int h);
	void LoadBmp( char *cName, unsigned char *image_data, int *w, int *h );
	void BMP_save(char *cName, unsigned char *g_paa, int w, int h);


	#endif