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

#include "iSound.h"
#include <pthread.h>
#import <AudioToolbox/AudioServices.h>

pthread_mutex_t mutex;




@implementation iSoundDelegate
@synthesize mParent;

- (void) audioPlayerDidFinishPlaying:(AVAudioPlayer *)player successfully:(BOOL) flag
{
		// TODO: do some sort of callback with player ptr
}

@end


iSound::iSound()
{
	AudioSessionInitialize(NULL,NULL,NULL,NULL);
	
	UInt32		propertySize, audioIsAlreadyPlaying;
	
	// do not open the track if the audio hardware is already in use (could be the iPod app playing music)
	propertySize = sizeof(UInt32);
	AudioSessionGetProperty(kAudioSessionProperty_OtherAudioIsPlaying, &propertySize, &audioIsAlreadyPlaying);
	if (audioIsAlreadyPlaying != 0)
	{
		mOtherAudioIsPlaying = YES;
		//NSLog(@"other audio is playing");
		
		UInt32	sessionCategory = kAudioSessionCategory_AmbientSound;
		AudioSessionSetProperty(kAudioSessionProperty_AudioCategory, sizeof(sessionCategory), &sessionCategory);
		AudioSessionSetActive(YES);
	}
	else
	{
		//NSLog(@"no other audio is playing ...");
		
		mOtherAudioIsPlaying = NO;
		
		// since no other audio is *supposedly* playing, then we will make darn sure by changing the audio session category temporarily
		// to kick any system remnants out of hardware (iTunes (or the iPod App, or whatever you wanna call it) sticks around)
		UInt32	sessionCategory = kAudioSessionCategory_MediaPlayback;
		AudioSessionSetProperty(kAudioSessionProperty_AudioCategory, sizeof(sessionCategory), &sessionCategory);
		AudioSessionSetActive(YES);
		
		// now change back to ambient session category so our app honors the "silent switch"
		sessionCategory = kAudioSessionCategory_AmbientSound;
		AudioSessionSetProperty(kAudioSessionProperty_AudioCategory, sizeof(sessionCategory), &sessionCategory);
	}
		
	pthread_mutex_init(&mutex, NULL);
	
	mVolumeMusic = 100;
	
	mSoundDelegate = [[iSoundDelegate alloc] init];
	mSoundDelegate.mParent = this;
	
	mPlayingMusic = nil;
}

iSound::~iSound()
{	
	AVAudioPlayer *player;
	
	if (mPlayingMusic != nil)
	{
		player = mPlayingMusic;
		[player stop];
		[player release];
		mPlayingMusic = nil;
	}
	
	for (int i=0; i < mPlayingSound.size(); i++)
	{
		if (mPlayingSound[i] != nil)
		{
			player = mPlayingSound[i];
			[player stop];
			[player release];
			mPlayingSound[i] = nil;
		}
	}
	
	[mSoundDelegate dealloc];

	pthread_mutex_destroy(&mutex);
}

void iSound::checkChannelSize(int channels)
{
	pthread_mutex_lock(&mutex);
	if (mPlayingSound.size() < channels+1)
	{
		mPlayingSound.resize(channels+1);
		mVolumeSound.resize(channels+1, 100);
	}
	pthread_mutex_unlock(&mutex);
}

void iSound::setVolume(int channel, int volume)
{
	checkChannelSize(channel);
	
	pthread_mutex_lock(&mutex);
	mVolumeSound[channel] = volume;
	if (mPlayingSound[channel] != nil)
	{
		mPlayingSound[channel].volume = ((float)mVolumeSound[channel])/100;
	}
	pthread_mutex_unlock(&mutex);
}

bool iSound::isMusicPlaying()
{
	pthread_mutex_lock(&mutex);
	bool result;
	
	if (mPlayingMusic == nil)
		result = false;
	else if ([mPlayingMusic isPlaying])
		result = true;
	else
		result = false;

	pthread_mutex_unlock(&mutex);
	return result;
}

void iSound::setMusicVolume(int volume)
{
	pthread_mutex_lock(&mutex);
	mVolumeMusic = volume;
	if (mPlayingMusic != nil)
		mPlayingMusic.volume = ((float)mVolumeMusic)/100;
	pthread_mutex_unlock(&mutex);
}

void iSound::stopAllSounds() 
{
	stopMusic();
	for (int i=0; i < mPlayingSound.size(); i++)
	{
		stopSound(i);
	}
}

void iSound::stopSound(int channel)
{
	pthread_mutex_lock(&mutex);
	if (mPlayingSound[channel] != nil)
	{
		if ([mPlayingSound[channel] isPlaying])
			[mPlayingSound[channel] stop];
		[mPlayingSound[channel] release];
		mPlayingSound[channel] = nil;
	}
	pthread_mutex_unlock(&mutex);
}

void iSound::stopMusic()
{
	pthread_mutex_lock(&mutex);
	
	if (mPlayingMusic != nil)
	{
		if ([mPlayingMusic isPlaying])
			[mPlayingMusic stop];
		[mPlayingMusic release];
		mPlayingMusic = nil;
	}
		
	pthread_mutex_unlock(&mutex);
}



void iSound::playMusic(const std::string& filename, int loopCount) 
{
	if (mOtherAudioIsPlaying)
		return;

	pthread_mutex_lock(&mutex);
	
	if (mPlayingMusic != nil)
	{
		if ([mPlayingMusic isPlaying])
			[mPlayingMusic stop];
		[mPlayingMusic release];
		mPlayingMusic = nil;
	}
	
	NSString *soundFilePath;
	
	std::string newFilename;
	int extPos = filename.rfind('.');
	newFilename = filename.substr(0, extPos);
	newFilename += ".mp3";
	
	soundFilePath = [NSString stringWithCString:newFilename.c_str()];
	
	
	NSURL *fileURL = [[NSURL alloc] initFileURLWithPath: soundFilePath];
	
	
	NSError *err; 
	AVAudioPlayer *newPlayer =[[AVAudioPlayer alloc] initWithContentsOfURL: fileURL error: &err];
	
	
	[fileURL release];

	if (err == nil)
	{		
		mPlayingMusic = newPlayer;
		
		mPlayingMusic.numberOfLoops = loopCount;
		mPlayingMusic.volume = ((float)mVolumeMusic)/100;
		
		[mPlayingMusic prepareToPlay];
		[mPlayingMusic play];
	}
	else
	{
		NSLog(@"Error loading music: %@", [err localizedDescription] );
	}
	
	pthread_mutex_unlock(&mutex);
}

void iSound::playSound(const std::string& filename, int channel) 
{
	checkChannelSize(channel);
	
	pthread_mutex_lock(&mutex);
	
	if (mPlayingSound[channel] != nil)
	{
		if ([mPlayingSound[channel] isPlaying])
			[mPlayingSound[channel] stop];
		[mPlayingSound[channel] release];
		mPlayingSound[channel] = nil;
	}
	
	NSString *soundFilePath;
	
	std::string newFilename;
	int extPos = filename.rfind('.');
	newFilename = filename.substr(0, extPos);
	newFilename += ".caf";							// sound files are stored as .caf, a compressed 'ima4' format
													// because hardware cannot decode multiple mp3 files at once
	
	soundFilePath = [NSString stringWithCString:newFilename.c_str()];
	
	
	NSURL *fileURL = [[NSURL alloc] initFileURLWithPath: soundFilePath];
	
	
	NSError *err; 
	AVAudioPlayer *newPlayer =[[AVAudioPlayer alloc] initWithContentsOfURL: fileURL error: &err];
	
	
	[fileURL release];

	if (err == nil)
	{
		mPlayingSound[channel] = newPlayer;
		
		mPlayingSound[channel].volume = ((float)mVolumeSound[channel])/100;
		mPlayingSound[channel].numberOfLoops = 0;
		
		[mPlayingSound[channel] prepareToPlay];
		[mPlayingSound[channel] play];
	}
	
	pthread_mutex_unlock(&mutex);
	
}

void iSound::finishedPlaying(AVAudioPlayer *player)
{
/*	
	int curSound = -1;
	for (int i=0; i < NUM_SOUNDS; i++)
	{
		if (player == mSounds[i])
		{
			curSound = i;
			break;
		}
	}
	
	pthread_mutex_lock(&mutex);
	if (curSound != soundIndex)
	{
		[mSounds[soundIndex] play];
	}
	pthread_mutex_unlock(&mutex);
*/ 
}



