//
//  sync_login.h
//  wesnoth
//
//  Created by Kyle Poole on 5/10/10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>


@interface sync_login : UIViewController <UITextFieldDelegate> {
	IBOutlet UITextField *email;
	IBOutlet UITextField *password;
	IBOutlet UIActivityIndicatorView *activity;
	IBOutlet UIButton *loginButton;
	IBOutlet UIButton *cancelButton;
}

@property (nonatomic,retain) IBOutlet UITextField *email;
@property (nonatomic,retain) IBOutlet UITextField *password;
@property (nonatomic,retain) IBOutlet UIActivityIndicatorView *activity;
@property (nonatomic,retain) IBOutlet UIButton *loginButton;
@property (nonatomic,retain) IBOutlet UIButton *cancelButton;

-(IBAction)login:(id)sender;
-(IBAction)cancel:(id)sender;
-(IBAction)hideEmailKeyboard:(id)sender;
-(IBAction)hidePasswordKeyboard:(id)sender;
-(IBAction)emailNext:(id)sender;


@end
