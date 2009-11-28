/* $Id: race.hpp 31859 2009-01-01 10:28:26Z mordante $ */
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

#ifndef RACE_HPP_INCLUDED
#define RACE_HPP_INCLUDED

#include <map>
#include <string>
#include <vector>

#include "serialization/string_utils.hpp"
#include "config.hpp"

namespace rand_rng {
	class simple_rng;
} // namespace rand_rng


typedef std::map<wide_string, std::vector<wchar_t> > markov_prefix_map;

class unit_race
{
public:
	enum GENDER { MALE, FEMALE, NUM_GENDERS };

	unit_race();
	unit_race(const config& cfg);

	const shared_string& id() const { return id_; };
	const shared_string& name(GENDER gender=MALE) const { return name_[gender]; };
	const shared_string& plural_name() const { return plural_name_; };
	const shared_string& description() const { return description_; };

	std::string generate_name(GENDER gender, rand_rng::simple_rng* rng = 0) const;

	bool uses_global_traits() const;

	const config::child_list& additional_traits() const;
	unsigned int num_traits() const;

private:
	shared_string id_;
	shared_string name_[NUM_GENDERS];
	shared_string plural_name_;
	shared_string description_;
	unsigned int ntraits_;
	markov_prefix_map next_[NUM_GENDERS];
	int chain_size_;

	const config::child_list* traits_;
	bool global_traits_;
};

unit_race::GENDER string_gender(const std::string& str,unit_race::GENDER def=unit_race::MALE);
std::string const& gender_string(unit_race::GENDER gender);

//typedef std::map<std::string,unit_race> race_map;
typedef std::map<shared_string,unit_race> race_map;

#endif
