//
//  sync_login.mm
//  wesnoth
//
//  Created by Kyle Poole on 5/10/10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "sync_login.h"
#include "game_preferences.hpp"

#include <vector>
#include "string_utils.hpp"
#include "sync_main.h"
#include "filesystem.hpp"

@implementation sync_login

@synthesize email, password, activity, loginButton, cancelButton;

extern UIView *gLandscapeView;
extern bool gPauseForOpenFeint;

extern std::vector<std::string> gServerFiles;
std::vector<std::string> gServerFiles;
extern std::vector<std::string> gLocalFiles;
std::vector<std::string> gLocalFiles;
extern sync_main *syncMainViewController;
sync_main *syncMainViewController;


/*
 // The designated initializer.  Override if you create the controller programmatically and want to perform customization that is not appropriate for viewDidLoad.
- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil {
    if ((self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil])) {
        // Custom initialization
    }
    return self;
}
*/


// Implement viewDidLoad to do additional setup after loading the view, typically from a nib.
- (void)viewDidLoad {
    [super viewDidLoad];
	email.text = [NSString stringWithUTF8String: preferences::sync_login_str().c_str()];
	password.text = [NSString stringWithUTF8String:preferences::sync_password().c_str()];
}



/*
// Override to allow orientations other than the default portrait orientation.
- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
    // Return YES for supported orientations
    return (interfaceOrientation == UIInterfaceOrientationLandscapeLeft || interfaceOrientation == UIInterfaceOrientationLandscapeRight);
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

- (void) doLogin
{
	NSAutoreleasePool *pool = [ [ NSAutoreleasePool alloc ] init ];
	
	preferences::set_sync_login([email.text UTF8String]);
	preferences::set_sync_password([password.text UTF8String]);
	
	NSString *post = [NSString stringWithFormat:@"username=%@&password=%@", email.text, password.text]; 
	NSData *postData = [post dataUsingEncoding:NSASCIIStringEncoding allowLossyConversion:YES];  
	
	NSString *postLength = [NSString stringWithFormat:@"%d", [postData length]];  
	
	NSMutableURLRequest *request = [[[NSMutableURLRequest alloc] init] autorelease];  
	[request setURL:[NSURL URLWithString:@"http://www.wesnothsync.com/index.php?mobile=1"]];  
	[request setHTTPMethod:@"POST"];  
	[request setValue:postLength forHTTPHeaderField:@"Content-Length"];  
	[request setValue:@"application/x-www-form-urlencoded" forHTTPHeaderField:@"Content-Type"];  
	[request setHTTPBody:postData];  
	
	NSData *returnData = [NSURLConnection sendSynchronousRequest:request returningResponse:nil error:nil];
	NSString *returnString = [[NSString alloc] initWithData:returnData encoding:NSUTF8StringEncoding];
	NSLog(@"%@", returnString);
	
	std::string returnStr = [returnString UTF8String];
	
	[activity stopAnimating];
	loginButton.enabled = YES;
	cancelButton.enabled = YES;

	
	if (returnStr.size() == 0)
	{
		UIAlertView *alert = [[UIAlertView alloc] initWithTitle:@"Connection Failed" message:@"Please check your connection and try again." delegate:nil cancelButtonTitle:@"OK" otherButtonTitles:nil, nil];
		[alert show];
		[alert release];		
	}
	else if (returnStr.find("Error") == 0)
	{
		UIAlertView *alert = [[UIAlertView alloc] initWithTitle:@"Server Response" message:returnString delegate:nil cancelButtonTitle:@"OK" otherButtonTitles:nil, nil];
		[alert show];
		[alert release];
	}
	else
	{
		gServerFiles = utils::split(returnStr, '\n');
		
		std::vector<shared_string> localFiles;
		get_files_in_dir(get_saves_dir(), &localFiles);
		gLocalFiles.clear();
		gLocalFiles.push_back(get_saves_dir() + "/");
		for(int i=0; i < localFiles.size(); i++)
		{
			gLocalFiles.push_back(localFiles[i]);
		}
		
		[self.view removeFromSuperview];
		
		syncMainViewController = [[sync_main alloc] initWithNibName:@"sync_main" bundle:nil];
		
		CGRect frameRect = syncMainViewController.view.frame;
#ifdef __IPAD__
		frameRect.origin.x = (1024-480)/2;
		frameRect.origin.y = (768-320)/2;
#else
		frameRect.origin.x = 0;
		frameRect.origin.y = 0;
#endif
		syncMainViewController.view.frame = frameRect;
		[gLandscapeView addSubview:syncMainViewController.view];	
	}
	
	[pool release];
}

-(IBAction)login:(id)sender
{
	[activity startAnimating];
	loginButton.enabled = NO;
	cancelButton.enabled = NO;

	[NSThread detachNewThreadSelector: @selector(doLogin) toTarget:self withObject:nil];
	
}

-(IBAction)cancel:(id)sender
{
	[self.view removeFromSuperview];
	gLandscapeView.hidden = YES;
	gPauseForOpenFeint = false;
}

-(IBAction)emailNext:(id)sender
{
	[email resignFirstResponder];
	[password becomeFirstResponder];
}

-(IBAction)hideEmailKeyboard:(id)sender
{
	[email resignFirstResponder];
}

-(IBAction)hidePasswordKeyboard:(id)sender
{
	[password resignFirstResponder];
}

- (BOOL)textFieldShouldReturn:(UITextField *)textField
{
	[textField resignFirstResponder];
	
	if ([textField isEqual:email])
	{
		[password becomeFirstResponder];
	}
	return YES;
}

@end
