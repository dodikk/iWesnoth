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


#ifndef TEXTURE_ATLAS_INCLUDED
#define TEXTURE_ATLAS_INCLUDED

#include "SDL.h"
#include <string>

#define	MAP_BASE				1
#define	MAP_BASE_TRANSITION		2

#define MAP_CASTLE_CASTLE		3
#define MAP_CASTLE_DWARVEN		4
#define MAP_CASTLE_ELVEN		5
#define MAP_CASTLE_ENCAMPMENT	6
#define MAP_CASTLE_RUIN			7
#define MAP_CASTLE_SUNKEN		8
#define MAP_CASTLE_SUNKENRUIN	9
#define MAP_CAVE				10
#define MAP_CHASM				11
#define MAP_CLOUD				12
#define MAP_DCASTLE_CHASM		13
#define MAP_DCASTLE_LAVA		14
#define MAP_FOREST_FALL			15
#define MAP_FOREST_GREAT_TREE	16
#define MAP_FOREST_MUSHROOMS	17
#define MAP_FOREST_PINE			18
#define MAP_FOREST_SNOW			19
#define MAP_FOREST_SUMMER		20
#define MAP_FOREST_TROPICAL		21
#define MAP_FOREST_WINTER		22
#define MAP_LAVA				23
#define MAP_MISC				24
#define MAP_MOUNTAIN_PEAK		25
#define MAP_MOUNTAIN_RANGE1		26
#define MAP_MOUNTAIN_RANGE3		27
#define MAP_MOUNTAIN_RANGE4		28
#define MAP_MOUNTAIN5			29
#define MAP_MOUNTAIN6			30
#define MAP_MOUNTAINS			31
#define MAP_SNOW_MOUNTAINS		32
#define MAP_SWAMP				33
#define MAP_WALLS				34
#define MAP_VILLAGE				35

#define MAP_OVERLAY_MISC		36
#define MAP_FOOTSTEPS			37
#define NUM_MAPS				38


struct textureAtlasInfo
{
	unsigned short texOffsetX;
	unsigned short texOffsetY;
	unsigned short flags;
	unsigned short texW;
	unsigned short texH;
	unsigned short trimmedX;
	unsigned short trimmedY;
	unsigned short originalW;
	unsigned short originalH;
	unsigned short mapId;
};


void initTextureAtlas(void);
void freeTextureAtlas(void);
unsigned int getTextureForMap(unsigned short mapId);
bool getTextureAtlasInfo(const std::string& filename, textureAtlasInfo &tinfo);
void renderAtlas(int x, int y, const textureAtlasInfo& tinfo, SDL_Rect *srcRect = NULL, SDL_Rect *dstRect = NULL, textureRenderFlags drawFlags = DRAW, unsigned long drawColor = 0xFFFFFFFF, float brightness = 1.0f);

#endif	// TEXTURE_ATLAS_INCLUDED