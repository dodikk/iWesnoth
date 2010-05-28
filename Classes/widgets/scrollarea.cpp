/* $Id: scrollarea.cpp 31858 2009-01-01 10:27:41Z mordante $*/
/*
   Copyright (C) 2004 - 2009 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/** @file widgets/scrollarea.cpp */

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "global.hpp"

#include "widgets/scrollarea.hpp"


namespace gui {

scrollarea::scrollarea(CVideo &video, const bool auto_join)
	: widget(video, auto_join), scrollbar_(video),
	  old_position_(0), recursive_(false), shown_scrollbar_(false),
	  shown_size_(0), full_size_(0)
{
	scrollbar_.hide(true);
}

bool scrollarea::has_scrollbar() const
{
	return shown_size_ < full_size_ && scrollbar_.is_valid_height(location().h);
}

handler_vector scrollarea::handler_members()
{
	handler_vector h;
	h.push_back(&scrollbar_);
	return h;
}

void scrollarea::update_location(SDL_Rect const &rect)
{
	SDL_Rect r = rect;
	shown_scrollbar_ = has_scrollbar();
	if (shown_scrollbar_) {
		int w = r.w - scrollbar_.width();
		r.x += w;
		r.w -= w;
		scrollbar_.set_location(r);
		r.x -= w;
		r.w = w;
	}

	if (!hidden())
		scrollbar_.hide(!shown_scrollbar_);
	set_inner_location(r);
}

void scrollarea::test_scrollbar()
{
	if (recursive_)
		return;
	recursive_ = true;
	if (shown_scrollbar_ != has_scrollbar()) {
		bg_restore();
		bg_cancel();
		update_location(location());
	}
	recursive_ = false;
}

void scrollarea::hide(bool value)
{
	widget::hide(value);
	if (shown_scrollbar_)
		scrollbar_.hide(value);
}

unsigned scrollarea::get_position() const
{
	return scrollbar_.get_position();
}

unsigned scrollarea::get_max_position() const
{
	return scrollbar_.get_max_position();
}

void scrollarea::set_position(unsigned pos)
{
	scrollbar_.set_position(pos);
}

void scrollarea::adjust_position(unsigned pos)
{
	scrollbar_.adjust_position(pos);
}

void scrollarea::move_position(int dep)
{
	scrollbar_.move_position(dep);
}

void scrollarea::set_shown_size(unsigned h)
{
	scrollbar_.set_shown_size(h);
	shown_size_ = h;
	test_scrollbar();
}

void scrollarea::set_full_size(unsigned h)
{
	scrollbar_.set_full_size(h);
	full_size_ = h;
	test_scrollbar();
}

void scrollarea::set_scroll_rate(unsigned r)
{
	scrollbar_.set_scroll_rate(r);
}

void scrollarea::process_event()
{
	int grip_position = scrollbar_.get_position();
	if (grip_position == old_position_)
		return;
	old_position_ = grip_position;
	scroll(grip_position);
}

SDL_Rect scrollarea::inner_location() const
{
	SDL_Rect r = location();
	if (shown_scrollbar_)
		r.w -= scrollbar_.width();
	return r;
}

unsigned scrollarea::scrollbar_width() const
{
	return scrollbar_.width();
}

void scrollarea::handle_event(const SDL_Event& event)
{
	
	if (mouse_locked() || hidden() || event.type != SDL_MOUSEBUTTONDOWN)
		return;

	SDL_MouseButtonEvent const &e = event.button;
	if (point_in_rect(e.x, e.y, inner_location())) {
		if (e.button == SDL_BUTTON_WHEELDOWN) {
			scrollbar_.scroll_down();
		} else if (e.button == SDL_BUTTON_WHEELUP) {
			scrollbar_.scroll_up();
		}
	}
	
}
	
bool scrollarea::handle_drag_event(const SDL_Event& event)
{
	// KP: added content dragging for iPhone
	static bool isDragging = false;
	static int startDrag = 0;
	static int startPos = 0;
	static bool didDrag = false;
	SDL_MouseButtonEvent const &e = event.button;
	if (point_in_rect(e.x, e.y, inner_location()))
	{
		switch(event.type)
		{
			case SDL_MOUSEBUTTONUP:
			{
				isDragging = false;
				break;
			}
			case SDL_MOUSEBUTTONDOWN:
			{
				isDragging = true;
				startDrag = e.y;
				startPos = get_position();
				didDrag = false;
				break;
			}
			case SDL_MOUSEMOTION:
			{
				if (isDragging)
				{
					int changeY = startDrag - e.y;
					//SDL_Rect inner_loc = inner_location();
					//float pixelsPerScroll = item_height_ / ((float) (item_height_ * nitems()) / inner_loc.h);
					//int move = changeY / pixelsPerScroll;
					int move = changeY;
					if (move != 0)
					{
						int newPos = startPos + move;
						if (newPos < 0)
							newPos = 0;
						if (newPos > get_max_position())
							newPos = get_max_position();
						set_position(newPos);
						didDrag = true;
						
						// fixes dragging off of control
						startDrag = e.y;
						startPos = newPos;
					}
				}
				break;
			}
			default:
				break;
		}
	}
	else if (event.type == SDL_MOUSEBUTTONUP)
	{
		isDragging = false;
	}
	
	// do not select after dragging
	if (didDrag == true && isDragging == false)
	{
		didDrag = false;
		return true;
	}
	
	return false;
	// END iPhone content dragging
	
}

} // end namespace gui

