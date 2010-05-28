//
//  sync_upload.mm
//  wesnoth
//
//  Created by Kyle Poole on 5/10/10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "sync_upload.h"
#import "sync_main.h"

@implementation sync_upload

@synthesize activity;

extern sync_main *syncMainViewController;
extern UIView *gLandscapeView;
extern std::vector<std::string> gLocalFiles;
bool gIsUploading = false;
int gUploadNum;

#pragma mark -
#pragma mark View lifecycle

/*
- (void)viewDidLoad {
    [super viewDidLoad];

    // Uncomment the following line to preserve selection between presentations.
    self.clearsSelectionOnViewWillAppear = NO;
 
    // Uncomment the following line to display an Edit button in the navigation bar for this view controller.
    // self.navigationItem.rightBarButtonItem = self.editButtonItem;
}
*/

/*
- (void)viewWillAppear:(BOOL)animated {
    [super viewWillAppear:animated];
}
*/
/*
- (void)viewDidAppear:(BOOL)animated {
    [super viewDidAppear:animated];
}
*/
/*
- (void)viewWillDisappear:(BOOL)animated {
    [super viewWillDisappear:animated];
}
*/
/*
- (void)viewDidDisappear:(BOOL)animated {
    [super viewDidDisappear:animated];
}
*/
/*
// Override to allow orientations other than the default portrait orientation.
- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
    // Return YES for supported orientations
    return (interfaceOrientation == UIInterfaceOrientationPortrait);
}
*/


#pragma mark -
#pragma mark Table view data source

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView {
    // Return the number of sections.
    return 1;
}


- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
    // Return the number of rows in the section.
   return (gLocalFiles.size() - 1);
}


// Customize the appearance of table view cells.
- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {
    
    static NSString *CellIdentifier = @"Cell";
    
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:CellIdentifier];
    if (cell == nil) {
        cell = [[[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:CellIdentifier] autorelease];
    }
    
    // Configure the cell...
	cell.textLabel.text = [NSString stringWithUTF8String:gLocalFiles[indexPath.row + 1].c_str()];
    
    return cell;
}


/*
// Override to support conditional editing of the table view.
- (BOOL)tableView:(UITableView *)tableView canEditRowAtIndexPath:(NSIndexPath *)indexPath {
    // Return NO if you do not want the specified item to be editable.
    return YES;
}
*/


/*
// Override to support editing the table view.
- (void)tableView:(UITableView *)tableView commitEditingStyle:(UITableViewCellEditingStyle)editingStyle forRowAtIndexPath:(NSIndexPath *)indexPath {
    
    if (editingStyle == UITableViewCellEditingStyleDelete) {
        // Delete the row from the data source
        [tableView deleteRowsAtIndexPaths:[NSArray arrayWithObject:indexPath] withRowAnimation:YES];
    }   
    else if (editingStyle == UITableViewCellEditingStyleInsert) {
        // Create a new instance of the appropriate class, insert it into the array, and add a new row to the table view
    }   
}
*/


/*
// Override to support rearranging the table view.
- (void)tableView:(UITableView *)tableView moveRowAtIndexPath:(NSIndexPath *)fromIndexPath toIndexPath:(NSIndexPath *)toIndexPath {
}
*/


/*
// Override to support conditional rearranging of the table view.
- (BOOL)tableView:(UITableView *)tableView canMoveRowAtIndexPath:(NSIndexPath *)indexPath {
    // Return NO if you do not want the item to be re-orderable.
    return YES;
}
*/


#pragma mark -
#pragma mark Table view delegate

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
	
	if (gIsUploading)
		return;
	
	// upload the file...
	gUploadNum = indexPath.row;
	gIsUploading = true;
	
	[activity startAnimating];
	
	[NSThread detachNewThreadSelector: @selector(doUpload) toTarget:self withObject:nil];
	
	
	
}

- (void)doUpload
{
	NSAutoreleasePool *pool = [ [ NSAutoreleasePool alloc ] init ];
	
	std::string upload = gLocalFiles[0] + gLocalFiles[gUploadNum + 1];
	NSString *uploadUrl = [NSString stringWithUTF8String:upload.c_str()];
	
	NSData *postdata = [[NSData alloc] initWithContentsOfFile:uploadUrl]; 
	
	NSMutableURLRequest *request = [[[NSMutableURLRequest alloc] init] autorelease];  
	[request setURL:[NSURL URLWithString:@"http://www.wesnothsync.com/upload.php?mobile=1"]]; 
	[request setHTTPMethod:@"POST"];
	NSString *boundary = @"---------------------------14737809831466499882746641449";
	NSString *contentType = [NSString stringWithFormat:@"multipart/form-data; boundary=%@",boundary];
	[request addValue:contentType forHTTPHeaderField: @"Content-Type"];
	NSMutableData *postbody = [NSMutableData data];
	[postbody appendData:[[NSString stringWithFormat:@"\r\n--%@\r\n",boundary] dataUsingEncoding:NSUTF8StringEncoding]];
	[postbody appendData:[[NSString stringWithFormat:@"Content-Disposition: form-data; name=\"uploaded\"; filename=\"%s\"\r\n", gLocalFiles[gUploadNum + 1].c_str()] dataUsingEncoding:NSUTF8StringEncoding]];
	[postbody appendData:[[NSString stringWithString:@"Content-Type: application/octet-stream\r\n\r\n"] dataUsingEncoding:NSUTF8StringEncoding]];
	[postbody appendData:[NSData dataWithData:postdata]];
	[postbody appendData:[[NSString stringWithFormat:@"\r\n--%@--\r\n",boundary] dataUsingEncoding:NSUTF8StringEncoding]];
	[request setHTTPBody:postbody];
	
	
	NSData *returnData = [NSURLConnection sendSynchronousRequest:request returningResponse:nil error:nil];
	NSString *returnString = [[NSString alloc] initWithData:returnData encoding:NSUTF8StringEncoding];
	NSLog(@"%@", returnString);
	
	[activity stopAnimating];
	gIsUploading = false;
	
	if ([returnData length] == 0)
	{
		UIAlertView *alert = [[UIAlertView alloc] initWithTitle:@"Connection Failed" message:@"Please check your connection and try again." delegate:nil cancelButtonTitle:@"OK" otherButtonTitles:nil, nil];
		[alert show];
		[alert release];		
	}
	else
	{
		std::string returnStr = [returnString UTF8String];
		UIAlertView *alert = [[UIAlertView alloc] initWithTitle:@"Server Response" message:returnString delegate:nil cancelButtonTitle:@"OK" otherButtonTitles:nil, nil];
		[alert show];
		[alert release];
	}
	
	[pool release];
}



#pragma mark -
#pragma mark Memory management

- (void)didReceiveMemoryWarning {
    // Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
    
    // Relinquish ownership any cached data, images, etc that aren't in use.
}

- (void)viewDidUnload {
    // Relinquish ownership of anything that can be recreated in viewDidLoad or on demand.
    // For example: self.myOutlet = nil;
}


- (void)dealloc {
    [super dealloc];
}

-(IBAction)onDone:(id)sender
{
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

@end

