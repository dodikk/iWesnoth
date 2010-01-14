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

#include "UnitTextureAtlas.h"


#include <OpenGLES/ES1/gl.h>
#include <map>

#include "SDL_image.h"
#include "filesystem.hpp"
#include "video.hpp"

#include <OpenGLES/ES1/glext.h>
#include "stdio.h"


#include "game_config.hpp"

#include "RenderQueue.h"

#include <assert.h>
#include "foreach.hpp"
#include <list>
#include "shared_string.hpp"

std::map<shared_string, textureAtlasInfo> gUnitAtlasMap;
GLuint gUnitTexIds[NUM_UNITMAPS];
unsigned int gUnitTexW[NUM_UNITMAPS];
unsigned int gUnitTexH[NUM_UNITMAPS];

#define MAX_UNIT_ATLAS_TOTAL_SIZE	3*1024*1024

typedef struct
{
	unsigned short mapId;
	unsigned long size;
} unitAtlasData;
std::list<unitAtlasData> gUnitAtlasLRU;
unsigned long gUnitAtlasTotalSize = 0;

void addMapEntry(const char *filename, unsigned short texOffsetX, unsigned short texOffsetY, unsigned short flags, unsigned short texW, 
				 unsigned short texH, unsigned short trimmedX, unsigned short trimmedY, unsigned short originalW, unsigned short originalH,
				 unsigned short mapId, bool justToBeDifferentFromTextureAtlas=true)
{
	textureAtlasInfo tinfo;
	tinfo.texOffsetX = texOffsetX;
	tinfo.texOffsetY = texOffsetY;
	tinfo.flags = flags;
	tinfo.texW = texW;
	tinfo.texH = texH;
	tinfo.trimmedX = trimmedX;
	tinfo.trimmedY = trimmedY;
	tinfo.originalW = originalW;
	tinfo.originalH = originalH;
	tinfo.mapId = mapId;
	gUnitAtlasMap.insert(std::pair<shared_string, textureAtlasInfo>(shared_string(filename), tinfo));
}

Uint32 argb_to_abgr(Uint32 color)
{
	Uint8 a = (color >> 24) & 0xFF;
	Uint8 r = (color >> 16) & 0xFF;
	Uint8 g = (color >>  8) & 0xFF;
	Uint8 b = (color      ) & 0xFF;
	
	Uint32 abgr = (a << 24) | (b << 16) | (g << 8) | r;
	return abgr;
}

void loadUnitMap(unsigned short mapId)
{
	std::string filename = game_config::path;
	filename += "/data/core/images/unitmaps/";
	
	short pvrtcSize = 0;
	
	int color = mapId % 10;
	mapId = mapId - color;
	
	switch(mapId)
	{
		case UNITMAP_ELLIPSE:
			filename = game_config::path + "/images/misc/map.ellipse.png";
			break;
		case MAP_BLACK_FLAG:
			filename = game_config::path + "/images/flags/map.black-flag.png";
			break;
		case MAP_FLAG:
			filename = game_config::path + "/images/flags/map.flag.png";
			break;
		case MAP_KNALGAN_FLAG:
			filename = game_config::path + "/images/flags/map.knalgan-flag.png";
			break;
		case MAP_LOYALIST_FLAG:
			filename = game_config::path + "/images/flags/map.loyalist-flag.png";
			break;
		case MAP_SG_FLAG:
			filename = game_config::path + "/images/flags/map.SG-flag.png";
			break;
		case MAP_UNDEAD_FLAG:
			filename = game_config::path + "/images/flags/map.undead-flag.png";
			break;
		case UNITMAP_DRAKES_DRAKES:
			filename += "map.drakes.drakes.png";
			break;
		case UNITMAP_DRAKES_ARMAGEDDON:
			filename += "map.drakes.armageddon.png";
			break;
		case UNITMAP_DRAKES_BLADEMASTER:
			filename += "map.drakes.blademaster.png";
			break;
		case UNITMAP_DRAKES_BURNER:
			filename += "map.drakes.burner.png";
			break;
		case UNITMAP_DRAKES_CLASHER:
			filename += "map.drakes.clasher.png";
			break;
		case UNITMAP_DRAKES_ENFORCER:
			filename += "map.drakes.enforcer.png";
			break;
		case UNITMAP_DRAKES_FIGHTER:
			filename += "map.drakes.fighter.png";
			break;
		case UNITMAP_DRAKES_FIRE:
			filename += "map.drakes.fire.png";
			break;
		case UNITMAP_DRAKES_FLAMEHEART:
			filename += "map.drakes.flameheart.png";
			break;
		case UNITMAP_DRAKES_FLARE:
			filename += "map.drakes.flare.png";
			break;
		case UNITMAP_DRAKES_GLADIATOR:
			filename += "map.drakes.gladiator.png";
			break;
		case UNITMAP_DRAKES_GLIDER:
			filename += "map.drakes.glider.png";
			break;
		case UNITMAP_DRAKES_HURRICANE:
			filename += "map.drakes.hurricane.png";
			break;
		case UNITMAP_DRAKES_INFERNO:
			filename += "map.drakes.inferno.png";
			break;
		case UNITMAP_DRAKES_SKY:
			filename += "map.drakes.sky.png";
			break;
		case UNITMAP_DRAKES_SLASHER:
			filename += "map.drakes.slasher.png";
			break;
		case UNITMAP_DRAKES_WARDEN:
			filename += "map.drakes.warden.png";
			break;
		case UNITMAP_DRAKES_WARRIOR:
			filename += "map.drakes.warrior.png";
			break;
			
		case UNITMAP_DWARVES_DWARVES:
			filename += "map.dwarves.dwarves.png";
			break;
		case UNITMAP_DWARVES_BERSERKER:
			filename += "map.dwarves.berserker.png";
			break;
		case UNITMAP_DWARVES_DRAGONGUARD:
			filename += "map.dwarves.dragonguard.png";
			break;
		case UNITMAP_DWARVES_FIGHTER:
			filename += "map.dwarves.fighter.png";
			break;
		case UNITMAP_DWARVES_GRYPHON_MASTER:
			filename += "map.dwarves.gryphon-master.png";
			break;
		case UNITMAP_DWARVES_GRYPHON_RIDER:
			filename += "map.dwarves.gryphon-rider.png";
			break;
		case UNITMAP_DWARVES_GUARD:
			filename += "map.dwarves.guard.png";
			break;
		case UNITMAP_DWARVES_LORD:
			filename += "map.dwarves.lord.png";
			break;
		case UNITMAP_DWARVES_RUNEMASTER:
			filename += "map.dwarves.runemaster.png";
			break;
		case UNITMAP_DWARVES_SENTINEL:
			filename += "map.dwarves.sentinel.png";
			break;
		case UNITMAP_DWARVES_STALWART:
			filename += "map.dwarves.stalwart.png";
			break;
		case UNITMAP_DWARVES_STEELCLAD:
			filename += "map.dwarves.steelclad.png";
			break;
		case UNITMAP_DWARVES_THUNDERER:
			filename += "map.dwarves.thunderer.png";
			break;
		case UNITMAP_DWARVES_THUNDERGUARD:
			filename += "map.dwarves.thunderguard.png";
			break;
		case UNITMAP_DWARVES_ULFSERKER:
			filename += "map.dwarves.ulfserker.png";
			break;
			
		case UNITMAP_ELVES_ELVES1:
			filename += "map.elves.elves1.png";
			break;
		case UNITMAP_ELVES_ELVES2:
			filename += "map.elves.elves2.png";
			break;
		case UNITMAP_ELVES_ARCHER:
			filename += "map.elves.archer.png";
			break;
		case UNITMAP_ELVES_ARCHER_FEMALE:
			filename += "map.elves.archer+female.png";
			break;
		case UNITMAP_ELVES_AVENGER:
			filename += "map.elves.avenger.png";
			break;
		case UNITMAP_ELVES_AVENGER_FEMALE:
			filename += "map.elves.avenger+female.png";
			break;
		case UNITMAP_ELVES_CAPTAIN:
			filename += "map.elves.captain.png";
			break;
		case UNITMAP_ELVES_CHAMPION:
			filename += "map.elves.champion.png";
			break;
		case UNITMAP_ELVES_DRUID:
			filename += "map.elves.druid.png";
			break;
		case UNITMAP_ELVES_ENCHANTRESS:
			filename += "map.elves.enchantress.png";
			break;
		case UNITMAP_ELVES_FIGHTER:
			filename += "map.elves.fighter.png";
			break;
		case UNITMAP_ELVES_HERO:
			filename += "map.elves.hero.png";
			break;
		case UNITMAP_ELVES_HIGH_LORD:
			filename += "map.elves.high-lord.png";
			break;
		case UNITMAP_ELVES_LORD:
			filename += "map.elves.lord.png";
			break;
		case UNITMAP_ELVES_MARKSMAN:
			filename += "map.elves.marksman.png";
			break;
		case UNITMAP_ELVES_MARKSMAN_FEMALE:
			filename += "map.elves.marksman+female.png";
			break;
		case UNITMAP_ELVES_MARSHAL:
			filename += "map.elves.marshal.png";
			break;
		case UNITMAP_ELVES_OUTRIDER:
			filename += "map.elves.outrider.png";
			break;
		case UNITMAP_ELVES_RANGER:
			filename += "map.elves.ranger.png";
			break;
		case UNITMAP_ELVES_RANGER_FEMALE:
			filename += "map.elves.ranger+female.png";
			break;
		case UNITMAP_ELVES_RIDER:
			filename += "map.elves.rider.png";
			break;
		case UNITMAP_ELVES_SCOUT:
			filename += "map.elves.scout.png";
			break;
		case UNITMAP_ELVES_SHAMAN:
			filename += "map.elves.shaman.png";
			break;
		case UNITMAP_ELVES_SHARPSHOOTER:
			filename += "map.elves.sharpshooter.png";
			break;
		case UNITMAP_ELVES_SHARPSHOOTER_FEMALE:
			filename += "map.elves.sharpshooter+female.png";
			break;
		case UNITMAP_ELVES_SHYDE:
			filename += "map.elves.shyde.png";
			break;
		case UNITMAP_ELVES_SORCERESS:
			filename += "map.elves.sorceress.png";
			break;
		case UNITMAP_ELVES_SYLPH:
			filename += "map.elves.sylph.png";
			break;
			
		case UNITMAP_GOBLINS_GOBLINS:
			filename += "map.goblins.goblins.png";
			break;
		case UNITMAP_GOBLINS_DIREWOLVER:
			filename += "map.goblins.direwolver.png";
			break;
		case UNITMAP_GOBLINS_IMPALER:
			filename += "map.goblins.impaler.png";
			break;
		case UNITMAP_GOBLINS_KNIGHT:
			filename += "map.goblins.knight.png";
			break;
		case UNITMAP_GOBLINS_PILLAGER:
			filename += "map.goblins.pillager.png";
			break;
		case UNITMAP_GOBLINS_ROUSER:
			filename += "map.goblins.rouser.png";
			break;
		case UNITMAP_GOBLINS_SPEARMAN:
			filename += "map.goblins.spearman.png";
			break;
		case UNITMAP_GOBLINS_WOLF_RIDER:
			filename += "map.goblins.wolf-rider.png";
			break;
			
		case UNITMAP_HUMANLOYALISTS_HUMANLOYALISTS:
			filename += "map.humanloyalists.human-loyalists.png";
			break;
		case UNITMAP_HUMANLOYALISTS_BOWMAN:
			filename += "map.humanloyalists.bowman.png";
			break;
		case UNITMAP_HUMANLOYALISTS_CAVALIER:
			filename += "map.humanloyalists.cavalier.png";
			break;
		case UNITMAP_HUMANLOYALISTS_CAVALRYMAN:
			filename += "map.humanloyalists.cavalryman.png";
			break;
		case UNITMAP_HUMANLOYALISTS_DRAGOON:
			filename += "map.humanloyalists.dragoon.png";
			break;
		case UNITMAP_HUMANLOYALISTS_DUELIST:
			filename += "map.humanloyalists.duelist.png";
			break;
		case UNITMAP_HUMANLOYALISTS_FENCER:
			filename += "map.humanloyalists.fencer.png";
			break;
		case UNITMAP_HUMANLOYALISTS_GENERAL:
			filename += "map.humanloyalists.general.png";
			break;
		case UNITMAP_HUMANLOYALISTS_GRAND_KNIGHT:
			filename += "map.humanloyalists.grand-knight.png";
			break;
		case UNITMAP_HUMANLOYALISTS_HALBERDIER:
			filename += "map.humanloyalists.halberdier.png";
			break;
		case UNITMAP_HUMANLOYALISTS_HEAVYINFANTRY:
			filename += "map.humanloyalists.heavyinfantry.png";
			break;
		case UNITMAP_HUMANLOYALISTS_HORSEMAN:
			filename += "map.humanloyalists.horseman.png";
			break;
		case UNITMAP_HUMANLOYALISTS_JAVELINEER:
			filename += "map.humanloyalists.javelineer.png";
			break;
		case UNITMAP_HUMANLOYALISTS_KNIGHT:
			filename += "map.humanloyalists.knight.png";
			break;
		case UNITMAP_HUMANLOYALISTS_LANCER:
			filename += "map.humanloyalists.lancer.png";
			break;
		case UNITMAP_HUMANLOYALISTS_LIEUTENANT:
			filename += "map.humanloyalists.lieutenant.png";
			break;
		case UNITMAP_HUMANLOYALISTS_LONGBOWMAN:
			filename += "map.humanloyalists.longbowman.png";
			break;
		case UNITMAP_HUMANLOYALISTS_MARSHAL:
			filename += "map.humanloyalists.marshal.png";
			break;
		case UNITMAP_HUMANLOYALISTS_MASTER_AT_ARMS:
			filename += "map.humanloyalists.master-at-arms.png";
			break;
		case UNITMAP_HUMANLOYALISTS_MASTERBOWMAN:
			filename += "map.humanloyalists.masterbowman.png";
			break;
		case UNITMAP_HUMANLOYALISTS_PALADIN:
			filename += "map.humanloyalists.paladin.png";
			break;
		case UNITMAP_HUMANLOYALISTS_PIKEMAN:
			filename += "map.humanloyalists.pikeman.png";
			break;
		case UNITMAP_HUMANLOYALISTS_ROYAL_WARRIOR:
			filename += "map.humanloyalists.royal-warrior.png";
			break;
		case UNITMAP_HUMANLOYALISTS_ROYALGUARD:
			filename += "map.humanloyalists.royalguard.png";
			break;
		case UNITMAP_HUMANLOYALISTS_SERGEANT:
			filename += "map.humanloyalists.sergeant.png";
			break;
		case UNITMAP_HUMANLOYALISTS_SHOCKTROOPER:
			filename += "map.humanloyalists.shocktrooper.png";
			break;
		case UNITMAP_HUMANLOYALISTS_SIEGETROOPER:
			filename += "map.humanloyalists.siegetrooper.png";
			break;
		case UNITMAP_HUMANLOYALISTS_SPEARMAN:
			filename += "map.humanloyalists.spearman.png";
			break;
		case UNITMAP_HUMANLOYALISTS_SWORDSMAN:
			filename += "map.humanloyalists.swordsman.png";
			break;
			
		case UNITMAP_HUMANMAGI_HUMANMAGI:
			filename += "map.humanmagi.human-magi.png";
			break;
		case UNITMAP_HUMANMAGI_ARCH_MAGE:
			filename += "map.humanmagi.arch-mage.png";
			break;
		case UNITMAP_HUMANMAGI_ARCH_MAGE_FEMALE:
			filename += "map.humanmagi.arch-mage+female.png";
			break;
		case UNITMAP_HUMANMAGI_ELDER_MAGE:
			filename += "map.humanmagi.elder-mage.png";
			break;
		case UNITMAP_HUMANMAGI_GREAT_MAGE:
			filename += "map.humanmagi.great-mage.png";
			break;
		case UNITMAP_HUMANMAGI_GREAT_MAGE_FEMALE:
			filename += "map.humanmagi.great-mage+female.png";
			break;
		case UNITMAP_HUMANMAGI_MAGE:
			filename += "map.humanmagi.mage.png";
			break;
		case UNITMAP_HUMANMAGI_MAGE_FEMALE:
			filename += "map.humanmagi.mage+female.png";
			break;
		case UNITMAP_HUMANMAGI_RED_MAGE:
			filename += "map.humanmagi.red-mage.png";
			break;
		case UNITMAP_HUMANMAGI_RED_MAGE_FEMALE:
			filename += "map.humanmagi.red-mage+female.png";
			break;
		case UNITMAP_HUMANMAGI_SILVER_MAGE:
			filename += "map.humanmagi.silver-mage.png";
			break;
		case UNITMAP_HUMANMAGI_SILVER_MAGE_FEMALE:
			filename += "map.humanmagi.silver-mage+female.png";
			break;
		case UNITMAP_HUMANMAGI_WHITE_CLERIC:
			filename += "map.humanmagi.white-cleric.png";
			break;
		case UNITMAP_HUMANMAGI_WHITE_CLERIC_FEMALE:
			filename += "map.humanmagi.white-cleric+female.png";
			break;
		case UNITMAP_HUMANMAGI_WHITE_MAGE:
			filename += "map.humanmagi.white-mage.png";
			break;
		case UNITMAP_HUMANMAGI_WHITE_MAGE_FEMALE:
			filename += "map.humanmagi.white-mage+female.png";
			break;
			
		case UNITMAP_HUMANOUTLAWS_HUMANOUTLAWS:
			filename += "map.humanoutlaws.human-outlaws.png";
			break;
		case UNITMAP_HUMANOUTLAWS_ASSASSIN:
			filename += "map.humanoutlaws.assassin.png";
			break;
		case UNITMAP_HUMANOUTLAWS_ASSASSIN_FEMALE:
			filename += "map.humanoutlaws.assassin+female.png";
			break;
		case UNITMAP_HUMANOUTLAWS_BANDIT:
			filename += "map.humanoutlaws.bandit.png";
			break;
		case UNITMAP_HUMANOUTLAWS_FOOTPAD:
			filename += "map.humanoutlaws.footpad.png";
			break;
		case UNITMAP_HUMANOUTLAWS_FOOTPAD_FEMALE:
			filename += "map.humanoutlaws.footpad+female.png";
			break;
		case UNITMAP_HUMANOUTLAWS_FUGITIVE:
			filename += "map.humanoutlaws.fugitive.png";
			break;
		case UNITMAP_HUMANOUTLAWS_FUGITIVE_FEMALE:
			filename += "map.humanoutlaws.fugitive+female.png";
			break;
		case UNITMAP_HUMANOUTLAWS_HIGHWAYMAN:
			filename += "map.humanoutlaws.highwayman.png";
			break;
		case UNITMAP_HUMANOUTLAWS_HUNTSMAN:
			filename += "map.humanoutlaws.huntsman.png";
			break;
		case UNITMAP_HUMANOUTLAWS_OUTLAW:
			filename += "map.humanoutlaws.outlaw.png";
			break;
		case UNITMAP_HUMANOUTLAWS_OUTLAW_FEMALE:
			filename += "map.humanoutlaws.outlaw+female.png";
			break;
		case UNITMAP_HUMANOUTLAWS_POACHER:
			filename += "map.humanoutlaws.poacher.png";
			break;
		case UNITMAP_HUMANOUTLAWS_RANGER:
			filename += "map.humanoutlaws.ranger.png";
			break;
		case UNITMAP_HUMANOUTLAWS_ROGUE:
			filename += "map.humanoutlaws.rogue.png";
			break;
		case UNITMAP_HUMANOUTLAWS_ROGUE_FEMALE:
			filename += "map.humanoutlaws.rogue+female.png";
			break;
		case UNITMAP_HUMANOUTLAWS_THIEF:
			filename += "map.humanoutlaws.thief.png";
			break;
		case UNITMAP_HUMANOUTLAWS_THIEF_FEMALE:
			filename += "map.humanoutlaws.thief+female.png";
			break;
		case UNITMAP_HUMANOUTLAWS_THUG:
			filename += "map.humanoutlaws.thug.png";
			break;
		case UNITMAP_HUMANOUTLAWS_TRAPPER:
			filename += "map.humanoutlaws.trapper.png";
			break;

		case UNITMAP_HUMANPEASANTS_HUMANPEASANTS:
			filename += "map.humanpeasants.human-peasants.png";
			break;
		case UNITMAP_HUMANPEASANTS_PEASANT:
			filename += "map.humanpeasants.peasant.png";
			break;
		case UNITMAP_HUMANPEASANTS_RUFFIAN:
			filename += "map.humanpeasants.ruffian.png";
			break;
		case UNITMAP_HUMANPEASANTS_WOODSMAN:
			filename += "map.humanpeasants.woodsman.png";
			break;
			
		case UNITMAP_MERFOLK_MERFOLK:
			filename += "map.merfolk.merfolk.png";
			break;
		case UNITMAP_MERFOLK_INITIATE:
			filename += "map.merfolk.initiate.png";
			break;
			
		case UNITMAP_MONSTERS_CAVESPIDER:
			filename += "map.monsters.cavespider.png";
			break;
		case UNITMAP_MONSTERS_CUTTLEFISH:
			filename += "map.monsters.cuttlefish.png";
			break;
		case UNITMAP_MONSTERS_DEEP_TENTACLE:
			filename += "map.monsters.deep-tentacle.png";
			break;
		case UNITMAP_MONSTERS_FIRE_DRAGON:
			filename += "map.monsters.fire-dragon.png";
			break;
		case UNITMAP_MONSTERS_GIANT_MUDCRAWLER:
			filename += "map.monsters.giant-mudcrawler.png";
			break;
		case UNITMAP_MONSTERS_GRYPHON:
			filename += "map.monsters.gryphon.png";
			break;
		case UNITMAP_MONSTERS_MUDCRAWLER:
			filename += "map.monsters.mudcrawler.png";
			break;
		case UNITMAP_MONSTERS_SCORPION:
			filename += "map.monsters.scorpion.png";
			break;
		case UNITMAP_MONSTERS_SEASERPENT:
			filename += "map.monsters.seaserpent.png";
			break;
		case UNITMAP_MONSTERS_SKELETAL_DRAGON:
			filename += "map.monsters.skeletal-dragon.png";
			break;
		case UNITMAP_MONSTERS_WOLF:
			filename += "map.monsters.wolf.png";
			break;
		case UNITMAP_MONSTERS_YETI:
			filename += "map.monsters.yeti.png";
			break;

		case UNITMAP_NAGAS_NAGAS:
			filename += "map.nagas.nagas.png";
			break;
		case UNITMAP_NAGAS_FIGHTER:
			filename += "map.nagas.fighter.png";
			break;
		case UNITMAP_NAGAS_MYRMIDON:
			filename += "map.nagas.myrmidon.png";
			break;
		case UNITMAP_NAGAS_WARRIOR:
			filename += "map.nagas.warrior.png";
			break;
			
		case UNITMAP_OGRES_OGRE:
			filename += "map.ogres.ogre.png";
			break;
		case UNITMAP_OGRES_YOUNG_OGRE:
			filename += "map.ogres.young-ogre.png";
			break;
			
		case UNITMAP_ORCS_ORCS:
			filename += "map.orcs.orcs.png";
			break;
		case UNITMAP_ORCS_ARCHER:
			filename += "map.orcs.archer.png";
			break;
		case UNITMAP_ORCS_ASSASSIN:
			filename += "map.orcs.assassin.png";
			break;
		case UNITMAP_ORCS_GRUNT:
			filename += "map.orcs.grunt.png";
			break;
		case UNITMAP_ORCS_LEADER:
			filename += "map.orcs.leader.png";
			break;
		case UNITMAP_ORCS_RULER:
			filename += "map.orcs.ruler.png";
			break;
		case UNITMAP_ORCS_SLAYER:
			filename += "map.orcs.slayer.png";
			break;
		case UNITMAP_ORCS_SLURBOW:
			filename += "map.orcs.slurbow.png";
			break;
		case UNITMAP_ORCS_SOVEREIGN:
			filename += "map.orcs.sovereign.png";
			break;
		case UNITMAP_ORCS_WARLORD:
			filename += "map.orcs.warlord.png";
			break;
		case UNITMAP_ORCS_WARRIOR:
			filename += "map.orcs.warrior.png";
			break;
		case UNITMAP_ORCS_XBOWMAN:
			filename += "map.orcs.xbowman.png";
			break;
			
		case UNITMAP_SAURIANS_SAURIANS:
			filename += "map.saurians.saurians.png";
			break;
		case UNITMAP_SAURIANS_AMBUSHER:
			filename += "map.saurians.ambusher.png";
			break;
		case UNITMAP_SAURIANS_AUGUR:
			filename += "map.saurians.augur.png";
			break;
		case UNITMAP_SAURIANS_FLANKER:
			filename += "map.saurians.flanker.png";
			break;
		case UNITMAP_SAURIANS_ORACLE:
			filename += "map.saurians.oracle.png";
			break;
		case UNITMAP_SAURIANS_SKIRMISHER:
			filename += "map.saurians.skirmisher.png";
			break;
		case UNITMAP_SAURIANS_SOOTHSAYER:
			filename += "map.saurians.soothsayer.png";
			break;
		
		case UNITMAP_TRANSPORT:
			filename += "map.transport.png";
			break;
			
		case UNITMAP_TROLLS_TROLLS:
			filename += "map.trolls.trolls.png";
			break;
		case UNITMAP_TROLLS_GRUNT:
			filename += "map.trolls.grunt.png";
			break;
		case UNITMAP_TROLLS_LOBBER:
			filename += "map.trolls.lobber.png";
			break;
		case UNITMAP_TROLLS_SHAMAN:
			filename += "map.trolls.shaman.png";
			break;
		case UNITMAP_TROLLS_WARRIOR:
			filename += "map.trolls.warrior.png";
			break;
		case UNITMAP_TROLLS_WHELP:
			filename += "map.trolls.whelp.png";
			break;
		
		case UNITMAP_UNDEAD_BAT:
			filename += "map.undead.bat.png";
			break;
		case UNITMAP_UNDEAD_BLOODBAT:
			filename += "map.undead.bloodbat.png";
			break;
		case UNITMAP_UNDEAD_DREADBAT:
			filename += "map.undead.dreadbat.png";
			break;
		case UNITMAP_UNDEAD_GHOST:
			filename += "map.undead.ghost.png";
			break;
		case UNITMAP_UNDEAD_GHOUL:
			filename += "map.undead.ghoul.png";
			break;
		case UNITMAP_UNDEAD_NECROPHAGE:
			filename += "map.undead.necrophage.png";
			break;
		case UNITMAP_UNDEAD_NIGHTGAUNT:
			filename += "map.undead.nightgaunt.png";
			break;
		case UNITMAP_UNDEAD_SHADOW:
			filename += "map.undead.shadow.png";
			break;
		case UNITMAP_UNDEAD_SOULLESS_DRAKE:
			filename += "map.undead.soulless-drake.png";
			break;
		case UNITMAP_UNDEAD_SOULLESS_DWARF:
			filename += "map.undead.soulless-dwarf.png";
			break;
		case UNITMAP_UNDEAD_SOULLESS_MOUNTED:
			filename += "map.undead.soulless-mounted.png";
			break;
		case UNITMAP_UNDEAD_SOULLESS_SAURIAN:
			filename += "map.undead.soulless-saurian.png";
			break;
		case UNITMAP_UNDEAD_SOULLESS_SWIMMER:
			filename += "map.undead.soulless-swimmer.png";
			break;
		case UNITMAP_UNDEAD_SOULLESS_TROLL:
			filename += "map.undead.soulless-troll.png";
			break;			
		case UNITMAP_UNDEAD_SOULLESS_WOSE:
			filename += "map.undead.soulless-wose.png";
			break;
		case UNITMAP_UNDEAD_SOULLESS:
			filename += "map.undead.soulless.png";
			break;
		case UNITMAP_UNDEAD_SPECTRE:
			filename += "map.undead.spectre.png";
			break;
		case UNITMAP_UNDEAD_WRAITH:
			filename += "map.undead.wraith.png";
			break;
		case UNITMAP_UNDEAD_ZOMBIE_BAT:
			filename += "map.undead.zombie-bat.png";
			break;
		case UNITMAP_UNDEAD_ZOMBIE_DRAKE:
			filename += "map.undead.zombie-drake.png";
			break;
		case UNITMAP_UNDEAD_ZOMBIE_DWARF:
			filename += "map.undead.zombie-dwarf.png";
			break;
		case UNITMAP_UNDEAD_ZOMBIE_MOUNTED:
			filename += "map.undead.zombie-mounted.png";
			break;
		case UNITMAP_UNDEAD_ZOMBIE_SAURIAN:
			filename += "map.undead.zombie-saurian.png";
			break;
		case UNITMAP_UNDEAD_ZOMBIE_SWIMMER:
			filename += "map.undead.zombie-swimmer.png";
			break;
		case UNITMAP_UNDEAD_ZOMBIE_TROLL:
			filename += "map.undead.zombie-troll.png";
			break;
		case UNITMAP_UNDEAD_ZOMBIE_WOSE:
			filename += "map.undead.zombie-wose.png";
			break;
		case UNITMAP_UNDEAD_ZOMBIE:
			filename += "map.undead.zombie.png";
			break;
			
		case UNITMAP_NECROMANCERS_NECROMANCERS:
			filename += "map.necromancers.undead-necromancers.png";
			break;
		case UNITMAP_NECROMANCERS_ADEPT:
			filename += "map.necromancers.adept.png";
			break;
		case UNITMAP_NECROMANCERS_ADEPT_FEMALE:
			filename += "map.necromancers.adept+female.png";
			break;
		case UNITMAP_NECROMANCERS_ANCIENT_LICH:
			filename += "map.necromancers.ancient-lich.png";
			break;
		case UNITMAP_NECROMANCERS_DARK_SORCERER:
			filename += "map.necromancers.dark-sorcerer.png";
			break;
		case UNITMAP_NECROMANCERS_DARK_SORCERER_FEMALE:
			filename += "map.necromancers.dark-sorcerer+female.png";
			break;
		case UNITMAP_NECROMANCERS_LICH:
			filename += "map.necromancers.lich.png";
			break;
		case UNITMAP_NECROMANCERS_NECROMANCER:
			filename += "map.necromancers.necromancer.png";
			break;
		case UNITMAP_NECROMANCERS_NECROMANCER_FEMALE:
			filename += "map.necromancers.necromancer+female.png";
			break;
			
		case UNITMAP_SKELETAL_SKELETAL:
			filename += "map.skeletal.undead-skeletal.png";
			break;
		case UNITMAP_SKELETAL_ARCHER:
			filename += "map.skeletal.archer.png";
			break;
		case UNITMAP_SKELETAL_BANEBOW:
			filename += "map.skeletal.banebow.png";
			break;
		case UNITMAP_SKELETAL_BONE_SHOOTER:
			filename += "map.skeletal.bone-shooter.png";
			break;
		case UNITMAP_SKELETAL_CHOCOBONE:
			filename += "map.skeletal.chocobone.png";
			break;
		case UNITMAP_SKELETAL_DEATHBLADE:
			filename += "map.skeletal.deathblade.png";
			break;
		case UNITMAP_SKELETAL_DEATHKNIGHT:
			filename += "map.skeletal.deathknight.png";
			break;
		case UNITMAP_SKELETAL_DRAUG:
			filename += "map.skeletal.draug.png";
			break;
		case UNITMAP_SKELETAL_REVENANT:
			filename += "map.skeletal.revenant.png";
			break;
		case UNITMAP_SKELETAL_SKELETON:
			filename += "map.skeletal.skeleton.png";
			break;
			
		case UNITMAP_WOSES_WOSES:
			filename += "map.woses.woses.png";
			break;
		case UNITMAP_WOSES_ANCIENT:
			filename += "map.woses.wose-ancient.png";
			break;
		case UNITMAP_WOSES_ELDER:
			filename += "map.woses.wose-elder.png";
			break;
		case UNITMAP_WOSES_WOSE:
			filename += "map.woses.wose.png";
			break;
			
		case UNITMAP_DID_APPRENTICE_MAGE:
			filename += "map.did.apprentice-mage.png";
			break;
		case UNITMAP_DID_APPRENTICE_NECROMANCER:
			filename += "map.did.apprentice-necromancer.png";
			break;
		case UNITMAP_DID_DARK_MAGE:
			filename += "map.did.dark-mage.png";
			break;
		case UNITMAP_DID_GHAST_RAT:
			filename += "map.did.ghast_rat.png";
			break;
		case UNITMAP_DID_NEUTRAL_OUTLAW_PRINCESS:
			filename += "map.did.neutral-outlaw-princess.png";
			break;
			
		case UNITMAP_EI_OWAEC:
			filename += "map.ei.owaec.png";
			break;
		
		case UNITMAP_HTTT_GRYPHON_SEA_ORC:
			filename += "map.httt.gryphon_sea-orc.png";
			break;
		case UNITMAP_HTTT_HUMAN_BATTLEPRINCESS:
			filename += "map.httt.human-battleprincess.png";
			break;
		case UNITMAP_HTTT_HUMAN_PRINCESS:
			filename += "map.httt.human-princess.png";
			break;
		case UNITMAP_HTTT_HUMAN_QUEEN:
			filename += "map.httt.human-queen.png";
			break;
		case UNITMAP_HTTT_KONRAD_COMMANDER_SCEPTER:
			filename += "map.httt.konrad-commander-scepter.png";
			break;
		case UNITMAP_HTTT_KONRAD_COMMANDER:
			filename += "map.httt.konrad-commander.png";
			break;
		case UNITMAP_HTTT_KONRAD_FIGHTER:
			filename += "map.httt.konrad-fighter.png";
			break;
		case UNITMAP_HTTT_KONRAD_LORD_SCEPTER:
			filename += "map.httt.konrad-lord-scepter.png";
			break;
		case UNITMAP_HTTT_KONRAD_LORD:
			filename += "map.httt.konrad-lord.png";
			break;
			
		case UNITMAP_LIBERTY_SKELETAL:
			filename += "map.liberty.undead-skeletal.png";
			break;
		case UNITMAP_LIBERTY_ROGUE_MAGE:
			filename += "map.liberty.rogue-mage.png";
			break;
		case UNITMAP_LIBERTY_SHADOW_LORD:
			filename += "map.liberty.shadow-lord.png";
			break;
		case UNITMAP_LIBERTY_SHADOW_MAGE:
			filename += "map.liberty.shadow-mage.png";
			break;
			
		case UNITMAP_SOF_DWARVES:
			filename += "map.sof.dwarves.png";
			break;
		case UNITMAP_SOF_HALDRIC_II:
			filename += "map.sof.haldric-ii.png";
			break;
			
		case UNITMAP_SOTBE_UNITS:
			filename += "map.sotbe.units.png";
			break;
			
		case UNITMAP_THOT_ANNALIST:
			filename += "map.thot.annalist.png";
			break;
		case UNITMAP_THOT_KARRAG:
			filename += "map.thot.karrag.png";
			break;
		case UNITMAP_THOT_LOREMASTER:
			filename += "map.thot.loremaster.png";
			break;
		case UNITMAP_THOT_MASKED_DRAGONGUARD:
			filename += "map.thot.masked_dragonguard.png";
			break;
		case UNITMAP_THOT_MASKED_FIGHTER:
			filename += "map.thot.masked_fighter.png";
			break;
		case UNITMAP_THOT_MASKED_GUARD:
			filename += "map.thot.masked_guard.png";
			break;
		case UNITMAP_THOT_MASKED_LORD:
			filename += "map.thot.masked_lord.png";
			break;
		case UNITMAP_THOT_MASKED_SENTINEL:
			filename += "map.thot.masked_sentinel.png";
			break;
		case UNITMAP_THOT_MASKED_STALWART:
			filename += "map.thot.masked_stalwart.png";
			break;
		case UNITMAP_THOT_MASKED_STEELCLAD:
			filename += "map.thot.masked_steelclad.png";
			break;
		case UNITMAP_THOT_MASKED_THUNDERER:
			filename += "map.thot.masked_thunderer.png";
			break;
		case UNITMAP_THOT_MASKED_THUNDERGUARD:
			filename += "map.thot.masked_thunderguard.png";
			break;
		case UNITMAP_THOT_WITNESS:
			filename += "map.thot.witness.png";
			break;
			
		case UNITMAP_TROW_NOBLE_COMMANDER:
			filename += "map.trow.noble-commander.png";
			break;
		case UNITMAP_TROW_NOBLE_FIGHTER:
			filename += "map.trow.noble-fighter.png";
			break;
		case UNITMAP_TROW_NOBLE_LORD:
			filename += "map.trow.noble-lord.png";
			break;
		case UNITMAP_TROW_NOBLE_YOUTH_UNDEAD:
			filename += "map.trow.noble-youth_undead.png";
			break;
		case UNITMAP_TROW_WESFOLK_LADY_MASKED:
			filename += "map.trow.wesfolk-lady-masked.png";
			break;
		case UNITMAP_TROW_WESFOLK_LADY:
			filename += "map.trow.wesfolk-lady.png";
			break;
		case UNITMAP_TROW_WESFOLK_LEADER_MASKED:
			filename += "map.trow.wesfolk-leader-masked.png";
			break;
		case UNITMAP_TROW_WESFOLK_LEADER:
			filename += "map.trow.wesfolk-leader.png";
			break;
		case UNITMAP_TROW_WESFOLK_OUTCAST_MASKED:
			filename += "map.trow.wesfolk-outcast-masked.png";
			break;
		case UNITMAP_TROW_WESFOLK_OUTCAST:
			filename += "map.trow.wesfolk-outcast.png";
			break;
		case UNITMAP_TROW_WOSE_SAPLING:
			filename += "map.trow.wose-sapling.png";
			break;
			
		case UNITMAP_TSG_COMMANDER:
			filename += "map.tsg.commander.png";
			break;
		case UNITMAP_TSG_EYESTALK:
			filename += "map.tsg.eyestalk.png";
			break;
		case UNITMAP_TSG_DISMOUNTED_COMMANDER:
			filename += "map.tsg.dismounted-commander.png";
			break;
		case UNITMAP_TSG_HORSEMAN_COMMANDER:
			filename += "map.tsg.horseman-commander.png";
			break;
		case UNITMAP_TSG_JUNIOR_COMMANDER:
			filename += "map.tsg.junior-commander.png";
			break;
		case UNITMAP_TSG_MOUNTED_GENERAL:
			filename += "map.tsg.mounted-general.png";
			break;
			
			
		case UNITMAP_TUTORIAL_HUMAN_PRINCESS:
			filename += "map.tutorial.human-princess.png";
			break;
		case UNITMAP_TUTORIAL_KONRAD_FIGHTER:
			filename += "map.tutorial.konrad-fighter.png";
			break;
		case UNITMAP_TUTORIAL_QUINTAIN:
			filename += "map.tutorial.quintain.png";
			break;
			
		case UNITMAP_UTBS_ALIEN:
			filename += "map.utbs.alien.png";
			break;
		case UNITMAP_UTBS_DWARVES_EXPLORER:
			filename += "map.utbs.dwarves.explorer.png";
			break;
		case UNITMAP_UTBS_DWARVES_PATHFINDER:
			filename += "map.utbs.dwarves.pathfinder.png";
			break;
		case UNITMAP_UTBS_DWARVES_SCOUT:
			filename += "map.utbs.dwarves.scout.png";
			break;
		case UNITMAP_UTBS_ELVES_ELVES:
			filename += "map.utbs.elves.elves-desert.png";
			break;
		case UNITMAP_UTBS_ELVES_ARCHER:
			filename += "map.utbs.elves.archer.png";
			break;
		case UNITMAP_UTBS_ELVES_ARCHER_FEMALE:
			filename += "map.utbs.elves.archer+female.png";
			break;
		case UNITMAP_UTBS_ELVES_AVENGER:
			filename += "map.utbs.elves.avenger.png";
			break;
		case UNITMAP_UTBS_ELVES_AVENGER_FEMALE:
			filename += "map.utbs.elves.avenger+female.png";
			break;
		case UNITMAP_UTBS_ELVES_CAPTAIN:
			filename += "map.utbs.elves.captain.png";
			break;
		case UNITMAP_UTBS_ELVES_CHAMPION:
			filename += "map.utbs.elves.champion.png";
			break;
		case UNITMAP_UTBS_ELVES_CORRUPTED_ELF:
			filename += "map.utbs.elves.corrupted-elf.png";
			break;
		case UNITMAP_UTBS_ELVES_DRUID_ELOH:
			filename += "map.utbs.elves.druid_eloh.png";
			break;
		case UNITMAP_UTBS_ELVES_FIGHTER:
			filename += "map.utbs.elves.fighter.png";
			break;
		case UNITMAP_UTBS_ELVES_HERO:
			filename += "map.utbs.elves.hero.png";
			break;
		case UNITMAP_UTBS_ELVES_HORSEMAN:
			filename += "map.utbs.elves.horseman.png";
			break;
		case UNITMAP_UTBS_ELVES_HUNTER:
			filename += "map.utbs.elves.hunter.png";
			break;
		case UNITMAP_UTBS_ELVES_HUNTER_FEMALE:
			filename += "map.utbs.elves.hunter+female.png";
			break;
		case UNITMAP_UTBS_ELVES_KALEH:
			filename += "map.utbs.elves.kaleh.png";
			break;
		case UNITMAP_UTBS_ELVES_MARKSMAN:
			filename += "map.utbs.elves.marksman.png";
			break;
		case UNITMAP_UTBS_ELVES_MARKSMAN_FEMALE:
			filename += "map.utbs.elves.marksman+female.png";
			break;
		case UNITMAP_UTBS_ELVES_MARSHAL:
			filename += "map.utbs.elves.marshal.png";
			break;
		case UNITMAP_UTBS_ELVES_NYM:
			filename += "map.utbs.elves.nym.png";
			break;
		case UNITMAP_UTBS_ELVES_OUTRIDER:
			filename += "map.utbs.elves.outrider.png";
			break;
		case UNITMAP_UTBS_ELVES_PROWLER:
			filename += "map.utbs.elves.prowler.png";
			break;
		case UNITMAP_UTBS_ELVES_RANGER:
			filename += "map.utbs.elves.ranger.png";
			break;
		case UNITMAP_UTBS_ELVES_RANGER_FEMALE:
			filename += "map.utbs.elves.ranger+female.png";
			break;
		case UNITMAP_UTBS_ELVES_RIDER:
			filename += "map.utbs.elves.rider.png";
			break;
		case UNITMAP_UTBS_ELVES_SCOUT:
			filename += "map.utbs.elves.scout.png";
			break;
		case UNITMAP_UTBS_ELVES_SENTINEL:
			filename += "map.utbs.elves.sentinel.png";
			break;
		case UNITMAP_UTBS_ELVES_SHAMAN:
			filename += "map.utbs.elves.shaman.png";
			break;
		case UNITMAP_UTBS_ELVES_SHARPSHOOTER:
			filename += "map.utbs.elves.sharpshooter.png";
			break;
		case UNITMAP_UTBS_ELVES_SHARPSHOOTER_FEMALE:
			filename += "map.utbs.elves.sharpshooter+female.png";
			break;
		case UNITMAP_UTBS_ELVES_SHYDE:
			filename += "map.utbs.elves.shyde.png";
			break;
		case UNITMAP_UTBS_HUMAN_COMMANDER:
			filename += "map.utbs.human-commander.png";
			break;
		case UNITMAP_UTBS_MONSTERS_DAWARF_DEMON:
			filename += "map.utbs.monsters.dawarf_demon.png";
			break;
		case UNITMAP_UTBS_MONSTERS_FIREGHOST:
			filename += "map.utbs.monsters.fireghost.png";
			break;
		case UNITMAP_UTBS_MONSTERS_MONSTERS:
			filename += "map.utbs.monsters.monsters.png";
			break;
		case UNITMAP_UTBS_NAGAS_NAGAS:
			filename += "map.utbs.nagas.nagas.png";
			break;
		case UNITMAP_UTBS_NAGAS_HUNTER:
			filename += "map.utbs.nagas.hunter.png";
			break;
		case UNITMAP_UTBS_ORCS_NIGHTSTALKER:
			filename += "map.utbs.orcs.nightstalker.png";
			break;
		case UNITMAP_UTBS_UNDEAD:
			filename += "map.utbs.undead.png";
			break;
			
			
		
		case UNITMAP_IFTU_ALIEN_PSY:
			filename += "map.iftu.alien-psy.png";
			break;
		case UNITMAP_IFTU_ALIEN_VERLISSH:
			filename += "map.iftu.alien-verlissh.png";
			break;
		case UNITMAP_IFTU_DEMONS:
			filename += "map.iftu.demons.png";
			break;
		case UNITMAP_IFTU_DWARVES:
			filename += "map.iftu.dwarves.png";
			break;
		case UNITMAP_IFTU_ELVES:
			filename += "map.iftu.elves.png";
			break;
		case UNITMAP_IFTU_FAIRIES_SYLVAN:
			filename += "map.iftu.fairies-sylvan.png";
			break;
		case UNITMAP_IFTU_FAKE:
			filename += "map.iftu.fake.png";
			break;
		case UNITMAP_IFTU_GOBLINS:
			filename += "map.iftu.goblins.png";
			break;
		case UNITMAP_IFTU_GRYPHONS:
			filename += "map.iftu.gryphons.png";
			break;
		case UNITMAP_IFTU_HUMAN_ARAGWAITHI_ARCHER:
			filename += "map.iftu.human-aragwaithi.archer.png";
			break;
		case UNITMAP_IFTU_HUMAN_ARAGWAITHI_CAPTAIN:
			filename += "map.iftu.human-aragwaithi.captain.png";
			break;
		case UNITMAP_IFTU_HUMAN_ARAGWAITHI_EAGLE_RIDER:
			filename += "map.iftu.human-aragwaithi.eagle-rider.png";
			break;
		case UNITMAP_IFTU_HUMAN_ARAGWAITHI_FLAGBEARER:
			filename += "map.iftu.human-aragwaithi.flagbearer.png";
			break;
		case UNITMAP_IFTU_HUMAN_ARAGWAITHI_GREATBOW:
			filename += "map.iftu.human-aragwaithi.greatbow.png";
			break;
		case UNITMAP_IFTU_HUMAN_ARAGWAITHI_GUARD:
			filename += "map.iftu.human-aragwaithi.guard.png";
			break;
		case UNITMAP_IFTU_HUMAN_ARAGWAITHI_GUARDIAN:
			filename += "map.iftu.human-aragwaithi.guardian.png";
			break;
		case UNITMAP_IFTU_HUMAN_ARAGWAITHI_LANCER:
			filename += "map.iftu.human-aragwaithi.lancer.png";
			break;
		case UNITMAP_IFTU_HUMAN_ARAGWAITHI_LONGSWORDSMAN:
			filename += "map.iftu.human-aragwaithi.longswordsman.png";
			break;
		case UNITMAP_IFTU_HUMAN_ARAGWAITHI_PIKEMAN:
			filename += "map.iftu.human-aragwaithi.pikeman.png";
			break;
		case UNITMAP_IFTU_HUMAN_ARAGWAITHI:
			filename += "map.iftu.human-aragwaithi.png";
			break;
		case UNITMAP_IFTU_HUMAN_ARAGWAITHI_SCOUT:
			filename += "map.iftu.human-aragwaithi.scout.png";
			break;
		case UNITMAP_IFTU_HUMAN_ARAGWAITHI_SHIELD_GUARD:
			filename += "map.iftu.human-aragwaithi.shield-guard.png";
			break;
		case UNITMAP_IFTU_HUMAN_ARAGWAITHI_SILVER_SHIELD:
			filename += "map.iftu.human-aragwaithi.silver-shield.png";
			break;
		case UNITMAP_IFTU_HUMAN_ARAGWAITHI_SLAYER:
			filename += "map.iftu.human-aragwaithi.slayer.png";
			break;
		case UNITMAP_IFTU_HUMAN_ARAGWAITHI_SPEARMAN:
			filename += "map.iftu.human-aragwaithi.spearman.png";
			break;
		case UNITMAP_IFTU_HUMAN_ARAGWAITHI_STRONGBOW:
			filename += "map.iftu.human-aragwaithi.strongbow.png";
			break;
		case UNITMAP_IFTU_HUMAN_ARAGWAITHI_SWORDMAN:
			filename += "map.iftu.human-aragwaithi.swordman.png";
			break;
		case UNITMAP_IFTU_HUMAN_ARAGWAITHI_SWORDMASTER:
			filename += "map.iftu.human-aragwaithi.swordmaster.png";
			break;
		case UNITMAP_IFTU_HUMAN_ARAGWAITHI_WARLOCK:
			filename += "map.iftu.human-aragwaithi.warlock.png";
			break;
		case UNITMAP_IFTU_HUMAN_CHAOS_DARK_KNIGHT:
			filename += "map.iftu.human-chaos.dark-knight.png";
			break;
		case UNITMAP_IFTU_HUMAN_CHAOS_INVADER:
			filename += "map.iftu.human-chaos.invader.png";
			break;
		case UNITMAP_IFTU_HUMAN_CHAOS:
			filename += "map.iftu.human-chaos.png";
			break;
		case UNITMAP_IFTU_HUMAN_CHAOS_RAZERMAN:
			filename += "map.iftu.human-chaos.razerman.png";
			break;
		case UNITMAP_IFTU_HUMAN_PEASANTS:
			filename += "map.iftu.human-peasants.png";
			break;
		case UNITMAP_IFTU_IMPS:
			filename += "map.iftu.imps.png";
			break;
		case UNITMAP_IFTU_MECHANICAL:
			filename += "map.iftu.mechanical.png";
			break;
		case UNITMAP_IFTU_MONSTERS:
			filename += "map.iftu.monsters.png";
			break;
		case UNITMAP_IFTU_MONSTERS2:
			filename += "map.iftu.monsters2.png";
			break;
		case UNITMAP_IFTU_MONSTERS3:
			filename += "map.iftu.monsters3.png";
			break;
		case UNITMAP_IFTU_MONSTERS4:
			filename += "map.iftu.monsters4.png";
			break;
		case UNITMAP_IFTU_MONSTERS5:
			filename += "map.iftu.monsters5.png";
			break;
		case UNITMAP_IFTU_ORCS:
			filename += "map.iftu.orcs.png";
			break;
		case UNITMAP_IFTU_SHAXTHAL:
			filename += "map.iftu.shaxthal.png";
			break;
		case UNITMAP_IFTU_SPIRITS_ANIMATED_ROCK:
			filename += "map.iftu.spirits.animated-rock.png";
			break;
		case UNITMAP_IFTU_SPIRITS_FIRE:
			filename += "map.iftu.spirits.fire.png";
			break;
		case UNITMAP_IFTU_SPIRITS_FIREWISP:
			filename += "map.iftu.spirits.firewisp.png";
			break;
		case UNITMAP_IFTU_SPIRITS_ROCK_GOLEM:
			filename += "map.iftu.spirits.rock-golem.png";
			break;
		case UNITMAP_IFTU_SPIRITS_WATER:
			filename += "map.iftu.spirits.water.png";
			break;
		case UNITMAP_IFTU_TROLLS:
			filename += "map.iftu.trolls.png";
			break;
		case UNITMAP_IFTU_UNDEAD_SKELETAL:
			filename += "map.iftu.undead-skeletal.png";
			break;
		case UNITMAP_IFTU_UNDEAD:
			filename += "map.iftu.undead.png";
			break;
			
		case UNITMAP_DEADWATER_BRAWLER:
			filename += "map.dw.brawler.png";
			break;
		case UNITMAP_DEADWATER_CHILD_KING:
			filename += "map.dw.child_king.png";
			break;
		case UNITMAP_DEADWATER_CITIZEN:
			filename += "map.dw.citizen.png";
			break;
		case UNITMAP_DEADWATER_DARK_SHAPE:
			filename += "map.dw.dark_shape.png";
			break;
		case UNITMAP_DEADWATER_FIREGHOST:
			filename += "map.dw.fireghost.png";
			break;
		case UNITMAP_DEADWATER_KRAKEN:
			filename += "map.dw.kraken.png";
			break;
		case UNITMAP_DEADWATER_SOLDIER_KING:
			filename += "map.dw.soldier_king.png";
			break;
		case UNITMAP_DEADWATER_WARRIOR_KING:
			filename += "map.dw.warrior_king.png";
			break;
		case UNITMAP_DEADWATER_YOUNG_KING:
			filename += "map.dw.young_king.png";
			break;
			
			
			
		case UNITMAP_BLANK:
			filename += "map.blank.png";
			break;
			
			
			
		case HALOMAP_ELVEN_DRUID_HEALING:
			filename = game_config::path + "/data/core/images/halo/map.elven.druid-healing.pvrtc";
			pvrtcSize = 256;
			break;
		case HALOMAP_ELVEN_ELVEN_SHIELD:
			filename = game_config::path + "/data/core/images/halo/map.elven.elven-shield.pvrtc";
			pvrtcSize = 512;
			break;
		case HALOMAP_ELVEN_FAERIE_FIRE:
			filename = game_config::path + "/data/core/images/halo/map.elven.faerie-fire.pvrtc";
			pvrtcSize = 256;
			break;
		case HALOMAP_ELVEN_ICE_HALO:
			filename = game_config::path + "/data/core/images/halo/map.elven.ice-halo.pvrtc";
			pvrtcSize = 256;
			break;
		case HALOMAP_ELVEN_NATURE_HALO:
			filename = game_config::path + "/data/core/images/halo/map.elven.nature-halo.pvrtc";
			pvrtcSize = 256;
			break;
		case HALOMAP_ELVEN_SHAMAN_HEAL:
			filename = game_config::path + "/data/core/images/halo/map.elven.shaman-heal.pvrtc";
			pvrtcSize = 256;
			break;
		case HALOMAP_ELVEN_SHYDE:
			filename = game_config::path + "/data/core/images/halo/map.elven.shyde.pvrtc";
			pvrtcSize = 256;
			break;
		case HALOMAP_FIRE_AURA:
			filename = game_config::path + "/data/core/images/halo/map.fire-aura.pvrtc";
			pvrtcSize = 512;
			break;
		case HALOMAP_FLAME_BURST:
			filename = game_config::path + "/data/core/images/halo/map.flame-burst.pvrtc";
			pvrtcSize = 256;
			break;
		case HALOMAP_HOLY_HALO:
			filename = game_config::path + "/data/core/images/halo/map.holy.halo.pvrtc";
			pvrtcSize = 256;
			break;
		case HALOMAP_HOLY_LIGHTBEAM:
			filename = game_config::path + "/data/core/images/halo/map.holy.lightbeam.pvrtc";
			pvrtcSize = 512;
			break;
		case HALOMAP_IFTU_AVATAR:
			filename = game_config::path + "/data/core/images/halo/map.iftu.avatar.pvrtc";
			pvrtcSize = 128;
			break;
		case HALOMAP_IFTU_BOMB_EXPLODE:
			filename = game_config::path + "/data/core/images/halo/map.iftu.bomb-explode.pvrtc";
			pvrtcSize = 256;
			break;
		case HALOMAP_IFTU_CHAOS:
			filename = game_config::path + "/data/core/images/halo/map.iftu.chaos.pvrtc";
			pvrtcSize = 256;
			break;
		case HALOMAP_IFTU_DARKNESS_BEAM:
			filename = game_config::path + "/data/core/images/halo/map.iftu.darkness-beam.pvrtc";
			pvrtcSize = 512;
			break;
		case HALOMAP_IFTU_ELYNIA_NOILLUM:
			filename = game_config::path + "/data/core/images/halo/map.iftu.elynia-noillum.pvrtc";
			pvrtcSize = 256;
			break;
		case HALOMAP_IFTU_ELYNIA:
			filename = game_config::path + "/data/core/images/halo/map.iftu.elynia.pvrtc";
			pvrtcSize = 256;
			break;
		case HALOMAP_IFTU_ILLUMINATES:
			filename = game_config::path + "/data/core/images/halo/map.iftu.illuminates.pvrtc";
			pvrtcSize = 256;
			break;
		case HALOMAP_IFTU_OBSCURES:
			filename = game_config::path + "/data/core/images/halo/map.iftu.obscures.pvrtc";
			pvrtcSize = 256;
			break;
		case HALOMAP_IFTU_WOSE_RANGED:
			filename = game_config::path + "/data/core/images/halo/map.iftu.wose-ranged.pvrtc";
			pvrtcSize = 128;
			break;
		case HALOMAP_IFTU_WOSE:
			filename = game_config::path + "/data/core/images/halo/map.iftu.wose.pvrtc";
			pvrtcSize = 128;
			break;
		case HALOMAP_ILLUMINATES_AURA:
			filename = game_config::path + "/data/core/images/halo/map.illuminates-aura.pvrtc";
			pvrtcSize = 256;
			break;
		case HALOMAP_LIBERTY_SHADOW_MAGE:
			filename = game_config::path + "/data/core/images/halo/map.liberty.shadow-mage.pvrtc";
			pvrtcSize = 256;
			break;
		case HALOMAP_LIGHTHOUSE_AURA:
			filename = game_config::path + "/data/core/images/halo/map.lighthouse-aura.pvrtc";
			pvrtcSize = 512;
			break;
		case HALOMAP_LIGHTNING_BOLT:
			filename = game_config::path + "/data/core/images/halo/map.lightning-bolt.pvrtc";
			pvrtcSize = 512;
			break;
		case HALOMAP_MAGE_HALO_BIG:
			filename = game_config::path + "/data/core/images/halo/map.mage-halo-big.pvrtc";
			pvrtcSize = 256;
			break;
		case HALOMAP_MAGE_HALO:
			filename = game_config::path + "/data/core/images/halo/map.mage-halo.pvrtc";
			pvrtcSize = 256;
			break;
		case HALOMAP_MAGE_PREPARATION:
			filename = game_config::path + "/data/core/images/halo/map.mage-preparation.pvrtc";
			pvrtcSize = 128;
			break;
		case HALOMAP_MERFOLK_STAFF_FLARE:
			filename = game_config::path + "/data/core/images/halo/map.merfolk.staff-flare.pvrtc";
			pvrtcSize = 128;
			break;
		case HALOMAP_MERFOLK_WATER:
			filename = game_config::path + "/data/core/images/halo/map.merfolk.water.pvrtc";
			pvrtcSize = 256;
			break;
		case HALOMAP_SAURIAN_MAGIC_HALO:
			filename = game_config::path + "/data/core/images/halo/map.saurian-magic-halo.pvrtc";
			pvrtcSize = 256;
			break;
		case HALOMAP_TELEPORT:
			filename = game_config::path + "/data/core/images/halo/map.teleport.pvrtc";
			pvrtcSize = 256;
			break;
		case HALOMAP_THOT_KARRAG:
			filename = game_config::path + "/data/core/images/halo/map.thot.karrag.pvrtc";
			pvrtcSize = 256;
			break;
		case HALOMAP_UNDEAD:
			filename = game_config::path + "/data/core/images/halo/map.undead.pvrtc";
			pvrtcSize = 256;
			break;
			
			
			
			
			
		default:
			assert(false);
			return;
	}
	
	assert (mapId < NUM_UNITMAPS);
	
	glGenTextures(1, &gUnitTexIds[mapId+color]);	
	glBindTexture(GL_TEXTURE_2D, gUnitTexIds[mapId+color]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	
	if (pvrtcSize == 0)
	{
		surface surf = surface(IMG_Load(filename.c_str()));
		
		if (surf == NULL)
		{
			std::cerr << "\n\n*** ERROR loading texture altas " << filename.c_str() << "\n\n";
			assert(false);
			return;
		}
		
		if (color != 0)
		{
			std::map<Uint32, Uint32> tmp_rgb, map_rgb;
			color_range const& new_color = game_config::color_info(std::string(1, '0'+color));
			
			if (mapId == UNITMAP_ELLIPSE)
			{
				std::vector<Uint32> const& old_color = game_config::tc_info("ellipse_red");
				tmp_rgb = recolor_range(new_color,old_color);
			}
			else if (mapId >= MAP_BLACK_FLAG && mapId <= MAP_UNDEAD_FLAG + 9)
			{
				std::vector<Uint32> const& old_color = game_config::tc_info("flag_green");
				tmp_rgb = recolor_range(new_color,old_color);
			}			
			else
			{
				std::vector<Uint32> const& old_color = game_config::tc_info("magenta");;
				tmp_rgb = recolor_range(new_color,old_color);
			}
				
			

			Uint32* beg = (Uint32*)surf->pixels;
			Uint32* end = beg + surf->w*surf->h;
			
			// surface is in ABGR format, so play with the conversion colors...
			for (std::map<Uint32, Uint32>::iterator it = tmp_rgb.begin(); it != tmp_rgb.end(); it++)
			{
				map_rgb[argb_to_abgr(it->first)] = argb_to_abgr(it->second);
			}
			
			
			std::map<Uint32, Uint32>::const_iterator map_rgb_end = map_rgb.end();	
			
			while(beg != end) 
			{
				Uint8 alpha = (*beg) >> 24;
				
				if(alpha)
				{	// don't recolor invisible pixels.
					// palette use only RGB channels, so remove alpha
					Uint32 oldrgb = (*beg) & 0x00FFFFFF;
					std::map<Uint32, Uint32>::const_iterator i = map_rgb.find(oldrgb);
					if(i != map_rgb.end())
					{
						*beg = (alpha << 24) + i->second;
					}
				}
				++beg;
			}
		}
		
		if (surf == NULL)
		{
			std::cerr << "\n\n*** ERROR (stage2) loading texture altas " << filename.c_str() << "\n\n";
			assert(false);
			return;
		}
		
		gUnitTexW[mapId+color] = surf->w;
		gUnitTexH[mapId+color] = surf->h;
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surf->w, surf->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surf->pixels);
		
		
		// add to LRU list
		unitAtlasData uad;
		uad.mapId = mapId+color;
		uad.size = surf->w*surf->h*4;
		gUnitAtlasTotalSize += uad.size;
		std::cerr << "UnitTextureAtlas: creating atlas ID " << mapId+color << " " << surf->w << "x" << surf->h << ", " << uad.size << " bytes, total " << gUnitAtlasTotalSize << " bytes\n";
		gUnitAtlasLRU.push_back(uad);
		
		while(gUnitAtlasTotalSize >= MAX_UNIT_ATLAS_TOTAL_SIZE && gUnitAtlasLRU.size() > 0)
		{
			uad = gUnitAtlasLRU.front();
			gUnitAtlasLRU.pop_front();
			glDeleteTextures(1, &gUnitTexIds[uad.mapId]);
			gUnitTexIds[uad.mapId] = 0;
			gUnitAtlasTotalSize -= uad.size;
			std::cerr << "UnitTextureAtlas: FREED atlas ID " << uad.mapId << " size " << uad.size << " bytes, total now " << gUnitAtlasTotalSize << " bytes\n";
		}			
		
		// note: the sdl surface is destroyed at the end of this context
	}
	else
	{
		GLsizei dataSize = (pvrtcSize * pvrtcSize * 4) / 8;
		FILE *fp = fopen(filename.c_str(), "rb");
		if (!fp)
		{
			std::cerr << "\n\n*** ERROR loading texture altas " << filename.c_str() << "\n\n";
			return;
		}
		unsigned char *data = (unsigned char*) malloc(dataSize);
		int bytesRead = fread(data, 1, dataSize, fp);
		assert(bytesRead == dataSize);
		fclose(fp);
		glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG, pvrtcSize, pvrtcSize, 0, dataSize, data);
		GLenum err = glGetError();
        if (err != GL_NO_ERROR)
        {
			char buffer[512];
			sprintf(buffer, "\n\n*** ERROR uploading compressed texture %s:  glError: 0x%04X\n\n", filename.c_str(), err);
			std::cerr << buffer;
        }
		
		free(data);
		
		gUnitTexW[mapId+color] = pvrtcSize;
		gUnitTexH[mapId+color] = pvrtcSize;
		
		
		// add to LRU list
		unitAtlasData uad;
		uad.mapId = mapId+color;
		uad.size = dataSize;
		gUnitAtlasTotalSize += uad.size;
		std::cerr << "UnitTextureAtlas: creating PVRTC atlas ID " << mapId+color << " " << pvrtcSize << "x" << pvrtcSize << ", " << uad.size << " bytes, total " << gUnitAtlasTotalSize << " bytes\n";
		gUnitAtlasLRU.push_back(uad);
		
		while(gUnitAtlasTotalSize >= MAX_UNIT_ATLAS_TOTAL_SIZE && gUnitAtlasLRU.size() > 0)
		{
			uad = gUnitAtlasLRU.front();
			gUnitAtlasLRU.pop_front();
			glDeleteTextures(1, &gUnitTexIds[uad.mapId]);
			gUnitTexIds[uad.mapId] = 0;
			gUnitAtlasTotalSize -= uad.size;
			std::cerr << "UnitTextureAtlas: FREED atlas ID " << uad.mapId << " size " << uad.size << " bytes, total now " << gUnitAtlasTotalSize << " bytes\n";
		}			
		
	}
}

void initUnitTextureAtlas(void)
{
	memset(gUnitTexIds, 0, sizeof(gUnitTexIds));
	
	#include "map.drakes.drakes.h"
	#include "map.drakes.armageddon.h"
	#include "map.drakes.blademaster.h"
	#include "map.drakes.burner.h"
	#include "map.drakes.clasher.h"
	#include "map.drakes.enforcer.h"
	#include "map.drakes.fighter.h"
	#include "map.drakes.fire.h"
	#include "map.drakes.flameheart.h"
	#include "map.drakes.flare.h"
	#include "map.drakes.gladiator.h"
	#include "map.drakes.glider.h"
	#include "map.drakes.hurricane.h"
	#include "map.drakes.inferno.h"
	#include "map.drakes.sky.h"
	#include "map.drakes.slasher.h"
	#include "map.drakes.warden.h"
	#include "map.drakes.warrior.h"

	#include "map.dwarves.dwarves.h"
	#include "map.dwarves.berserker.h"
	#include "map.dwarves.dragonguard.h"
	#include "map.dwarves.fighter.h"
	#include "map.dwarves.gryphon-master.h"
	#include "map.dwarves.gryphon-rider.h"
	#include "map.dwarves.guard.h"
	#include "map.dwarves.lord.h"
	#include "map.dwarves.runemaster.h"
	#include "map.dwarves.sentinel.h"
	#include "map.dwarves.stalwart.h"
	#include "map.dwarves.steelclad.h"
	#include "map.dwarves.thunderer.h"
	#include "map.dwarves.thunderguard.h"
	#include "map.dwarves.ulfserker.h"
	
	#include "map.elves.elves1.h"
	#include "map.elves.elves2.h"
	#include "map.elves.archer.h"
	#include "map.elves.archer+female.h"
	#include "map.elves.avenger.h"
	#include "map.elves.avenger+female.h"
	#include "map.elves.captain.h"
	#include "map.elves.champion.h"
	#include "map.elves.druid.h"
	#include "map.elves.enchantress.h"
	#include "map.elves.fighter.h"
	#include "map.elves.hero.h"
	#include "map.elves.high-lord.h"
	#include "map.elves.lord.h"
	#include "map.elves.marksman.h"
	#include "map.elves.marksman+female.h"
	#include "map.elves.marshal.h"
	#include "map.elves.outrider.h"
	#include "map.elves.ranger.h"
	#include "map.elves.ranger+female.h"
	#include "map.elves.rider.h"
	#include "map.elves.scout.h"
	#include "map.elves.shaman.h"
	#include "map.elves.sharpshooter.h"
	#include "map.elves.sharpshooter+female.h"
	#include "map.elves.shyde.h"
	#include "map.elves.sorceress.h"
	#include "map.elves.sylph.h"
	
	#include "map.goblins.goblins.h"
	#include "map.goblins.direwolver.h"
	#include "map.goblins.impaler.h"
	#include "map.goblins.knight.h"
	#include "map.goblins.pillager.h"
	#include "map.goblins.rouser.h"
	#include "map.goblins.spearman.h"
	#include "map.goblins.wolf-rider.h"
	
	#include "map.humanloyalists.human-loyalists.h"
	#include "map.humanloyalists.bowman.h"
	#include "map.humanloyalists.cavalier.h"
	#include "map.humanloyalists.cavalryman.h"
	#include "map.humanloyalists.dragoon.h"
	#include "map.humanloyalists.duelist.h"
	#include "map.humanloyalists.fencer.h"
	#include "map.humanloyalists.general.h"
	#include "map.humanloyalists.grand-knight.h"
	#include "map.humanloyalists.halberdier.h"
	#include "map.humanloyalists.heavyinfantry.h"
	#include "map.humanloyalists.horseman.h"
	#include "map.humanloyalists.javelineer.h"
	#include "map.humanloyalists.knight.h"
	#include "map.humanloyalists.lancer.h"
	#include "map.humanloyalists.lieutenant.h"
	#include "map.humanloyalists.longbowman.h"
	#include "map.humanloyalists.marshal.h"
	#include "map.humanloyalists.master-at-arms.h"
	#include "map.humanloyalists.masterbowman.h"
	#include "map.humanloyalists.paladin.h"
	#include "map.humanloyalists.pikeman.h"
	#include "map.humanloyalists.royal-warrior.h"
	#include "map.humanloyalists.royalguard.h"
	#include "map.humanloyalists.sergeant.h"
	#include "map.humanloyalists.shocktrooper.h"
	#include "map.humanloyalists.siegetrooper.h"
	#include "map.humanloyalists.spearman.h"
	#include "map.humanloyalists.swordsman.h"
	
	#include "map.humanmagi.human-magi.h"
	#include "map.humanmagi.arch-mage.h"
	#include "map.humanmagi.arch-mage+female.h"
	#include "map.humanmagi.elder-mage.h"
	#include "map.humanmagi.great-mage.h"
	#include "map.humanmagi.great-mage+female.h"
	#include "map.humanmagi.mage.h"
	#include "map.humanmagi.mage+female.h"
	#include "map.humanmagi.red-mage.h"
	#include "map.humanmagi.red-mage+female.h"
	#include "map.humanmagi.silver-mage.h"
	#include "map.humanmagi.silver-mage+female.h"
	#include "map.humanmagi.white-cleric.h"
	#include "map.humanmagi.white-cleric+female.h"
	#include "map.humanmagi.white-mage.h"
	#include "map.humanmagi.white-mage+female.h"
	
	#include "map.humanoutlaws.human-outlaws.h"
	#include "map.humanoutlaws.assassin.h"
	#include "map.humanoutlaws.assassin+female.h"
	#include "map.humanoutlaws.bandit.h"
	#include "map.humanoutlaws.footpad.h"
	#include "map.humanoutlaws.footpad+female.h"
	#include "map.humanoutlaws.fugitive.h"
	#include "map.humanoutlaws.fugitive+female.h"
	#include "map.humanoutlaws.highwayman.h"
	#include "map.humanoutlaws.huntsman.h"
	#include "map.humanoutlaws.outlaw.h"
	#include "map.humanoutlaws.outlaw+female.h"
	#include "map.humanoutlaws.poacher.h"
	#include "map.humanoutlaws.ranger.h"
	#include "map.humanoutlaws.rogue.h"
	#include "map.humanoutlaws.rogue+female.h"
	#include "map.humanoutlaws.thief.h"
	#include "map.humanoutlaws.thief+female.h"
	#include "map.humanoutlaws.thug.h"
	#include "map.humanoutlaws.trapper.h"
	
	#include "map.humanpeasants.human-peasants.h"
	#include "map.humanpeasants.peasant.h"
	#include "map.humanpeasants.ruffian.h"
	#include "map.humanpeasants.woodsman.h"
	
	#include "map.merfolk.merfolk.h"
	#include "map.merfolk.initiate.h"
	
	#include "map.monsters.cavespider.h"
	#include "map.monsters.cuttlefish.h"
	#include "map.monsters.deep-tentacle.h"
	#include "map.monsters.fire-dragon.h"
	#include "map.monsters.giant-mudcrawler.h"
	#include "map.monsters.gryphon.h"
	#include "map.monsters.mudcrawler.h"
	#include "map.monsters.scorpion.h"
	#include "map.monsters.seaserpent.h"
	#include "map.monsters.skeletal-dragon.h"
	#include "map.monsters.wolf.h"
	#include "map.monsters.yeti.h"
	
	#include "map.nagas.nagas.h"
	#include "map.nagas.fighter.h"
	#include "map.nagas.myrmidon.h"
	#include "map.nagas.warrior.h"
	
	#include "map.ogres.ogre.h"
	#include "map.ogres.young-ogre.h"
	
	#include "map.orcs.orcs.h"
	#include "map.orcs.archer.h"
	#include "map.orcs.assassin.h"
	#include "map.orcs.grunt.h"
	#include "map.orcs.leader.h"
	#include "map.orcs.ruler.h"
	#include "map.orcs.slayer.h"
	#include "map.orcs.slurbow.h"
	#include "map.orcs.sovereign.h"
	#include "map.orcs.warlord.h"
	#include "map.orcs.warrior.h"
	#include "map.orcs.xbowman.h"
	
	#include "map.saurians.saurians.h"
	#include "map.saurians.ambusher.h"
	#include "map.saurians.augur.h"
	#include "map.saurians.flanker.h"
	#include "map.saurians.oracle.h"
	#include "map.saurians.skirmisher.h"
	#include "map.saurians.soothsayer.h"
	
	#include "map.transport.h"
	
	#include "map.trolls.trolls.h"
	#include "map.trolls.grunt.h"
	#include "map.trolls.lobber.h"
	#include "map.trolls.shaman.h"
	#include "map.trolls.warrior.h"
	#include "map.trolls.whelp.h"
	
	#include "map.undead.bat.h"
	#include "map.undead.bloodbat.h"
	#include "map.undead.dreadbat.h"
	#include "map.undead.ghost.h"
	#include "map.undead.ghoul.h"
	#include "map.undead.necrophage.h"
	#include "map.undead.nightgaunt.h"
	#include "map.undead.shadow.h"
	#include "map.undead.soulless-drake.h"
	#include "map.undead.soulless-dwarf.h"
	#include "map.undead.soulless-mounted.h"
	#include "map.undead.soulless-saurian.h"
	#include "map.undead.soulless-swimmer.h"
	#include "map.undead.soulless-troll.h"
	#include "map.undead.soulless-wose.h"
	#include "map.undead.soulless.h"
	#include "map.undead.spectre.h"
	#include "map.undead.wraith.h"
	#include "map.undead.zombie-bat.h"
	#include "map.undead.zombie-drake.h"
	#include "map.undead.zombie-dwarf.h"
	#include "map.undead.zombie-mounted.h"
	#include "map.undead.zombie-saurian.h"
	#include "map.undead.zombie-swimmer.h"
	#include "map.undead.zombie-troll.h"
	#include "map.undead.zombie-wose.h"
	#include "map.undead.zombie.h"
	
	#include "map.necromancers.undead-necromancers.h"
	#include "map.necromancers.adept.h"
	#include "map.necromancers.adept+female.h"
	#include "map.necromancers.ancient-lich.h"
	#include "map.necromancers.dark-sorcerer.h"
	#include "map.necromancers.dark-sorcerer+female.h"
	#include "map.necromancers.lich.h"
	#include "map.necromancers.necromancer.h"
	#include "map.necromancers.necromancer+female.h"
	
	#include "map.skeletal.undead-skeletal.h"
	#include "map.skeletal.archer.h"
	#include "map.skeletal.banebow.h"
	#include "map.skeletal.bone-shooter.h"
	#include "map.skeletal.chocobone.h"
	#include "map.skeletal.deathblade.h"
	#include "map.skeletal.deathknight.h"
	#include "map.skeletal.draug.h"
	#include "map.skeletal.revenant.h"
	#include "map.skeletal.skeleton.h"
	
	#include "map.woses.woses.h"
	#include "map.woses.wose-ancient.h"
	#include "map.woses.wose-elder.h"
	#include "map.woses.wose.h"
	
	#include "map.did.apprentice-mage.h"
	#include "map.did.apprentice-necromancer.h"
	#include "map.did.dark-mage.h"
	#include "map.did.ghast_rat.h"
	#include "map.did.neutral-outlaw-princess.h"
	
	#include "map.ei.owaec.h"
	
	#include "map.httt.gryphon_sea-orc.h"
	#include "map.httt.human-battleprincess.h"
	#include "map.httt.human-princess.h"
	#include "map.httt.human-queen.h"
	#include "map.httt.konrad-commander-scepter.h"
	#include "map.httt.konrad-commander.h"
	#include "map.httt.konrad-fighter.h"
	#include "map.httt.konrad-lord-scepter.h"
	#include "map.httt.konrad-lord.h"
	
	#include "map.liberty.undead-skeletal.h"
	#include "map.liberty.rogue-mage.h"
	#include "map.liberty.shadow-lord.h"
	#include "map.liberty.shadow-mage.h"
	
	#include "map.sof.dwarves.h"
	#include "map.sof.haldric-ii.h"
	
	#include "map.sotbe.units.h"
	
	#include "map.thot.annalist.h"
	#include "map.thot.karrag.h"
	#include "map.thot.loremaster.h"
	#include "map.thot.masked_dragonguard.h"
	#include "map.thot.masked_fighter.h"
	#include "map.thot.masked_guard.h"
	#include "map.thot.masked_lord.h"
	#include "map.thot.masked_sentinel.h"
	#include "map.thot.masked_stalwart.h"
	#include "map.thot.masked_steelclad.h"
	#include "map.thot.masked_thunderer.h"
	#include "map.thot.masked_thunderguard.h"
	#include "map.thot.witness.h"
	
	#include "map.trow.noble-commander.h"
	#include "map.trow.noble-fighter.h"
	#include "map.trow.noble-lord.h"
	#include "map.trow.noble-youth_undead.h"
	#include "map.trow.wesfolk-lady-masked.h"
	#include "map.trow.wesfolk-lady.h"
	#include "map.trow.wesfolk-leader-masked.h"
	#include "map.trow.wesfolk-leader.h"
	#include "map.trow.wesfolk-outcast-masked.h"
	#include "map.trow.wesfolk-outcast.h"
	#include "map.trow.wose-sapling.h"
	
	#include "map.tsg.commander.h"
	#include "map.tsg.eyestalk.h"
	#include "map.tsg.dismounted-commander.h"
	#include "map.tsg.horseman-commander.h"
	#include "map.tsg.junior-commander.h"
	#include "map.tsg.mounted-general.h"
	
	#include "map.tutorial.human-princess.h"
	#include "map.tutorial.konrad-fighter.h"
	#include "map.tutorial.quintain.h"
	
	#include "map.utbs.alien.h"
	#include "map.utbs.dwarves.explorer.h"
	#include "map.utbs.dwarves.pathfinder.h"
	#include "map.utbs.dwarves.scout.h"
	#include "map.utbs.elves.elves-desert.h"
	#include "map.utbs.elves.archer.h"
	#include "map.utbs.elves.archer+female.h"
	#include "map.utbs.elves.avenger.h"
	#include "map.utbs.elves.avenger+female.h"
	#include "map.utbs.elves.captain.h"
	#include "map.utbs.elves.champion.h"
	#include "map.utbs.elves.corrupted-elf.h"
	#include "map.utbs.elves.druid_eloh.h"
	#include "map.utbs.elves.fighter.h"
	#include "map.utbs.elves.hero.h"
	#include "map.utbs.elves.horseman.h"
	#include "map.utbs.elves.hunter.h"
	#include "map.utbs.elves.hunter+female.h"
	#include "map.utbs.elves.kaleh.h"
	#include "map.utbs.elves.marksman.h"
	#include "map.utbs.elves.marksman+female.h"
	#include "map.utbs.elves.marshal.h"
	#include "map.utbs.elves.nym.h"
	#include "map.utbs.elves.outrider.h"
	#include "map.utbs.elves.prowler.h"
	#include "map.utbs.elves.ranger.h"
	#include "map.utbs.elves.ranger+female.h"
	#include "map.utbs.elves.rider.h"
	#include "map.utbs.elves.scout.h"
	#include "map.utbs.elves.sentinel.h"
	#include "map.utbs.elves.shaman.h"
	#include "map.utbs.elves.sharpshooter.h"
	#include "map.utbs.elves.sharpshooter+female.h"
	#include "map.utbs.elves.shyde.h"
	#include "map.utbs.human-commander.h"
	#include "map.utbs.monsters.dawarf_demon.h"
	#include "map.utbs.monsters.fireghost.h"
	#include "map.utbs.monsters.monsters.h"
	#include "map.utbs.nagas.nagas.h"
	#include "map.utbs.nagas.hunter.h"
	#include "map.utbs.orcs.nightstalker.h"
	#include "map.utbs.undead.h"
	
	#include "map.iftu.alien-psy.h"
	#include "map.iftu.alien-verlissh.h"
	#include "map.iftu.demons.h"
	#include "map.iftu.dwarves.h"
	#include "map.iftu.elves.h"
	#include "map.iftu.fairies-sylvan.h"
	#include "map.iftu.fake.h"
	#include "map.iftu.goblins.h"
	#include "map.iftu.gryphons.h"
	#include "map.iftu.human-aragwaithi.archer.h"
	#include "map.iftu.human-aragwaithi.captain.h"
	#include "map.iftu.human-aragwaithi.eagle-rider.h"
	#include "map.iftu.human-aragwaithi.flagbearer.h"
	#include "map.iftu.human-aragwaithi.greatbow.h"
	#include "map.iftu.human-aragwaithi.guard.h"
	#include "map.iftu.human-aragwaithi.guardian.h"
	#include "map.iftu.human-aragwaithi.h"
	#include "map.iftu.human-aragwaithi.lancer.h"
	#include "map.iftu.human-aragwaithi.longswordsman.h"
	#include "map.iftu.human-aragwaithi.pikeman.h"
	#include "map.iftu.human-aragwaithi.scout.h"
	#include "map.iftu.human-aragwaithi.shield-guard.h"
	#include "map.iftu.human-aragwaithi.silver-shield.h"
	#include "map.iftu.human-aragwaithi.slayer.h"
	#include "map.iftu.human-aragwaithi.spearman.h"
	#include "map.iftu.human-aragwaithi.strongbow.h"
	#include "map.iftu.human-aragwaithi.swordman.h"
	#include "map.iftu.human-aragwaithi.swordmaster.h"
	#include "map.iftu.human-aragwaithi.warlock.h"
	#include "map.iftu.human-chaos.dark-knight.h"
	#include "map.iftu.human-chaos.h"
	#include "map.iftu.human-chaos.invader.h"
	#include "map.iftu.human-chaos.razerman.h"
	#include "map.iftu.human-peasants.h"
	#include "map.iftu.imps.h"
	#include "map.iftu.mechanical.h"
	#include "map.iftu.monsters.h"
	#include "map.iftu.monsters2.h"
	#include "map.iftu.monsters3.h"
	#include "map.iftu.monsters4.h"
	#include "map.iftu.monsters5.h"
	#include "map.iftu.orcs.h"
	#include "map.iftu.shaxthal.h"
	#include "map.iftu.spirits.animated-rock.h"
	#include "map.iftu.spirits.fire.h"
	#include "map.iftu.spirits.firewisp.h"
	#include "map.iftu.spirits.rock-golem.h"
	#include "map.iftu.spirits.water.h"
	#include "map.iftu.trolls.h"
	#include "map.iftu.undead-skeletal.h"
	#include "map.iftu.undead.h"
	
	#include "map.dw.brawler.h"
	#include "map.dw.child_king.h"
	#include "map.dw.citizen.h"
	#include "map.dw.dark_shape.h"
	#include "map.dw.fireghost.h"
	#include "map.dw.kraken.h"
	#include "map.dw.soldier_king.h"
	#include "map.dw.warrior_king.h"
	#include "map.dw.young_king.h"
	
	#include "map.blank.h"
	#include "../res/images/misc/map.ellipse.h"
	
	// FLAGS
	#include "map.black-flag.h"
	#include "map.flag.h"
	#include "map.knalgan-flag.h"
	#include "map.loyalist-flag.h"
	#include "map.SG-flag.h"
	#include "map.undead-flag.h"
	
	// HALOS
	#include "map.elven.druid-healing.h"
	#include "map.elven.elven-shield.h"
	#include "map.elven.faerie-fire.h"
	#include "map.elven.ice-halo.h"
	#include "map.elven.nature-halo.h"
	#include "map.elven.shaman-heal.h"
	#include "map.elven.shyde.h"
	#include "map.fire-aura.h"
	#include "map.flame-burst.h"
	#include "map.holy.halo.h"
	#include "map.holy.lightbeam.h"
	#include "map.iftu.avatar.h"
	#include "map.iftu.bomb-explode.h"
	#include "map.iftu.chaos.h"
	#include "map.iftu.darkness-beam.h"
	#include "map.iftu.elynia-noillum.h"
	#include "map.iftu.elynia.h"
	#include "map.iftu.illuminates.h"
	#include "map.iftu.obscures.h"
	#include "map.iftu.wose-ranged.h"
	#include "map.iftu.wose.h"
	#include "map.illuminates-aura.h"
	#include "map.liberty.shadow-mage.h"
	#include "map.lighthouse-aura.h"
	#include "map.lightning-bolt.h"
	#include "map.mage-halo-big.h"
	#include "map.mage-halo.h"
	#include "map.mage-preparation.h"
	#include "map.merfolk.staff-flare.h"
	#include "map.merfolk.water.h"
	#include "map.saurian-magic-halo.h"
	#include "map.teleport.h"
	#include "map.thot.karrag.h"
	#include "map.undead.h"
		
	
}

void freeUnitTextureAtlas(void)
{
	for (int i=0; i < NUM_UNITMAPS; i++)
	{
		if (gUnitTexIds[i] != 0)
		{
			glDeleteTextures(1, &gUnitTexIds[i]);
			//renderQueueDeleteTexture(gUnitTexIds[i]);
			gUnitTexIds[i] = 0;
		}
	}
	gUnitAtlasLRU.clear();
	gUnitAtlasTotalSize = 0;
}

bool getUnitTextureAtlasInfo(const std::string& filename, const std::string& modifications, textureAtlasInfo& tinfo)
{
	std::string searchStr;
	if (filename.compare(0, 5, "halo/") == 0)
	{
		// chop off redundant "halo/"
		searchStr = filename.substr(5);
		// check for imbedded ~ character
		int pos = searchStr.find('~', 0);
		if (pos != std::string::npos)
		{
			searchStr[pos] = 0;
		}
	}
	else
	{
		// normal unit images
		searchStr = filename;
	}
	
	std::map<shared_string, textureAtlasInfo>::iterator it;
	it = gUnitAtlasMap.find(searchStr);
	if (it == gUnitAtlasMap.end())
	{
		if (filename != "")
		{
			std::cerr << "\n\n** ERROR: UnitTextureAtlas - could not find map for file " << filename << "\n\n";
		}
		//assert(false);
		//return false;
		// in non-debug mode, well... at least show the wrong unit instead of nothing...
		//it = gUnitAtlasMap.begin();
		tinfo.mapId = 0;
		return false;
	}
	
	tinfo = it->second;

	int rcPos = modifications.find("~RC(magenta>");
	if (rcPos != std::string::npos)
	{
		// adjust tinfo.mapId based on modifications string
		rcPos += 12;
		int colorInt = 0;
		if (modifications.size() > rcPos+2)
		{
			// also convert color words
			if (modifications[rcPos] == 'r')
				colorInt = 1;	// red
			else if (modifications[rcPos] == 'b' && modifications[rcPos+2] == 'u')
				colorInt = 2;	// blue
			else if (modifications[rcPos] == 'g')
				colorInt = 3;	// green
			else if (modifications[rcPos] == 'p')
				colorInt = 4;	// purple
			else if (modifications[rcPos] == 'b' && modifications[rcPos+2] == 'a')
				colorInt = 5;	// black
			else if (modifications[rcPos] == 'b' && modifications[rcPos+2] == 'o')
				colorInt = 6;	// brown
			else if (modifications[rcPos] == 'o')
				colorInt = 7;	// orange
			else if (modifications[rcPos] == 'w')
				colorInt = 8;	// white
			else if (modifications[rcPos] == 't')
				colorInt = 9;	// teal
			
		}
		else
		{
			char colorChar = modifications[rcPos];
			colorInt = colorChar - '0';
		}
		if (colorInt >= 10)
			colorInt = 0;
		tinfo.mapId += colorInt;
	}
	// new special case for ellipse graphics
	rcPos = modifications.find("~RC(ellipse_red>");
	if (rcPos != std::string::npos)
	{
		// adjust tinfo.mapId based on modifications string
		rcPos += 16;
		int colorInt = 0;
		if (modifications.size() > rcPos+2)
		{
			// also convert color words
			if (modifications[rcPos] == 'r')
				colorInt = 1;	// red
			else if (modifications[rcPos] == 'b' && modifications[rcPos+2] == 'u')
				colorInt = 2;	// blue
			else if (modifications[rcPos] == 'g')
				colorInt = 3;	// green
			else if (modifications[rcPos] == 'p')
				colorInt = 4;	// purple
			else if (modifications[rcPos] == 'b' && modifications[rcPos+2] == 'a')
				colorInt = 5;	// black
			else if (modifications[rcPos] == 'b' && modifications[rcPos+2] == 'o')
				colorInt = 6;	// brown
			else if (modifications[rcPos] == 'o')
				colorInt = 7;	// orange
			else if (modifications[rcPos] == 'w')
				colorInt = 8;	// white
			else if (modifications[rcPos] == 't')
				colorInt = 9;	// teal
			
		}
		else
		{
				char colorChar = modifications[rcPos];
				colorInt = colorChar - '0';
		}
		if (colorInt >= 10)
			colorInt = 0;
		tinfo.mapId += colorInt;
	}
	// new case for flags
	rcPos = modifications.find("~RC(flag_green>");
	if (rcPos != std::string::npos)
	{
		// adjust tinfo.mapId based on modifications string
		rcPos += 15;
		int colorInt = 0;
		if (modifications.size() > rcPos+2)
		{
			// also convert color words
			if (modifications[rcPos] == 'r')
				colorInt = 1;	// red
			else if (modifications[rcPos] == 'b' && modifications[rcPos+2] == 'u')
				colorInt = 2;	// blue
			else if (modifications[rcPos] == 'g')
				colorInt = 3;	// green
			else if (modifications[rcPos] == 'p')
				colorInt = 4;	// purple
			else if (modifications[rcPos] == 'b' && modifications[rcPos+2] == 'a')
				colorInt = 5;	// black
			else if (modifications[rcPos] == 'b' && modifications[rcPos+2] == 'o')
				colorInt = 6;	// brown
			else if (modifications[rcPos] == 'o')
				colorInt = 7;	// orange
			else if (modifications[rcPos] == 'w')
				colorInt = 8;	// white
			else if (modifications[rcPos] == 't')
				colorInt = 9;	// teal
			
		}
		else
		{
			char colorChar = modifications[rcPos];
			colorInt = colorChar - '0';
		}
		if (colorInt >= 10)
			colorInt = 0;
		tinfo.mapId += colorInt;
	}
	
	
	return true;
}

unsigned int getUnitTextureForMap(unsigned short mapId)
{
	return gUnitTexIds[mapId];
}

void renderUnitAtlas(int x, int y, const textureAtlasInfo& tinfo, SDL_Rect *srcRect /*= NULL*/, SDL_Rect *dstRect /*= NULL*/,
				 textureRenderFlags drawFlags /*= DRAW*/, unsigned long drawColor /*= 0xFFFFFFFF*/, float brightness)
{
	assert (tinfo.mapId < NUM_UNITMAPS);
		
	if (gUnitTexIds[tinfo.mapId] == 0)
	{
		loadUnitMap(tinfo.mapId);
    
		if (gUnitTexIds[tinfo.mapId] == 0)
			return;		// problem loading texture...
	}
	else
	{
		// move to front of LRU list
		std::list<unitAtlasData>::iterator it;
		for (it = gUnitAtlasLRU.begin(); it != gUnitAtlasLRU.end(); it++)
		{
			if (it->mapId == tinfo.mapId)
			{
				unitAtlasData uad = *it;
				//gUnitAtlasLRU.erase((++rit).base());	// need to compile this way, it's the same as erase(rit);
				gUnitAtlasLRU.erase(it);
				gUnitAtlasLRU.push_back(uad);
				break;
			}
		}
	}
	
	int minx, miny, maxx, maxy;
    GLfloat minu, maxu, minv, maxv;
 	
	
	//	if (lastTexture != texturedata->texture)
	{
		//		lastTexture = texturedata->texture;
		//		glBindTexture(GL_TEXTURE_2D, gTexIds[tinfo.mapId]);
	}
	
	//	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	
	// get the src/dst rects sorted out
	SDL_Rect clip;
	
	//	if(clip_rect != NULL) 
	//	{
	//		clip = *clip_rect;
	//	} 
	//	else 
	//	{
	getClipRect(&clip);
	//	}
	SDL_Rect dst = {x, y, tinfo.originalW, tinfo.originalH};
	if (dstRect)
		dst = *dstRect;
	SDL_Rect src = {0, 0, tinfo.originalW, tinfo.originalH};
	if (srcRect)
		src = *srcRect;
	
	// the atlas images have already been cropped, so compensate the rects...
	float scale = (float)dst.w/src.w;
	dst.x += tinfo.trimmedX*scale;
	dst.y += tinfo.trimmedY*scale;
	dst.w -= (tinfo.originalW - tinfo.texW)*scale;
	dst.h -= (tinfo.originalH - tinfo.texH)*scale;
	
	// KP: fix #18
	if ((drawFlags & FLOP) != 0)
	{
		int trimmedRight = tinfo.originalW - tinfo.trimmedX - tinfo.texW;
		dst.x -= tinfo.trimmedX - trimmedRight;
	}
	
	
	src.x -= tinfo.trimmedX;
	if (src.x < 0)
		src.x = 0;
	src.y -= tinfo.trimmedY;
	if (src.y < 0)
		src.y = 0;
	src.w -= (tinfo.originalW - tinfo.texW);
	src.h -= (tinfo.originalH - tinfo.texH);
	
	
	// perform clipping
	SDL_Rect clippedDst;
	SDL_Rect clippedSrc;
	bool result = SDL_IntersectRect(&dst, &clip, &clippedDst);
	if (!result)
		return;	// outside clip area
	
	//clippedSrc = src;
	clippedSrc.x = tinfo.texOffsetX + src.x;
	clippedSrc.y = tinfo.texOffsetY + src.y;
	clippedSrc.w = tinfo.texW - (tinfo.texW-src.w);
	clippedSrc.h = tinfo.texH - (tinfo.texH-src.h);
	
	SDL_Rect clipAmount;
	clipAmount.x = clippedDst.x - dst.x;
	clipAmount.y = clippedDst.y - dst.y;
	clipAmount.w = dst.w - clippedDst.w;
	clipAmount.h = dst.h - clippedDst.h;
	
	// unit images are only flopped with the drawFlags, not tinfo flags
	short flags = drawFlags;
	
	if ((flags & FLOP) != 0)
	{
		clippedSrc.x = clippedSrc.x + clippedSrc.w - clipAmount.x;
		clippedSrc.w = -clippedSrc.w + clipAmount.w;
	}
	else
	{
		clippedSrc.x += clipAmount.x;
		clippedSrc.w -= clipAmount.w;
	}
	if ((flags & FLIP) != 0)
	{
		clippedSrc.y = clippedSrc.y + clippedSrc.h - clipAmount.y;
		clippedSrc.h = -clippedSrc.h + clipAmount.h;
	}
	else
	{
		clippedSrc.y += clipAmount.y;
		clippedSrc.h -= clipAmount.h;
	}
	
	minx = clippedDst.x;
	miny = clippedDst.y;
	maxx = clippedDst.x + clippedDst.w;
	maxy = clippedDst.y + clippedDst.h;
	
	
	minu = (GLfloat) clippedSrc.x / gUnitTexW[tinfo.mapId];
	maxu = (GLfloat) (clippedSrc.x + clippedSrc.w) / gUnitTexW[tinfo.mapId];
	minv = (GLfloat) clippedSrc.y / gUnitTexH[tinfo.mapId];
	maxv = (GLfloat) (clippedSrc.y + clippedSrc.h) / gUnitTexH[tinfo.mapId];
	
	GLshort vertices[8];
	GLfloat texCoords[8];
	
	vertices[0] = minx;
	vertices[1] = miny;
	vertices[2] = maxx;
	vertices[3] = miny;
	vertices[4] = minx;
	vertices[5] = maxy;
	vertices[6] = maxx;
	vertices[7] = maxy;
	/*	
	 B---C       ->      A B
	 A---D				| |
	 | |
	 C D
	 */
	if ((flags & ROT) != 0)
	{
		texCoords[0] = minu;
		texCoords[1] = minv;
		texCoords[2] = minu;
		texCoords[3] = maxv;
		texCoords[4] = maxu;
		texCoords[5] = minv;
		texCoords[6] = maxu;
		texCoords[7] = maxv;
	}
	else
	{
		texCoords[0] = minu;
		texCoords[1] = minv;
		texCoords[2] = maxu;
		texCoords[3] = minv;
		texCoords[4] = minu;
		texCoords[5] = maxv;
		texCoords[6] = maxu;
		texCoords[7] = maxv;
	}
	
	//	glVertexPointer(3, GL_SHORT, 0, vertices);
	//	glTexCoordPointer(2, GL_FLOAT, 0, texCoords);
	//	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	renderQueueAddTexture(vertices, texCoords, gUnitTexIds[tinfo.mapId], drawColor, brightness);
	
}
