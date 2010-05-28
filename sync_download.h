//
//  sync_download.h
//  wesnoth
//
//  Created by Kyle Poole on 5/10/10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>


@interface sync_download : UIViewController <UITableViewDelegate,UITableViewDataSource> {
	IBOutlet UIActivityIndicatorView *activity;
	IBOutlet UITableView *table;
	IBOutlet UIBarButtonItem *barButton;
}

@property (nonatomic,retain) IBOutlet UIActivityIndicatorView *activity;
@property (nonatomic,retain) IBOutlet UITableView *table;
@property (nonatomic,retain) IBOutlet UIBarButtonItem *barButton;

-(IBAction)onDone:(id)sender;


@end
