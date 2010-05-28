//
//  UnitDetails.h
//  wesnoth
//
//  Created by Kyle Poole on 5/16/10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>


@interface UnitDetails : UIViewController {
	IBOutlet UIWebView *webView;
}

@property (nonatomic,retain) IBOutlet UIWebView *webView;

-(IBAction)onOK:(id)sender;

@end
