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

#ifndef MEMORY_WRAPPER_INCLUDED
#define MEMORY_WRAPPER_INCLUDED

extern void memory_profiler_start(const char *path);
extern void memory_stats(const char *reason);
extern void init_custom_malloc(void);

#endif // MEMORY_WRAPPER_INCLUDED
