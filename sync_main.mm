//
//  sync_main.mm
//  wesnoth
//
//  Created by Kyle Poole on 5/10/10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "sync_main.h"
#import "sync_upload.h"
#import "sync_download.h"

@implementation sync_main

extern UIView *gLandscapeView;
extern bool gPauseForOpenFeint;
sync_upload *syncUploadViewController;
sync_download *syncDownloadViewController;


/*
 // The designated initializer.  Override if you create the controller programmatically and want to perform customization that is not appropriate for viewDidLoad.
- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil {
    if ((self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil])) {
        // Custom initialization
    }
    return self;
}
*/

/*
// Implement viewDidLoad to do additional setup after loading the view, typically from a nib.
- (void)viewDidLoad {
    [super viewDidLoad];
}
*/

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

-(IBAction)onUpload:(id)sender
{
	[self.view removeFromSuperview];
	
	syncUploadViewController = [[sync_upload alloc] initWithNibName:@"sync_upload" bundle:nil];
	
	CGRect frameRect = syncUploadViewController.view.frame;
#ifdef __IPAD__
	frameRect.origin.x = (1024-480)/2;
	frameRect.origin.y = (768-320)/2;
#else
	frameRect.origin.x = 0;
	frameRect.origin.y = 0;
#endif
	syncUploadViewController.view.frame = frameRect;
	[gLandscapeView addSubview:syncUploadViewController.view];	
}

-(IBAction)onDownload:(id)sender
{
	[self.view removeFromSuperview];
	
	syncDownloadViewController = [[sync_download alloc] initWithNibName:@"sync_download" bundle:nil];
	
	CGRect frameRect = syncDownloadViewController.view.frame;
#ifdef __IPAD__
	frameRect.origin.x = (1024-480)/2;
	frameRect.origin.y = (768-320)/2;
#else
	frameRect.origin.x = 0;
	frameRect.origin.y = 0;
#endif
	syncDownloadViewController.view.frame = frameRect;
	[gLandscapeView addSubview:syncDownloadViewController.view];	
}

-(IBAction)onDone:(id)sender
{
	[self.view removeFromSuperview];
	gLandscapeView.hidden = YES;
	gPauseForOpenFeint = false;	
}


@end
