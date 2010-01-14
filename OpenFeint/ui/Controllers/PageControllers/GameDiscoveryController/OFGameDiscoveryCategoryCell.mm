////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// 
///  Copyright 2009 Aurora Feint, Inc.
/// 
///  Licensed under the Apache License, Version 2.0 (the "License");
///  you may not use this file except in compliance with the License.
///  You may obtain a copy of the License at
///  
///  	http://www.apache.org/licenses/LICENSE-2.0
///  	
///  Unless required by applicable law or agreed to in writing, software
///  distributed under the License is distributed on an "AS IS" BASIS,
///  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
///  See the License for the specific language governing permissions and
///  limitations under the License.
/// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#import "OFDependencies.h"
#import "OFGameDiscoveryCategoryCell.h"
#import "OFGameDiscoveryCategory.h"
#import "OFImageView.h"
#import "OpenFeint+Private.h"
#import "OFImageLoader.h"
#import "OFTableCellBackgroundView.h"

@implementation OFGameDiscoveryCategoryCell

@synthesize iconView, nameLabel, subtextLabel, secondaryTextLabel, topDividerView, bottomDividerView;

- (void)onResourceChanged:(OFResource*)resource
{
	OFGameDiscoveryCategory* category = (OFGameDiscoveryCategory*)resource;

	self.iconView.unframed = YES;
	self.iconView.imageUrl = category.iconUrl;
	self.iconView.shouldScaleImageToFillRect = NO;
	self.nameLabel.text = category.name;
	self.subtextLabel.text = category.subtext;
	self.secondaryTextLabel.text = category.secondaryText;
}

- (void)dealloc
{
	self.iconView = nil;
	self.nameLabel = nil;
	self.subtextLabel = nil;
	self.secondaryTextLabel = nil;
	self.bottomDividerView = nil;
	self.topDividerView = nil;
	
	[super dealloc];
}

- (BOOL)wantsToConfigureSelf
{
	return YES;
}

- (void)configureSelfAsLeading:(BOOL)_isLeading asTrailing:(BOOL)_isTrailing asOdd:(BOOL)_isOdd
{
	if(![OpenFeint isInLandscapeMode])
	{
		NSLog(@"Portrait is not yet supported for OFGameDiscoveryCategory cells.");
	}

	OFTableCellBackgroundView* background = (OFTableCellBackgroundView*)self.backgroundView;
	OFTableCellBackgroundView* selectedBackground = (OFTableCellBackgroundView*)self.selectedBackgroundView;
	if (![background isKindOfClass:[OFTableCellBackgroundView class]])
	{
		self.backgroundView = background = [OFTableCellBackgroundView defaultBackgroundView];
		self.selectedBackgroundView = selectedBackground = [OFTableCellBackgroundView defaultBackgroundView];
	}

	background.backgroundColor = [UIColor colorWithPatternImage:[OFImageLoader loadImage:@"OFGameDiscoveryGreyBG.png"]];
	selectedBackground.backgroundColor = [UIColor colorWithPatternImage:[OFImageLoader loadImage:@"OFGameDiscoveryGreyBGSelected.png"]];
				
	self.topDividerView.backgroundColor = [UIColor colorWithPatternImage:[OFImageLoader loadImage:@"OFGameDiscoveryDividerTop.png"]];
	self.bottomDividerView.backgroundColor = [UIColor colorWithPatternImage:[OFImageLoader loadImage:@"OFGameDiscoveryDividerBottom.png"]];
}

@end
