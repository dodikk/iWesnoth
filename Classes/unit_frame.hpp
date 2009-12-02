/* $Id: unit_frame.hpp 35578 2009-05-11 21:45:03Z boucman $ */
/*
   Copyright (C) 2006 - 2009 by Jeremy Rosen <jeremy.rosen@enst-bretagne.fr>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 *  @file unit_frame.hpp
 *  Frame for unit's animation sequence.
 */

#ifndef UNIT_FRAME_H_INCLUDED
#define UNIT_FRAME_H_INCLUDED

#include "map_location.hpp"
#include "util.hpp"
#include "image.hpp"
#include "serialization/string_utils.hpp"
#include "game_display.hpp"

class config;

#include <string>
#include <vector>

class progressive_string {
	public:
		progressive_string(const shared_string& data = "",int duration = 0);
		int duration() const;
		const shared_string & get_current_element(int time) const;
		bool does_not_change() const { return data_.size() <= 1; }
		shared_string get_original(){return input_;}
	private:
		std::vector<std::pair<shared_string,int> > data_;
		shared_string input_;
};

template <class T>
class progressive_
{
	std::vector<std::pair<std::pair<T, T>, int> > data_;
	shared_string input_;
public:
	progressive_(const shared_string& data = "", int duration = 0);
	int duration() const;
	const T get_current_element(int time,T default_val=0) const;
	bool does_not_change() const;
		shared_string get_original(){return input_;}
};

typedef progressive_<int> progressive_int;
typedef progressive_<double> progressive_double;

#endif
// This hack prevents MSVC++ 6 to issue several warnings
#ifndef UNIT_FRAME_H_PART2
#define UNIT_FRAME_H_PART2
/** All parameters from a frame at a given instant */
class frame_parameters{
	public:
	frame_parameters():
	image(),
	image_diagonal(),
	image_mod(""),
	halo(""),
	halo_x(0),
	halo_y(0),
	halo_mod(""),
	sound(""),
	text(""),
	text_color(0),
	duration(0),
	blend_with(0),
	blend_ratio(0.0),
	highlight_ratio(1.0),
	offset(0),
	submerge(0.0),
	x(0),
	y(0),
	drawing_layer(display::LAYER_UNIT_DEFAULT-display::LAYER_UNIT_FIRST)
	{};

	image::locator image;
	image::locator image_diagonal;
	shared_string image_mod;
	shared_string halo;
	short halo_x;
	short halo_y;
	shared_string halo_mod;
	shared_string sound;
	shared_string text;
	Uint32 text_color;
	int duration;
	Uint32 blend_with;
	double blend_ratio;
	double highlight_ratio;
	double offset;
	double submerge;
	short x;
	short y;
	short drawing_layer;
	bool in_hex;
	bool diagonal_in_hex;
} ;
/**
 * keep most parameters in a separate class to simplify handling of large
 * number of parameters
 */
class frame_builder {
	public:
		/** initial constructor */
		frame_builder():
		image_(image::locator()),
		image_diagonal_(image::locator()),
		image_mod_(""),
		halo_(""),
		halo_x_(""),
		halo_y_(""),
		halo_mod_(""),
		sound_(""),
		text_(""),
		text_color_(0),
		duration_(1),
		blend_with_(0),
		blend_ratio_(""),
		highlight_ratio_(""),
		offset_(""),
		submerge_(""),
		x_(""),
		y_(""),
		drawing_layer_(lexical_cast<std::string>(display::LAYER_UNIT_DEFAULT-display::LAYER_UNIT_FIRST))
	{};
		frame_builder(const config& cfg,const shared_string &frame_string = "");
		/** allow easy chained modifications will raised assert if used after initialization */
		frame_builder & image(const image::locator image ,const shared_string & image_mod="");
		frame_builder & image_diagonal(const image::locator image_diagonal,const shared_string & image_mod="");
		frame_builder & sound(const shared_string& sound);
		frame_builder & text(const shared_string& text,const  Uint32 text_color);
		frame_builder & halo(const shared_string &halo, const shared_string &halo_x, const shared_string& halo_y,const shared_string& halo_mod);
		frame_builder & duration(const int duration);
		frame_builder & blend(const shared_string& blend_ratio,const Uint32 blend_color);
		frame_builder & highlight(const shared_string& highlight);
		frame_builder & offset(const shared_string& offset);
		frame_builder & submerge(const shared_string& submerge);
		frame_builder & x(const shared_string& x);
		frame_builder & y(const shared_string& y);
		frame_builder & drawing_layer(const shared_string& drawing_layer);
		/** getters for the different parameters */
		const frame_parameters parameters(int current_time) const ;

		int duration() const{ return duration_;};
		void recalculate_duration();
		bool does_not_change() const;
		bool need_update() const;
	private:
		image::locator image_;
		image::locator image_diagonal_;
		shared_string image_mod_;
		progressive_string halo_;
		progressive_int halo_x_;
		progressive_int halo_y_;
		shared_string halo_mod_;
		shared_string sound_;
		shared_string text_;
		Uint32 text_color_;
		int duration_;
		Uint32 blend_with_;
		progressive_double blend_ratio_;
		progressive_double highlight_ratio_;
		progressive_double offset_;
		progressive_double submerge_;
		progressive_int x_;
		progressive_int y_;
		progressive_int drawing_layer_;
};

/** Describe a unit's animation sequence. */
class unit_frame {
	public:
		// Constructors
		unit_frame(const frame_builder builder=frame_builder()):builder_(builder){};
		void redraw(const int frame_time,bool first_time,const map_location & src,const map_location & dst,int*halo_id,const frame_parameters & animation_val,const frame_parameters & engine_val,const bool primary)const;
		const frame_parameters merge_parameters(int current_time,const frame_parameters & animation_val,const frame_parameters & engine_val=frame_parameters(),bool primary=false) const;
		const frame_parameters parameters(int current_time) const {return builder_.parameters(current_time);};

		int duration() const { return builder_.duration();};
		bool does_not_change() const{ return builder_.does_not_change();};
		bool need_update() const{ return builder_.need_update();};
		bool invalidate(const bool force,const int frame_time,const map_location & src,const map_location & dst,const frame_parameters & animation_val,const frame_parameters & engine_val,const bool primary) const;
	private:
		frame_builder builder_;

};

#endif
