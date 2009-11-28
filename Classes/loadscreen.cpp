/* $Id: loadscreen.cpp 33126 2009-02-27 16:32:20Z soliton $ */
/*
   Copyright (C) 2005 - 2009 by Joeri Melis <joeri_melis@hotmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file loadscreen.cpp
 * Screen with logo and "Loading ..."-progressbar during program-startup.
 */

#include <OpenGLES/ES1/gl.h>
#include <OpenGLES/ES1/glext.h>

#include "loadscreen.hpp"

#include "log.hpp"
#include "font.hpp"
#include "marked-up_text.hpp"
#include "gettext.hpp"
#include "filesystem.hpp"
#include "game_config.hpp"

#include "config.hpp"
#include "construct_dialog.hpp"
#include "serialization/parser.hpp"
#include "serialization/preprocessor.hpp"
#include "game_preferences.hpp"
#include "show_dialog.hpp"

#include <SDL_image.h>

#define INFO_DISP LOG_STREAM(info, display)
#define ERR_DISP LOG_STREAM(err, display)

#define MIN_PERCENTAGE   0
#define MAX_PERCENTAGE 100

loadscreen::global_loadscreen_manager* loadscreen::global_loadscreen_manager::manager = 0;

loadscreen::global_loadscreen_manager::global_loadscreen_manager(CVideo& screen)
  : owns(global_loadscreen == 0)
{
	if(owns) {
		manager = this;
		global_loadscreen = new loadscreen(screen);
		//global_loadscreen->clear_screen();
	}
}

loadscreen::global_loadscreen_manager::~global_loadscreen_manager()
{
	reset();
}

void loadscreen::global_loadscreen_manager::reset()
{
	if(owns) {
		owns = false;
		manager = 0;
		assert(global_loadscreen);
		//global_loadscreen->clear_screen();
		delete global_loadscreen;
		global_loadscreen = 0;
	}
}


/** Read the file with the tips-of-the-day. */
static void read_tips_of_day(config& tips_of_day)
{
	loadscreen::global_loadscreen->disable_increments = true;
	
	tips_of_day.clear();

#ifdef FREE_VERSION
	std::string filename = get_cache_dir() + "/tips_free";
#else
	std::string filename = get_cache_dir() + "/tips";
#endif
	std::string checkFilename = filename + ".cache.dat";
	if (!file_exists(checkFilename))
	{
		try {
#ifdef FREE_VERSION
			scoped_istream stream = preprocess_file(get_wml_location("hardwired/tips_free.cfg"));
#else
			scoped_istream stream = preprocess_file(get_wml_location("hardwired/tips.cfg"));
#endif
			read(tips_of_day, *stream);
		} catch(config::error&) {
	//		ERR_CONFIG << "Could not read data/hardwired/tips.cfg\n";
		}
	
		tips_of_day.saveCache(filename);
	}
	else
	{
		tips_of_day.loadCache(filename);
	}
	
	
	//we shuffle the tips after each initial loading. We only shuffle if
	//the upload_log preference has been set. If it hasn't been set, it's the
	//user's first time playing since this feature has been added, so we'll
	//leave the tips in their default order, which will always contain a tip
	//regarding the upload log first, so the user sees it.
	config::child_itors tips = tips_of_day.child_range("tip");
	if (tips.first != tips.second && preferences::has_upload_log()) {
		std::random_shuffle(tips.first, tips.second);
	}
	
	//Make sure that the upload log preference is set, if it's not already, so
	//that we know next time we've already seen the message about uploads.
	if(!preferences::has_upload_log()) 
	{
		//preferences::set_upload_log(preferences::upload_log());
		preferences::set_upload_log(false);
	}
	loadscreen::global_loadscreen->disable_increments = false;

}

/** Go to the next tips-of-the-day */
static void next_tip_of_day(config& tips_of_day, bool reverse = false)
{
	// we just rotate the tip list, to avoid the need to keep track
	// of the current one, and keep it valid, cycle it, etc...
	config::child_itors tips = tips_of_day.child_range("tip");
	if (tips.first != tips.second) {
		config::child_iterator direction = reverse ? tips.first+1 : tips.second-1;
		std::rotate(tips.first, direction, tips.second);
	}
}

/** Return the text for one of the tips-of-the-day. */
static const config* get_tip_of_day(config& tips_of_day)
{
	if (tips_of_day.empty()) {
		read_tips_of_day(tips_of_day);
	}
	
	const config::child_list& tips = tips_of_day.get_children("tip");
	
	// next_tip_of_day rotate tips, so better stay iterator-safe
	for (size_t t=0; t < tips.size(); t++, next_tip_of_day(tips_of_day)) {
		const config* tip = tips.front();
		if (tip == NULL) continue;
		
		const std::vector<shared_string> needed_units = utils::split_shared((*tip)["encountered_units"],',');
		if (needed_units.empty()) {
			return tip;
		}
		const std::set<shared_string>& seen_units = preferences::encountered_units();
		
		// test if one of the listed unit types is already encountered
		// if if's a number, test if we have encountered more than this
		for (std::vector<shared_string>::const_iterator i = needed_units.begin();
			 i != needed_units.end(); i++) {
			int needed_units_nb = lexical_cast_default<int>(*i,-1);
			if (needed_units_nb !=-1) {
				if (needed_units_nb <= static_cast<int>(seen_units.size())) {
					return tip;
				}
			} else if (seen_units.find(*i) != seen_units.end()) {
				return tip;
			}
		}
	}
	// not tip match, someone forget to put an always-match one
	return NULL;
}

/**
 *  Show one tip-of-the-day in a frame on the titlescreen.
 *  This frame has 2 buttons: Next-Tip, and Show-Help.
 */
static void draw_tip_of_day(CVideo& screen,
							config& tips_of_day,
							const gui::dialog_frame::style& style,
							const SDL_Rect* const main_dialog_area
							)
{
	if(preferences::show_tip_of_day() == false) {
		return;
	}
	
    // Restore the previous tip of day area to its old state (section of the title image).
    //tip_of_day_restorer.restore();
		
    // Draw tip of the day
    const config* tip = get_tip_of_day(tips_of_day);
    if(tip != NULL) {
    	int tip_width = main_dialog_area->w; //game_config::title_tip_width * screen.w() / 1024;
		
		try {
	        const std::string& text =
			font::word_wrap_text((*tip)["text"], font::SIZE_NORMAL, tip_width);
			const std::string& source =
			font::word_wrap_text((*tip)["source"], font::SIZE_NORMAL, tip_width);
			
			const int pad = game_config::title_tip_padding;
			
			SDL_Rect area = font::text_area(text,font::SIZE_NORMAL);
			area.w = tip_width;
			SDL_Rect source_area = font::text_area(source, font::SIZE_NORMAL, TTF_STYLE_ITALIC);
			area.w = std::max<size_t>(area.w, source_area.w) + 2*pad;
			//area.x = main_dialog_area->x; // - (game_config::title_tip_x * screen.w() / 1024) - area.w;
			//area.y = main_dialog_area->y + (main_dialog_area->h - area.h)/2;
			
			SDL_Rect total_area;
			total_area.w = area.w;
			total_area.h = area.h + source_area.h + 3*pad;
			total_area.x = main_dialog_area->x;
			total_area.y = main_dialog_area->y + (main_dialog_area->h - total_area.h)/2;

			source_area.y = total_area.y + area.h + pad;
			
						
			// Note: The buttons' locations need to be set before the dialog frame is drawn.
			// Otherwise, when the buttons restore their area, they
			// draw parts of the old dialog frame at their old locations.
			// This way, the buttons draw a part of the title image,
			// because the call to restore above restored the area
			// of the old tip of the day to its initial state (the title image).
//			int button_x = area.x + area.w - next_tip_button->location().w - pad;
//			int button_y = area.y + area.h - pad - next_tip_button->location().h;
//			next_tip_button->set_location(button_x, button_y);
//			next_tip_button->set_dirty(); //force redraw even if location did not change.
			
//			button_x -= previous_tip_button->location().w + pad;
//			previous_tip_button->set_location(button_x, button_y);
//			previous_tip_button->set_dirty();
			
//			button_x = area.x + pad;
//			if (help_tip_button != NULL)
//			{
//				help_tip_button->set_location(button_x, button_y);
//				help_tip_button->set_dirty();
//			}
			
			gui::dialog_frame tip_frame(screen, "", gui::dialog_frame::titlescreen_style, false);
			SDL_Rect borderRect = total_area;
			tip_frame.layout(borderRect);	
			tip_frame.draw_background();
			tip_frame.draw_border();
			
			
//			gui::dialog_frame f(screen, "", style, false);
//			tip_of_day_restorer = surface_restorer(&screen.video(), f.layout(area).exterior);
//			f.draw_background();
//			f.draw_border();
			
			font::draw_text(&screen, total_area, font::SIZE_NORMAL, font::NORMAL_COLOUR,
							text, total_area.x + pad, total_area.y + pad);

			font::draw_text(&screen, total_area, font::SIZE_NORMAL, font::NORMAL_COLOUR,
							source, total_area.x + total_area.w - source_area.w - pad,
							source_area.y,
							false, TTF_STYLE_ITALIC);
		} catch (utils::invalid_utf8_exception&) {
			LOG_STREAM(err, engine) << "Invalid utf-8 found, tips of day aren't drawn.\n";
			return;
		}
		
//	    LOG_DP << "drew tip of day\n";
	}
}

config tips_of_day;

loadscreen::loadscreen(CVideo &screen, const int &percent):
	filesystem_counter(0),
	setconfig_counter(0),
	parser_counter(0),
	disable_increments(false),
	screen_(screen),
	textarea_(),
	logo_surface_(NULL),
	logo_drawn_(false),
	pby_offset_(0),
	prcnt_(percent)
{
	int randNum = (rand() % 6) + 1;
	char num[3];
	sprintf(num, "%d", randNum);
	std::string file = "misc/loading";
	file.append(1, num[0]);
	file += ".pvrtc";
	std::string path = game_config::path + "/data/core/images/" + file; 
	
	int pvrtcSize = 512;
	GLsizei dataSize = (pvrtcSize * pvrtcSize * 4) / 8;
	FILE *fp = fopen(path.c_str(), "rb");
	if (!fp)
	{
		std::cerr << "\n\n*** ERROR loading loadscreen " << path.c_str() << "\n\n";
		return;
	}
	unsigned char *data = (unsigned char*) malloc(dataSize);
	int bytesRead = fread(data, 1, dataSize, fp);
	assert(bytesRead == dataSize);
	fclose(fp);
	
	glGenTextures(1, &logo_texture_);	
	glBindTexture(GL_TEXTURE_2D, logo_texture_);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG, pvrtcSize, pvrtcSize, 0, dataSize, data);
	GLenum err = glGetError();
	if (err != GL_NO_ERROR)
	{
		char buffer[512];
		sprintf(buffer, "\n\n*** ERROR uploading compressed texture %s:  glError: 0x%04X\n\n", path.c_str(), err);
		std::cerr << buffer;
	}
	
	free(data);
	
	textarea_.x = textarea_.y = textarea_.w = textarea_.h = 0;

	next_tip_of_day(tips_of_day);
}

void loadscreen::set_progress(const int percentage, const std::string &text, const bool commit)
{
	// Saturate percentage.
	prcnt_ = percentage < MIN_PERCENTAGE ? MIN_PERCENTAGE: percentage > MAX_PERCENTAGE ? MAX_PERCENTAGE: percentage;
	// Set progress bar parameters:
	int fcr =  21, fcg =  53, fcb =  80;		// RGB-values for finished piece.
	int lcr =  21, lcg =  22, lcb =  24;		// Leftover piece.
	//int bcr = 188, bcg = 176, bcb = 136;		// Border color.
	int bcr = 104, bcg = 74, bcb = 28;			// Border color.
	int bw = 1;								//< Border width.
	int bispw = 1;								//< Border inner spacing width.
	bw = 2*(bw+bispw) > screen_.getx() ? 0: 2*(bw+bispw) > screen_.gety() ? 0: bw;
	int scrx = screen_.getx() - 2*(bw+bispw);	//< Available width.
	int scry = screen_.gety() - 2*(bw+bispw);	//< Available height.
	int pbw = scrx/2;							//< Used width.
	int pbh = scry/16;							//< Used heigth.
	int	lightning_thickness = 2;

//	surface const gdis = screen_.getSurface();
	SDL_Rect area;
	
	// Clear the last text and draw new if text is provided.
//	if(text.length() > 0 && commit)
//	{
//		logo_drawn_ = false;
//	}
	
	bool updateFull = false;
	
	
	// Draw logo if it was succesfully loaded.
//	if (logo_surface_ && !logo_drawn_) 
	{
/*		
		area.x = (screen_.getx () - logo_surface_->w) / 2;
		area.y = ((scry - logo_surface_->h) / 2) - pbh;
		area.w = logo_surface_->w;
		area.h = logo_surface_->h;
		// Check if we have enough pixels to display it.
		if (area.x > 0 && area.y > 0) {
			pby_offset_ = (pbh + area.h)/2;
			SDL_BlitSurface (logo_surface_, 0, gdis, &area);
		} else {
			ERR_DISP << "loadscreen: Logo image is too big." << std::endl;
		}
*/
		area.x = 0;
		area.y = 0;
		//area.w = logo_surface_->w;
		//area.h = logo_surface_->h;
		//SDL_BlitSurface (logo_surface_, 0, gdis, &area);		
		//screen_.blit_surface(0, 0, logo_surface_);
		GLshort vertices[8];
		GLfloat texCoords[8];
		
		vertices[0] = 0;
		vertices[1] = 0;
		vertices[2] = 512;
		vertices[3] = 0;
		vertices[4] = 0;
		vertices[5] = 512;
		vertices[6] = 512;
		vertices[7] = 512;

		texCoords[0] = 0;
		texCoords[1] = 0;
		texCoords[2] = 1;
		texCoords[3] = 0;
		texCoords[4] = 0;
		texCoords[5] = 1;
		texCoords[6] = 1;
		texCoords[7] = 1;
		renderQueueAddTexture(vertices, texCoords, logo_texture_, 0xFFFFFFFF, 1.0);
		
		//	glVertexPointer(3, GL_SHORT, 0, vertices);
		//	glTexCoordPointer(2, GL_FLOAT, 0, texCoords);
		//	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		
		//SDL_RenderCopy(logo_texture_, NULL, NULL,DRAW);
		//SDL_RenderPresent();
		//SDL_UpdateRect(gdis, area.x, area.y, area.w, area.h);
	}
	
//	if (!logo_drawn_)
	{
		SDL_Rect tip_area = {242, 100, 225, 140};
		draw_tip_of_day(screen_, tips_of_day, gui::dialog_frame::titlescreen_style,&tip_area);

		logo_drawn_ = true;

		updateFull = true;
	}
	
	int pbx = (scrx - pbw)/2;					// Horizontal location.
	int pby = 276; //(scry - pbh)/2 + pby_offset_;		// Vertical location.

	// Draw top border.
	area.x = pbx; area.y = pby;
	area.w = pbw + 2*(bw+bispw); area.h = bw;
//	SDL_FillRect(gdis,&area,SDL_MapRGB(gdis->format,bcr,bcg,bcb));
	SDL_SetRenderDrawColor(bcr, bcg, bcb, 0xff);
	SDL_RenderFill(&area);
	// Draw bottom border.
	area.x = pbx; area.y = pby + pbh + bw + 2*bispw;
	area.w = pbw + 2*(bw+bispw); area.h = bw;
	//SDL_FillRect(gdis,&area,SDL_MapRGB(gdis->format,bcr,bcg,bcb));
	SDL_RenderFill(&area);
	// Draw left border.
	area.x = pbx; area.y = pby + bw;
	area.w = bw; area.h = pbh + 2*bispw;
	//SDL_FillRect(gdis,&area,SDL_MapRGB(gdis->format,bcr,bcg,bcb));
	SDL_RenderFill(&area);
	// Draw right border.
	area.x = pbx + pbw + bw + 2*bispw; area.y = pby + bw;
	area.w = bw; area.h = pbh + 2*bispw;
	//SDL_FillRect(gdis,&area,SDL_MapRGB(gdis->format,bcr,bcg,bcb));
	SDL_RenderFill(&area);
	// Draw the finished bar area.
	area.x = pbx + bw + bispw; area.y = pby + bw + bispw;
	area.w = (prcnt_ * pbw) / (MAX_PERCENTAGE - MIN_PERCENTAGE); area.h = pbh;
	//SDL_FillRect(gdis,&area,SDL_MapRGB(gdis->format,fcr,fcg,fcb));
	SDL_SetRenderDrawColor(fcr, fcg, fcb, 0xff);
	SDL_RenderFill(&area);

	SDL_Rect lightning = area;
	lightning.h = lightning_thickness;
	//we add 25% of white to the color of the bar to simulate a light effect
	//SDL_FillRect(gdis,&lightning,SDL_MapRGB(gdis->format,(fcr*3+255)/4,(fcg*3+255)/4,(fcb*3+255)/4));
	SDL_SetRenderDrawColor((fcr*3+255)/4,(fcg*3+255)/4,(fcb*3+255)/4, 0xff);
	SDL_RenderFill(&lightning);
	lightning.y = area.y+area.h-lightning.h;
	//remove 50% of color to simulate a shadow effect
	//SDL_FillRect(gdis,&lightning,SDL_MapRGB(gdis->format,fcr/2,fcg/2,fcb/2));
	SDL_SetRenderDrawColor(fcr/2,fcg/2,fcb/2, 0xff);
	SDL_RenderFill(&lightning);

	// Draw the leftover bar area.
	area.x = pbx + bw + bispw + (prcnt_ * pbw) / (MAX_PERCENTAGE - MIN_PERCENTAGE); area.y = pby + bw + bispw;
	area.w = ((MAX_PERCENTAGE - prcnt_) * pbw) / (MAX_PERCENTAGE - MIN_PERCENTAGE); area.h = pbh;
	//SDL_FillRect(gdis,&area,SDL_MapRGB(gdis->format,lcr,lcg,lcb));
	SDL_SetRenderDrawColor(lcr, lcg, lcb, 0xff);
	SDL_RenderFill(&area);

	// Clear the last text and draw new if text is provided.
/*	if(text.length() > 0 && commit)
	{
		SDL_Rect oldarea = textarea_;
//		SDL_FillRect(gdis,&textarea_,SDL_MapRGB(gdis->format,0,0,0));
		textarea_ = font::line_size(text, font::SIZE_NORMAL);
		textarea_.x = scrx/2 + bw + bispw - textarea_.w / 2;
		textarea_.y = pby + pbh + 4*(bw + bispw);
		//draw_solid_tinted_rectangle(textarea_.x-4, textarea_.y-2, textarea_.w + 8, textarea_.h + 4,0,0,0,0.50,gdis);
		SDL_Rect tintRect = {textarea_.x-4, textarea_.y-2, textarea_.w + 8, textarea_.h + 4};
		SDL_SetRenderDrawColor(0, 0, 0, 0x80);
		SDL_RenderFill(&tintRect);
		textarea_ = font::draw_text(&screen_,textarea_,font::SIZE_NORMAL,font::NORMAL_COLOUR,text,textarea_.x,textarea_.y);
		oldarea.x = std::min<int>(textarea_.x, oldarea.x);
		oldarea.y = std::min<int>(textarea_.y, oldarea.y);
		oldarea.w = std::max<int>(textarea_.w, oldarea.w);
		oldarea.h = std::max<int>(textarea_.h, oldarea.h);
//		SDL_UpdateRect(gdis, oldarea.x, oldarea.y, oldarea.w, oldarea.h);
	}
*/ 
	// Update the rectangle if needed
	if(commit)
	{
//		SDL_UpdateRect(gdis, pbx, pby, pbw + 2*(bw + bispw), pbh + 2*(bw + bispw));
	}
	
//	if (updateFull)
//	{
//		SDL_UpdateRect(gdis, 0, 0, 480, 320);
//	}
//	else if (commit)
//	{
//		SDL_UpdateRect(gdis, 116, 274, 251, 47);
//	}
	
	SDL_RenderPresent();
}


void loadscreen::increment_progress(const int percentage, const std::string &text, const bool commit) {
	set_progress(prcnt_ + percentage, text, commit);
}

void loadscreen::clear_screen(const bool commit)
{
	int scrx = screen_.getx();					//< Screen width.
	int scry = screen_.gety();					//< Screen height.
	SDL_Rect area = {0, 0, scrx, scry};		// Screen area.
//	surface const disp(screen_.getSurface());	// Screen surface.
	// Make everything black.
//	SDL_FillRect(disp,&area,SDL_MapRGB(disp->format,0,0,0));
	SDL_SetRenderDrawColor(0, 0, 0, 0xff);
	SDL_RenderFill(&area);
	if(commit)
	{
		//SDL_Flip(disp);						// Flip the double buffering.
		SDL_RenderPresent();
	} 
}
loadscreen *loadscreen::global_loadscreen = 0;

void loadscreen::dump_counters() const
{
	std::cerr << "loadscreen: filesystem counter = " << filesystem_counter << '\n';
	std::cerr << "loadscreen: setconfig counter = "  << setconfig_counter  << '\n';
	std::cerr << "loadscreen: parser counter = "     << parser_counter     << '\n';
}


 // Amount of work to expect during the startup-stages,
 // for scaling the progressbars:
 #define CALLS_TO_FILESYSTEM 112
 #define PRCNT_BY_FILESYSTEM  20
 #define CALLS_TO_SETCONFIG   45
 #define PRCNT_BY_SETCONFIG   30
 #define CALLS_TO_PARSER   36212
 #define PRCNT_BY_PARSER      20
 
void increment_filesystem_progress () {
	unsigned newpct, oldpct;
	// Only do something if the variable is filled in.
	// I am assuming non parallel access here!
	if (loadscreen::global_loadscreen != 0) {
		if (loadscreen::global_loadscreen->disable_increments)
			return;

		if (loadscreen::global_loadscreen->filesystem_counter == 0) {
			loadscreen::global_loadscreen->increment_progress(0, _("Verifying cache"));
		}
		oldpct = (PRCNT_BY_FILESYSTEM * loadscreen::global_loadscreen->filesystem_counter) / CALLS_TO_FILESYSTEM;
		newpct = (PRCNT_BY_FILESYSTEM * ++(loadscreen::global_loadscreen->filesystem_counter)) / CALLS_TO_FILESYSTEM;
		//std::cerr << "Calls " << num;
		if(oldpct != newpct) {
			//std::cerr << " percent " << newpct;
			loadscreen::global_loadscreen->increment_progress(newpct - oldpct);
		}
		//std::cerr << std::endl;
	}
}

void increment_set_config_progress () {
	unsigned newpct, oldpct;
	// Only do something if the variable is filled in.
	// I am assuming non parallel access here!
	if (loadscreen::global_loadscreen != 0) {
		if (loadscreen::global_loadscreen->disable_increments)
			return;

		if (loadscreen::global_loadscreen->setconfig_counter == 0) {
			loadscreen::global_loadscreen->increment_progress(0, _("Reading unit files."));
		}
		oldpct = (PRCNT_BY_SETCONFIG * loadscreen::global_loadscreen->setconfig_counter) / CALLS_TO_SETCONFIG;
		newpct = (PRCNT_BY_SETCONFIG * ++(loadscreen::global_loadscreen->setconfig_counter)) / CALLS_TO_SETCONFIG;
		//std::cerr << "Calls " << num;
		if(oldpct != newpct) {
			//std::cerr << " percent " << newpct;
			loadscreen::global_loadscreen->increment_progress(newpct - oldpct);
		}
		//std::cerr << std::endl;
	}
}

void increment_parser_progress () {
	unsigned newpct, oldpct;
	// Only do something if the variable is filled in.
	// I am assuming non parallel access here!
	if (loadscreen::global_loadscreen != 0) {
		if (loadscreen::global_loadscreen->disable_increments)
			return;
		if (loadscreen::global_loadscreen->parser_counter == 0) {
			loadscreen::global_loadscreen->increment_progress(0, _("Reading files and creating cache."));
		}
		oldpct = (PRCNT_BY_PARSER * loadscreen::global_loadscreen->parser_counter) / CALLS_TO_PARSER;
		newpct = (PRCNT_BY_PARSER * ++(loadscreen::global_loadscreen->parser_counter)) / CALLS_TO_PARSER;
		//std::cerr << "Calls " << loadscreen::global_loadscreen->parser_counter;
		if(oldpct != newpct) {
		//	std::cerr << " percent " << newpct;
			loadscreen::global_loadscreen->increment_progress(newpct - oldpct, _("Reading files and creating cache."));
		}
		//std::cerr << std::endl;
	}
}

