/*
 Copyright (C) 2009 by Kyle Poole <kyle.poole@gmail.com>
 Part of the Battle for Wesnoth Project http://www.wesnoth.org/
 
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License version 2
 or at your option any later version.
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY.
 
 See the COPYING file for more details.
 */


#include "RenderQueue.h"
#include "stdlib.h"
#include <vector>
#include <iostream>
#include <set>

#include "TextureAtlas.h"
#include "SDL_image.h"
#include "game_config.hpp"
#include "video.hpp"

#define QUEUE_TYPE_TEXTURE	0
#define QUEUE_TYPE_FILL		1
#define QUEUE_TYPE_LINE		2

void cacheTextureMode(bool isTextured);
extern void checkUnitTextureAtlas(void);

struct renderTextureInfo
{
	GLshort type;
	GLshort vertices[12];
	GLfloat texCoords[8];
	GLuint texture;
	GLshort z;
	unsigned long color;
	float brightness;	// 0..2, with 1 being normal, and 0 marked special as greyscale
	bool done;
};

struct dirtyTile
{
	short x;
	short y;
	bool operator<( const dirtyTile& other) const
	{
		return (x+y*100) < (other.x + other.y*100);
	}
	bool operator==( const dirtyTile& other) const
	{
		return (x == other.x && y == other.y);
	}
};
std::set<dirtyTile> mDirtyTiles;

std::vector<renderTextureInfo> mTextureQueue;
std::vector<GLuint> mTextureDeletes;
bool mIsEnabled = false;
GLshort mCurZ = 0;

static const float inv255 = 1.0f / 255.0f;

GLuint gMaskTexture = 0;

void renderQueueInit()
{
	mIsEnabled = false;
	mTextureQueue.reserve(100);
	std::string filename = game_config::path;
	filename += "/data/core/images/terrain/alphamask.png";
	glGenTextures(1, &gMaskTexture);	
	cacheBindTexture(GL_TEXTURE_2D, gMaskTexture, true);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	SDL_Surface *surf = IMG_Load(filename.c_str());
	if (surf == NULL)
	{
		std::cerr << "\n\n*** ERROR loading texture altas " << filename.c_str() << "\n\n";
		return;
	}
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surf->w, surf->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surf->pixels);
	SDL_FreeSurface(surf);
}

void setupBrightness(float brightness)
{
	if (brightness == 1.0f)
		return;
	else if (brightness == 0.0f)
	{
		// greyscale
		float t = 1;	// standard perceptual weighting
		GLfloat lerp[4] = { 1.0, 1.0, 1.0, 0.5 };
		GLfloat avrg[4] = { .667, .667, .667, 0.5 };	// average
		GLfloat prcp[4] = { .646, .794, .557, 0.5 };	// perceptual NTSC
		GLfloat dot3[4] = { prcp[0]*t+avrg[0]*(1-t), prcp[1]*t+avrg[1]*(1-t), prcp[2]*t+avrg[2]*(1-t), 0.5 };
		
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB,      GL_INTERPOLATE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB,         GL_TEXTURE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_RGB,         GL_CONSTANT);
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC2_RGB,         GL_CONSTANT);
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA,    GL_REPLACE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA,       GL_TEXTURE);
		glTexEnvfv(GL_TEXTURE_ENV,GL_TEXTURE_ENV_COLOR, lerp);
		
		// Note: we prefer to dot product with primary color, because
		// the constant color is stored in limited precision on MBX
		glActiveTexture(GL_TEXTURE1);
		glEnable(GL_TEXTURE_2D);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB,      GL_DOT3_RGB);
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB,         GL_PREVIOUS);
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_RGB,         GL_PRIMARY_COLOR);
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA,    GL_REPLACE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA,       GL_PREVIOUS);
		
		glColor4f(dot3[0], dot3[1], dot3[2], dot3[3]);
		
	}
	else
	{
		// brightness
		
		// One pass using one unit:
		// brightness < 1.0 biases towards black
		// brightness > 1.0 biases towards white
		//
		// Note: this additive definition of brightness is
		// different than what matrix-based adjustment produces,
		// where the brightness factor is a scalar multiply.
		//
		// A +/-1 bias will produce the full range from black to white,
		// whereas the scalar multiply can never reach full white.
		float t = ((brightness-1.0f) / 2) + 1.0f;
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
		if (t > 1.0f)
		{
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB,      GL_ADD);
			//glColor4f(t-1, t-1, t-1, t-1);
			cacheColor4f(t-1, t-1, t-1, t-1);
		}
		else
		{
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB,      GL_SUBTRACT);
			//glColor4f(1-t, 1-t, 1-t, 1-t);
			cacheColor4f(1-t, 1-t, 1-t, 1-t);
		}
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB,         GL_TEXTURE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_RGB,         GL_PRIMARY_COLOR);
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA,    GL_REPLACE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA,       GL_TEXTURE);
		
	}
	
}

void cleanupBrightness(float brightness)
{
	if (brightness == 1.0f)
		return;
	else if (brightness == 0.0f)
	{
		glDisable(GL_TEXTURE_2D);
		glActiveTexture(GL_TEXTURE0);
		
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		//glColor4f(1,1,1,1);
		cacheColor4f(1,1,1,1);
	}
	else
	{
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		//glColor4f(1,1,1,1);
		cacheColor4f(1,1,1,1);
	}
}

void renderQueueAddTexture(GLshort *vertices, GLfloat *texCoords, GLuint texture, unsigned long color, float brightness)
{
	if (mIsEnabled)
	{
		renderTextureInfo rti;
		rti.type = QUEUE_TYPE_TEXTURE;
		rti.z = mCurZ;
		memcpy(&rti.vertices, vertices, sizeof(GLshort)*12);
		memcpy(&rti.texCoords, texCoords, sizeof(GLfloat)*8);
		rti.texture = texture;
		rti.color = color;
		rti.brightness = brightness;
		rti.done = false;
		mTextureQueue.push_back(rti);
	}
	else
	{
		// draw it right away...
		float r = (float)((color&0x00FF0000) >> 16)*inv255;
		float g = (float)((color&0x0000FF00) >>  8)*inv255;
		float b = (float)((color&0x000000FF)      )*inv255;
		float a = (float)((color&0xFF000000) >> 24)*inv255;
		cacheTextureMode(true);
		//glColor4f(r, g, b, a);
		cacheColor4f(r, g, b, a);
//		glBindTexture(GL_TEXTURE_2D, texture);
		cacheBindTexture(GL_TEXTURE_2D, texture, 0);
		glVertexPointer(3, GL_SHORT, 0, vertices);
		glTexCoordPointer(2, GL_FLOAT, 0, texCoords);
		
		// special settings
		setupBrightness(brightness);
				
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);		

		cleanupBrightness(brightness);		
	}
}

void renderQueueAddFill(GLshort *vertices, GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
	if (mIsEnabled)
	{
		renderTextureInfo rti;
		rti.type = QUEUE_TYPE_FILL;
		rti.z = mCurZ;
		memcpy(&rti.vertices, vertices, sizeof(GLshort)*12);
		rti.texCoords[0] = r;
		rti.texCoords[1] = g;
		rti.texCoords[2] = b;
		rti.texCoords[3] = a;
		rti.done = false;
		mTextureQueue.push_back(rti);		
	}
	else
	{
		// draw right away
		cacheTextureMode(false);
		//glColor4f(r,g,b,a);	
		cacheColor4f(r,g,b,a);	
		glVertexPointer(3, GL_SHORT, 0, vertices);
		//glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		//glDisable(GL_TEXTURE_2D);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		//glEnable(GL_TEXTURE_2D);
		//glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	}
}

#define TEX_INTERSECTS_DIRTY(verts,dirty)	( !(verts[0] > dirty.x2 || verts[2] < dirty.x1 || verts[1] > dirty.y2 || verts[3] < dirty.y1) )

bool gRenderMaskOn = false;
int gRenderMaskX = 0;
int gRenderMaskY = 0;

void clearDepthBuffer(void)
{
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);		// turn on depth writing
	glDepthFunc(GL_ALWAYS);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);	// do not write to color buffer

	cacheTextureMode(false);
	GLfloat verts[12];
	verts[0] = 0;
	verts[1] = 0;
	verts[2] = 1.0f;
	verts[3] = CVideo::getx();
	verts[4] = 0;
	verts[5] = 1.0f;
	verts[6] = 0;
	verts[7] = CVideo::gety();
	verts[8] = 1.0f;
	verts[9] = CVideo::getx();
	verts[10] = CVideo::gety();
	verts[11] = 1.0f;
	glVertexPointer(3, GL_FLOAT, 0, verts);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);		
	cacheTextureMode(true);
}

void renderMaskTile(int x, int y)
{
	if (gRenderMaskOn == true && gRenderMaskX == x && gRenderMaskY == y)
		return;
	
	if (gMaskTexture == 0)
		renderQueueInit();
		
	// iPhone doesn't have a stencil buffer :(
	// use the depth buffer instead...

	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);		// turn on depth writing
	glDepthFunc(GL_ALWAYS);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);	// do not write to color buffer

	// this actually took 49% of the cpu when scrolling, so optimize to just clear the last portion
	//glClearDepthf(1.0f);
	//glClear(GL_DEPTH_BUFFER_BIT);
	
	/*
	 // this is slow on device too...
	glEnable(GL_SCISSOR_TEST);
	glScissor(CVideo::gety()-gRenderMaskY-72, CVideo::getx()-gRenderMaskX-72, 72, 72);
	glClear(GL_DEPTH_BUFFER_BIT);
	glDisable(GL_SCISSOR_TEST);
	 */
	
	// this is nice and fast
	cacheTextureMode(false);
	cacheColor4f(1, 1, 1, 1);
	GLfloat clr_verts[12];
	clr_verts[0] = gRenderMaskX;
	clr_verts[1] = gRenderMaskY;
	clr_verts[2] = -50.0f;
	clr_verts[3] = gRenderMaskX+72;
	clr_verts[4] = gRenderMaskY;
	clr_verts[5] = -50.0f;
	clr_verts[6] = gRenderMaskX;
	clr_verts[7] = gRenderMaskY+72;
	clr_verts[8] = -50.0f;
	clr_verts[9] = gRenderMaskX+72;
	clr_verts[10] = gRenderMaskY+72;
	clr_verts[11] = -50.0f;
	glVertexPointer(3, GL_FLOAT, 0, clr_verts);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);		
	

	
	gRenderMaskOn = true;
	gRenderMaskX = x;
	gRenderMaskY = y;
	
	//glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	
	// draw the mask to the depth buffer
	glEnable(GL_ALPHA_TEST);	// turn on alpha test, so transparent pixels don't write to z buffer
	glAlphaFunc(GL_GREATER,0.5f);
	cacheTextureMode(true);
	cacheBindTexture(GL_TEXTURE_2D, gMaskTexture, true);
	cacheColor4f(1, 1, 1, 1);
	GLshort verts[12];
	GLfloat uv[8];
	uv[0] = 0;
	uv[1] = 0;
	uv[2] = 1;
	uv[3] = 0;
	uv[4] = 0;
	uv[5] = 1;
	uv[6] = 1;
	uv[7] = 1;
	glTexCoordPointer(2, GL_FLOAT, 0, uv);
	verts[0] = x;
	verts[1] = y;
	verts[2] = 0;
	verts[3] = x+72;
	verts[4] = y;
	verts[5] = 0;
	verts[6] = x;
	verts[7] = y+72;
	verts[8] = 0;
	verts[9] = x+72;
	verts[10] = y+72;
	verts[11] = 0;
	glVertexPointer(3, GL_SHORT, 0, verts);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);		
	 
/*	
	// draw using simple geometry
	cacheTextureMode(false);
	
	// unfortunately, diagonal lines are not too exact...
	GLshort verts[6*3];
	verts[0] = x;
	verts[1] = y+35;
	verts[2] = 0;
	verts[3] = x+18;
	verts[4] = y;
	verts[5] = 0;
	verts[6] = x+18;
	verts[7] = y+35;
	verts[8] = 0;
	verts[9] = x+53;
	verts[10] = y;
	verts[11] = 0;
	verts[12] = x+53;
	verts[13] = y+35;
	verts[14] = 0;
	verts[15] = x+71;
	verts[16] = y+35;
	verts[17] = 0;
	glVertexPointer(3, GL_SHORT, 0, verts);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 6);	
	verts[0] = x;
	verts[1] = y+36;
	verts[2] = 0;
	verts[3] = x+18;
	verts[4] = y+71;
	verts[5] = 0;
	verts[6] = x+18;
	verts[7] = y+36;
	verts[8] = 0;
	verts[9] = x+53;
	verts[10] = y+71;
	verts[11] = 0;
	verts[12] = x+53;
	verts[13] = y+36;
	verts[14] = 0;
	verts[15] = x+71;
	verts[16] = y+36;
	verts[17] = 0;
	glVertexPointer(3, GL_SHORT, 0, verts);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 6);	
*/	
	
/*
	// this doesn't work, openGL does not like very small rectangles...
	GLshort verts[12];
	
#define maskHelper(x1, y1, x2, y2)		verts[0]=x1; verts[1]=y1; verts[2]=0; verts[3]=x2; verts[4]=y1; verts[5]=0; verts[6]=x1; verts[7]=y2; verts[8]=0; verts[9]=x2; verts[10]=y2; verts[11]=0; glVertexPointer(3, GL_SHORT, 0, verts); glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		
	maskHelper(x+18, y+0, x+53, y+0);
	maskHelper(x+17, y+1, x+54, y+2);
	maskHelper(x+16, y+3, x+55, y+4);
	maskHelper(x+15, y+5, x+56, y+6);
	maskHelper(x+14, y+7, x+57, y+8);
	maskHelper(x+13, y+9, x+58, y+10);
	maskHelper(x+12, y+11, x+59, y+12);
	maskHelper(x+11, y+13, x+60, y+14);
	maskHelper(x+10, y+15, x+61, y+16);
	maskHelper(x+9, y+17, x+62, y+18);
	maskHelper(x+8, y+19, x+63, y+20);
	maskHelper(x+7, y+21, x+64, y+22);
	maskHelper(x+6, y+23, x+65, y+24);
	maskHelper(x+5, y+25, x+66, y+26);
	maskHelper(x+4, y+27, x+67, y+28);
	maskHelper(x+3, y+29, x+68, y+30);
	maskHelper(x+2, y+31, x+69, y+32);
	maskHelper(x+1, y+33, x+70, y+34);
	maskHelper(x+0, y+35, x+71, y+36);
	maskHelper(x+1, y+37, x+70, y+38);
	maskHelper(x+2, y+39, x+69, y+40);
	maskHelper(x+3, y+41, x+68, y+42);
	maskHelper(x+4, y+43, x+67, y+44);
	maskHelper(x+5, y+45, x+66, y+46);
	maskHelper(x+6, y+47, x+65, y+48);
	maskHelper(x+7, y+49, x+64, y+50);
	maskHelper(x+8, y+51, x+63, y+52);
	maskHelper(x+9, y+53, x+62, y+54);
	maskHelper(x+10, y+55, x+61, y+56);
	maskHelper(x+11, y+57, x+60, y+58);
	maskHelper(x+12, y+59, x+59, y+60);
	maskHelper(x+13, y+61, x+58, y+62);
	maskHelper(x+14, y+63, x+57, y+64);
	maskHelper(x+15, y+65, x+56, y+66);
	maskHelper(x+16, y+67, x+55, y+68);
	maskHelper(x+17, y+68, x+54, y+70);
	maskHelper(x+18, y+71, x+53, y+71);
	
	cacheTextureMode(true);

*/
	
	
	// turn on masking
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDepthMask(GL_FALSE);		// turn off depth writing
	glDepthFunc(GL_EQUAL);
	glDisable(GL_ALPHA_TEST);
	//glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glEnable(GL_BLEND);
}

void renderMaskOff(void)
{
	if (gRenderMaskOn == true)
	{
		glDisable(GL_DEPTH_TEST);
		gRenderMaskOn = false;
	}
}

void renderQueueRender(void)
{
	//glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	cacheColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	unsigned long oldColor = 0xFFFFFFFF;


	std::cerr << " " << mDirtyTiles.size();
/*	
	bool needMask = true;
	
	if (mDirtyTiles.size() > 20)
	{
		needMask = false;
	}
*/	
	short renderCount = 0;
	
/*
	if (needMask)
	{
		// iPhone doesn't have a stencil buffer :(
		// use the depth buffer instead...
		glClearDepthf(1.0f);
		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);	// turn on depth writing
		glDepthFunc(GL_ALWAYS);
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);	// do not write to color buffer
		glAlphaFunc(GL_GREATER,0.5f);
		glEnable(GL_ALPHA_TEST);	// turn on alpha test, so transparent pixels don't write to z buffer
		glDisable(GL_BLEND);
		glClear(GL_DEPTH_BUFFER_BIT); //| GL_COLOR_BUFFER_BIT);

		cacheTextureMode(true);
		cacheBindTexture(GL_TEXTURE_2D, gMaskTexture, 0);
		GLshort verts[12];
		GLfloat uv[8];
		uv[0] = 0;
		uv[1] = 0;
		uv[2] = 1;
		uv[3] = 0;
		uv[4] = 0;
		uv[5] = 1;
		uv[6] = 1;
		uv[7] = 1;
		glTexCoordPointer(2, GL_FLOAT, 0, uv);
		for (std::set<dirtyTile>::iterator it = mDirtyTiles.begin(); it != mDirtyTiles.end(); it++)
		{
			verts[0] = it->x;
			verts[1] = it->y;
			verts[2] = 0;
			verts[3] = it->x+72;
			verts[4] = it->y;
			verts[5] = 0;
			verts[6] = it->x;
			verts[7] = it->y+72;
			verts[8] = 0;
			verts[9] = it->x+72;
			verts[10] = it->y+72;
			verts[11] = 0;
			glVertexPointer(3, GL_SHORT, 0, verts);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);		
		}
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glDepthMask(GL_FALSE);		// turn off depth writing
		glDepthFunc(GL_EQUAL);
		glDisable(GL_ALPHA_TEST);
		glEnable(GL_BLEND);
	}
*/
	
//	for (int dirtyI = 0; dirtyI < mDirtyRects.size(); dirtyI++)
	{
	
		// sort by layer ascending, then texture in order
		short curLayer = 0;
		while (renderCount < mTextureQueue.size())
		{
			for (int i=0; i < mTextureQueue.size(); i++)
			{
				if (mTextureQueue[i].z == curLayer && mTextureQueue[i].done == false)
				{
					if (mTextureQueue[i].type == QUEUE_TYPE_TEXTURE)
					{
						// render all other same textures on this layer
						GLuint renderTexture = mTextureQueue[i].texture;

						for (int j=i; j < mTextureQueue.size(); j++)
						{
							if (mTextureQueue[j].done == false && mTextureQueue[j].type == QUEUE_TYPE_TEXTURE && mTextureQueue[j].texture == renderTexture && mTextureQueue[j].z == curLayer)
							{
								// KP: NEW! Clip to all the dirty rects
								// In some cases this will render it more than once, but 90% of the time it is outside the clip area
//								for (int dirtyI = 0; dirtyI < mDirtyRects.size(); dirtyI++)
								{
//									if (TEX_INTERSECTS_DIRTY(mTextureQueue[j].vertices, mDirtyRects[dirtyI]) == true)
									{
										cacheTextureMode(true);
										cacheBindTexture(GL_TEXTURE_2D, renderTexture, 0);
										if (mTextureQueue[j].color != oldColor)
										{
											float r = (float)((mTextureQueue[j].color&0x00FF0000) >> 16)*inv255;
											float g = (float)((mTextureQueue[j].color&0x0000FF00) >>  8)*inv255;
											float b = (float)((mTextureQueue[j].color&0x000000FF)      )*inv255;
											float a = (float)((mTextureQueue[j].color&0xFF000000) >> 24)*inv255;
											//glColor4f(r, g, b, a);
											cacheColor4f(r, g, b, a);
											oldColor = mTextureQueue[j].color;
										}
										
										glVertexPointer(3, GL_SHORT, 0, mTextureQueue[j].vertices);
										glTexCoordPointer(2, GL_FLOAT, 0, mTextureQueue[j].texCoords);
										
										setupBrightness(mTextureQueue[j].brightness);
										
										glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
										
										cleanupBrightness(mTextureQueue[j].brightness);
										renderCount++;
									}									
								}
								mTextureQueue[j].done = true;
							}
						}
					}
				}
			}
			curLayer++;
			if (curLayer >= 10) //LAYER_TERRAIN_TMP_BG
				break;
		}
		for (int i=0; i < mTextureQueue.size(); i++)
		{
			if (mTextureQueue[i].done == false)
			{
				if (mTextureQueue[i].type == QUEUE_TYPE_FILL)
				{
					// KP: fills now moved outside of stencil mask loop
				}
				else
				{
					// render textures in order
					// KP: NEW! Clip to all the dirty rects
					// In some cases this will render it more than once, but 90% of the time it is outside the clip area
//					for (int dirtyI = 0; dirtyI < mDirtyRects.size(); dirtyI++)
					{
//						if (TEX_INTERSECTS_DIRTY(mTextureQueue[i].vertices, mDirtyRects[dirtyI]) == true)
						{							
							if (mTextureQueue[i].color != oldColor)
							{
								float r = (float)((mTextureQueue[i].color&0x00FF0000) >> 16)*inv255;
								float g = (float)((mTextureQueue[i].color&0x0000FF00) >>  8)*inv255;
								float b = (float)((mTextureQueue[i].color&0x000000FF)      )*inv255;
								float a = (float)((mTextureQueue[i].color&0xFF000000) >> 24)*inv255;
								//std::cerr << "rgba " << r << " " << g << " " << b << " " << a << "\n";
								//glColor4f(r, g, b, a);
								cacheColor4f(r, g, b, a);
								oldColor = mTextureQueue[i].color;
							}				
							GLuint renderTexture = mTextureQueue[i].texture;
							cacheTextureMode(true);
							//glBindTexture(GL_TEXTURE_2D, renderTexture);
							cacheBindTexture(GL_TEXTURE_2D, renderTexture, 0);
							glVertexPointer(3, GL_SHORT, 0, mTextureQueue[i].vertices);
							glTexCoordPointer(2, GL_FLOAT, 0, mTextureQueue[i].texCoords);
							setupBrightness(mTextureQueue[i].brightness);
							glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
							cleanupBrightness(mTextureQueue[i].brightness);
							renderCount++;
						}
					}
					mTextureQueue[i].done = true;
				}
			}
		}
	}	// END dirty rect loop
/*	
	if (needMask)
	{
		glDisable(GL_DEPTH_TEST);
	}
*/	
	
	for (int i=0; i < mTextureQueue.size(); i++)
	{
		if (mTextureQueue[i].done == false)
		{
			if (mTextureQueue[i].type == QUEUE_TYPE_FILL)
			{
				// render all other fills on this layer
				//glDisableClientState(GL_TEXTURE_COORD_ARRAY);
				//glDisable(GL_TEXTURE_2D);
				cacheTextureMode(false);
				
				for (int j=i; j < mTextureQueue.size(); j++)
				{
					if (mTextureQueue[j].done == false && mTextureQueue[j].type == QUEUE_TYPE_FILL /*&& mTextureQueue[j].z == curLayer*/)
					{
						float color1, color2, color3, color4;
						color1 = mTextureQueue[j].texCoords[0];
						color2 = mTextureQueue[j].texCoords[1];
						color3 = mTextureQueue[j].texCoords[2];
						color4 = mTextureQueue[j].texCoords[3];
						//glColor4f(color1,color2,color3,color4);
						cacheColor4f(color1,color2,color3,color4);
						// do all of same color together (minimize glColor4f calls too)
						for (int k=j; k < mTextureQueue.size(); k++)
						{
							if (mTextureQueue[k].done == false && mTextureQueue[k].type == QUEUE_TYPE_FILL && mTextureQueue[k].texCoords[0] == color1 && mTextureQueue[k].texCoords[1] == color2 && mTextureQueue[k].texCoords[2] == color3 && mTextureQueue[k].texCoords[3] == color4)
							{
								glVertexPointer(3, GL_SHORT, 0, mTextureQueue[k].vertices);
								glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
								
								mTextureQueue[k].done = true;
								renderCount++;
							}
						}
					}
				}
				
				//glEnable(GL_TEXTURE_2D);
				//glEnableClientState(GL_TEXTURE_COORD_ARRAY);
				//glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
				cacheColor4f(1.0f, 1.0f, 1.0f, 1.0f);
				oldColor = 0xFFFFFFFF;
			}
		}
	}
	
	mDirtyTiles.clear();
	mTextureQueue.clear();
	mCurZ = 0;
	std::cerr << " (" << renderCount << ") ";
}

void renderQueueClean(void)
{
	for (int i=0; i < mTextureDeletes.size(); i++)
	{
		GLuint texture = mTextureDeletes[i];
		glDeleteTextures(1, &texture);
	}
	mTextureDeletes.clear();
	
	// KP: now it is okay to delete textures
	checkUnitTextureAtlas();
}

void renderQueueDeleteTexture(GLuint aTexture)
{
	if (mIsEnabled)
		mTextureDeletes.push_back(aTexture);
	else
	{
		for (int i=0; i < mTextureDeletes.size(); i++)
		{
			GLuint texture = mTextureDeletes[i];
			glDeleteTextures(1, &texture);
		}
		mTextureDeletes.clear();
		glDeleteTextures(1, &aTexture);
	}
}

// Enabling the render queue means that renders are sorted by texture
// Also, only rectangles marked as dirty are rendered, and clipped to that dirty rectangle
void renderQueueEnable(void)
{
	if (gMaskTexture == 0)
		renderQueueInit();
	
	mIsEnabled = true;
}

void renderQueueDisable(void)
{
	mIsEnabled = false;
	renderQueueRender();
}

// layers are rendered in increasing z order
void renderQueueSetZ(int z)
{
	mCurZ = z;
}

// KP: no, OpenGL does not cache texture changes, we have to do it ourselves...
GLuint gActiveTexture = 0;
void cacheBindTexture(GLenum texType, GLuint texture, unsigned char force)
{
	if (texture != gActiveTexture || force == 1)
	{
		gActiveTexture = texture;
		glBindTexture(texType, texture);
	}
}

GLfloat gActiveR = 0;
GLfloat gActiveG = 0;
GLfloat gActiveB = 0;
GLfloat gActiveA = 0;
void cacheColor4f(GLfloat aR, GLfloat aG, GLfloat aB, GLfloat aA)
{
	if (aR != gActiveR || aG != gActiveG || aB != gActiveB || aA != gActiveA)
	{
		gActiveR = aR;
		gActiveG = aG;
		gActiveB = aB;
		gActiveA = aA;
		glColor4f(aR, aG, aB, aA);
	}
}

bool gIsTextured = false;
void cacheTextureMode(bool isTextured)
{
	if (gIsTextured != isTextured)
	{
		if (isTextured)
		{
			glEnable(GL_TEXTURE_2D);
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		}
		else
		{
			glDisable(GL_TEXTURE_2D);
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		}
		gIsTextured = isTextured;
	}
}

// Marks a 72x72 tile at screen coords x,y as dirty
void renderQueueMarkDirty(int x, int y)
{
	dirtyTile dt = {x, y};
	
	// insert a new dirty tile
	mDirtyTiles.insert(dt);
}

