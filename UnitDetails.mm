//
//  UnitDetails.m
//  wesnoth
//
//  Created by Kyle Poole on 5/16/10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "UnitDetails.h"

#include "unit_types.hpp"
#include "game_preferences.hpp"
#include "game_display.hpp"
#include "map.hpp"
#include "filesystem.hpp"

@implementation UnitDetails

@synthesize webView;

extern UIView *gLandscapeView;
extern bool gPauseForOpenFeint;

extern const unit_type* gUnitType;



/*
 // The designated initializer.  Override if you create the controller programmatically and want to perform customization that is not appropriate for viewDidLoad.
- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil {
    if ((self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil])) {
        // Custom initialization
    }
    return self;
}
*/

#define HTML_SPACER "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"


// Implement viewDidLoad to do additional setup after loading the view, typically from a nib.
- (void)viewDidLoad {
    [super viewDidLoad];
	
//	[scrollView setContentSize:
//    CGSizeMake(470, 768)
//	 ];
	
	// this will force the lazy loading to build this unit
	unit_type_data::types().find_unit_type(gUnitType->id(), unit_type::WITHOUT_ANIMATIONS);
	
	std::stringstream ss;
	std::string clear_stringstream;
	const std::string detailed_description = gUnitType->unit_description();
	const unit_type& female_type = gUnitType->get_gender_unit_type(unit_race::FEMALE);
	const unit_type& male_type = gUnitType->get_gender_unit_type(unit_race::MALE);
	
	ss << "<html><body text='white' style='background-color: transparent; -webkit-user-select: none;'>";
	
	ss << "<h2>" << gUnitType->type_name() << "</h2>";
	
	// Show the unit's image and its level.
	std::string filename = get_binary_file_location("images", male_type.image());
	//SDL_SaveBMP(map_screenshot_surf_, filename.c_str());
	ss << HTML_SPACER << "<img src='" << filename << "' style='border: 2px black solid;'></img> ";
	
	if (&female_type != &male_type)
	{
		filename = get_binary_file_location("images", female_type.image());
		ss << "<img src='" << filename << "' style='border: 1px black solid;'></img> ";
	}
	
	
	ss << "<br>Level " << gUnitType->level();
	/*
	 const std::string& male_portrait = male_type.image_profile();
	 const std::string& female_portrait = female_type.image_profile();
	 
	 if (male_portrait.empty() == false && male_portrait != male_type.image()) {
	 ss << "<img>src='" << male_portrait << "' align='right'</img> ";
	 }
	 
	 if (female_portrait.empty() == false && female_portrait != male_portrait && female_portrait != female_type.image()) {
	 ss << "<img>src='" << female_portrait << "' align='right'</img> ";
	 }
	 */
	ss << "<br>";
	
	// Print cross-references to units that this unit advances from.
	std::vector<shared_string> from_units = gUnitType->advances_from();
	if (!from_units.empty())
	{
		ss << ("Advances from: ");
		for (std::vector<shared_string>::const_iterator from_iter = from_units.begin();
			 from_iter != from_units.end();
			 ++from_iter)
		{
			std::string unit_id = *from_iter;
			std::map<std::string,unit_type>::const_iterator type = unit_type_data::types().find_unit_type(unit_id);
			if (type != unit_type_data::types().end() && !type->second.hide_help())
			{
				if (from_iter != from_units.begin()) ss << ", ";
				std::string lang_unit = type->second.type_name();
//				std::string ref_id;
//				if (description_type(type->second) == FULL_DESCRIPTION) {
//					ref_id = unit_prefix + type->second.id();
//				} else {
//					ref_id = unknown_unit_topic;
//					lang_unit += " (?)";
//				}
//				ss << "<ref>dst='" << escape(ref_id) << "' text='" << escape(lang_unit) << "'</ref>";
				ss << lang_unit;
			}
		}
		ss << "<br>";
	}
	
	// Print the units this unit can advance to. Cross reference
	// to the topics containing information about those units.
	std::vector<shared_string> next_units = gUnitType->advances_to();
	if (!next_units.empty()) {
		ss << ("Advances to: ");
		for (std::vector<shared_string>::const_iterator advance_it = next_units.begin(),
			 advance_end = next_units.end();
			 advance_it != advance_end; ++advance_it) {
			std::string unit_id = *advance_it;
			std::map<std::string,unit_type>::const_iterator type = unit_type_data::types().find_unit_type(unit_id);
			if(type != unit_type_data::types().end() && !type->second.hide_help()) {
				if (advance_it != next_units.begin()) ss << ", ";
				std::string lang_unit = type->second.type_name();
//				std::string ref_id;
//				if (description_type(type->second) == FULL_DESCRIPTION) {
//					ref_id = unit_prefix + type->second.id();
//				} else {
//					ref_id = unknown_unit_topic;
//					lang_unit += " (?)";
//				}
//				ss << "<ref>dst='" << escape(ref_id) << "' text='" << escape(lang_unit) << "'</ref>";
				ss << lang_unit;
			}
		}
		ss << "<br>";
	}
	if (gUnitType->can_advance())
		ss << ("Required XP: ") << gUnitType->experience_needed() << "<br>";

	
	// Print the race of the unit, cross-reference it to the
	// respective topic.
/*	const std::string race_id = gUnitType->race();
	std::string race_name;
	const race_map::const_iterator race_it = unit_type_data::types().races().find(race_id);
	if (race_it != unit_type_data::types().races().end()) {
		race_name = race_it->second.plural_name().c_str();
	} else {
		race_name = ("Miscellaneous");
	}
	ss << ("Race: ");
	ss << race_name;
	ss << "<br>";
*/	
	// Print some basic information such as HP and movement points.
	ss << ("HP: ") << gUnitType->hitpoints() << HTML_SPACER
	<< ("Moves: ") << gUnitType->movement() << HTML_SPACER
	<< ("Cost: ") << gUnitType->cost() << HTML_SPACER
	<< ("<br>Alignment: ")
	//<< "<ref>dst='time_of_day' text='"
	<< gUnitType->alignment_description(gUnitType->alignment(), gUnitType->genders().front())
	//<< "'</ref>"
	<< "<br>";
	
	// Print the abilities the units has, cross-reference them
	// to their respective topics.
	if (!gUnitType->abilities().empty()) {
		ss << ("<br>Abilities: ");
		for(std::vector<shared_string>::const_iterator ability_it = gUnitType->abilities().begin(),
			ability_end = gUnitType->abilities().end();
			ability_it != ability_end; ++ability_it) {
			const std::string ref_id = "ability_" + *ability_it;
			std::string lang_ability = ability_it->c_str();
			ss << lang_ability;
			if (ability_it + 1 != ability_end)
				ss << ", ";
		}
		ss << "<br>";
	}
	
	// Print the extra AMLA upgrage abilities, cross-reference them
	// to their respective topics.
	if (!gUnitType->adv_abilities().empty()) {
		ss << ("Ability Upgrades: ");
		for(std::vector<shared_string>::const_iterator ability_it = gUnitType->adv_abilities().begin(),
			ability_end = gUnitType->adv_abilities().end();
			ability_it != ability_end; ++ability_it) {
			const std::string ref_id = "ability_" + *ability_it;
			std::string lang_ability = ability_it->c_str();
			ss << lang_ability;
			if (ability_it + 1 != ability_end)
				ss << ", ";
		}
		ss << "<br>";
	}
	ss << "<br>";
	
	
	// Print the detailed description about the unit.
	ss << detailed_description;
	
	// Print the different attacks a unit has, if it has any.
	std::vector<attack_type> attacks = gUnitType->attacks();
	if (!attacks.empty()) {
		// Print headers for the table.
		//ss << "\n\n<header>text='" << escape(_("unit help^Attacks"))
		//<< "'</header>\n\n";
		ss << "<h2>Attacks</h2>";
		//table_spec table;
		
		//std::vector<item> first_row;
		// Dummy element, icons are below.
		//first_row.push_back(item("", 0));
		//push_header(first_row, _("unit help^Name"));
		//push_header(first_row, _("Type"));
		//push_header(first_row, _("Dmg"));
		//push_header(first_row, _("Range"));
		//push_header(first_row, _("Special"));
		ss << "<table><tr><td></td><td><b>Name</b></td><td><b>Dmg</b></td><td><b>Range</b></td><td><b>Special</b></td></tr>";
		//table.push_back(first_row);
		// Print information about every attack.
		for(std::vector<attack_type>::const_iterator attack_it = attacks.begin(),
			attack_end = attacks.end();
			attack_it != attack_end; ++attack_it) {
			std::string lang_weapon = attack_it->name();
			std::string lang_type = attack_it->type().c_str();
			//std::vector<item> row;
			//std::stringstream attack_ss;
			//attack_ss << "<img src='" << (*attack_it).icon() << "'</img>";
			ss << "<tr><td><img src='" << get_binary_file_location("images", (*attack_it).icon()) << "'</img></td><td>";
			//row.push_back(std::make_pair(attack_ss.str(),
			//							 image_width(attack_it->icon())));
			ss << lang_weapon << HTML_SPACER << "</td>";
			//push_tab_pair(row, lang_weapon);
			//push_tab_pair(row, lang_type);
			//attack_ss.str(clear_stringstream);
			ss << "<td>" << attack_it->damage() << '-' << attack_it->num_attacks() << " " << attack_it->accuracy_parry_description();
			ss << HTML_SPACER << "</td>";
			//push_tab_pair(row, attack_ss.str());
			//attack_ss.str(clear_stringstream);
			//push_tab_pair(row, _((*attack_it).range().c_str()));
			ss << "<td>" << (*attack_it).range() << HTML_SPACER << "</td>";
			// Show this attack's special, if it has any. Cross
			// reference it to the section describing the
			// special.
			std::vector<t_string> specials = attack_it->special_tooltips(true);
			ss << "<td>";
			if(!specials.empty())
			{
				std::string lang_special = "";
				std::vector<t_string>::iterator sp_it;
				for (sp_it = specials.begin(); sp_it != specials.end(); sp_it++) {
					const std::string ref_id = std::string("weaponspecial_")
					+ *sp_it;
					lang_special = (*sp_it);
					//attack_ss << "<ref>dst='" << escape(ref_id)
					// << "' text='" << escape(lang_special) << "'</ref>";
					ss << lang_special;
					
					if((sp_it + 1) != specials.end() && (sp_it + 2) != specials.end())
					{
						//attack_ss << ", "; //comma placed before next special
						ss << ", ";
					}
					sp_it++; //skip description
				}
				//row.push_back(std::make_pair(attack_ss.str(),
				//							 font::line_width(lang_special, normal_font_size)));
				
			}
			ss << "</td>";
			//table.push_back(row);
			ss << "</tr>";
		}
		//ss << generate_table(table);
		ss << "</table>";
	}
	
	// Print the resistance table of the unit.
	ss << "<h2>" << "Resistances"
	<< "</h2>";
	//table_spec resistance_table;
	//std::vector<item> first_res_row;
	ss << "<table><tr><td><b>Attack Type</b>" << HTML_SPACER << "</td><td><b>Resistance</b></td></tr>";
	//push_header(first_res_row, _("Attack Type"));
	//push_header(first_res_row, _("Resistance"));
	//resistance_table.push_back(first_res_row);
	const unit_movement_type &movement_type = gUnitType->movement_type();
	string_map dam_tab = movement_type.damage_table();
	for(string_map::const_iterator dam_it = dam_tab.begin(), dam_end = dam_tab.end();
		dam_it != dam_end; ++dam_it) {
		//std::vector<item> row;
		int resistance = 100 - atoi((*dam_it).second.c_str());
		char resi[16];
		snprintf(resi,sizeof(resi),"% 4d%%",resistance);	// range: -100% .. +70%
		//FIXME: "white" is currently not a supported color key
		//so the default grey-white will be used
		std::string color;
		if (resistance < 0)
			color = "red";
		else if (resistance <= 20)
			color = "yellow";
		else if (resistance <= 40)
			color = "white";
		else
			color = "green";
		
		std::string lang_weapon = dam_it->first;
		//push_tab_pair(row, lang_weapon);
		
		//std::stringstream str;
		//str << "<format>color=" << color << " text='"<< resi << "'</format>";
		//const std::string markup = str.str();
		//str.str(clear_stringstream);
		ss << "<tr><td>" << lang_weapon << "</td>";
		ss << "<td><font color='" << color << "'>" << resi << "</font></td>";
		//str << resi;
		//row.push_back(std::make_pair(markup,
		//							 font::line_width(str.str(), normal_font_size)));
		//resistance_table.push_back(row);
		ss << "</tr>";
	}
	//ss << generate_table(resistance_table);
	ss << "</table>";
	
	const gamemap *map = &game_display::get_singleton()->get_map();
	if (map != NULL) {
		// Print the terrain modifier table of the unit.
		ss << "<h2>" << "Terrain Modifiers"
		<< "</h2>";
		//std::vector<item> first_row;
		//table_spec table;
		ss << "<table><tr><td><b>Terrain</b></td><td><b>Defense</b>" << HTML_SPACER << "</td><td><b>Move Cost</b></td></tr>";
		//push_header(first_row, _("Terrain"));
		//push_header(first_row, _("Defense"));
		//push_header(first_row, _("Movement Cost"));
		
		//table.push_back(first_row);
		std::set<t_translation::t_terrain>::const_iterator terrain_it =
		preferences::encountered_terrains().begin();
		
		for (; terrain_it != preferences::encountered_terrains().end();
			 terrain_it++) {
			const t_translation::t_terrain terrain = *terrain_it;
			if (terrain == t_translation::FOGGED || terrain == t_translation::VOID_TERRAIN || terrain == t_translation::OFF_MAP_USER)
				continue;
			const terrain_type& info = map->get_terrain_info(terrain);
			
			if (info.union_type().size() == 1 && info.union_type()[0] == info.number() && info.is_nonnull()) {
				//std::vector<item> row;
				const std::string& name = info.name();
				const std::string id = info.id();
				const int moves = movement_type.movement_cost(*map,terrain);
				std::stringstream str;
				//str << "<ref>text='" << escape(name) << "' dst='"
				//<< escape(std::string("terrain_") + id) << "'</ref>";
				//row.push_back(std::make_pair(str.str(),
				//							 font::line_width(name, normal_font_size)));
				ss << "<tr><td>" << name << HTML_SPACER << "</td>";
				
				//defense  -  range: +10 % .. +70 %
				//str.str(clear_stringstream);
				const int defense =
				100 - movement_type.defense_modifier(*map,terrain);
				std::string color;
				if (defense <= 10)
					color = "red";
				else if (defense <= 30)
					color = "yellow";
				else if (defense <= 50)
					color = "white";
				else
					color = "green";
				
				//str << "<format>color=" << color << " text='"<< defense << "%'</format>";
				ss << "<td><font color='" << color << "'>" << defense << "%</font></td>";
				
				//const std::string markup = str.str();
				//str.str(clear_stringstream);
				//str << defense << "%";
				//row.push_back(std::make_pair(markup,
				//							 font::line_width(str.str(), normal_font_size)));
				
				//movement  -  range: 1 .. 5, unit_movement_type::UNREACHABLE=impassable
				//str.str(clear_stringstream);
				const bool cannot_move = moves > gUnitType->movement();
				if (cannot_move)		// cannot move in this terrain
					color = "red";
				else if (moves > 1)
					color = "yellow";
				else
					color = "white";
				
				//str << "<format>color=" << color << " text='";
				ss << "<td><font color='" << color << "'>";
				
				// A 5 MP margin; if the movement costs go above
				// the unit's max moves + 5, we replace it with dashes.
				if(cannot_move && (moves > gUnitType->movement() + 5)) {
					ss << "-";
				} else {
					ss << moves;
				}
				//str << "'</format>";
				//push_tab_pair(row, str.str());
				ss << "</font></td>";
				
				//table.push_back(row);
				ss << "</tr>";
			}
		}
		//ss << generate_table(table);
		ss << "</table>";
	}
	//return ss.str();
	
	ss << "</body></html>";
	
	NSString *html = [NSString stringWithUTF8String:ss.str().c_str()];
	webView.opaque = NO;
	webView.backgroundColor = [UIColor clearColor];
	[webView loadHTMLString:html baseURL:[NSURL fileURLWithPath:[[NSBundle mainBundle] bundlePath]]];
	
}

/*
// Override to allow orientations other than the default portrait orientation.
- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
    // Return YES for supported orientations
    return (interfaceOrientation == UIInterfaceOrientationPortrait);
}
*/

- (void)didReceiveMemoryWarning {
    // Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
    
    // Release any cached data, images, etc that aren't in use.
}

- (void)viewDidUnload {
    [super viewDidUnload];
    // Release any retained subviews of the main view.
    // e.g. self.myOutlet = nil;
}


- (void)dealloc {
    [super dealloc];
}

-(IBAction)onOK:(id)sender
{
	[self.view removeFromSuperview];
	gLandscapeView.hidden = YES;
	gPauseForOpenFeint = false;
}

@end
