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

#include "imagepcx.h"
#include "dsregs.h"
#include "dsregs_asm.h"
#include "typedefsTGDS.h"
#include <stdio.h>
#include "posixHandleTGDS.h"
#include "VideoGL.h"
#include "videoTGDS.h"
#include "Texture_Cube.h"

int textureID=0;
//---------------------------------------------------------------------------------
void imageDestroy(sImage* img) {
//---------------------------------------------------------------------------------
	if(img->image.data8) TGDSARM9Free (img->image.data8);
	if(img->palette && img->bpp == 8) TGDSARM9Free (img->palette);
}

//---------------------------------------------------------------------------------
void image8to16(sImage* img) {
//---------------------------------------------------------------------------------
	int i;
	u16* temp = (u16*)TGDSARM9Malloc(img->height*img->width*2);

	for(i = 0; i < img->height * img->width; i++)
		temp[i] = img->palette[img->image.data8[i]] | (1<<15);

	TGDSARM9Free (img->image.data8);
	TGDSARM9Free (img->palette);

	img->palette = NULL;

	img->bpp = 16;
	img->image.data16 = temp;
}

//---------------------------------------------------------------------------------
int loadPCX(const unsigned char* pcx, sImage* image) {
//---------------------------------------------------------------------------------
	//struct rgb {unsigned char b,g,r;};
	RGB_24* pal;
	
	PCXHeader* hdr = (PCXHeader*) pcx;

	pcx += sizeof(PCXHeader);
	
	unsigned char c;
	int size;
	int count;
	int run;
	int i;
	int iy;
	int width, height;
	int scansize = hdr->bytesPerLine;
	unsigned char *scanline;


	width = image->width  = hdr->xmax - hdr->xmin + 1 ;
	height = image->height = hdr->ymax - hdr->ymin + 1;

	size = image->width * image->height;

	if(hdr->bitsPerPixel != 8)
		return 0;
	
	scanline = image->image.data8 = (unsigned char*)TGDSARM9Malloc(size);
	image->palette = (unsigned short*)TGDSARM9Malloc(256 * 2);

	count = 0;

	for(iy = 0; iy < height; iy++) {
		count = 0;
		while(count < scansize)
		{
			c = *pcx++;
			
			if(c < 192) {
				scanline[count++] = c;
			} else {
				run = c - 192;
			
				c = *pcx++;
				
				for(i = 0; i < run && count < scansize; i++)
					scanline[count++] = c;
			}
		}
		scanline += width;
	}

	//check for the palette marker.
	//I have seen PCX files without this, but the docs don't seem ambiguous--it must be here.
	//Anyway, the support among other apps is poor, so we're going to reject it.
	if(*pcx != 0x0C)
	{
		TGDSARM9Free(image->image.data8);
		image->image.data8 = 0;
		TGDSARM9Free(image->palette);
		image->palette = 0;
		return 0;
	}

	pcx++;

	pal = (RGB_24*)(pcx);

	image->bpp = 8;

	for(i = 0; i < 256; i++)
	{
		u8 r = (pal[i].r + 4 > 255) ? 255 : (pal[i].r + 4);
		u8 g = (pal[i].g + 4 > 255) ? 255 : (pal[i].g + 4);
		u8 b = (pal[i].b + 4 > 255) ? 255 : (pal[i].b + 4);
		image->palette[i] = RGB15(r >> 3 , g >> 3 , b >> 3) ;
	}
	return 1;
}

int LoadGLTextures(u8 * textureSource)									// Load PCX files And Convert To Textures
{
	sImage pcx;

	//load our texture
	loadPCX((u8*)textureSource, &pcx);
	image8to16(&pcx);

	//DS supports no filtering of anykind so no need for more than one texture
	glGenTextures(1, &textureID);
	glBindTexture(0, textureID);
	glTexImage2D(0, 0, GL_RGB, TEXTURE_SIZE_128 , TEXTURE_SIZE_128, 0, TEXGEN_TEXCOORD, pcx.image.data8);

	imageDestroy(&pcx);

	return 0;
}

//Usage:
//Load 2 textures and map each one to a texture slot
//u32 arrayOfTextures[2]; 
//arrayOfTextures[0] = (u32)&Texture_Cube; //textures required to be in memory.
//arrayOfTextures[1] = (u32)&Texture_Cellphone;
//int textureArrayNDS[2]; //0 : Cube tex / 1 : Cellphone tex... enumerated as NDS GX texture format
//int texturesInSlot = LoadLotsOfGLTextures((u32*)&arrayOfTextures, (int*)&textureArrayNDS, 2);	

//returns: texture count generated
int LoadLotsOfGLTextures(u32 * textureSourceArray, int * textureArray, int textureCount){	// Load lots of PCX files And Convert To Textures
	glGenTextures(textureCount, textureArray);
	int curTexture = 0;
	for(curTexture = 0; curTexture < textureCount; curTexture++){
		glBindTexture(0, textureArray[curTexture]);
		sImage pcx;
		//load our texture
		loadPCX((u8*)textureSourceArray[curTexture], &pcx);
		image8to16(&pcx);
		glTexImage2D(0, 0, GL_RGB, TEXTURE_SIZE_64 , TEXTURE_SIZE_64, 0, TEXGEN_TEXCOORD, pcx.image.data8); //maps textures automatically to VRAM map
		imageDestroy(&pcx);	
	}
	return curTexture;
}