//---------------------------------------------------------------------------
#define UNICODE
#include <winsock2.h>
#include <vcl\vcl.h>

#pragma hdrstop

#include "Main.h"
#include "Client.h"
#include "NetState.h"
#include "ConfigDlg.h"
#include "Parform.h"
#include "Exclude.h"
#include "About.h"
#include "LoginData.h"
#include "Environment.h"
#include "Include.h"

#include "RexHelp.h"

#include <stdio.h>

//---------------------------------------------------------------------------
#pragma resource "*.dfm"
TMainWindow *MainWindow;
CRITICAL_SECTION CS;



//---------------------------------------------------------------------------
// Message handlers
//---------------------------------------------------------------------------

// Msg REFRESH_FINISH
void __fastcall TMainWindow::ThreadExit(TMessage &Message) {
	//PluginCombo->Enabled = true;
    Setglobalaccount1->Enabled = true;
    SpeedButton1->Enabled = true;
    SpeedButton4->Enabled = true;
    Auswaehlen->Enabled = true;
    LoadConfig1->Enabled = true;
    ChangePlugin->Enabled = true;
    Screen->Cursor=crDefault;
    Hint = "Ready";
}
// Msg REFRESH_START
void __fastcall TMainWindow::ThreadStart(TMessage &Message) {
 	//PluginCombo->Enabled = false;
    Setglobalaccount1->Enabled = false;
    SpeedButton1->Enabled = false;
    SpeedButton4->Enabled = false;
    Auswaehlen->Enabled = false;
    LoadConfig1->Enabled = false;
    ChangePlugin->Enabled = false;
    //Show Background-Activity Cursor during query
    Screen->Cursor=crAppStart;

    Hint = "Querying hosts";
}
//---------------------------------------------------------------------------
void __fastcall TMainWindow::FormCreate(TObject *Sender)
{
    InitializeCriticalSection(&CS);
   WSADATA wsaData,*lpwsaData;
   char *domain,user[256];

		lpwsaData = &wsaData;

		WORD wVersionRequested = MAKEWORD(1, 1);
		int nResult = WSAStartup(wVersionRequested, lpwsaData);
		if (nResult != 0) {
      	Application->MessageBox("WSAStartup failed","Error",MB_OK|MB_ICONERROR);
			return;
      }

		if (LOBYTE(lpwsaData->wVersion) != 1 || HIBYTE(lpwsaData->wVersion) != 1)
		{
			WSACleanup();
         Application->MessageBox("Wrong wsock version","Error",MB_OK|MB_ICONERROR);
			return;
		}
        DWORD size=256;
        domain=getenv("USERDOMAIN");
		GetUserNameA(user,&size);
        sprintf(name,"%s/%s",domain,user);
        Application->ShowHint = true;
        Application->OnHint = ShowHint;
        ShownBefore=false;


	Servers = new CServers(FALSE);
	PluginManager.LoadPlugins();
    CreatePluginMenu();
    PluginManager.RegisterClientWindow(Handle);
    Servers->RegisterClientWindow(Handle);

    //read checked Menue items from ini-file
    BOOL retval = true;
    DWORD errcode = NO_ERROR;
    char IniFileName[1024];
    AnsiString inivalue;
    if (!GetFullExeName(IniFileName))
       strcpy(IniFileName,"RexecShell.ini");
    else
    {
          ChangeFileExt(IniFileName,".ini") ;
        }
     IniFile = new TIniFile(IniFileName);
     inivalue = IniFile->ReadString("RexecShell" , "SetRPCEncryption","Default");
     if (inivalue == FALSESTR)
        RPCEncryption->Checked = false;
     SetRPCEncryption(RPCEncryption->Checked);   

     inivalue = IniFile->ReadString("RexecShell" , "checkprofilevalidity","Default");
     if (inivalue == FALSESTR)
        checkprofilevalidity->Checked = false;
     CheckUserAccountOnChange = checkprofilevalidity->Checked;
}

//---------------------------------------------------------------------------
__fastcall TMainWindow::TMainWindow(TComponent* Owner)
	: TForm(Owner)
{
   /*InitializeCriticalSection(&CS);
   WSADATA wsaData,*lpwsaData;
   char *domain,user[256];

		lpwsaData = &wsaData;

		WORD wVersionRequested = MAKEWORD(1, 1);
		int nResult = WSAStartup(wVersionRequested, lpwsaData);
		if (nResult != 0) {
      	Application->MessageBox("WSAStartup failed","Error",MB_OK|MB_ICONERROR);
			return;
      }

		if (LOBYTE(lpwsaData->wVersion) != 1 || HIBYTE(lpwsaData->wVersion) != 1)
		{
			WSACleanup();
         Application->MessageBox("Wrong wsock version","Error",MB_OK|MB_ICONERROR);
			return;
		}
        DWORD size=256;
        domain=getenv("USERDOMAIN");
		GetUserNameA(user,&size);
        sprintf(name,"%s/%s",domain,user);
        Application->ShowHint = true;
        Application->OnHint = ShowHint;
        ShownBefore=false;*/
}

__fastcall TMainWindow::~TMainWindow()
{
   WSACleanup();
   DeleteCriticalSection(&CS);

}


void __fastcall TMainWindow::ShowHint(TObject * Sender) {
	StatusBar1->SimpleText = Application->Hint;
}
//---------------------------------------------------------------------------
void __fastcall TMainWindow::BeendenClick(TObject *Sender)
{
	Servers->StopProcessing();
	Close();
}
//---------------------------------------------------------------------------
void __fastcall TMainWindow::SpeedButton1Click(TObject *Sender)
{
	NodesBox->Invalidate();
	if(ConfDlg->ShowModal()!=mrCancel) {
         NodesBox->Items->Assign(ConfDlg->GetHostList());
       	 Run1->Enabled=(NodesBox->Items->Count>0);
         StartButton->Enabled=Run1->Enabled;
	}
    NodesBox->Invalidate();
}

//---------------------------------------------------------------------------
void __fastcall TMainWindow::StartButtonClick(TObject *Sender)
{

   TClientForm *d;
   CHostRef *Ref;
   bool started = false;
   TStrings *Procs;

   Procs = ConfDlg->GetHostList();
   for (int i=0;i<Procs->Count;++i) {
     Ref=(CHostRef*)(Procs->Objects[i]);
     if(!Ref || !Ref->IsValidConfig()) continue;

     d=Ref->GetWindow();
     if(!d) {
     	d=new TClientForm(this);
     	Ref->SetWindow(d);
     }
     d->Execute(Ref);
     Sleep(50);
     if(d->state != error)
     	started = true;
   }
   if(started) {
	   SpeedButton2->Enabled=true;
       NodesBox->Repaint();
	   //StartButton->Enabled=false;
	   KillAll1->Enabled=true;
	   Clearall1->Enabled=true;
       if(ParWindow->Visible) ParWindow->FormShow(0);
   }
}
//---------------------------------------------------------------------------
void __fastcall TMainWindow::Tile1Click(TObject *Sender)
{
	Tile();
}
//---------------------------------------------------------------------------
void __fastcall TMainWindow::Cascade1Click(TObject *Sender)
{
	Cascade();
}
//---------------------------------------------------------------------------
void __fastcall TMainWindow::SpeedButton2Click(TObject *Sender)
{

      if(MDIChildCount) {
             ((TClientForm*)ActiveMDIChild)->Kill1Click(this);
      }
      if(!MDIChildCount)
          SpeedButton2->Enabled=False;
}
//---------------------------------------------------------------------------
void __fastcall TMainWindow::NodesBoxDblClick(TObject *Sender)
{
	CHostRef *ref;
     for (int i=0;i<NodesBox->Items->Count;i++) {
     	ref = (CHostRef*)NodesBox->Items->Objects[i];
       if(NodesBox->Selected[i] && ref) {
         ref->ShowWindow();
         break;
       }
     }
}
//---------------------------------------------------------------------------
void __fastcall TMainWindow::CloseAll1Click(TObject *Sender)
{    int i;
	  for(i=MDIChildCount-1;i>=0;i--)
			MDIChildren[i]->Close();
            /*
     SpeedButton1->Enabled=true;
     StartButton->Enabled=true;
     Run1->Enabled=true;
     SpeedButton4->Enabled=true;
     Auswaehlen->Enabled=true;
     LoadConfig1->Enabled=MainWindow->SaveConfig1->Enabled=true;
     KillAll1->Enabled=false;
     CloseAll1->Enabled=false;
     Tile1->Enabled=false;
     Cascade1->Enabled=false;
     Clearall1->Enabled=false;
     */

}
//---------------------------------------------------------------------------
void __fastcall TMainWindow::FontDialog1Apply(TObject *Sender, HWND Wnd)
{
     int i;
     TClientForm::MemoFont=FontDialog1->Font;

     for(i=0;i<MDIChildCount;i++)
       ((TClientForm*)(MDIChildren[i]))->applyFont();
}
//---------------------------------------------------------------------------
void __fastcall TMainWindow::SetFont1Click(TObject *Sender)
{
    if(FontDialog1->Execute()) FontDialog1Apply(this, 0);
}
//---------------------------------------------------------------------------
void __fastcall TMainWindow::NodesBoxDrawItem(TWinControl *Control, int Index,
   TRect &Rect, TOwnerDrawState State)
{
	
    TClientForm *Window;
    CHostRef *ref = NULL;
    HostData *refdata = NULL; //si
   	TColor FC,BC;
    TFontStyles FS;

	long int i = 0;  //si
    bool DoStrikeOut = false; //si

    if (Index >= NodesBox->Items->Count)
     return;
    memcpy(&ref,&i,sizeof(ref));//ref = NULL; is ignored by compiler //si
    memcpy(&refdata,&i,sizeof(refdata));//refdata = NULL; is ignored by compiler  //si
	TListBox *ListBox = dynamic_cast<TListBox*>(Control);
	TCanvas *Canvas = ListBox->Canvas;
   	BC=Canvas->Brush->Color;

   	Canvas->Brush->Color=clWindow;
	// display the text
   	FC=Canvas->Font->Color;
    FS = Canvas->Font->Style;
    ref = (CHostRef*)NodesBox->Items->Objects[Index];
    refdata = (*ref);

    /*Si: memory-access error occured sometimes in original code:
    if(!ref || !(*ref)->ProcData ||
       !(*ref)->ProcData->Executable ||
       !(*ref)->Account) {
       	Canvas->Font->Style = FS<<fsStrikeOut;
        Canvas->Font->Color=clWindowText;
    }   */

    //replace (*ref) with refdata and divide if-expression
    // original fails when *ref==NULL
    if (!ref)
    {
        DoStrikeOut = true;
    }
    else
    {
    	if (! refdata)
        {
           DoStrikeOut = true;
        }
        else
          if(!(refdata->ProcData))  //ERROR refdata invalid!!!
          {
            DoStrikeOut = true;
         	}
    	  else
          {
        	if ((!(refdata->ProcData->Executable)) || (!(refdata->Account)))
            {
            	DoStrikeOut = true;
            }
          }
     }   

    if(DoStrikeOut) {
       	Canvas->Font->Style = FS<<fsStrikeOut; //strike out invalid host
        Canvas->Font->Color=clWindowText;
    }
	else {
    	Window = ref->GetWindow();
   		if(Window) {
			switch(Window->state) {
                case killing: Canvas->Font->Color=clFuchsia; break;
                case running: Canvas->Font->Color=clGreen; break;
       			//case init: Canvas->Font->Color=clOlive; break;  //si
                case init: Canvas->Font->Color=clBlue; break;
       			default: Canvas->Font->Color=clWindowText; break;
     		}
   		} else Canvas->Font->Color=clGrayText;
    }
   	if(State.Contains(odSelected)) {
    	Canvas->Brush->Color=Canvas->Font->Color;
        Canvas->Font->Color=clHighlightText;
   	}

	Canvas->FillRect(Rect);                       // clear the rectangle
   	Canvas->TextOut(Rect.Left + 2, Rect.Top, ListBox->Items->Strings[Index]);
   	Canvas->Font->Color=FC;
   	Canvas->Brush->Color=BC;
    Canvas->Font->Style = FS;
}
//---------------------------------------------------------------------------
void __fastcall TMainWindow::KillAll1Click(TObject *Sender)
{
    for(int i=0;i<MDIChildCount;i++)
       ((TClientForm*)(MDIChildren[i]))->Kill1Click(this);
     KillAll1->Enabled=false;
}
//---------------------------------------------------------------------------
void __fastcall TMainWindow::LoadConfig1Click(TObject *Sender)
{
     if(OpenDialog1->Execute()) {
       if(ConfDlg->LoadConfig(OpenDialog1->FileName)) {
	       ConfName = OpenDialog1->FileName;
    	   SpeedButton1Click(this);
       }
     }
}
//---------------------------------------------------------------------------
void __fastcall TMainWindow::SaveConfig1Click(TObject *Sender)
{
     int endpos,startpos=0;


     if((ConfName.IsEmpty() && ConfDlg->GlobalConfig.ProcData &&
        ConfDlg->GlobalConfig.ProcData->Executable) || (ConfName == "Noname.rsc"))
      ConfName=ConfDlg->GlobalConfig.ProcData->Executable;
     if(ConfName.IsEmpty())
	     ConfName="Noname.rsc";
     endpos=ConfName.Length()+1;
     for (int i=ConfName.Length();i>0;i--) {
     	 if(ConfName[i]=='.') endpos=i;
       else if(ConfName[i]=='\\') {
         startpos=i;
         break;
       }
     }
     SaveDialog1->FileName=ConfName.SubString(startpos+1,endpos-startpos-1)+".rsc";
     if(SaveDialog1->Execute()) {
       if(SaveDialog1->FileName.Pos(".")<=0)
        SaveDialog1->FileName=SaveDialog1->FileName+".rsc";
       ConfName = SaveDialog1->FileName;
       ConfDlg->SaveConfig(ConfName);

     }
}
//---------------------------------------------------------------------------
void __fastcall TMainWindow::ShowParTool1Click(TObject *Sender)
{
	ParWindow->Show();
}
//---------------------------------------------------------------------------

void __fastcall TMainWindow::About1Click(TObject *Sender)
{
     //siAboutBox= new TAboutBox(this);
     AboutBox->ShowModal();
     //sidelete AboutBox;
}
//---------------------------------------------------------------------------
void __fastcall TMainWindow::Clearall1Click(TObject *Sender)
{
     for(int i=0;i<MDIChildCount;i++)
       ((TClientForm*)(MDIChildren[i]))->ClearOutput1Click(this);
}
//---------------------------------------------------------------------------
void __fastcall TMainWindow::Setglobalaccount1Click(TObject *Sender)
{
	TLoginDlg *Login;

    Login = new TLoginDlg(this);
    //do not show OK to force user to use set for all
    Login->OKButton->Visible = false;
    Login->CancelButton->Caption = "Exit";
    if(Login->ShowModal() != mrCancel) {
		Servers->Lock();
    	for(int i=0;i<Servers->Count();i++) {
           Login->GetLoginData((*Servers)[i]);
    	}
        Servers->Unlock();
        Servers->Refresh();
    }
    delete Login;
}
//---------------------------------------------------------------------------
void __fastcall TMainWindow::FormClose(TObject *Sender, TCloseAction &Action)
{
  // store actual plugin	
  CloseAll1Click(this);
}
//---------------------------------------------------------------------------


void __fastcall TMainWindow::OnPluginChange(TMessage &Message) {
	PlgItem->Items[oldPlugin]->Checked = false;
	oldPlugin=PluginManager.GetActualIndex();
	PlgItem->Items[oldPlugin]->Checked = true;
}

void __fastcall TMainWindow::PluginClick(TObject *Sender) {
    PluginManager.SetActualPlugin(PlgItem->IndexOf((TMenuItem*)Sender));
}

void __fastcall TMainWindow::CreatePluginMenu() {
	int i;
    TStrings *PlgNames;
    PlgItem = 0;
	TMenuItem *Item = MainMenu1->Items[0].Items[0];
    for(i=0;i<Item->Count;++i) {
    	if(Item->Items[i]->Caption=="C&hange Plug-in") {
        	PlgItem=Item->Items[i];
            break;
        }
    }
	if(!PlgItem) return;
    PlgNames=PluginManager.GetPluginList();
    for(i=0;i<PlgNames->Count;++i) {
    	Item=new TMenuItem(PlgItem);
        Item->Caption = PlgNames->Strings[i];
        //Item->RadioItem=true;
        Item->OnClick = PluginClick;
        PlgItem->Add(Item);
    }
    oldPlugin = PluginManager.GetActualIndex();
    PlgItem->Items[oldPlugin]->Checked=true;
}


void __fastcall TMainWindow::FormDestroy(TObject *Sender)
{
     delete Servers;
    Servers = 0;
    delete IniFile;
    IniFile = NULL;
}
//---------------------------------------------------------------------------
void __fastcall TMainWindow::EditIncludelist1Click(TObject *Sender)
{
	//IncludeForm = new TIncludeForm(this);
 	IncludeForm->ShowModal();
 	//delete IncludeForm;
}
//---------------------------------------------------------------------------
void __fastcall TMainWindow::Window1Click(TObject *Sender)
{
	Tile1->Enabled = (MDIChildCount >0);
    Cascade1->Enabled = Tile1->Enabled;	
}
//---------------------------------------------------------------------------
void __fastcall TMainWindow::Editexcludelist1Click(TObject *Sender)
{
	//ExcludeForm = new TExcludeForm(this);
 	ExcludeForm->ShowModal();
 	//delete ExcludeForm;
}
//---------------------------------------------------------------------------

void __fastcall TMainWindow::FormShow(TObject *Sender)
{
	if (!ShownBefore)
    {
    	ShownBefore = true;
        Servers->Refresh();
        if (Servers->useMachinestxt)
        {
        	EditIncludelist1->Enabled = false;
            Editexcludelist1->Enabled = false;
        }
    }
}
//---------------------------------------------------------------------------
void __fastcall TMainWindow::Info1Click(TObject *Sender)
{
    	AnsiString EName;
    	char  Msg[500];
    	EName = Application->ExeName;

         EName += "\n";
#if (!(BCBVER > 1))
	//CBuilder1
	EName += "CBuilder1 Version";
#else
	//CBuilder5
    EName += "CBuilder5 Version";
#endif

        StrPCopy(Msg,EName);
		Application->MessageBox(Msg,"Info",MB_OK);

}
//---------------------------------------------------------------------------
void __fastcall TMainWindow::RPCEncryptionClick(TObject *Sender)
{
   
   RPCEncryption->Checked = (! RPCEncryption->Checked);
   SetRPCEncryption(RPCEncryption->Checked);
   if (RPCEncryption->Checked)
      IniFile->WriteString("RexecShell","SetRPCEncryption",TRUESTR);
   else
      IniFile->WriteString("RexecShell","SetRPCEncryption",FALSESTR);

   Servers->Refresh();
}
//---------------------------------------------------------------------------
void __fastcall TMainWindow::checkprofilevalidityClick(TObject *Sender)
{
  checkprofilevalidity->Checked = (! checkprofilevalidity->Checked);

   CheckUserAccountOnChange = checkprofilevalidity->Checked;
   if (checkprofilevalidity->Checked)
      IniFile->WriteString("RexecShell","checkprofilevalidity",TRUESTR);
   else
      IniFile->WriteString("RexecShell","checkprofilevalidity",FALSESTR);

}
//---------------------------------------------------------------------------
