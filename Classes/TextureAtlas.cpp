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


#include "TextureAtlas.h"

#include <OpenGLES/ES1/gl.h>
#include <map>

#include "SDL_image.h"
#include "filesystem.hpp"
#include "video.hpp"

#include <OpenGLES/ES1/glext.h>
#include "stdio.h"


#include "game_config.hpp"

#include "RenderQueue.h"

#include <assert.h>

#include "shared_string.hpp"

std::map<shared_string, textureAtlasInfo> gAtlasMap;
GLuint gTexIds[NUM_MAPS];
unsigned int gTexW[NUM_MAPS];
unsigned int gTexH[NUM_MAPS];

void addMapEntry(const char *filename, unsigned short texOffsetX, unsigned short texOffsetY, unsigned short flags, unsigned short texW, 
				 unsigned short texH, unsigned short trimmedX, unsigned short trimmedY, unsigned short originalW, unsigned short originalH,
				 unsigned short mapId)
{
	textureAtlasInfo tinfo;
	tinfo.texOffsetX = texOffsetX;
	tinfo.texOffsetY = texOffsetY;
	tinfo.flags = flags;
	tinfo.texW = texW;
	tinfo.texH = texH;
	tinfo.trimmedX = trimmedX;
	tinfo.trimmedY = trimmedY;
	tinfo.originalW = originalW;
	tinfo.originalH = originalH;
	tinfo.mapId = mapId;
	gAtlasMap.insert(std::pair<shared_string, textureAtlasInfo>(shared_string(filename), tinfo));
}

extern void loadUnitMap(unsigned short mapId);

void loadMap(unsigned short mapId)
{
	// load the base maps, since they are always used
	std::string filename = game_config::path;
	filename += "/data/core/images/terrain/";

	short pvrtcSize = 0;
	
	switch(mapId)
	{
		case MAP_BASE:
			filename += "map.base.png";
			break;
		case MAP_BASE_TRANSITION:
#ifdef __IPAD__			
			filename += "map.base_transition.png";
#else
			filename += "map.base_transition.pvrtc";
			pvrtcSize = 1024;
#endif
			break;
		case MAP_OVERLAY_MISC:
			filename = game_config::path + "/images/misc/map.misc.png";
			break;
		case MAP_FOOTSTEPS:
			filename = game_config::path + "/images/footsteps/map.footsteps.png";
			break;
		case MAP_CASTLE_CASTLE:
			filename += "map.castle.castle.png";
			break;
		case MAP_CASTLE_DWARVEN:
			filename += "map.castle.dwarven.png";
			break;
		case MAP_CASTLE_ELVEN:
			filename += "map.castle.elven.png";
			break;
		case MAP_CASTLE_ENCAMPMENT:
			filename += "map.castle.encampment.png";
			break;
		case MAP_CASTLE_RUIN:
			filename += "map.castle.ruin.png";
			break;
		case MAP_CASTLE_SUNKEN:
			filename += "map.castle.sunken.png";
			break;
		case MAP_CASTLE_SUNKENRUIN:
			filename += "map.castle.sunkenruin.png";
			break;
		case MAP_CAVE:
			filename += "map.cave.png";
			break;
		case MAP_CHASM:
			filename += "map.chasm.png";
			break;
		case MAP_CLOUD:
			filename += "map.cloud.pvrtc";
			pvrtcSize = 256;
			break;
		case MAP_DCASTLE_CHASM:
			filename += "map.dcastle-chasm.png";
			break;
		case MAP_DCASTLE_LAVA:
			filename += "map.dcastle-lava.png";
			break;
		case MAP_FOREST_FALL:
			filename += "map.forest.fall.pvrtc";
			pvrtcSize = 512;
			break;
		case MAP_FOREST_GREAT_TREE:
			filename += "map.forest.greattree.pvrtc";
			pvrtcSize = 256;
			break;
		case MAP_FOREST_MUSHROOMS:
			filename += "map.forest.mushrooms.pvrtc";
			pvrtcSize = 256;
			break;
		case MAP_FOREST_PINE:
			filename += "map.forest.pine.pvrtc";
			pvrtcSize = 512;
			break;
		case MAP_FOREST_SNOW:
			filename += "map.forest.snow.pvrtc";
			pvrtcSize = 512;
			break;
		case MAP_FOREST_SUMMER:
			filename += "map.forest.summer.pvrtc";
			pvrtcSize = 512;
			break;
		case MAP_FOREST_TROPICAL:
			filename += "map.forest.tropical.pvrtc";
			pvrtcSize = 256;
			break;
		case MAP_FOREST_WINTER:
			filename += "map.forest.winter.pvrtc";
			pvrtcSize = 512;
			break;
		case MAP_LAVA:
			filename += "map.lava.pvrtc";
			pvrtcSize = 512;
			break;
		case MAP_MISC:
			filename += "map.misc.pvrtc";
			pvrtcSize = 256;
			break;
		case MAP_MOUNTAIN_PEAK:
			filename += "map.mountain_peak.pvrtc";
			pvrtcSize = 256;
			break;
		case MAP_MOUNTAIN_RANGE1:
			filename += "map.mountain_range1.png";
			break;
		case MAP_MOUNTAIN_RANGE3:
			filename += "map.mountain_range3.png";
			break;
		case MAP_MOUNTAIN_RANGE4:
			filename += "map.mountain_range4.png";
			break;
		case MAP_MOUNTAIN5:
			filename += "map.mountain5.png";
			break;
		case MAP_MOUNTAIN6:
			filename += "map.mountain6.png";
			break;
		case MAP_MOUNTAINS:
			filename += "map.mountains.png";
			break;
		case MAP_SNOW_MOUNTAINS:
			filename += "map.snow-mountains.png";
			break;
		case MAP_SWAMP:
			filename += "map.swamp.pvrtc";
			pvrtcSize = 512;
			break;
		case MAP_VILLAGE:
			filename += "map.village.pvrtc";
			pvrtcSize = 512;
			break;			
		case MAP_WALLS:
			filename += "map.walls.pvrtc";
			pvrtcSize = 1024;
			break;
		default:
			return;
	}
	
	assert (mapId < NUM_MAPS);
	
	glGenTextures(1, &gTexIds[mapId]);	
	//glBindTexture(GL_TEXTURE_2D, gTexIds[mapId]);
	cacheBindTexture(GL_TEXTURE_2D, gTexIds[mapId], true);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	if (pvrtcSize == 0)
	{
		SDL_Surface *surf = IMG_Load(filename.c_str());
		if (surf == NULL)
		{
			std::cerr << "\n\n*** ERROR loading texture altas " << filename.c_str() << "\n\n";
			return;
		}
		
		gTexW[mapId] = surf->w;
		gTexH[mapId] = surf->h;
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surf->w, surf->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surf->pixels);
		SDL_FreeSurface(surf);
	}
	else
	{
		GLsizei dataSize = (pvrtcSize * pvrtcSize * 4) / 8;
		FILE *fp = fopen(filename.c_str(), "rb");
		if (!fp)
		{
			std::cerr << "\n\n*** ERROR loading texture altas " << filename.c_str() << "\n\n";
			return;
		}
		unsigned char *data = (unsigned char*) malloc(dataSize);
		int bytesRead = fread(data, 1, dataSize, fp);
		assert(bytesRead == dataSize);
		fclose(fp);
		glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG, pvrtcSize, pvrtcSize, 0, dataSize, data);
		
#ifndef NDEBUG		
		GLenum err = glGetError();
        if (err != GL_NO_ERROR)
        {
			char buffer[512];
			sprintf(buffer, "\n\n*** ERROR uploading compressed texture %s:  glError: 0x%04X\n\n", filename.c_str(), err);
			std::cerr << buffer;
        }
#endif
		
		free(data);
		
		gTexW[mapId] = pvrtcSize;
		gTexH[mapId] = pvrtcSize;
	}
}

void initTextureAtlas(void)
{
	memset(gTexIds, 0, sizeof(gTexIds));

#include "../res/images/misc/map.misc.h"
#include "../res/images/footsteps/map.footsteps.h"
#include "../res/data/core/images/terrain/map.base.h"
#include "../res/data/core/images/terrain/map.base_transition.h"
#include "../res/data/core/images/terrain/map.castle.castle.h"
#include "../res/data/core/images/terrain/map.castle.dwarven.h"
#include "../res/data/core/images/terrain/map.castle.elven.h"
#include "../res/data/core/images/terrain/map.castle.encampment.h"
#include "../res/data/core/images/terrain/map.castle.ruin.h"
#include "../res/data/core/images/terrain/map.castle.sunken.h"
#include "../res/data/core/images/terrain/map.castle.sunkenruin.h"
#include "../res/data/core/images/terrain/map.cave.h"
#include "../res/data/core/images/terrain/map.chasm.h"
#include "../res/data/core/images/terrain/map.cloud.h"
#include "../res/data/core/images/terrain/map.dcastle-chasm.h"
#include "../res/data/core/images/terrain/map.dcastle-lava.h"
#include "../res/data/core/images/terrain/map.forest.fall.h"
#include "../res/data/core/images/terrain/map.forest.greattree.h"
#include "../res/data/core/images/terrain/map.forest.mushrooms.h"
#include "../res/data/core/images/terrain/map.forest.pine.h"
#include "../res/data/core/images/terrain/map.forest.snow.h"
#include "../res/data/core/images/terrain/map.forest.summer.h"
#include "../res/data/core/images/terrain/map.forest.tropical.h"
#include "../res/data/core/images/terrain/map.forest.winter.h"
#include "../res/data/core/images/terrain/map.lava.h"
#include "../res/data/core/images/terrain/map.misc.h"
#include "../res/data/core/images/terrain/map.mountain_peak.h"
#include "../res/data/core/images/terrain/map.mountain_range1.h"
#include "../res/data/core/images/terrain/map.mountain_range3.h"
#include "../res/data/core/images/terrain/map.mountain_range4.h"
#include "../res/data/core/images/terrain/map.mountain5.h"
#include "../res/data/core/images/terrain/map.mountain6.h"
#include "../res/data/core/images/terrain/map.mountains.h"
#include "../res/data/core/images/terrain/map.snow-mountains.h"
#include "../res/data/core/images/terrain/map.swamp.h"
#include "../res/data/core/images/terrain/map.village.h"
#include "../res/data/core/images/terrain/map.walls.h"
	
	// load the base maps, since they are always used
	loadMap(MAP_BASE);
	loadMap(MAP_BASE_TRANSITION);
	loadMap(MAP_MISC);
	loadMap(MAP_FOOTSTEPS);
}

void freeTextureAtlas(void)
{
	// KP: the base maps are not freed, since they are always used
	for (int i=3; i < NUM_MAPS; i++)
	{
		if (gTexIds[i] != 0)
		{
			glDeleteTextures(1, &gTexIds[i]);
			//renderQueueDeleteTexture(gTexIds[i]);
			gTexIds[i] = 0;
		}
	}
}

bool getTextureAtlasInfo(const std::string& filename, textureAtlasInfo& tinfo)
{
	std::string searchStr;
	if (filename[0] == 't')
	{
		// chop off redundant "terrain/"
		searchStr = filename.substr(8);
	}
	else
	{
		// footsteps, misc, etc
		searchStr = filename;
	}
	std::map<shared_string, textureAtlasInfo>::iterator it;
	it = gAtlasMap.find(searchStr);
	if (it == gAtlasMap.end())
	{
		tinfo.mapId = 0;
		tinfo.texW = 0;
		tinfo.texH = 0;
		return false;
	}
	
	tinfo = it->second;
	
	return true;
}

extern void renderUnitAtlas(int x, int y, const textureAtlasInfo& tinfo, SDL_Rect *srcRect /*= NULL*/, SDL_Rect *dstRect /*= NULL*/,
							textureRenderFlags drawFlags /*= DRAW*/, unsigned long drawColor, float brightness);

void renderAtlas(int x, int y, const textureAtlasInfo& tinfo, SDL_Rect *srcRect /*= NULL*/, SDL_Rect *dstRect /*= NULL*/,
				 textureRenderFlags drawFlags /*= DRAW*/, unsigned long drawColor, float brightness)
{
	
	if (tinfo.mapId >= 100)
	{
		renderUnitAtlas(x, y, tinfo, srcRect, dstRect, drawFlags, drawColor, brightness);
		return;
	}

	assert (tinfo.mapId < NUM_MAPS);

	
	if (gTexIds[tinfo.mapId] == 0)
		loadMap(tinfo.mapId);
    
	if (gTexIds[tinfo.mapId] == 0)
		return;		// problem loading texture...

	int minx, miny, maxx, maxy;
    GLfloat minu, maxu, minv, maxv;
 	
	
//	if (lastTexture != texturedata->texture)
	{
//		lastTexture = texturedata->texture;
//		glBindTexture(GL_TEXTURE_2D, gTexIds[tinfo.mapId]);
	}
	
//	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	// get the src/dst rects sorted out
	SDL_Rect clip;
	
//	if(clip_rect != NULL) 
//	{
//		clip = *clip_rect;
//	} 
//	else 
//	{
		getClipRect(&clip);
//	}
	SDL_Rect dst = {x, y, tinfo.originalW, tinfo.originalH};
	if (dstRect)
		dst = *dstRect;
	SDL_Rect src = {0, 0, tinfo.originalW, tinfo.originalH};
	if (srcRect)
		src = *srcRect;
	
	// the atlas images have already been cropped, so compensate the rects...
	dst.x += tinfo.trimmedX;
	dst.y += tinfo.trimmedY;
	dst.w -= (tinfo.originalW - tinfo.texW);
	dst.h -= (tinfo.originalH - tinfo.texH);
	
	
	// fix flip/flopping cropped images
	if ((tinfo.flags & FLOP) != 0)
	{
		int trimmedRight = tinfo.originalW - tinfo.trimmedX - tinfo.texW;
		dst.x -= tinfo.trimmedX - trimmedRight;
	}
	if ((tinfo.flags & FLIP) != 0)
	{
		int trimmedBot = tinfo.originalH - tinfo.trimmedY - tinfo.texH;
		dst.y -= tinfo.trimmedY - trimmedBot;
	}
	

	src.x -= tinfo.trimmedX;
	if (src.x < 0)
		src.x = 0;
	src.y -= tinfo.trimmedY;
	if (src.y < 0)
		src.y = 0;
	src.w -= (tinfo.originalW - tinfo.texW);
	src.h -= (tinfo.originalH - tinfo.texH);
	
	
	// perform clipping
	SDL_Rect clippedDst;
	SDL_Rect clippedSrc;
	bool result = SDL_IntersectRect(&dst, &clip, &clippedDst);
	if (!result)
		return;	// outside clip area
	
	//clippedSrc = src;
	clippedSrc.x = tinfo.texOffsetX + src.x;
	clippedSrc.y = tinfo.texOffsetY + src.y;
	clippedSrc.w = tinfo.texW - (tinfo.texW-src.w);
	clippedSrc.h = tinfo.texH - (tinfo.texH-src.h);
	
	SDL_Rect clipAmount;
	clipAmount.x = clippedDst.x - dst.x;
	clipAmount.y = clippedDst.y - dst.y;
	clipAmount.w = dst.w - clippedDst.w;
	clipAmount.h = dst.h - clippedDst.h;
	
	short flags = tinfo.flags;
	
	if ((flags & FLOP) != 0)
	{
		clippedSrc.x = clippedSrc.x + clippedSrc.w - clipAmount.x;
		clippedSrc.w = -clippedSrc.w + clipAmount.w;
	}
	else
	{
		clippedSrc.x += clipAmount.x;
		clippedSrc.w -= clipAmount.w;
	}
	if ((flags & FLIP) != 0)
	{
		clippedSrc.y = clippedSrc.y + clippedSrc.h - clipAmount.y;
		clippedSrc.h = -clippedSrc.h + clipAmount.h;
	}
	else
	{
		clippedSrc.y += clipAmount.y;
		clippedSrc.h -= clipAmount.h;
	}
	
	minx = clippedDst.x;
	miny = clippedDst.y;
	maxx = clippedDst.x + clippedDst.w;
	maxy = clippedDst.y + clippedDst.h;


	minu = (GLfloat) clippedSrc.x / gTexW[tinfo.mapId];
	maxu = (GLfloat) (clippedSrc.x + clippedSrc.w) / gTexW[tinfo.mapId];
	minv = (GLfloat) clippedSrc.y / gTexH[tinfo.mapId];
	maxv = (GLfloat) (clippedSrc.y + clippedSrc.h) / gTexH[tinfo.mapId];
	
	GLshort vertices[12];
	GLfloat texCoords[8];

	vertices[0] = minx;
	vertices[1] = miny;
	vertices[2] = 0;
	vertices[3] = maxx;
	vertices[4] = miny;
	vertices[5] = 0;
	vertices[6] = minx;
	vertices[7] = maxy;
	vertices[8] = 0;
	vertices[9] = maxx;
	vertices[10] = maxy;
	vertices[11] = 0;
/*	
	B---C       ->      A B
	A---D				| |
						| |
						C D
 */
	if ((flags & ROT) != 0)
	{
		texCoords[0] = minu;
		texCoords[1] = minv;
		texCoords[2] = minu;
		texCoords[3] = maxv;
		texCoords[4] = maxu;
		texCoords[5] = minv;
		texCoords[6] = maxu;
		texCoords[7] = maxv;
	}
	else
	{
		texCoords[0] = minu;
		texCoords[1] = minv;
		texCoords[2] = maxu;
		texCoords[3] = minv;
		texCoords[4] = minu;
		texCoords[5] = maxv;
		texCoords[6] = maxu;
		texCoords[7] = maxv;
	}

//	glVertexPointer(3, GL_SHORT, 0, vertices);
//	glTexCoordPointer(2, GL_FLOAT, 0, texCoords);
//	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	
	renderQueueAddTexture(vertices, texCoords, gTexIds[tinfo.mapId], drawColor, brightness);	
}

unsigned int getTextureForMap(unsigned short mapId)
{
	return gTexIds[mapId];
}
