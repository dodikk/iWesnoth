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

#include "TextureAtlas.h"

#define QUEUE_TYPE_TEXTURE	0
#define QUEUE_TYPE_FILL		1
#define QUEUE_TYPE_LINE		2
#define QUEUE_TYPE_DONE		5


struct renderTextureInfo
{
	GLshort type;
	GLshort vertices[8];
	GLfloat texCoords[8];
	GLuint texture;
	GLshort z;
	unsigned long color;
	float brightness;	// 0..2, with 1 being normal, and 0 marked special as greyscale
};

std::vector<renderTextureInfo> mTextureQueue;
std::vector<GLuint> mTextureDeletes;
bool mIsEnabled = false;
GLshort mCurZ = 0;

static const float inv255 = 1.0f / 255.0f;

void renderQueueInit()
{
	mIsEnabled = false;
	mTextureQueue.reserve(100);
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
			glColor4f(t-1, t-1, t-1, t-1);
		}
		else
		{
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB,      GL_SUBTRACT);
			glColor4f(1-t, 1-t, 1-t, 1-t);
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
		glColor4f(1,1,1,1);
	}
	else
	{
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glColor4f(1,1,1,1);
	}
}

void renderQueueAddTexture(GLshort *vertices, GLfloat *texCoords, GLuint texture, unsigned long color, float brightness)
{
	if (mIsEnabled)
	{
		renderTextureInfo rti;
		rti.type = QUEUE_TYPE_TEXTURE;
		rti.z = mCurZ;
		memcpy(&rti.vertices, vertices, sizeof(GLshort)*8);
		memcpy(&rti.texCoords, texCoords, sizeof(GLfloat)*8);
		rti.texture = texture;
		rti.color = color;
		rti.brightness = brightness;
		mTextureQueue.push_back(rti);
	}
	else
	{
		// draw it right away...
		float r = (float)((color&0x00FF0000) >> 16)*inv255;
		float g = (float)((color&0x0000FF00) >>  8)*inv255;
		float b = (float)((color&0x000000FF)      )*inv255;
		float a = (float)((color&0xFF000000) >> 24)*inv255;
		glColor4f(r, g, b, a);
		glBindTexture(GL_TEXTURE_2D, texture);
		glVertexPointer(2, GL_SHORT, 0, vertices);
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
		memcpy(&rti.vertices, vertices, sizeof(GLshort)*8);
		rti.texCoords[0] = r;
		rti.texCoords[1] = g;
		rti.texCoords[2] = b;
		rti.texCoords[3] = a;
		mTextureQueue.push_back(rti);		
	}
	else
	{
		// draw right away
		glColor4f(r,g,b,a);	
		glVertexPointer(2, GL_SHORT, 0, vertices);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisable(GL_TEXTURE_2D);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glEnable(GL_TEXTURE_2D);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	}
}

void renderQueueRender(void)
{
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	unsigned long oldColor = 0xFFFFFFFF;

	// sort by layer ascending, then texture in order
	short renderCount = 0;
	short curLayer = 0;
	while (renderCount < mTextureQueue.size())
	{
		for (int i=0; i < mTextureQueue.size(); i++)
		{
			if (mTextureQueue[i].z == curLayer && mTextureQueue[i].type != QUEUE_TYPE_DONE)
			{
				if (mTextureQueue[i].type == QUEUE_TYPE_TEXTURE)
				{
					// render all other same textures on this layer
					GLuint renderTexture = mTextureQueue[i].texture;
					glBindTexture(GL_TEXTURE_2D, renderTexture);

					for (int j=i; j < mTextureQueue.size(); j++)
					{
						if (mTextureQueue[j].type == QUEUE_TYPE_TEXTURE && mTextureQueue[j].texture == renderTexture && mTextureQueue[j].z == curLayer)
						{
							if (mTextureQueue[j].color != oldColor)
							{
								float r = (float)((mTextureQueue[j].color&0x00FF0000) >> 16)*inv255;
								float g = (float)((mTextureQueue[j].color&0x0000FF00) >>  8)*inv255;
								float b = (float)((mTextureQueue[j].color&0x000000FF)      )*inv255;
								float a = (float)((mTextureQueue[j].color&0xFF000000) >> 24)*inv255;
								glColor4f(r, g, b, a);
								oldColor = mTextureQueue[j].color;
							}
							glVertexPointer(2, GL_SHORT, 0, mTextureQueue[j].vertices);
							glTexCoordPointer(2, GL_FLOAT, 0, mTextureQueue[j].texCoords);
							
							setupBrightness(mTextureQueue[j].brightness);
							
							glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
							
							cleanupBrightness(mTextureQueue[j].brightness);
							
							mTextureQueue[j].type = QUEUE_TYPE_DONE;
							renderCount++;
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
		if (mTextureQueue[i].type != QUEUE_TYPE_DONE)
		{
			if (mTextureQueue[i].type == QUEUE_TYPE_FILL)
			{
				// render all other fills on this layer
				glDisableClientState(GL_TEXTURE_COORD_ARRAY);
				glDisable(GL_TEXTURE_2D);
				
				for (int j=i; j < mTextureQueue.size(); j++)
				{
					if (mTextureQueue[j].type == QUEUE_TYPE_FILL /*&& mTextureQueue[j].z == curLayer*/)
					{
						glColor4f(mTextureQueue[j].texCoords[0],mTextureQueue[j].texCoords[1],mTextureQueue[j].texCoords[2],mTextureQueue[j].texCoords[3]);								
						glVertexPointer(2, GL_SHORT, 0, mTextureQueue[j].vertices);
						glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
						
						mTextureQueue[j].type = QUEUE_TYPE_DONE;
						renderCount++;
					}
				}
				
				glEnable(GL_TEXTURE_2D);
				glEnableClientState(GL_TEXTURE_COORD_ARRAY);
				glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
				oldColor = 0xFFFFFFFF;
			}
			else
			{
				// render textures in order
				if (mTextureQueue[i].color != oldColor)
				{
					float r = (float)((mTextureQueue[i].color&0x00FF0000) >> 16)*inv255;
					float g = (float)((mTextureQueue[i].color&0x0000FF00) >>  8)*inv255;
					float b = (float)((mTextureQueue[i].color&0x000000FF)      )*inv255;
					float a = (float)((mTextureQueue[i].color&0xFF000000) >> 24)*inv255;
					//std::cerr << "rgba " << r << " " << g << " " << b << " " << a << "\n";
					glColor4f(r, g, b, a);
					oldColor = mTextureQueue[i].color;
				}				
				GLuint renderTexture = mTextureQueue[i].texture;
				glBindTexture(GL_TEXTURE_2D, renderTexture);
				glVertexPointer(2, GL_SHORT, 0, mTextureQueue[i].vertices);
				glTexCoordPointer(2, GL_FLOAT, 0, mTextureQueue[i].texCoords);
				setupBrightness(mTextureQueue[i].brightness);
				glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
				cleanupBrightness(mTextureQueue[i].brightness);
				mTextureQueue[i].type = QUEUE_TYPE_DONE;
			}
		}
	}
	
	
	mTextureQueue.clear();
	mCurZ = 0;
	
	for (int i=0; i < mTextureDeletes.size(); i++)
	{
		GLuint texture = mTextureDeletes[i];
		glDeleteTextures(1, &texture);
	}
	mTextureDeletes.clear();
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

void renderQueueEnable(void)
{
	mIsEnabled = true;
//	glEnable(GL_DEPTH_TEST);
//	glDepthFunc(GL_LEQUAL);
//	glClearDepthf(1.0f);	
//	glEnable(GL_ALPHA_TEST);
//	glAlphaFunc(GL_NOTEQUAL, 0);
//	glClear(GL_DEPTH_BITS);
}

void renderQueueDisable(void)
{
	mIsEnabled = false;
	renderQueueRender();
//	glDisable(GL_DEPTH_TEST);
//	glDisable(GL_ALPHA_TEST);
}

// layers are rendered in increasing z order
void renderQueueSetZ(int z)
{
	mCurZ = z;
}
