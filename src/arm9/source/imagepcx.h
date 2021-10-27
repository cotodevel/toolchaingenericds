/*---------------------------------------------------------------------------------

 	Copyright (C) 2005
		Jason Rogers (dovoto)
		Dave Murphy (WinterMute)

	This software is provided 'as-is', without any express or implied
	warranty.  In no event will the authors be held liable for any
	damages arising from the use of this software.

	Permission is granted to anyone to use this software for any
	purpose, including commercial applications, and to alter it and
	redistribute it freely, subject to the following restrictions:

	1.	The origin of this software must not be misrepresented; you
		must not claim that you wrote the original software. If you use
		this software in a product, an acknowledgment in the product
		documentation would be appreciated but is not required.
	2.	Altered source versions must be plainly marked as such, and
		must not be misrepresented as being the original software.
	3.	This notice may not be removed or altered from any source
		distribution.

---------------------------------------------------------------------------------*/

#ifndef __imagepcx9_h__
#define __imagepcx9_h__

#include "typedefsTGDS.h"
#include "dsregs.h"
#include "limitsTGDS.h"
#include "dldi.h"
#include "utilsTGDS.h"

typedef struct PCXHeader
{
   char         manufacturer;   //should be 0
   char         version;        //should be 5
   char         encoding;       //should be 1
   char         bitsPerPixel; //should be 8
   short int    xmin,ymin;      //coordinates for top left,bottom right
   short int    xmax,ymax;
   short int    hres;           //resolution
   short int    vres;
   char         palette16[48];  //16 color palette if 16 color image
   char         reserved;       //ignore
   char         colorPlanes;   //ignore
   short int    bytesPerLine;
   short int    paletteYype;   //should be 2
   char         filler[58];     //ignore
}__attribute__ ((packed)) PCXHeader, *pPCXHeader;

//!	\brief holds a red green blue triplet
typedef struct RGB_24
{
	unsigned char r;	//!< 8 bits for the red value.
	unsigned char g;	//!< 8 bits for the green value.
	unsigned char b;	//!< 8 bits for the blue value.
} __attribute__ ((packed)) RGB_24;

//!	A generic image structure.
typedef struct sImage
{
	short height; 				/*!< \brief The height of the image in pixels */
	short width; 				/*!< \brief The width of the image in pixels */
	int bpp;					/*!< \brief Bits per pixel (should be 4 8 16 or 24) */
	unsigned short* palette;	/*!< \brief A pointer to the palette data */

	//! A union of data pointers to the pixel data.
	union
	{
		u8* data8;		//!< pointer to 8 bit data.
		u16* data16;	//!< pointer to 16 bit data.
		u32* data32;	//!< pointer to 32 bit data.
	} image;

} sImage, *psImage;


#ifdef __cplusplus
extern "C" {
#endif

extern int textureID;
extern void imageDestroy(sImage* img);
extern void image8to16(sImage* img);
extern int LoadGLTextures(u8 * textureSource);
extern int loadPCX(const unsigned char* pcx, sImage* image);
extern int LoadLotsOfGLTextures(u32 * textureSourceArray, int * textureArray, int textureCount);

#ifdef __cplusplus
}
#endif

#endif
