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


#ifndef RENDERQUEUE_INCLUDED
#define RENDERQUEUE_INCLUDED

#include <OpenGLES/ES1/gl.h>

#ifdef __cplusplus
extern "C"
{
#endif
	
	void renderQueueInit(void);
	void renderQueueAddTexture(GLshort *vertices, GLfloat *texCoords, GLuint texture, unsigned long color, float brightness);
	void renderQueueAddFill(GLshort *vertices, GLfloat r, GLfloat g, GLfloat b, GLfloat a);
	void renderQueueRender(void);
	void renderQueueDeleteTexture(GLuint texture);
	void renderQueueEnable(void);
	void renderQueueDisable(void);
	void renderQueueSetZ(int z);
	
#ifdef __cplusplus	
};
#endif


#endif // RENDERQUEUE_INCLUDED
