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


#ifndef MEMFILE_INCLUDED
#define MEMFILE_INCLUDED


// this is a hacky way to read data from a block of buffered memory
// streams are not used because of performance issues on iPhone...

struct MEMFILE
{
	unsigned char *curPtr;
	
	MEMFILE(void *ptr)
	{
		curPtr = (unsigned char *) ptr;
	}
	
	void read(void *dstPtr, unsigned long size)
	{
		memcpy(dstPtr, curPtr, size);
		curPtr += size;
	}
};

//fread(&numstd::strings, sizeof(numstd::strings), 1, file);
#define mread(ptr, size, count, file)	( file->read(ptr, size) )

#endif // MEMFILE_INCLUDED