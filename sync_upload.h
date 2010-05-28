//
//  sync_upload.h
//  wesnoth
//
//  Created by Kyle Poole on 5/10/10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>


@interface sync_upload : UIViewController <UITableViewDelegate,UITableViewDataSource> {
	IBOutlet UIActivityIndicatorView *activity;
}

@property (nonatomic,retain) IBOutlet UIActivityIndicatorView *activity;

-(IBAction)onDone:(id)sender;

@end
