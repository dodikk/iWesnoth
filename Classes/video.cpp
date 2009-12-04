/* $Id: video.cpp 33157 2009-02-28 16:12:49Z ivanovic $ */
/*
   Copyright (C) 2003 - 2009 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 *  @file video.cpp
 *  Video-testprogram, standalone
 */

#include "global.hpp"


#include "font.hpp"
#include "image.hpp"
#include "log.hpp"
#include "preferences_display.hpp"
#include "video.hpp"
#include "marked-up_text.hpp"

#include "TextureAtlas.h"
#include "UnitTextureAtlas.h"
#include "memory_wrapper.h"

#include <OpenGLES/ES1/gl.h>
#include <OpenGLES/ES1/glext.h>
#include <SDL_image.h>
#include "RenderQueue.h"

extern "C" {
#include "SDL_RLEaccel_c.h"
};

#define LOG_DP LOG_STREAM(info, display)
#define ERR_DP LOG_STREAM(err, display)

// KP: global redraw request flag lives here
extern bool gRedraw;
bool gRedraw = false;



#define TEST_VIDEO_ON 0

#if (TEST_VIDEO_ON==1)


// Testprogram takes three args: x-res y-res colour-depth
int main( int argc, char** argv )
{
	if( argc != 4 ) {
		printf( "usage: %s x-res y-res bitperpixel\n", argv[0] );
		return 1;
	}
	SDL_Init(SDL_INIT_VIDEO);
	CVideo video;

	printf( "args: %s, %s, %s\n", argv[1], argv[2], argv[3] );

	printf( "(%d,%d,%d)\n", strtoul(argv[1],0,10), strtoul(argv[2],0,10),
	                        strtoul(argv[3],0,10) );

	if( video.setMode( strtoul(argv[1],0,10), strtoul(argv[2],0,10),
	                        strtoul(argv[3],0,10), FULL_SCREEN ) ) {
		printf( "video mode possible\n" );
	} else  printf( "video mode NOT possible\n" );
	printf( "%d, %d, %d\n", video.getx(), video.gety(),
	                        video.getBitsPerPixel() );

	for( int s = 0; s < 50; s++ ) {
		video.lock();
		for( int i = 0; i < video.getx(); i++ )
				video.setPixel( i, 90, 255, 0, 0 );
		if( s%10==0)
			printf( "frame %d\n", s );
		video.unlock();
		video.update( 0, 90, video.getx(), 1 );
	}
	return 0;
}

#endif

namespace {
	bool fullScreen = false;
	int disallow_resize = 0;
}
void resize_monitor::process(events::pump_info &info) {
	if(info.resize_dimensions.first >= preferences::min_allowed_width()
	&& info.resize_dimensions.second >= preferences::min_allowed_height()
	&& disallow_resize == 0) {
		preferences::set_resolution(info.resize_dimensions);
	}
}

resize_lock::resize_lock()
{
	++disallow_resize;
}

resize_lock::~resize_lock()
{
	--disallow_resize;
}

static unsigned int get_flags(unsigned int flags)
{
	// SDL under Windows doesn't seem to like hardware surfaces
	// for some reason.
#if !(defined(_WIN32) || defined(__APPLE__) || defined(__AMIGAOS4__))
		flags |= SDL_HWSURFACE;
#endif
	if((flags&SDL_FULLSCREEN) == 0)
		flags |= SDL_RESIZABLE;

	return flags;
}

namespace {
std::vector<SDL_Rect> update_rects;
bool update_all = false;
}

static bool rect_contains(const SDL_Rect& a, const SDL_Rect& b) {
	return a.x <= b.x && a.y <= b.y && a.x+a.w >= b.x+b.w && a.y+a.h >= b.y+b.h;
}

static void clear_updates()
{
	update_all = false;
	update_rects.clear();
}

namespace {

//surface frameBuffer = NULL;
bool fake_interactive = false;
}

bool non_interactive()
{
	if (fake_interactive)
		return false;
//	return SDL_GetVideoSurface() == NULL;
	return false;
}
/*
surface display_format_alpha(surface surf)
{

	if(SDL_GetVideoSurface() != NULL)
		return SDL_DisplayFormatAlpha(surf);
	else if(frameBuffer != NULL)
		return SDL_ConvertSurface(surf,frameBuffer->format,0);
	else
		return NULL;
	return surf;
}
*/
/*
 surface get_video_surface()
{
	return frameBuffer;
}
*/

SDL_Rect screen_area()
{
	const SDL_Rect res = {0,0,480,320}; //frameBuffer->w,frameBuffer->h};
	return res;
}
/*
void update_rect(size_t x, size_t y, size_t w, size_t h)
{
	const SDL_Rect rect = {x,y,w,h};
	update_rect(rect);
}

void update_rect(const SDL_Rect& rect_value)
{
	if(update_all)
		return;

	SDL_Rect rect = rect_value;

//	surface const fb = SDL_GetVideoSurface();
//	if(fb != NULL) 
	int width = 480;
	int height = 320;
	{
		if(rect.x < 0) {
			if(rect.x*-1 >= int(rect.w))
				return;

			rect.w += rect.x;
			rect.x = 0;
		}

		if(rect.y < 0) {
			if(rect.y*-1 >= int(rect.h))
				return;

			rect.h += rect.y;
			rect.y = 0;
		}

		if(rect.x + rect.w > width) {
			rect.w = width - rect.x;
		}

		if(rect.y + rect.h > height) {
			rect.h = height - rect.y;
		}

		if(rect.x >= width) {
			return;
		}

		if(rect.y >= height) {
			return;
		}
	}

	for(std::vector<SDL_Rect>::iterator i = update_rects.begin();
	    i != update_rects.end(); ++i) {
		if(rect_contains(*i,rect)) {
			return;
		}

		if(rect_contains(rect,*i)) {
			*i = rect;
			for(++i; i != update_rects.end(); ++i) {
				if(rect_contains(rect,*i)) {
					i->w = 0;
				}
			}

			return;
		}
	}

	update_rects.push_back(rect);
}

void update_whole_screen()
{
	update_all = true;
}
*/
CVideo::CVideo(FAKE_TYPES type) : mode_changed_(false), bpp_(0), fake_screen_(false), help_string_(0), updatesLocked_(0)
{
	initSDL();
	switch(type)
	{
		case NO_FAKE:
			break;
		case FAKE:
			//make_fake();
			break;
		case FAKE_TEST:
			//make_test_fake();
			break;
	}
}

CVideo::CVideo( int x, int y, int bits_per_pixel, int flags)
		 : mode_changed_(false), bpp_(0), fake_screen_(false), help_string_(0), updatesLocked_(0)
{
	initSDL();

	const int mode_res = setMode( x, y, bits_per_pixel, flags );
	if (mode_res == 0) {
		ERR_DP << "Could not set Video Mode\n";
		throw CVideo::error();
	}
}

void CVideo::initSDL()
{
	const int res = SDL_InitSubSystem(SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE);

	if(res < 0) {
		ERR_DP << "Could not initialize SDL_video: " << SDL_GetError() << "\n";
		throw CVideo::error();
	}
}

void freeTerrainCache(void);
void freeTerrainCacheTexture(SDL_Surface *surfPtr);

CVideo::~CVideo()
{
	free_cache_textures();
	freeTerrainCache();
	font::free_cached_text();
	LOG_DP << "calling SDL_Quit()\n";
	SDL_Quit();
	LOG_DP << "called SDL_Quit()\n";
}

// KP: surface -> texture cache
// the textures are freed with the surfaces, so it is assumed to be automatically managed (no need for fixed size LRU cache)
std::map<SDL_Surface*, SDL_TextureID> gTextureCache;

// KP: optimized terrain blitting LRU cache
typedef struct
{
	SDL_Surface *surfPtr;
	int x;		// cached spot in the texture
	int y;
	SDL_TextureID textureId;
} terrainCacheType;

std::list<terrainCacheType> gTerrainCache;		// LRU cache
SDL_TextureID gTerrainCacheTexture1 = 0;
SDL_TextureID gTerrainCacheTexture2 = 0;
unsigned char *gTempPix;
bool gIsTerrainCacheEnabled = false;
int gTerrainCacheCount = 0;
unsigned int gCacheCount = 0;
unsigned int gCacheSize = 0;

void free_cache_texture(SDL_Surface *surfPtr)
{
	if (gIsTerrainCacheEnabled && surfPtr->w == 72 && surfPtr->h == 72)
	{
		freeTerrainCacheTexture(surfPtr);
	}
	else if (gTextureCache.count(surfPtr) != 0)
	{
		gCacheCount--;
		gCacheSize -= surfPtr->w*surfPtr->h*4;
		//std::cerr << "   deleting cached texture id " << gTextureCache[surfPtr] << " size " << surfPtr->w << "x" << surfPtr->h << ". Total: " << gCacheCount << " files, " << gCacheSize << " bytes\n";
		SDL_DestroyTexture(gTextureCache[surfPtr]);
		gTextureCache.erase(surfPtr);
	}
}

void free_cache_textures(void)
{
	std::cerr << "freeing texture caches: count: " << gCacheCount << " size: " << gCacheSize << "\n";
	std::map<SDL_Surface*, SDL_TextureID>::iterator it;
	for ( it=gTextureCache.begin() ; it != gTextureCache.end(); it++ )
	{
		SDL_DestroyTexture((*it).second);
	}
	gTextureCache.clear();
	gCacheCount = 0;
	gCacheSize = 0;
}

void blit_texture_scaled(int x, int y, int w, int h, SDL_TextureID tex, textureRenderFlags flags)
{
	if (tex == 0)
		return;
	
	int texW, texH;
	SDL_QueryTexture(tex, NULL, NULL, &texW, &texH);

	SDL_Rect srcrect = {0,0,texW,texH};
	SDL_Rect dstrect = {x,y,w,h};
	
	SDL_Rect clip;
	getClipRect(&clip);
	
	SDL_Rect clippedDst;
	bool result = SDL_IntersectRect(&dstrect, &clip, &clippedDst);
	if (!result)
		return;	// outside clip area
	
	SDL_RenderCopy(tex, &srcrect, &clippedDst, flags);
}

void blit_surface_scaled(int x, int y, int w, int h, surface surf, textureRenderFlags flags)
{
	if (surf == NULL)
		return;
	
	SDL_TextureID tex;
	SDL_Rect srcrect = {0,0,surf->w,surf->h};
	SDL_Rect dstrect = {x,y,w,h};
	
	// scope for SDL_Surface*
	{
		SDL_Surface *surfPtr = (SDL_Surface*) surf;
		if (gTextureCache.count(surfPtr) == 0)
		{
			tex = SDL_CreateTextureFromSurface(SDL_PIXELFORMAT_ABGR8888, surfPtr);
			gTextureCache[surfPtr] = tex;
			gCacheCount++;
			gCacheSize += surfPtr->w * surfPtr->h * 4;
			//std::cerr << "caching (scaled) texture id " << tex << " size " << surfPtr->w << "x" << surfPtr->h << ". Total: " << gCacheCount << " files, " << gCacheSize << " bytes\n";			
		}
		else
		{
			tex = gTextureCache[surfPtr];
		}
	}
	
	SDL_Rect clip;
	getClipRect(&clip);
	
	SDL_Rect clippedDst;
	bool result = SDL_IntersectRect(&dstrect, &clip, &clippedDst);
	if (!result)
		return;	// outside clip area
	
	// clipping a scaled surface means we need to adjust the source rect down by the clipped amount
	SDL_Rect clippedSrc;
	clippedSrc = srcrect;
	clippedSrc.x += clippedDst.x - dstrect.x;
	clippedSrc.y += clippedDst.y - dstrect.y;
	clippedSrc.w -= dstrect.w - clippedDst.w;
	clippedSrc.h -= dstrect.h - clippedDst.h;
	
	SDL_RenderCopy(tex, &clippedSrc, &clippedDst, flags);
}

void initTerrainCache(void)
{
	if (gTerrainCacheTexture1 != 0)
		return;
	
	gTempPix = (unsigned char *) malloc(72*72*4);	// 20,736 bytes
	
	// two 1024x1024 textures will give us 392 slots
	gTerrainCacheTexture1 = SDL_CreateTexture(SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, 1024, 1024);
	gTerrainCacheTexture2 = SDL_CreateTexture(SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, 1024, 1024);
	
	for (int y=0; y < 14; y++)
	{
		for (int x=0; x < 14; x++)
		{
			terrainCacheType tc;
			tc.surfPtr = NULL;
			tc.x = x * 72;
			tc.y = y * 72;
			tc.textureId = gTerrainCacheTexture1;
			gTerrainCache.push_back(tc);
		}
	}
	for (int y=0; y < 14; y++)
	{
		for (int x=0; x < 14; x++)
		{
			terrainCacheType tc;
			tc.surfPtr = NULL;
			tc.x = x * 72;
			tc.y = y * 72;
			tc.textureId = gTerrainCacheTexture2;
			gTerrainCache.push_back(tc);
		}
	}
}



void freeTerrainCacheTexture(SDL_Surface *surfPtr)
{
	terrainCacheType tc;
	std::list<terrainCacheType>::reverse_iterator rit;
	bool found = false;
	for (rit = gTerrainCache.rbegin(); rit != gTerrainCache.rend(); ++rit)
	{
		if ((*rit).surfPtr == surfPtr)
		{
			found = true;
			break;
		}
	}
	
	if (found == true)
	{
		// move to front of list
		tc = (*rit);
		gTerrainCache.erase((++rit).base());	// need to compile this way, it's the same as gTerrainCache.erase(rit);
		tc.surfPtr = NULL;
		gTerrainCache.push_front(tc);
		gTerrainCacheCount--;
	}
}

void freeTerrainCache(void)
{
	if (gTerrainCacheTexture1 == 0)
		return;
	gTerrainCache.clear();
	SDL_DestroyTexture(gTerrainCacheTexture1);
	SDL_DestroyTexture(gTerrainCacheTexture2);
	gTerrainCacheTexture1 = 0;
	gTerrainCacheTexture2 = 0;
	gTerrainCacheCount = 0;
	free(gTempPix);
}

void enableTerrainCache(void)
{
	if (gIsTerrainCacheEnabled == true)
		return;
	
	initTerrainCache();
	gIsTerrainCacheEnabled = true;
}

void disableTerrainCache(void)
{
	if (gIsTerrainCacheEnabled == false)
		return;
	
	gIsTerrainCacheEnabled = false;
	freeTerrainCache();
}

// caches a terrain tile, without rendering it
// this is used to pre-cache the map tiles before the first frame
void cacheTerrain(surface surf)
{
	unsigned char *pixels;
	int pitch;
	
	if (surf == NULL)
		return;
	
	terrainCacheType tc;
	
	// scope for SDL_Surface*
	{
		SDL_Surface *surfPtr = (SDL_Surface*) surf;
		std::list<terrainCacheType>::reverse_iterator rit;
		bool found = false;
		for (rit = gTerrainCache.rbegin(); rit != gTerrainCache.rend(); ++rit)
		{
			if ((*rit).surfPtr == surfPtr)
			{
				// already cached
				found = true;
				return;
			}
		}
		
		{
			gTerrainCacheCount++;
			
			// take the place of the least used one (list head)
			tc = gTerrainCache.front();
			gTerrainCache.pop_front();
			tc.surfPtr = surfPtr;
			
			gTerrainCache.push_back(tc);
			
			SDL_Rect dstrect = {tc.x, tc.y, 72, 72};
						
			assert((surfPtr->flags & SDL_RLEACCEL) == 0);
			
			// to avoid lots of calls to glTexSubImage2d the first frame, update the texture data directly
			// the texture will then be re-created in one go, when it is rendered
			SDL_LockTexture(tc.textureId, &dstrect, 1, (void**)&pixels, &pitch);
			
			unsigned char *tempPix;
			const unsigned char *srcPixels = (unsigned char *) surfPtr->pixels;
			
			// need to convert ARGB to BGRA, hopefully the compiler will optimize this...
			for (int y=0; y < 72; y++)
			{
				tempPix = pixels + y*pitch;
				for (int x=0; x < 72; x++)
				{
					*tempPix++ = *(srcPixels+2);		// red in result
					*tempPix++ = *(srcPixels+1);		// green in result
					*tempPix++ = *(srcPixels+0);		// blue in result
					*tempPix++ = *(srcPixels+3);		// alpha in result
					srcPixels += 4;
				}
			}

			SDL_UnlockTexture(tc.textureId);
		}
	}
	
}


void blit_terrain(int x, int y, surface surf, SDL_Rect* srcrect, SDL_Rect* clip_rect)
{
	if (surf == NULL)
		return;
	
	SDL_Rect dst = {x,y,surf->w,surf->h};
	if (srcrect)
	{
		if (srcrect->w < surf->w)
			dst.w = srcrect->w;
		if (srcrect->h < surf->h)
			dst.h = srcrect->h;
	}
	
	SDL_Rect src = {0,0,surf->w,surf->h};
	if (srcrect)
		src = *srcrect;
	
	
	terrainCacheType tc;
	
	// scope for SDL_Surface*
	{
		SDL_Surface *surfPtr = (SDL_Surface*) surf;
		std::list<terrainCacheType>::reverse_iterator rit;
		bool found = false;
		for (rit = gTerrainCache.rbegin(); rit != gTerrainCache.rend(); ++rit)
		{
			if ((*rit).surfPtr == surfPtr)
			{
				found = true;
				break;
			}
		}
		
		if (found == true)
		{
			// move to back of the list
			tc = (*rit);
			gTerrainCache.erase((++rit).base());	// need to compile this way, it's the same as gTerrainCache.erase(rit);

			gTerrainCache.push_back(tc);
		}
		else //if (found == false)
		{
			gTerrainCacheCount++;
			
			// take the place of the least used one (list head)
			tc = gTerrainCache.front();
			gTerrainCache.pop_front();
			tc.surfPtr = surfPtr;
			
			gTerrainCache.push_back(tc);
			
			SDL_Rect dstrect = {tc.x, tc.y, 72, 72};
			
			
			//assert((surfPtr->flags & SDL_RLEACCEL) == 0);
			if ((surfPtr->flags & SDL_RLEACCEL) != 0) {
				SDL_UnRLESurface(surfPtr, 1);
			}
			
			 const unsigned char *pixels = (unsigned char *) surfPtr->pixels;
			 int pitch = 72*4;
			
			 unsigned char *tempPix = gTempPix;
			 // need to convert ARGB to BGRA, hopefully the compiler will optimize this...
			 for (int i=0; i < 72*72; i++)
			 {
				 *tempPix++ = *(pixels+2);		// red in result
				 *tempPix++ = *(pixels+1);		// green in result
				 *tempPix++ = *(pixels+0);		// blue in result
				 *tempPix++ = *(pixels+3);		// alpha in result
				 pixels += 4;
			 }
			 
			 SDL_UpdateTexture(tc.textureId, &dstrect, gTempPix, pitch);
		}
	}
	
	SDL_Rect clip;
	
	if(clip_rect != NULL) 
	{
		clip = *clip_rect;
	} 
	else 
	{
		getClipRect(&clip);
	}
	
	// perform clipping
	SDL_Rect clippedDst;
	SDL_Rect clippedSrc;
	bool result = SDL_IntersectRect(&dst, &clip, &clippedDst);
	if (!result)
		return;	// outside clip area
	
	//clippedSrc = src;
	clippedSrc.x = tc.x + src.x;
	clippedSrc.y = tc.y + src.y;
	clippedSrc.w = 72 - (72-src.w);
	clippedSrc.h = 72 - (72-src.h);

	clippedSrc.x += clippedDst.x - dst.x;
	clippedSrc.y += clippedDst.y - dst.y;
	clippedSrc.w -= dst.w - clippedDst.w;
	clippedSrc.h -= dst.h - clippedDst.h;
	
	static unsigned long renderCount = 0;
	renderCount++;
	
	SDL_RenderCopy(tc.textureId, &clippedSrc, &clippedDst, DRAW);
}

void blit_surface(int x, int y, surface surf, SDL_Rect* srcrect, SDL_Rect* clip_rect, textureRenderFlags flags)
{
	if (surf == NULL)
		return;
	
	if (gIsTerrainCacheEnabled && surf->w == 72 && surf->h == 72)
	{
		// KP: specialized terrain cache blitter
		blit_terrain(x, y, surf, srcrect, clip_rect);
		return;
	}
	
	SDL_Rect dst = {x,y,surf->w,surf->h};
	if (srcrect)
	{
		if (srcrect->w < surf->w)
			dst.w = srcrect->w;
		if (srcrect->h < surf->h)
			dst.h = srcrect->h;
	}
	
	SDL_Rect src = {0,0,surf->w,surf->h};
	if (srcrect)
		src = *srcrect;
		

	SDL_TextureID tex;
	
	// scope for SDL_Surface*
	{
		SDL_Surface *surfPtr = (SDL_Surface*) surf;
		if (gTextureCache.count(surfPtr) == 0)
		{
			tex = SDL_CreateTextureFromSurface(SDL_PIXELFORMAT_ABGR8888, surfPtr);
			//gTextureCache[surfPtr] = tex;
			gTextureCache.insert( std::make_pair(surfPtr, tex) );
			gCacheCount++;
			gCacheSize += surfPtr->w * surfPtr->h * 4;
			//std::cerr << "caching texture id " << tex << " size " << surfPtr->w << "x" << surfPtr->h << ". Total: " << gCacheCount << " files, " << gCacheSize << " bytes\n";
		}
		else
		{
			tex = gTextureCache[surfPtr];
		}
	}
	
	SDL_Rect clip;
	
	if(clip_rect != NULL) 
	{
		clip = *clip_rect;
	} 
	else 
	{
		getClipRect(&clip);
	}
	
	// perform clipping
	SDL_Rect clippedDst;
	SDL_Rect clippedSrc;
	bool result = SDL_IntersectRect(&dst, &clip, &clippedDst);
	if (!result)
		return;	// outside clip area
	
/*	clippedSrc = src;
	clippedSrc.x += clippedDst.x - dst.x;
	clippedSrc.y += clippedDst.y - dst.y;
	clippedSrc.w -= dst.w - clippedDst.w;
	clippedSrc.h -= dst.h - clippedDst.h;
*/
	clippedSrc = src;
	
	SDL_Rect clipAmount;
	clipAmount.x = clippedDst.x - dst.x;
	clipAmount.y = clippedDst.y - dst.y;
	clipAmount.w = dst.w - clippedDst.w;
	clipAmount.h = dst.h - clippedDst.h;
	
	
	if ((flags & FLOP) != 0)
	{
		clippedSrc.x += (clipAmount.w-clipAmount.x);
		clippedSrc.w -= clipAmount.x;
	}
	else
	{
		clippedSrc.x += clipAmount.x;
		clippedSrc.w -= clipAmount.w;
	}
	if ((flags & FLIP) != 0)
	{
		clippedSrc.y += (clipAmount.h-clipAmount.y);
		clippedSrc.h -= clipAmount.y;
	}
	else
	{
		clippedSrc.y += clipAmount.y;
		clippedSrc.h -= clipAmount.h;
	}
	
	static unsigned long renderCount = 0;
	renderCount++;
	
	SDL_RenderCopy(tex, &clippedSrc, &clippedDst, flags);	
}

void blit_texture(int x, int y, SDL_TextureID tex, SDL_Rect* srcrect, SDL_Rect* clip_rect, textureRenderFlags flags)
{
	if (tex == 0)
		return;	
	
	int texW, texH;
	SDL_QueryTexture(tex, NULL, NULL, &texW, &texH);
	
	SDL_Rect dst = {x,y,texW,texH};
	if (srcrect)
	{
		if (srcrect->w < texW)
			dst.w = srcrect->w;
		if (srcrect->h < texH)
			dst.h = srcrect->h;
	}
	
	SDL_Rect src = {0,0,texW,texH};
	if (srcrect)
		src = *srcrect;
	
	SDL_Rect clip;
	
	if(clip_rect != NULL) 
	{
		clip = *clip_rect;
	} 
	else 
	{
		getClipRect(&clip);
	}
	
	// perform clipping
	SDL_Rect clippedDst;
	SDL_Rect clippedSrc;
	bool result = SDL_IntersectRect(&dst, &clip, &clippedDst);
	if (!result)
		return;	// outside clip area
	
	clippedSrc = src;
	clippedSrc.x += clippedDst.x - dst.x;
	clippedSrc.y += clippedDst.y - dst.y;
	clippedSrc.w -= dst.w - clippedDst.w;
	clippedSrc.h -= dst.h - clippedDst.h;
		
	SDL_RenderCopy(tex, &clippedSrc, &clippedDst, flags);	
}

void convertSurfaceToFill(int x, int y, surface surf)
{
	// look inside this surface and change it to a bunch of rects/fillrects instead
	int w = surf->w;
	int h = surf->h;
	int dataSize = surf->w*surf->h*4;
	if (w <= 0 || h <= 0)
		return;
	
	Uint32 fillColor = *(Uint32*)(((unsigned char*)surf->pixels + dataSize-4));
	
	Uint32 *pixelPtr = (Uint32*) surf->pixels;
	int fillStart = 0;
	for (; fillStart < h; fillStart++)
	{
		Uint32 curColor = *pixelPtr;
		if (curColor == fillColor)
			break;
		pixelPtr += w;
	}
	
	
	SDL_Rect outside = {x-1, y-1, w+2, h+2};
	fill_rect(0xffffffff, &outside);
	outside.x++;
	outside.y++;
	outside.w -= 2;
	outside.h -= 2;
	fill_rect(0x505050, &outside);
	SDL_Rect inside = {x, y+fillStart, w, h-fillStart};
	fill_rect(fillColor, &inside);
}

void draw_line(Uint32 color, int x1, int y1, int x2, int y2)
{
	unsigned char alpha = (color >> 24) & 0xff;
	if (alpha == 0)
		alpha = 0xff;	// too common to ignore alpha
	
	// perform clipping
	SDL_Rect clip;	
	getClipRect(&clip);
	
	if (x2 < clip.x || x1 > clip.x+clip.w || y2 < clip.y || y1 > clip.y+clip.h)
		return;
	
	if (x1 < clip.x)
		x1 = clip.x;
	if (x2 > clip.x+clip.w)
		x2 = clip.x+clip.w;
	if (y1 < clip.y)
		y1 = clip.y;
	if (y2 > clip.y+clip.h)
		y2 = clip.y+clip.h;
	
	
	
	SDL_SetRenderDrawColor((color >> 16) & 0xff, (color >> 8) & 0xff, color & 0xff, alpha);
	SDL_RenderLine(x1, y1, x2, y2);
}

void fill_rect(Uint32 color, SDL_Rect *rect)
{
	unsigned char alpha = (color >> 24) & 0xff;
	if (alpha == 0)
		alpha = 0xff;	// too common to ignore alpha

	SDL_Rect clip;
	getClipRect(&clip);
	
	// perform clipping
	SDL_Rect clippedDst;
	bool result = SDL_IntersectRect(rect, &clip, &clippedDst);
	if (!result)
		return;	// outside clip area
	
	SDL_SetRenderDrawColor((color >> 16) & 0xff, (color >> 8) & 0xff, color & 0xff, alpha);
	SDL_RenderFill(&clippedDst);
}

/*
void CVideo::make_fake()
{
	fake_screen_ = true;
	frameBuffer = SDL_CreateRGBSurface(SDL_SWSURFACE,16,16,24,0xFF0000,0xFF00,0xFF,0);
	image::set_pixel_format(frameBuffer->format);
}

void CVideo::make_test_fake()
{
	// Create fake screen that is 1024x768 24bpp
	// We can then use this in tests to draw
	frameBuffer = SDL_CreateRGBSurface(SDL_SWSURFACE,1024,768,32,0xFF0000,0xFF00,0xFF,0);
	image::set_pixel_format(frameBuffer->format);

	fake_interactive = true;

}
*/
int CVideo::modePossible( int x, int y, int bits_per_pixel, int flags )
{
	return SDL_VideoModeOK( x, y, bits_per_pixel, get_flags(flags) );
}

int CVideo::setMode( int x, int y, int bits_per_pixel, int flags )
{
	update_rects.clear();
	if (fake_screen_) return 0;
	mode_changed_ = true;

	flags = get_flags(flags);
//	const int res = SDL_VideoModeOK( x, y, bits_per_pixel, flags );

//	if( res == 0 )
//		return 0;

	fullScreen = (flags & FULL_SCREEN) != 0;
//	frameBuffer = SDL_SetVideoMode( x, y, bits_per_pixel, flags );

//	if( frameBuffer != NULL ) {
//		image::set_pixel_format(frameBuffer->format);
//		return bits_per_pixel;
//	} else	return 0;
	
	// KP: added
	/* create window and renderer */
    SDL_VideoWindow = SDL_CreateWindow(NULL, 0, 0, 480, 320, SDL_WINDOW_SHOWN);
	SDL_CreateRenderer(SDL_VideoWindow, -1, 0);
	SDL_SetRenderDrawBlendMode(SDL_BLENDMODE_BLEND);
	SDL_Rect clip = screen_area();
	setClipRect(&clip);
	
	
	SDL_PixelFormat pf;
	pf.palette = NULL;
	pf.BitsPerPixel = 4*8;
	pf.BytesPerPixel = 4;
	pf.Rloss = 0;
	pf.Gloss = 0;
	pf.Bloss = 0;
	pf.Aloss = 0;
	pf.Rshift = 16;
	pf.Gshift = 8;
	pf.Bshift = 0;
	pf.Ashift = 24;
	pf.Rmask = 0x00FF0000;
	pf.Gmask = 0x0000FF00;
	pf.Bmask = 0x000000FF;
	pf.Amask = 0xFF000000;
	image::set_pixel_format(&pf);
	return pf.BitsPerPixel;
}

bool CVideo::modeChanged()
{
	bool ret = mode_changed_;
	mode_changed_ = false;
	return ret;
}

int CVideo::getx() const
{
	//return frameBuffer->w;
	return 480;
}

int CVideo::gety() const
{
	//return frameBuffer->h;
	return 320;
}

int CVideo::getBitsPerPixel()
{
	//return frameBuffer->format->BitsPerPixel;
	return 32;
}

int CVideo::getBytesPerPixel()
{
	//return frameBuffer->format->BytesPerPixel;
	return 4;
}

int CVideo::getRedMask()
{
	//return frameBuffer->format->Rmask;
	return 0x00FF0000;
}

int CVideo::getGreenMask()
{
	//return frameBuffer->format->Gmask;
	return 0x0000FF00;
}

int CVideo::getBlueMask()
{
	//return frameBuffer->format->Bmask;
	return 0x000000FF;
}
/*
void CVideo::flip()
{
	if(fake_screen_)
		return;

	if(update_all) {
		::SDL_Flip(frameBuffer);
	} else if(update_rects.empty() == false) {
		size_t sum = 0;
		for(size_t n = 0; n != update_rects.size(); ++n) {
			sum += update_rects[n].w*update_rects[n].h;
		}

		const size_t redraw_whole_screen_threshold = 80;
		if(sum > ((getx()*gety())*redraw_whole_screen_threshold)/100) {
			::SDL_Flip(frameBuffer);
		} else {
			SDL_UpdateRects(frameBuffer,update_rects.size(),&update_rects[0]);
		}
	}

	clear_updates();
}
*/
void CVideo::lock_updates(bool value)
{
	if(value == true)
		++updatesLocked_;
	else
		--updatesLocked_;
}

bool CVideo::update_locked() const
{
	return updatesLocked_ > 0;
}
/*
void CVideo::lock()
{
	if( SDL_MUSTLOCK(frameBuffer) )
		SDL_LockSurface( frameBuffer );
}

void CVideo::unlock()
{
	if( SDL_MUSTLOCK(frameBuffer) )
		SDL_UnlockSurface( frameBuffer );
}

int CVideo::mustLock()
{
	return SDL_MUSTLOCK(frameBuffer);
}

surface CVideo::getSurface()
{
	return frameBuffer;
}
*/
bool CVideo::isFullScreen() const { return fullScreen; }

void CVideo::setBpp( int bpp )
{
	bpp_ = bpp;
}

int CVideo::getBpp()
{
	return bpp_;
}

int CVideo::set_help_string(const std::string& str)
{
	font::remove_floating_label(help_string_);

	const SDL_Color colour = {0x0,0x00,0x00,0x77};

#ifdef USE_TINY_GUI
	int size = font::SIZE_NORMAL;
#else
	int size = font::SIZE_LARGE;
#endif

	while(size > 0) {
		if(font::line_width(str, size) > getx()) {
			size--;
		} else {
			break;
		}
	}

#ifdef USE_TINY_GUI
	const int border = 2;
#else
	const int border = 5;
#endif

	help_string_ = font::add_floating_label(str,size,font::NORMAL_COLOUR,getx()/2,gety(),0.0,0.0,-1,screen_area(),font::CENTER_ALIGN,&colour,border);
	const SDL_Rect& rect = font::get_floating_label_rect(help_string_);
	font::move_floating_label(help_string_,0.0,-double(rect.h));
	return help_string_;
}

void CVideo::clear_help_string(int handle)
{
	if(handle == help_string_) {
		font::remove_floating_label(handle);
		help_string_ = 0;
	}
}

void CVideo::clear_all_help_strings()
{
	clear_help_string(help_string_);
}

void free_all_caches(void)
{
	// KP: free all caches now
	std::cout << "Freeing all caches...\n";
	memory_stats("Before cache free");
	freeTextureAtlas();
	freeUnitTextureAtlas();
	image::flush_cache();
	font::clear_text_caches();
	memory_stats("After cache free");
}

GLuint wait_cursor = 0;
void draw_wait_cursor(void)
{
	if (wait_cursor == 0)
	{
		std::string filename = game_config::path + "/images/cursors/wait.png";
		glGenTextures(1, &wait_cursor);	
		glBindTexture(GL_TEXTURE_2D, wait_cursor);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		surface surf = surface(IMG_Load(filename.c_str()));
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surf->w, surf->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surf->pixels);
	}
	
	GLshort vertices[8];
	GLfloat texCoords[8];
	
	int size = 64;
	vertices[0] = (480-size)/2;
	vertices[1] = (320-size)/2;
	vertices[2] = vertices[0] + size;;
	vertices[3] = vertices[1];
	vertices[4] = vertices[0];
	vertices[5] = vertices[1] + size;
	vertices[6] = vertices[0] + size;
	vertices[7] = vertices[1] + size;
	
	texCoords[0] = 0;
	texCoords[1] = 0;
	texCoords[2] = 1;
	texCoords[3] = 0;
	texCoords[4] = 0;
	texCoords[5] = 1;
	texCoords[6] = 1;
	texCoords[7] = 1;
	renderQueueAddTexture(vertices, texCoords, wait_cursor, 0xFFFFFFFF, 1.0);
	SDL_RenderPresent();
}
