/* $Id: key.cpp 31859 2009-01-01 10:28:26Z mordante $ */
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

#include "global.hpp"
#include "key.hpp"

CKey::CKey() :
	key_list(0),
	is_enabled(true)
{
	static int num_keys = 300;
	key_list = SDL_GetKeyboardState( &num_keys );	// SDL 1.3: SDL_GetKeyState changed to SDL_GetKeyboardState
}

int CKey::operator[]( int code ) const
{
	// SDL 1.3: need to convert SDLK_RETUN to SDL_SCANCODE_RETURN
	int keycode = code;
	int scancode = SDL_GetScancodeFromKey(keycode);
	return int(key_list[scancode]);
}

void CKey::SetEnabled( bool enable )
{
	is_enabled = enable;
}
