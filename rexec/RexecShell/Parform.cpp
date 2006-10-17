#define UNICODE
//---------------------------------------------------------------------------
#include <winsock2.h>
#include <vcl\vcl.h>

#include <malloc>

#pragma hdrstop

#include "Main.h"
#include "RexecClient.h"
#include "parform.h"
#include "Details.h"
#include "LoginData.h"
#include "..\mpirun\plugins\Plugin.h"
#include "PluginManager.h"
#include "ProcWindow.h"

//---------------------------------------------------------------------------
#pragma resource "*.dfm"

extern char *MakeErrorMessage(DWORD ErrorId);

TParWindow *ParWindow;
//---------------------------------------------------------------------------
__fastcall TParWindow::TParWindow(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TParWindow::QuitButtonClick(TObject *Sender)
{
	Timer1->Enabled=false;
	Close();
}

//---------------------------------------------------------------------------
void __fastcall TParWindow::RefreshButtonClick(TObject *Sender)
{

//     ListView->Items->Clear();
     ProgressBar->Position=0;
//     Servers.RegisterClientWindow(Handle);
     if(!Servers->Refresh()) Incomplete=true;

}

void __fastcall TParWindow::ParData(TMessage Message) {
     DWORD j;
     HostData *Data;
    TListItem *Item;
    char *UserString,*pos;
    char ConnectedUser[255];
    DWORD StringSize=0;
    PlgDesc *ActivePlugin;
    ProgressBar->Position = Message.WParam;
    ListView->Items->BeginUpdate();
    Data=(HostData*)Message.LParam;
    Item= ListView->Items->Add();
    //stateEntry* actEntry;

    if(!Item) return;
    Item->Caption=Data->Name;
    for(j=0;j<5;j++)
       	Item->SubItems->Add(AnsiString(" "));

    if(!Data->State){
       	Item->SubItems->Strings[0]="Cannot connect to server";
        if(Data->LastError != ERROR_SUCCESS) {
        	UserString = MakeErrorMessage(Data->LastError);
            if(UserString) {
            	pos=strchr(UserString,'\r');
                if(pos) *pos=0;
        		Item->SubItems->Strings[2] = UserString;
            }
        }
        Item->SubItems->Strings[3]=GetHostAccountName(Data,ConnectedUser,254); //Si display connected user
        Item->StateIndex=2;
        ListView->Items->EndUpdate();
        return;
    }
    UserString=0;
    Item->StateIndex=1;
    ActivePlugin = PluginManager.GetActualPlugin();
    if(ActivePlugin->Convert&&Data->State->UserData) {
    	ActivePlugin->Convert(Data->State,UserString,&StringSize);
        if(StringSize) {
        	UserString=(char*)alloca(StringSize);
            ActivePlugin->Convert(Data->State,UserString,&StringSize);
        }
    }
    Data->State->Valid = TRUE;
    Item->SubItems->Strings[0]=AnsiString(Data->State->LockedBy);
    Item->SubItems->Strings[1]=AnsiString(Data->State->ConsoleUser);
    Item->StateIndex=!Data->State->Locked;
	
   if(ActivePlugin->NewData) Data->State->Valid &= ActivePlugin->NewData(Data);
   if(UserString)   {
    	pos=strchr(UserString,'\n');
    	while(pos) {
        	*pos=' ';
        	pos=strchr(pos+1,'\n');
    	}                                            
   		Item->SubItems->Strings[2]=UserString;
   }
   Item->SubItems->Strings[3]=GetHostAccountName(Data,ConnectedUser,254); //Si display connected user
   Item->SubItems->Strings[4]=Data->RPCProtocol; //Si display connected user

   ListView->Items->EndUpdate();
}    

void __fastcall TParWindow::ThreadExit(TMessage Message) {
	ProgressBar->Position=0;

     	//Servers.RemoveClientWindow(Handle);
	    Cursor=crArrow;
     	ListView->Cursor=crArrow;
     	RefreshButton->Enabled=true;
     	TimerButton->Enabled=true;
     	Timer1->Enabled=(Visible&&TimerButton->Down);
	if(Message.Msg==REFRESH_FINISH) {
        if(Incomplete) {
            ListView->Items->BeginUpdate();
            ListView->Items->Clear();
        	Servers->EnumData(Handle);
        }
    } else {
        Incomplete=false;
    	ListView->Items->EndUpdate();
    }
}

void __fastcall TParWindow::ThreadStart(TMessage Message) {
     ProgressBar->Max=(TProgressRange)Message.LParam;
     LastSize=Message.LParam;
     if(Message.Msg==REFRESH_START) {
        ListView->Items->Clear();
     	Timer1->Enabled=false;
     	TimerButton->Enabled=false;
     	Cursor=crHourGlass;
     	ListView->Cursor=crHourGlass;
     	RefreshButton->Enabled=false;
     }
}
//---------------------------------------------------------------------------
void __fastcall TParWindow::UnlockButtonClick(TObject *Sender)
{
	 TListItem *Item;
    int mbRes;
    BOOL result;
    wchar_t name[256];
	Item=ListView->ItemFocused;
    if(!Item)
      return;
    if(Item->StateIndex==0) {
	    do {
            //  UnlockServer: RexecClient.cpp
    		result=UnlockServer((*Servers)[Item->Caption.c_str()]);
	    	if(result != ERROR_SUCCESS)
        	  mbRes=Application->MessageBox((char*)GetLastErrorText(result,name,256)
            	                            ,"Unlock failed",MB_ICONERROR|MB_RETRYCANCEL);
    	} while(result != ERROR_SUCCESS&&mbRes==IDRETRY) ;
	    if(result == ERROR_SUCCESS) {
       		Item->SubItems->Strings[0]=AnsiString(" ");
    	    Item->StateIndex=1;
	        ListView->Refresh();
        	Servers->Refresh(Item->Caption.c_str());
    	}  else LockButton->Down = true;
    }

}
//---------------------------------------------------------------------------
void __fastcall TParWindow::LockButtonClick(TObject *Sender)
{
	TListItem *Item;
    int mbRes;
    BOOL result;
    wchar_t error[256];
    HostData *Host;
    //DWORD size=256;

	Item=ListView->ItemFocused;
    if(!Item||Item->SubItems->Count<1)
      return;
    if(Item->StateIndex==1) {
    	do {
            // LockServer: RexecClient.cpp
    		result=LockServer((*Servers)[Item->Caption.c_str()]);
    		if(result != ERROR_SUCCESS)
          		mbRes=Application->MessageBox((char*)GetLastErrorText(result,error,256)
                                              ,"Lock failed",MB_ICONERROR|MB_RETRYCANCEL);
    	} while((result != ERROR_SUCCESS)&&mbRes==IDRETRY) ;

    	if(result == ERROR_SUCCESS) {
	        //GetUserName(name,&size);
            Servers->Refresh(Item->Caption.c_str());
            Host = (*Servers)[Item->Caption.c_str()];
            if(Host && Host->State) {
            	Item->SubItems->Strings[0]=Host->State->LockedBy;
            } else {
        		Item->SubItems->Strings[0]=name;
            }
            Item->StateIndex=0;
            ListView->Refresh();
        }  else UnlockButton->Down = true;
    }
}
//---------------------------------------------------------------------------
void __fastcall TParWindow::ListViewChange(TObject *Sender, TListItem *Item,
	TItemChange Change)
{
	if((Change==ctState)&&(Item==ListView->ItemFocused)) {
        if(Item->StateIndex<2) {
	        LockUnlockServer->Enabled=true;
    	        LockButton->Enabled=true;
	        UnlockButton->Enabled=true;
                if(Item->StateIndex==0)
                   LockButton->Down=true;
                else
                   UnlockButton->Down=true;
	        LockUnlockServer->Checked=(Item->StateIndex==0);
                Locked1->Checked = LockUnlockServer->Checked;
          } else {
          	LockButton->Enabled=false;
          	UnlockButton->Enabled=false;
          	LockUnlockServer->Enabled=false;
                Locked1->Enabled = false;
          }
    }
}
//---------------------------------------------------------------------------
void __fastcall TParWindow::LockUnlockServerClick(TObject *Sender)
{
	if(LockUnlockServer->Checked) UnlockButtonClick(LockUnlockServer);
    else LockButtonClick(LockUnlockServer);
}
//---------------------------------------------------------------------------
void __fastcall TParWindow::FormShow(TObject *Sender)
{
	/*TMessage m;
    m.Msg=REFRESH_START;
    m.LParam=LastSize;
	*/
    if(!Servers->Refreshing) {
    	if(Sender) Timer1->Enabled=TimerButton->Down;
        ListView->Items->BeginUpdate();
    	ListView->Items->Clear();
    	Servers->EnumData(Handle);
    } else {
        Incomplete=true;
        //Servers.RegisterClientWindow(Handle);
    }
}
//---------------------------------------------------------------------------
void __fastcall TParWindow::StickButtonClick(TObject *Sender)
{
	HWND Style;
	Style = StickButton->Down?HWND_TOPMOST:HWND_NOTOPMOST;
   	SetWindowPos(Handle,Style,0,0,0,0,SWP_NOMOVE | SWP_NOSIZE);
    Stayontop1->Checked = StickButton->Down;
}
//---------------------------------------------------------------------------
void __fastcall TParWindow::FormCreate(TObject *Sender)
{
	Servers->RegisterClientWindow(Handle);
    Incomplete=false;

}
//---------------------------------------------------------------------------
void __fastcall TParWindow::Showdetails1Click(TObject *Sender)
{
    HostData *actHost;
	TListItem *Item;
    TDetailForm *DetailForm;
  	Item=ListView->ItemFocused;
    if(!Item) return;
    actHost=(*Servers)[Item->Caption.c_str()];
    if(!actHost) return; //ListView->Items->Delete(Item->Index);
    DetailForm = new TDetailForm(this);
    if(StickButton->Down)
   	   	SetWindowPos(DetailForm->Handle,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE | SWP_NOSIZE);

    DetailForm->CreateDesc(actHost);
    DetailForm->Show();
}
//---------------------------------------------------------------------------
void __fastcall TParWindow::SetAccount1Click(TObject *Sender)
{
	TLoginDlg *LoginData;
  	HostData *Host;
	TListItem *Item;
    static bool MessageBoxShown = false;   //Si

    Item=ListView->ItemFocused;
    if(!Item) return;
    Host=(*Servers)[Item->Caption.c_str()];
    if(!Host) return;
    LoginData = new TLoginDlg(this);
	LoginData->SetLoginData(Host);
    if(LoginData->ShowModal() == mrCancel) {
    	delete LoginData;
        return;
    }
    LoginData->GetLoginData(Host);

   	delete LoginData;
    //Si display message once
    if (!MessageBoxShown)
     Application->MessageBox(
        "manual refresh after 'Set Account...' neccessary",
        "Information", MB_OK);
    MessageBoxShown = true;    

}
//---------------------------------------------------------------------------
void __fastcall TParWindow::Stayontop1Click(TObject *Sender)
{
	StickButton->Down = !StickButton->Down;
    StickButtonClick(this);
}
//---------------------------------------------------------------------------
void __fastcall TParWindow::Processes1Click(TObject *Sender)
{
	HostData *Host;
	TListItem *Item;
    TProcForm *PW;

  	Item=ListView->ItemFocused;
    if(!Item) return;
    Host=(*Servers)[Item->Caption.c_str()];
    if(!Host || !Host->Alive) return;
    PW = new TProcForm(MainWindow);
    PW->Show();
	GetHostProcs(PW->Handle,Host);
}
//---------------------------------------------------------------------------
void __fastcall TParWindow::PopupMenu1Popup(TObject *Sender)
{
    HostData *Host;
	TListItem *Item;
    BOOL valid = TRUE;

   	Item=ListView->ItemFocused;
    valid &= (Item != 0);
    if(valid) {
    	Host=(*Servers)[Item->Caption.c_str()];
        Showdetails1->Enabled = true;
        SetAccount1->Enabled = true;
        Restart1->Enabled = true;
        ShutDown1->Enabled = true;
    	valid &= (Host != 0);
		if(valid) valid &= (Host->Alive);
    } else {
    	Showdetails1->Enabled = false;
        SetAccount1->Enabled = false;
        Restart1->Enabled = false;
        ShutDown1->Enabled = false;
    }
//    Showdetails1->Enabled = valid;
    Processes1->Enabled = valid;
    Locked1->Enabled = valid;
}
//---------------------------------------------------------------------------
void __fastcall TParWindow::RefreshSelectedClick(TObject *Sender)
{
    TListItem *Item;
    int IIndex;
    char * IName;
    IName =  (char*) calloc (100,sizeof(char));
    Item=ListView->ItemFocused;
    if(!Item)
      return;
    ListView->Items->BeginUpdate();
// Servers->Refresh defined in NetState.cpp
    IIndex =  Item->Index;
    IName = Item->Caption.c_str();
    ListView->Items->Delete(IIndex);
     Servers->RefreshOne(IName);
    ListView->Items->EndUpdate();
    free(IName);
}
//---------------------------------------------------------------------------
void __fastcall TParWindow::Restart1Click(TObject *Sender)    //10.6.
{
	 TListItem *Item;
    int mbRes;
    BOOL result;
    wchar_t name[256];
	Item=ListView->ItemFocused;
    if(!Item)
      return;
    char * IName;
    IName = Item->Caption.c_str();
    mbRes=Application->MessageBox("Do you really want to reboot this computer?",IName,MB_YESNO);
    if (mbRes == IDYES)
    {
	    do {
            //  ShutDown: RexecClient.cpp
    		result=ShutDown((*Servers)[Item->Caption.c_str()],TRUE);
	    	if(result != ERROR_SUCCESS)
        	  mbRes=Application->MessageBox((char*)GetLastErrorText(result,name,256)
            	                            ,"ShutDown failed",MB_ICONERROR|MB_RETRYCANCEL);
    	} while(result != ERROR_SUCCESS&&mbRes==IDRETRY) ;

       RebootTimer->Enabled=true;
     }
}
//---------------------------------------------------------------------------
void __fastcall TParWindow::ShutDown1Click(TObject *Sender)
{
	 TListItem *Item;
    int mbRes;
    BOOL result= ERROR_SUCCESS;
    wchar_t name[256];
	Item=ListView->ItemFocused;
    if(!Item)
      return;
    char * IName;
    IName = Item->Caption.c_str();
    mbRes=Application->MessageBox("Do you really want to shut down this computer?",IName,MB_YESNO);
    if (mbRes == IDYES)
    {
        ListView->Items->BeginUpdate();
	    do {
            //  ShutDown: RexecClient.cpp
    		result=ShutDown((*Servers)[Item->Caption.c_str()],FALSE);
	    	if(result != ERROR_SUCCESS)
        	  mbRes=Application->MessageBox((char*)GetLastErrorText(result,name,256)
            	                            ,"ShutDown failed",MB_ICONERROR|MB_RETRYCANCEL);
            else
            {

              Servers->Delete(IName);
              ListView->Items->Delete(Item->Index);
              }
    	} while(result != ERROR_SUCCESS&&mbRes==IDRETRY) ;

         ListView->Items->EndUpdate();
         ListView->Repaint();
     }

}
//---------------------------------------------------------------------------
void __fastcall TParWindow::RebootTimerTimer(TObject *Sender)
{
    RebootTimer->Enabled = false;
    RefreshButtonClick(Sender);
}
//---------------------------------------------------------------------------
