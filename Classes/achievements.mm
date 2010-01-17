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

#include "achievements.h"

#include "OpenFeint.h"

#include "OFAchievementService.h"
#include "OFAchievement.h"

#include "game_preferences.hpp"

#include "construct_dialog.hpp"
#include "game_display.hpp"
#include "stdio.h"

std::string names[] = { "Bloodied", "Slayer", "Battle Master", "Bone Crusher", "Predator", "Penny Pincher", "Money Hoarder", "Veteran Unit",
	"Heroic Unit", "Fearless Leader", "Beserk", "Rampage", "Proficient Commander", "Great General", "Divine Blessing", "Lightning Quick Blades",
	"Recruit of Wesnoth", "Defender of Wesnoth", "Hero of Wesnoth", "Champion of Wesnoth"};

NSString* ids[] = { @"112754", @"112764", @"112774", @"112784", @"112794", @"112804", @"112814", @"112824", @"112844", @"112904", @"113334",
	@"113344", @"113354", @"113364", @"113374", @"113384", @"113394", @"113404", @"113414", @"113424"};

std::string desc[] = { "Kill 25 enemy units", "Kill 50 enemy units", "Kill 100 enemy units", "Kill 500 enemy units", "Kill 1000 enemy units",
"Collect 300 gold", "Collect 500 gold", "Level up a unit to level 3", "Level up a unit to level 4", "Level up your leader", "Kill 3 units in a turn",
"Kill 5 units in a turn", "Control 20 units", "Control 30 units", "Finish a mission 10 turns early", "Finish a mission 15 turns early",
"Complete the tutorial", "Complete a full campaign on easy difficulty", "Complete a full campaign on normal difficulty",
"Complete a full campaign on hard difficulty"};

bool gInitialized = false;

void of_init(void)
{
	if (gInitialized)
		return;
	
	NSDictionary* settings = [NSDictionary dictionaryWithObjectsAndKeys:
							  [NSNumber numberWithInt:UIInterfaceOrientationLandscapeRight], OpenFeintSettingDashboardOrientation,
							  @"Wesnoth", OpenFeintSettingShortDisplayName, 
							  [NSNumber numberWithBool:YES], OpenFeintSettingEnablePushNotifications,
							  [NSNumber numberWithBool:NO], OpenFeintSettingDisableUserGeneratedContent,
							  [NSNumber numberWithBool:YES], OpenFeintSettingPromptToPostAchievementUnlock,
							  nil
							  ];
	
	[OpenFeint initializeWithProductKey:@"WDYPKxGL2aP5ujZFrnKmQ" andSecret:@"WoRs8Pt7ch9r0Re1UI7Zx5d62gezqfHDLPy4t5suQE" andDisplayName:@"Wesnoth" andSettings:settings andDelegates:nil];
	gInitialized = true;
}

void of_dashboard(void)
{
	[OpenFeint launchDashboard];
}

std::string achievement_name(int achievement)
{
	return names[achievement];
}

void earn_achievement(int achievement, bool show_dlg)
{
	if (preferences::achievement_earned(achievement))
		return;
	
	preferences::achievement_add(achievement);
	
	// display dialog
	if (show_dlg == true)
	{
		char img_str[50];
		sprintf(img_str, "data/core/images/achievements/%02d.png", achievement+1);
		surface surf(image::get_image(img_str, image::UNSCALED, false));
		std::string text, caption;
		caption = "Achievement Unlocked: ";
		caption += names[achievement];
		//wml_event_dialog to_show(*screen,"",desc[achievement]);
		gui::message_dialog dlg(*(game_display::get_singleton()), "", desc[achievement]);
		if(!surf.null()) {
			dlg.set_image(surf, caption);
		}
		dlg.layout();
		dlg.show(300);
	}
	
	// do OpenFeint stuff
	[OFAchievementService unlockAchievement:ids[achievement]];
}
