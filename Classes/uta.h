/*
    SEL - Simple DirectMedia Layer Extension Library
    Copyright (C) 2002 Matej Knopp <knopp@users.sf.net>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef __sel_uta_h__
#define __sel_uta_h__

#include "sdl.h"

namespace SEL
{

//! Class that represents a microtile array.
/*!
  Microtile array is an efficient method of representing regions that
  need to be redrawn. For more information about the microtile arrays
  in general visit the LibArt homepage at 
  http://www.levien.com/libart/uta.html .
  
  Note that this implementation is not the one of LibArt and
  differs from it.
 */
class DECLSPEC Uta
{
public:
	//! Create an empty unitialized uta.
	/*!
	  \note You must initialize the class using
	        Uta::init() before doing anything with it.
	 */
	Uta ();

	//! Create a new microtile array with given coordinates.
	Uta (int x, int y, int w, int h);

	//! Destroy the microtile array.
	~Uta ();

	//! Clear the microtile array.
	void clear ();

	//! Initialize the array.
	/*!
	  You can use this funtion to resize the array as well, 
	  but you should be aware that the content of the array
	  will be cleared.
	 */
	void init (int x, int y, int w, int h);
	
	//! Return the microtile array as rectangles.
	/*!
	  The \a rects array should be freed when no longer needed.
	  \return The number of rectangles in \a rects.
	 */
	int getRectangles (SDL::Rect * & rects,
			   int min_x, int max_x, int min_y, int max_y) const;

	//! Return the microtile array as rectangles.
	/*!
	  The \a rects array should be freed when no longer needed.
	  \return The number of rectangles in \a rects.
	 */
	int getRectangles (SDL::Rect * & rects) const;			  

	//! Whether the microtile array contains given rectangle.
	bool contains (const SDL::Rect &) const;
	
	//! Whether the microtile array intersects given rectangle.
	bool intersects (const SDL::Rect &) const;
	
	//! Unite the microtile array with a rectangle.
	void unite (const SDL::Rect &);
	
	//! Unite the microtile array with a rectangle.
	Uta & operator+= (const SDL::Rect &);
	
	//! Unite the array with another microtile array.
	void unite (const Uta &);

	//! Unite the array with another microtile array.
	Uta & operator+= (const Uta &);

protected:	
	typedef struct _GemUta GemUta;
	GemUta *m_uta;
	int m_x, m_y, m_w, m_h;
}; // class Uta

} // namespace SEL


#endif // __sel_uta_h__
