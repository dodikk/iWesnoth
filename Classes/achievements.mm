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


#define KILLS_25 @"112754"
#define KILLS_50 @"112764"
#define KILLS_100 @"112774"
#define KILLS_500 @"112784"
#define KILLS_1000 @"112794"
#define RICH_ARMY @"112804"
#define MONEY_HOARDER @"112814"
#define VETERAN_UNIT @"112824"
#define HEROIC_UNIT @"112844"
#define FEARLESS_LEADER @"112904"
#define RAMPAGE @"113334"
#define DOMINATING @"113344"
#define LARGE_ARMY @"113354"
#define MASSIVE_ARMY @"113364"
#define EFFICIENT_ARMY @"113374"
#define PERFECT_STRATEGY @"113384"
#define EDUCATED_PLAYER @"113394"
#define VETERAN_PLAYER @"113404"
#define HEROIC_PLAYER @"113414"
#define CHAMPION_PLAYER @"113424" 

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

void earn_achievement(int achievement)
{
	[OFAchievementService unlockAchievement:KILLS_25];
}


