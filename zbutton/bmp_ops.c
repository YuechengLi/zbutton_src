#include <stdio.h>
#include "bmp_ops.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define        Ok            0
#define        False        1
#define        Error        -1

int YR = 19595, YG = 38470, YB = 7471, CB_R = -11059, CB_G = -21709, CB_B = 32768, CR_R = 32768, CR_G = -27439, CR_B = -5329;
uint8 clamp(int i)
{
	if ((uint32)(i) > 255U)
	{
		if (i < 0)
			i = 0;
		else if (i > 255)
			i = 255;
	}
	return (uint8)(i);
}

void RGB_to_YCC(uint8 *pDst_Y,uint8 *pDst_U,uint8 *pDst_V, uint8 *pSrc, int num_pixels)
{
	uint8 *pY,*pU,*pV,*pS;
	pY = pDst_Y;
	pU = pDst_U;
	pV = pDst_V;
	pS = pSrc;
  for ( ; num_pixels; pS += 3, num_pixels--)
  {
    int r = pS[0], g = pS[1], b = pS[2];
    *pY++ = (uint8)((r * YR + g * YG + b * YB + 32768) >> 16);
    *pU++ = clamp(128 + ((r * CB_R + g * CB_G + b * CB_B + 32768) >> 16));
    *pV++ = clamp(128 + ((r * CR_R + g * CR_G + b * CR_B + 32768) >> 16));
  }
}

void YCBCR422_RGB(unsigned char *ycbcr422, unsigned char *g_paa, int w, int h)
{
	double Y,Cb,Cr;
	int i,j;
	unsigned char *pDst;
	pDst = g_paa;

	 for(i=0;i<h;i++)
		 for(j=0;j<w;j++)
		 {
			 Y = ycbcr422[(i*w+j)*2+1];
			if(j%2)
			{				
				Cb = ycbcr422[(i*w+j-1)*2];
				Cr = ycbcr422[(i*w+j)*2];
			}
			else
			{
				Cb = ycbcr422[(i*w+j)*2];
				Cr = ycbcr422[(i*w+j+1)*2];
			}

			*pDst++ = clamp((int)(Y + 1.40200 * (Cr - 128)));
			*pDst++ = clamp((int)(Y - 0.34414 * (Cb - 128) - 0.71414 * (Cr - 128)));
			*pDst++ = clamp((int)(Y + 1.77200 * (Cb - 128)));
		 }
}

void LoadBmp( char *cName, unsigned char *g_paa, int *w, int *h )
{
	FILE *fp = 0;
	BMPFILEHEADER     fileheader;
	BMPINF     infoheader;
	int g_nWidth = 0;                // image width
	int g_nHeight = 0;                // image height
	int g_nSize = 0;                // read image data byte count

	int ni = 0 ;
	int nj = 0 ;        // no user

	int LineBytes ;

	unsigned char PadZeros[4] ;

	if ( cName == NULL )
	{
	printf( "[LoadBmp]:please enter right file name\n" );
	return ;
	}

	if ( ( fp = fopen( cName, "rb" ) ) == NULL )
	{
	printf( "load bitmap false \n" );

	return ;
	}

	fread( &fileheader, sizeof( fileheader), 1, fp );            // read bmp file header information
	fread( &infoheader, sizeof( infoheader), 1, fp );            // read bmp info header informaiton

	g_nWidth  = infoheader.bWidth;                // picture width
	g_nHeight = infoheader.bHeight;                // picture height

	LineBytes = ( g_nWidth * 24 + 31) / 32 * 4;

	if ( fileheader.bType != 0x4D42 )            // check picture is or not bmp
	{
		printf( "this pic is not bmp\n" );
		fclose(fp);
		fp = NULL ;
		return ;
	}

	if ( infoheader.bCompression == 1 )            // check picture is or not compression
	{
	printf( "this pic is Compression\n" );
	fclose(fp);
	fp = NULL ;
	return ;
	}

	fseek( fp, fileheader.bOffset, 0 );

	//    g_nSize = fread( g_paa, sizeof(unsigned char), (long) g_nHeight * g_nWidth * 3, fp );

	for ( ni = 0 ; ni < g_nHeight ; ni ++   )
	{
		for ( nj = 0 ; nj < g_nWidth  ; nj ++ )
		{
			fread( g_paa + (((g_nHeight-1-ni) * g_nWidth) + nj) *3+2 , 1, 1, fp ) ;
			fread( g_paa + (((g_nHeight-1-ni) * g_nWidth) + nj) *3 + 1, 1, 1, fp ) ;

			fread( g_paa + (((g_nHeight-1-ni) * g_nWidth) + nj) *3, 1, 1, fp ) ;
		}
		if ( (LineBytes - g_nWidth * 3) > 0 )
		{
			fread(PadZeros, sizeof(unsigned char), (LineBytes - g_nWidth * 3), fp);
		}
	}

	fclose(fp);
	fp = NULL ;

	*w = g_nWidth;
	*h = g_nHeight;
}



void BMP_save(char *cName, unsigned char *g_paa, int w, int h)
{
	FILE *f;
	int i,j;

	unsigned char bmpfileheader[14] = {'B','M', 0,0,0,0, 0,0, 0,0, 54,0,0,0};
	unsigned char bmpinfoheader[40] = {40,0,0,0, 0,0,0,0, 0,0,0,0, 1,0, 24,0};
	unsigned char bmppad[3] = {0,0,0};

	unsigned char *img = NULL;
	int filesize = 54 + 3*w*h;  //w is your image width, h is image height, both int
	if( img )
		free( img );
	img = (unsigned char *)malloc(3*w*h);
	memset(img,0,sizeof(img));

	for(i=0; i<h; i++)
	{
		for(j=0; j<w; j++)
		{
			img[((j)+(i)*w)*3+2] = (*g_paa++);
			img[((j)+(i)*w)*3+1] = (*g_paa++);
			img[((j)+(i)*w)*3+0] = (*g_paa++);
		}
	}

	bmpfileheader[ 2] = (unsigned char)(filesize    );
	bmpfileheader[ 3] = (unsigned char)(filesize>> 8);
	bmpfileheader[ 4] = (unsigned char)(filesize>>16);
	bmpfileheader[ 5] = (unsigned char)(filesize>>24);

	bmpinfoheader[ 4] = (unsigned char)(       w    );
	bmpinfoheader[ 5] = (unsigned char)(       w>> 8);
	bmpinfoheader[ 6] = (unsigned char)(       w>>16);
	bmpinfoheader[ 7] = (unsigned char)(       w>>24);
	bmpinfoheader[ 8] = (unsigned char)(       h    );
	bmpinfoheader[ 9] = (unsigned char)(       h>> 8);
	bmpinfoheader[10] = (unsigned char)(       h>>16);
	bmpinfoheader[11] = (unsigned char)(       h>>24);

	f = fopen(cName,"wb");
	fwrite(bmpfileheader,1,14,f);
	fwrite(bmpinfoheader,1,40,f);
	for(i=0; i<h; i++)
	{
		fwrite(img+(w*(h-i-1)*3),3,w,f);
		fwrite(bmppad,1,(4-(w*3)%4)%4,f);
	}
	fclose(f);
}
