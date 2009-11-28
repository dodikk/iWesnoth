/*
 SDL - Simple DirectMedia Layer
 Copyright (C) 1997-2009 Sam Lantinga
 
 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.
 
 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.
 
 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 
 Sam Lantinga
 slouken@libsdl.org
*/

#import "SDL_uikitappdelegate.h"
#import "SDL_uikitopenglview.h"
#import "SDL_events_c.h"
#import "jumphack.h"

#import "FlurryAPI.h"


#ifdef main
#undef main
#endif

extern int SDL_main(int argc, char *argv[]);
static int forward_argc;
static char **forward_argv;

#define VALGRIND_PATH "/usr/local/bin/valgrind"

int main(int argc, char **argv) {
#ifdef VALGRIND
	if (argc < 2 || (argc >= 2 && strcmp(argv[1], "-valgrind") != 0)) 
	{
		// memory access checking
		execl(VALGRIND_PATH, VALGRIND_PATH, "--gen-suppressions=all", argv[0], "-valgrind", NULL);
		
		// memory profiling
		//execl(VALGRIND_PATH, VALGRIND_PATH, "--tool=massif", "--massif-out-file=/Users/kyle/wesnoth.massif", argv[0], "-valgrind", NULL);
    }
#endif
	
	int i;
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	
	/* store arguments */
	forward_argc = argc;
	forward_argv = (char **)malloc(argc * sizeof(char *));
	for (i=0; i<argc; i++) {
		forward_argv[i] = malloc( (strlen(argv[i])+1) * sizeof(char));
		strcpy(forward_argv[i], argv[i]);
	}

	/* Give over control to run loop, SDLUIKitDelegate will handle most things from here */
	int retVal = UIApplicationMain(argc, argv, NULL, @"SDLUIKitDelegate");
	
	[pool release];
	return retVal;
}

@implementation SDLUIKitDelegate

@synthesize window;

/* convenience method */
+(SDLUIKitDelegate *)sharedAppDelegate {
	/* the delegate is set in UIApplicationMain(), which is garaunteed to be called before this method */
	return (SDLUIKitDelegate *)[[UIApplication sharedApplication] delegate];
}

- (id)init {
	self = [super init];
	window = nil;
	return self;
}

- (void)runSDLMain:(NSString *)aStr
{
	/* run the user's application, passing argc and argv */
	int exit_status = SDL_main(forward_argc, forward_argv);
	
	/* free the memory we used to hold copies of argc and argv */
	int i;
	for (i=0; i<forward_argc; i++) {
		free(forward_argv[i]);
	}
	free(forward_argv);	
	
	/* exit, passing the return status from the user's application */
	exit(exit_status);	
}

void uncaughtExceptionHandler(NSException *exception) {
    [FlurryAPI logError:@"Uncaught" message:@"Crash!" exception:exception];
}

- (void)applicationDidFinishLaunching:(UIApplication *)application {
	
	[[UIApplication sharedApplication] setStatusBarOrientation: UIInterfaceOrientationLandscapeRight];
	
	/* Set working directory to resource path */
	[[NSFileManager defaultManager] changeCurrentDirectoryPath: [[NSBundle mainBundle] resourcePath]];
	
	// run the user's application, passing argc and argv 
	/*
	int exit_status = SDL_main(forward_argc, forward_argv);
	
	// free the memory we used to hold copies of argc and argv
	int i;
	for (i=0; i<forward_argc; i++) {
		free(forward_argv[i]);
	}
	free(forward_argv);	
		
	// exit, passing the return status from the user's application
	exit(exit_status);
	*/	
	
	NSSetUncaughtExceptionHandler(&uncaughtExceptionHandler);
#ifdef FREE_VERSION
	[FlurryAPI startSession:@"75UDWZTNGLKTKLNQ8YPG"];
#else
	[FlurryAPI startSession:@"ZNJRJHD1DYT8MJV4VME9"];
#endif
	
	// KP: using a selector gets around the "failed to launch application in time" if the startup code takes too long
	// This is easy to see if running with Valgrind
	[self performSelector:@selector(runSDLMain:) withObject:@"" afterDelay: 0.1f];

}

- (void)applicationWillTerminate:(UIApplication *)application {
	
	SDL_SendQuit();
	 /* hack to prevent automatic termination.  See SDL_uikitevents.m for details */
	longjmp(*(jump_env()), 1);
	
}

- (void)applicationDidReceiveMemoryWarning:(UIApplication *)application  
{	
//	UIAlertView *alert = [[UIAlertView alloc] initWithTitle:@"Low Memory" message:@"Detected low memory warning! Try resetting device to free more memory. Game will now exit..." delegate:nil cancelButtonTitle:@"Ok" otherButtonTitles:nil];
//	[alert show];
//	[alert release];
}  

-(void)dealloc {
	[window release];
	[super dealloc];
}

@end
