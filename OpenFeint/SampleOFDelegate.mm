#import "SampleOFDelegate.h"
#import "OpenFeint+UserOptions.h"

extern bool gPauseForOpenFeint;

bool gPauseForOpenFeint = false;

@implementation SampleOFDelegate

- (void)dashboardWillAppear
{
	gPauseForOpenFeint = true;
}

- (void)dashboardDidAppear
{
}

- (void)dashboardWillDisappear
{
}

- (void)dashboardDidDisappear
{
	gPauseForOpenFeint = false;
}

- (void)userLoggedIn:(NSString*)userId
{
	OFLog(@"New user logged in! Hello %@", [OpenFeint lastLoggedInUserName]);
}

- (BOOL)showCustomOpenFeintApprovalScreen
{
	return NO;
}

@end
