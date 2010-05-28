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

#ifndef ACHIEVEMENTS_H_INCLUDED
#define ACHIEVEMENTS_H_INCLUDED

#define ACHIEVEMENT_BLOODIED		0
#define ACHIEVEMENT_SLAYER			1
#define ACHIEVEMENT_BATTLE_MASTER	2
#define ACHIEVEMENT_BONE_CRUSHER	3
#define ACHIEVEMENT_PREDATOR		4
#define ACHIEVEMENT_PENNY_PINCHER	5
#define ACHIEVEMENT_MONEY_HOARDER	6
#define ACHIEVEMENT_VETERAN_UNIT	7
#define ACHIEVEMENT_HEROIC_UNIT		8
#define ACHIEVEMENT_FEARLESS_LEADER	9
#define ACHIEVEMENT_BESERK			10
#define ACHIEVEMENT_RAMPAGE			11
#define ACHIEVEMENT_PROFICIENT_COMMANDER	12
#define ACHIEVEMENT_GREAT_GENERAL	13
#define ACHIEVEMENT_DIVINE_BLESSING	14
#define ACHIEVEMENT_LIGHTNING_QUICK_BLADES	15
#define ACHIEVEMENT_RECRUIT_OF_WESNOTH	16
#define ACHIEVEMENT_DEFENDER_OF_WESNOTH	17
#define ACHIEVEMENT_HERO_OF_WESNOTH	18
#define ACHIEVEMENT_CHAMPION_OF_WESNOTH	19

#include <string>

void of_init(void);					// Initialize OpenFeint
void of_dashboard(void);			// Open the OpenFeint dashboard
void earn_achievement(int achievement, bool show_dlg = true);
std::string achievement_name(int achievement);
#endif
