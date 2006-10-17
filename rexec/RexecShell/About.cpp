//---------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "About.h"
//--------------------------------------------------------------------- 
#pragma resource "*.dfm"

#define  VersionNo "Version 1.36"
//Version 1.24 R: check if user account is valid
//Version 1.25 R: remote shutdown
//Version 1.26 R: fixed encryption problems, show message if account for
//                one server changes, change account in config possible
//Version 1.27 R: error correction, specify process priority
//Version 1.28 R: check plugin version
//Version 1.29 R: added RPC-protocols for test purposes, use rcluma V2.15 or higher
//Version 1.30 R: show account for connection, new button for set account
//Version 1.31 R: show RPC-protocols for connection, bug fixture set single account
//Version 1.32 R: CTRL-C and CTRL-V now works in client output window
//Version 1.33 R: switch off profile check, read checked menues from ini-file
//Version 1.34 R: use protocol ncacn_np for XP SP 2
//Version 1.35 R: Tedit for plugin options, version 11 of plugin.h needed in plugins
//Version 1.35b R: Changes in load and save config
//Version 1.35c R: do not lock nodes on default
//Version 1.35d R: do not update servers at TExcludeForm::FormShow
//Version 1.36: added sleep(1)s to prevent main window from freezing no "R"->RPC is mandatory (13.7.05)

TAboutBox *AboutBox;
//--------------------------------------------------------------------- 
__fastcall TAboutBox::TAboutBox(TComponent* AOwner)
	: TForm(AOwner)
{
        Version->Caption = VersionNo;
    #if (BCBVER > 1)
        LBuilderVersion->Caption = "32-bit compiled";
    #endif
        Copyright->Caption="Copyright 1997-2005 Lehrstuhl für Betriebssysteme";
}
//--------------------------------------------------------------------- 
