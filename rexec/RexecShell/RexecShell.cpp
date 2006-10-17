//---------------------------------------------------------------------------
#define UNICODE
#include <vcl\vcl.h>
#pragma hdrstop
//#include "main.h"
//#include "client.h"

//---------------------------------------------------------------------------
USEFORM("Main.cpp", MainWindow);
USEFORM("LoginData.cpp", LoginDlg);
USEFORM("Parform.cpp", ParWindow);
USEFORM("Exclude.cpp", ExcludeForm);
USEFORM("Include.cpp", IncludeForm);
USEFORM("About.cpp", AboutBox);
USEUNIT("Client.cpp");
USEUNIT("inSocket.cpp");
USEUNIT("Encryption.cpp");
USEUNIT("RexecClient.cpp");
USEUNIT("NetState.cpp");
USEUNIT("Details.cpp");
USEUNIT("cluma_c.c");
USEUNIT("Environment.cpp");
USEUNIT("IndividualConfig.cpp");
USEUNIT("ProcWindow.cpp");
USEUNIT("PluginManager.cpp");
USEUNIT("FileFunctions.cpp");
USEUNIT("AccountManager.cpp");
USERES("RexecShell.res");
USEUNIT("RexHelp.cpp");
USEFORM("MsgBox.cpp", MsgDlg);
USEFORM("ConfigDlg.cpp", ConfDlg);
//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE hi, LPSTR, int)
{
	try
	{
		Application->Initialize();
		Application->Title = "RexecShell";
		Application->CreateForm(__classid(TMainWindow), &MainWindow);
                Application->CreateForm(__classid(TLoginDlg), &LoginDlg);
                Application->CreateForm(__classid(TParWindow), &ParWindow);
                Application->CreateForm(__classid(TExcludeForm), &ExcludeForm);
                Application->CreateForm(__classid(TIncludeForm), &IncludeForm);
                Application->CreateForm(__classid(TAboutBox), &AboutBox);
                Application->CreateForm(__classid(TConfDlg), &ConfDlg);
                Application->Run();
	}
	catch (Exception &exception)
	{
		Application->ShowException(&exception);
	}
	return 0;
}//---------------------------------------------------------------------------
