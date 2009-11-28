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

#ifndef ISOUND_H_INCLUDED
#define ISOUND_H_INCLUDED

#include <vector>
#include <string>

#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h> 


class iSound;
@interface iSoundDelegate : NSObject <AVAudioPlayerDelegate>
{
	iSound *mParent;
}
@property(nonatomic) iSound *mParent;

@end


class iSound
{
public:
	static const int CONTINUOUS_LOOPING = -1;
	
	iSound();
	~iSound();
	void stopAllSounds();
	void playMusic(const std::string& filename, int loopCount=1);
	void setMusicVolume(int volume);
	void setVolume(int channel, int volume);
	void playSound(const std::string& filename, int channel);
	void stopMusic();
	void stopSound(int channel);
	void finishedPlaying(AVAudioPlayer *player);
	bool isMusicPlaying();

private:
	void checkChannelSize(int channels);
	
	int mVolumeMusic;
	AVAudioPlayer *mPlayingMusic;
	
	std::vector<int> mVolumeSound;						// volume for each channel
	std::vector<AVAudioPlayer*> mPlayingSound;			// currently playing sound on each channel

	bool mOtherAudioIsPlaying;						// is the user playing iPod music already?
	iSoundDelegate *mSoundDelegate;
};


#endif
