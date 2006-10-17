//---------------------------------------------------------------------------
#include <winsock2.h>
#include <vcl\vcl.h>
#pragma hdrstop

#include "Client.h"
#include "Main.h"
#include "ntrexec.h"
#include "ConfigDlg.h"
#include "LoginData.h"
#include "parform.h"
#include "RexecClient.h"
#include "NetState.h"
#include <Printers.hpp>
#include "MsgBox.h"
#include "RexHelp.h"

#include <stdio.h>

//---------------------------------------------------------------------------
#pragma resource "*.dfm"
//TClientForm *ClientForm;
//#define DBG(m) Application->MessageBox(m,"Error",1);
//#define DBG(m) { DebugMemo->Lines->Add(m);}


#ifndef CREATE_WITH_USERPROFILE
#define CREATE_WITH_USERPROFILE     0x02000000
#endif

#define DBG(m)
TFont *TClientForm::MemoFont=0;

static char ErrorMessage[256];
//static bool Processing=false;
bool CloseAll = false;

char *MakeErrorMessage(DWORD ErrorId) {

	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
			NULL,ErrorId,MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
			(LPTSTR) ErrorMessage,255,NULL);

   return ErrorMessage;

}
/*
VOID CALLBACK TimerProc(
    HWND hwnd,        // handle of window for timer messages
    UINT message,     // WM_TIMER message
    UINT idTimer,     // timer identifier
    DWORD dwTime)     // current system time
{
    TMessage msg;
    HANDLE hClient;

    KillTimer(hwnd,idTimer);
    while(StoredMsgs.size())  {
        hClient = StoredMsgs.front().first;
	    msg = StoredMsgs.front().second;
		StoredMsgs.pop_front();
        PostMessage(hClient,msg.Msg,msg.WParam,msg.LParam);
    }
}
*/

#define BUFSIZE 4096

void __fastcall TClientForm::PipeData(TMessage Message) {
	static char buf[BUFSIZE];
    int res;
    TMemo *Memo;
    BOOL err,finish;
    char *pos,*pos2,*pos3;
    MSG Msg;

	if(PeekMessage(&Msg,NULL,0,0,PM_NOREMOVE))
   		Application->HandleMessage();

    if(WSAGETSELECTEVENT(Message.LParam) == FD_CLOSE) {
    	finish = TRUE;
        WSAAsyncSelect((SOCKET)Message.WParam,Handle,0,0);
    } else finish=FALSE;

    if(Message.Msg==IN_DATA) {
        err=FALSE;
        Memo=OutMemo;
    } else {
        err=TRUE;
        Memo=ErrMemo;
    }

	res=ReadData(hRemoteProc,buf,BUFSIZE-1,err);

    Memo->Lines->BeginUpdate();

    while(res>0) {
    	buf[res]=0;
        if(!complete[err]) {
        	pos = strchr(buf,'\n');
            if(pos) {
            	pos3 = pos-1;
            	*pos=0;
                pos++;

            } else {
            	pos = buf+res;
                pos3 = pos-1;
            }
            while((*pos3=='\r' || *pos3 == '\t') && pos3 >= buf) {
               	*pos3 = 0;
                 pos3--;
            }
            Memo->Lines->Strings[Memo->Lines->Count-1] =
   	            Memo->Lines->Strings[Memo->Lines->Count-1]+buf;
        } else pos = buf;
        pos2=pos;
        while(*pos2 !=0) {
	        if(*pos2 =='\r' && *(pos2+1) == '\n') {
            	*pos2 = 0;
                pos3=pos2-1;
                while((*pos3=='\r' || *pos3 == '\t') && pos3 >= pos) {
                	*pos3 = 0;
                    pos3--;
                }
                pos2 += 2;
                Memo->Lines->Add(pos);
                pos = pos2;
            } else ++pos2;
        }
        if(pos != pos2) Memo->Lines->Add(pos);
/*
        if(((DWORD)pos-(DWORD)buf)<res) {
	        Strings->SetText(pos);
    		Memo->Lines->AddStrings(Strings);
        }
*/
        complete[err] = (buf[res-1] == '\n' || buf[res-1] == 0);
        if(finish)
        	res=ReadData(hRemoteProc,buf,BUFSIZE-1,err);
        else res =0;
    }

    Memo->Lines->EndUpdate();
    if(WindowState != wsMinimized)
    	SendMessage(Memo->Handle,EM_SCROLLCARET ,0,0);


    if(finish) {
    	shutdown((SOCKET)Message.WParam,SD_BOTH);
        Connections--;
        if(!Connections) Finished();
    }
}

void __fastcall TClientForm::Finished() {
	BOOL result;
    Cursor=crDefault;
    OutMemo->Cursor=crDefault;
    ErrMemo->Cursor=crDefault;
    ErrBox->Cursor=crDefault;
    state=finished;
    Caption = AnsiString((*Ref)->Name) + AnsiString(" -- disconnected -- ");
    MainWindow->StartButton->Enabled=true;
    MainWindow->Run1->Enabled=true;
    MainWindow->NodesBox->Repaint();
    if(MainWindow->ActiveMDIChild==this) {
    	MainWindow->SpeedButton2->Enabled=false;
    	Kill1->Enabled=false;
    }
    complete[0] = complete[1] = true;
    StatusBar1->SimpleText="Program exited";
    CloseRemoteHandle(hRemoteProc);
    if((*Ref)->ProcData->LockIt==1) {
    	 result=UnlockServer(*Ref);
       	 if(result == ERROR_SUCCESS) {
        	Servers->Refresh((*Ref)->Name);
            if(ParWindow->Visible)
            	ParWindow->FormShow(0);
         }
      }
      Strings->Clear();
      if(closed) Close();
}
//---------------------------------------------------------------------------
__fastcall TClientForm::TClientForm(TComponent* Owner)
	: TForm(Owner)
{
   closed=FALSE;
   if(MemoFont) applyFont();
   pos=-1;
   //InitializeCriticalSection(&ClientCS);
   //Server = (HostData*)malloc(sizeof(HostData));
   //memset(Server,0,sizeof(HostData));
   Strings = new TStringList;

}

__fastcall TClientForm::~TClientForm() {
   	//FreeHost(Server);
    delete Strings;
  }

//---------------------------------------------------------------------------
void __fastcall TClientForm::FormCloseQuery(TObject *Sender, bool &CanClose)
{   WORD Buttons = 0;
    int RetVal = 0;
    if(state == init) {
    	CanClose = false;
        return;
    }
	CanClose =
    	(state!=running&&state!=killing)||
        IsServerAlive((*Ref)->Name) != ERROR_SUCCESS
        /*||
      (Application->MessageBox("This will kill the running task.\n Continue?",
      (*Ref)->Name,MB_YESNO|MB_ICONWARNING)==6)*/
      ;
      if (!CanClose)
        CanClose = CloseAll;
      if (!CanClose)
      {
        Buttons = BtnYes + BtnNo +BtnAll;
        RetVal = MyMsgBox("Confirmation","This will kill the running task.\n Continue?",Buttons);
        CanClose = (RetVal != mrNo);
        CloseAll = (RetVal == 8);
        }
}
//---------------------------------------------------------------------------
void __fastcall TClientForm::FormClose(TObject *Sender, TCloseAction &Action)
{
   if(state==running) {
     Caption = AnsiString((*Ref)->Name) + " -- Shutting Down --";
     MainWindow->SpeedButton2->Enabled=false;
     Kill1->Enabled=false;
     StatusBar1->SimpleText="Trying to terminate remote process";
     //WindowState=wsMinimized;
     closed=true;
     Enabled=false;
     Kill1Click(this);
     //KillRemoteProcess(hRemoteProc);
     //Action=caNone;
     Action = caMinimize;
   } else {
   		Ref->RemoveWindow();
        /*
        pos=MainWindow->NodesBox->Items->IndexOfObject(Ref);
   		if(pos>=0) {
     		((CHostRef*)(MainWindow->NodesBox->Items->Objects[pos]))->Window=0;
     		MainWindow->NodesBox->Repaint();
        }*/
        MainWindow->NodesBox->Invalidate();
        Action=caFree;
   }
}
//---------------------------------------------------------------------------

#define SUCCESS 0
#define LOCK_FAILED 1
#define CREATE_FAILED 2

struct ThreadParams {
	HostData *Server;
    BOOL Locked;
    RemoteStartupInfo *SI;
};

int __fastcall ThreadFunc(ThreadParams *Params) {
	DWORD result;
    HANDLE hRemoteProc;
	if(!Params) return 1;

	if(Params->Locked) {
		result=LockServer(Params->Server);
       	//GetHostStatus(Params->Server,0);
   		if(result != ERROR_SUCCESS) {
        	CloseHost(Params->Server);
        	SendMessage(Params->SI->Window,INIT_FINISH,(WPARAM)LOCK_FAILED,result);
            free(Params->SI->Commandline);
            free(Params->SI);
            free(Params);
       		return 1;
	    }
        Servers->Refresh(Params->Server->Name);
    }
   	result=CreateRemoteProcess(Params->Server,Params->SI,&hRemoteProc);
   	if(result != ERROR_SUCCESS) {
        if(Params->Locked) {
       	 	if(UnlockServer(Params->Server) == ERROR_SUCCESS) {
        		Servers->Refresh(Params->Server->Name);
         	}
      	}
	    SendMessage(Params->SI->Window,INIT_FINISH,(WPARAM)CREATE_FAILED,result);
   	} else
    	SendMessage(Params->SI->Window,INIT_FINISH,(WPARAM)SUCCESS,(LPARAM)hRemoteProc);

   	CloseHost(Params->Server);        
   	free(Params->SI->Commandline);
   	free(Params->SI);
   	free(Params);
   	System::EndThread(0);
   	return 0;
}

void __fastcall TClientForm::InitFinished(TMessage Message) {
	char msg[256];
	switch(Message.WParam) {
    case SUCCESS: state=running;
    			  hRemoteProc=(HANDLE)Message.LParam;
                  if((*Ref)->ProcData->LockIt==1 && ParWindow->Visible)
                  	ParWindow->FormShow(0);
                  break;
    case LOCK_FAILED: state = error;
                  ErrMemo->Lines->Add("Could not lock server");
	        	  ErrMemo->Lines->Add(MakeErrorMessage(Message.LParam));
                  if((*Ref)->State && (*Ref)->State->Locked) {
            			sprintf(msg,"Server is locked by %s",(*Ref)->State->LockedBy);
                  		ErrMemo->Lines->Add(msg);
                  }
                  break;
    case CREATE_FAILED: state = error;
    				   ErrMemo->Lines->Add("Creation of process failed");
                       ErrMemo->Lines->Add(MakeErrorMessage(Message.LParam));
                       break;
    default:	  break;
    }

    if(state == error) {
    	Caption=AnsiString((*Ref)->Name) + " -- Init failed --";
    	StatusBar1->SimpleText="Init of program failed";
    	Kill1->Enabled=false;

    } else {
    	Caption=AnsiString((*Ref)->Name) + " -- Running --";
        StatusBar1->SimpleText="Program running";
	    Kill1->Enabled=true;
    }

    if(MainWindow->ActiveMDIChild==this) {
    	MainWindow->SpeedButton2->Enabled= (state!=error);
        MainWindow->Run1->Enabled=(state==error);
    }
    MainWindow->StartButton->Enabled=MainWindow->Run1->Enabled;
    MainWindow->NodesBox->Invalidate();
}

void TClientForm::Execute(CHostRef *r) {

    RemoteStartupInfo *SI;
    BOOL result;
    ThreadParams *Params;
    HANDLE hThread;
    DWORD tID = 0;
    int pos;
    HostData * rHost; //unable to debug construct using overloaded (*r) !
    rHost = (*r);  //Replace (*r)-> everywhere with rHost->

   if(state==running||state==killing) return;
   if(!r || !rHost->ProcData || !rHost->ProcData->Executable || !rHost->Account)
   		return;

   complete[0] = complete[1] = true;
   Ref=r;

   Caption=AnsiString((*Ref)->Name) + " -- Init --" ;
   StatusBar1->SimpleText="Creating Process";
   Show();
   if(MainWindow->ActiveMDIChild==this) {
   		pos=MainWindow->NodesBox->Items->IndexOfObject(Ref);
		if(pos>=0) MainWindow->NodesBox->ItemIndex = pos;
        MainWindow->StartButton->Enabled=false;
        MainWindow->Run1->Enabled=false;
   }

   SI = (RemoteStartupInfo*)malloc(sizeof(RemoteStartupInfo));
   SI->Commandline = (char*)malloc(
              rHost->ProcData->ExeSize+
              rHost->ProcData->WDSize+
              rHost->ProcData->CmdSize+
              rHost->ProcData->OptSize+
              rHost->ProcData->PluginOptSize+4);

	strcpy(SI->Commandline,rHost->ProcData->Executable);

    if(rHost->ProcData->PluginOptions) {
    	    strcat(SI->Commandline," ");
            strcat(SI->Commandline,rHost->ProcData->PluginOptions);
    }

    if(rHost->ProcData->Commandline) {
   	    strcat(SI->Commandline," ");
	    strcat(SI->Commandline,rHost->ProcData->Commandline);
    }

	if(rHost->ProcData->UserOptions) {
    	    strcat(SI->Commandline," ");
            strcat(SI->Commandline,rHost->ProcData->UserOptions);
    }

   OutMemo->Lines->Add(SI->Commandline);

   SI->Window      = this->Handle;
   SI->Environment = Ref->GetEnvironment();
   SI->EnvSize     = Ref->GetEnvSize();
   SI->WorkingDir  = rHost->ProcData->WorkingDir;
   if(rHost->ProcData->LoadProfile ==1)
   	SI->CreationFlags = CREATE_WITH_USERPROFILE;
   else
   	SI->CreationFlags = 0;
   //enhancement with priority class

   SI->CreationFlags = SI->CreationFlags +(rHost->ProcData->PriorityClass);

   if(!SI->WorkingDir) SI->WorkingDir  = "";

   //LoginDlg->GetLoginData(Server);

   Connections=2;
   Params = (ThreadParams*)malloc(sizeof(ThreadParams));
   Params->Server = (HostData*)*Ref;
   Params->SI = SI;
   Params->Locked=((*Ref)->ProcData->LockIt==1);
   state = init;
   hThread=(HANDLE)System::BeginThread(0,0,(TThreadFunc)ThreadFunc,Params,0,tID);
   if(!hThread) {
    result=GetLastError();
  	free(Params);
    free(SI);
    ErrMemo->Lines->Add("Could not create thread.");
    ErrMemo->Lines->Add(MakeErrorMessage(result));
	state=error;
   } else CloseHandle(hThread);

   return ;
}



void __fastcall TClientForm::OutMemoKeyPress(TObject *Sender, char &Key)
{
   if(state != running)
     return;
   if (Key == 0x03)
      return; //CTRL-C
   if (Key == 0x16)
   {
      char * ClipboardEntry = NULL;
      int ClipSize = 0;
      ClipboardEntry = GetClipboardEntry();
      if (ClipboardEntry == NULL)
        return;
      ClipSize = strlen(ClipboardEntry);
      for (int i=0;i<ClipSize;i++)
      {
          SendInputKey(hRemoteProc,&ClipboardEntry[i],1);
          if (EchoInput1->Checked)
          {
             AnsiString hstr;
             hstr = OutMemo->Lines->Strings[OutMemo->Lines->Count -1];
             hstr = hstr + ClipboardEntry[i];
             OutMemo->Lines->Strings[OutMemo->Lines->Count -1] = hstr;
          }
      }
      free(ClipboardEntry);
      return; //CTRL-V
      }
   if(Key==13) {
     SendInputKey(hRemoteProc,"\r\n",2);
     if (EchoInput1->Checked)
         OutMemo->Lines->Add("");
   } else
   {
     SendInputKey(hRemoteProc,&Key,1);
     if (EchoInput1->Checked)
     {
         AnsiString hstr;
         hstr = OutMemo->Lines->Strings[OutMemo->Lines->Count -1];
         hstr = hstr + Key;
         OutMemo->Lines->Strings[OutMemo->Lines->Count -1] = hstr;
     }
   }

}

struct KillParams {
	HWND Window;
    HANDLE hRemoteProc;
};

DWORD KillThread(KillParams *Params) {
	DWORD Status;
    if(!Params) return 1;
	Status=KillRemoteProcess(Params->hRemoteProc);
    if(Status != ERROR_SUCCESS)
    	SendMessage(Params->Window,KILL_FAILED,0,(LPARAM)Status);
    free(Params);
    return 0;
}

void __fastcall TClientForm::KillFailed(TMessage Msg) {
	ErrMemo->Lines->Add("Could not terminate process");
    ErrMemo->Lines->Add(MakeErrorMessage(Msg.LParam));
	Finished();
}
//---------------------------------------------------------------------------
void __fastcall TClientForm::Kill1Click(TObject *Sender)
{
      DWORD tID;
      KillParams *Params;
      HANDLE hThread;      
      if(state==running) {
      	  Params = (KillParams*)malloc(sizeof(KillParams));
          Params->Window = Handle;
          Params->hRemoteProc = hRemoteProc;
          state=killing;
          Cursor=crHourGlass;
          OutMemo->Cursor=crHourGlass;
          ErrMemo->Cursor=crHourGlass;
          ErrBox->Cursor=crHourGlass;
          Caption = AnsiString((*Ref)->Name) + " -- Killing --" ;
          MainWindow->SpeedButton2->Enabled=false;
          Kill1->Enabled=false;
          StatusBar1->SimpleText="Trying to terminate remote process";
          hThread=CreateThread(0,0,(LPTHREAD_START_ROUTINE)KillThread,Params,0,&tID);
          if(!hThread) {
            free(Params);
          	SendMessage(Handle,KILL_FAILED,0,(LPARAM)GetLastError());
          } else CloseHandle(hThread);
          //KillRemoteProcess(hRemoteProc);
      }
}
//---------------------------------------------------------------------------
void __fastcall TClientForm::OutMemoEnter(TObject *Sender)
{
	 int pos;
     MainWindow->SpeedButton2->Enabled=(state==running);
     MainWindow->StartButton->Enabled=(state!=running);
     MainWindow->Run1->Enabled=(state!=running);

     OutMemo->SetFocus();
     pos=MainWindow->NodesBox->Items->IndexOfObject(Ref);
     if(pos>=0)
      	MainWindow->NodesBox->ItemIndex = pos;
}
//---------------------------------------------------------------------------
void __fastcall TClientForm::ShowStderr1Click(TObject *Sender)
{
     ShowStderr1->Checked=!ShowStderr1->Checked;
     ErrBox->Visible=ShowStderr1->Checked;
     PrintStderr1->Enabled=ShowStderr1->Checked;
}
//---------------------------------------------------------------------------
void TClientForm::applyFont() {
  OutMemo->Font=MemoFont;
  ErrMemo->Font=MemoFont;
}
void __fastcall TClientForm::ErrBoxMouseDown(TObject *Sender,
   TMouseButton Button, TShiftState Shift, int X, int Y)
{
   if(Y<=2||Y>=12) return;
   Splitting=true;
   SetCapture(ErrBox->Handle);
}
//---------------------------------------------------------------------------
void __fastcall TClientForm::ErrBoxMouseUp(TObject *Sender, TMouseButton Button,
   TShiftState Shift, int X, int Y)
{
	if(!Splitting) return;
   Splitting=false;
   ReleaseCapture();
   ratio=(double)(OutMemo->Height)/(double)(ErrMemo->Height);

}
//---------------------------------------------------------------------------
void __fastcall TClientForm::ErrBoxMouseMove(TObject *Sender, TShiftState Shift,
   int X, int Y)
{
   if(Splitting) {
     if(ErrBox->Top+Y<OutMemo->Top) Y=OutMemo->Top-ErrBox->Top;
     else if(ErrBox->ClientHeight-Y<15) Y=ErrBox->ClientHeight-15;

     ErrBox->Height-=Y;
   } else if(Y>2&&Y<12) ErrBox->Cursor=crVSplit;
          else ErrBox->Cursor=crDefault;

}
//---------------------------------------------------------------------------
void __fastcall TClientForm::FormCreate(TObject *Sender)
{

  MainWindow->SpeedButton1->Enabled=false;
  state=finished;
  //MainWindow->StartButton->Enabled=false;
  MainWindow->SpeedButton4->Enabled=false;
  MainWindow->Auswaehlen->Enabled=false;
  MainWindow->LoadConfig1->Enabled=false;
  MainWindow->SaveConfig1->Enabled=false;
  MainWindow->KillAll1->Enabled=true;
  MainWindow->CloseAll1->Enabled=true;
  MainWindow->SpeedButtonCloseAll->Enabled=true;
  MainWindow->Tile1->Enabled=true;
  MainWindow->Cascade1->Enabled=true;
  MainWindow->Clearall1->Enabled=true;
  ratio=(double)(OutMemo->Height)/(double)(ErrMemo->Height) ;
  CloseAll = false;
}
//---------------------------------------------------------------------------
void __fastcall TClientForm::FormResize(TObject *Sender)
{
   int newSize=(OutBox->ClientHeight-10)/(ratio+1);
   if(newSize<15) newSize=15;
   ErrBox->Height=newSize;
}
//---------------------------------------------------------------------------
void __fastcall TClientForm::FormDestroy(TObject *Sender)
{
   if(MainWindow->MDIChildCount==1) {
     MainWindow->SpeedButton1->Enabled=true;
     MainWindow->StartButton->Enabled=true;
     MainWindow->Run1->Enabled=true;
     MainWindow->SpeedButton4->Enabled=true;
     MainWindow->Auswaehlen->Enabled=true;
     MainWindow->LoadConfig1->Enabled=true;
     MainWindow->SaveConfig1->Enabled=true;
     MainWindow->KillAll1->Enabled=false;
     MainWindow->CloseAll1->Enabled=false;
     MainWindow->SpeedButtonCloseAll->Enabled=false;
     MainWindow->Tile1->Enabled=false;
     MainWindow->Cascade1->Enabled=false;
     MainWindow->Clearall1->Enabled=false;
   }
   //DeleteCriticalSection(&ClientCS);
}
//---------------------------------------------------------------------------



void __fastcall TClientForm::ClearOutput1Click(TObject *Sender)
{
     OutMemo->Lines->BeginUpdate();
     ErrMemo->Lines->BeginUpdate();
     OutMemo->Lines->Clear();
     ErrMemo->Lines->Clear();
     OutMemo->Lines->EndUpdate();
     ErrMemo->Lines->EndUpdate();

}
//---------------------------------------------------------------------------



void __fastcall TClientForm::PrintStdout1Click(TObject *Sender)
{
     bool DoPrint;

     DoPrint = PrintDialog1->Execute();
     if (DoPrint)
     {
     	TPrinter * Prntr = Printer();
        int height = abs(Prntr->Canvas->Font->Height);
        int PageTop = abs(Prntr->Canvas->ClipRect.Top);
        int PageBottom = abs(Prntr->Canvas->ClipRect.Bottom);
        int PageBorder = 10;
        int PageClientHight = PageBottom - PageTop - (2* PageBorder);
        int j=0;
        int count = OutMemo->Lines->Count;
     	Prntr->BeginDoc();                             // Übertragungsbeginn
        for (int i = 0; i<count;i++)
        {
        	if (((height*j*1.5)+height) >  PageClientHight)
            {
              Prntr->NewPage();
              j=0;
              }
  	    	Prntr->Canvas->TextOut(PageBorder,PageBorder+(height*j*1.5),(OutMemo->Lines->Strings[i]));
            j++;
        }
  		Prntr->EndDoc();
     }
     
}
//---------------------------------------------------------------------------
void __fastcall TClientForm::PrintStderr1Click(TObject *Sender)
{
	bool DoPrint;

     DoPrint = PrintDialog1->Execute();
     if (DoPrint)
     {
     	TPrinter * Prntr = Printer();
        int height = abs(Prntr->Canvas->Font->Height);
        int PageTop = abs(Prntr->Canvas->ClipRect.Top);
        int PageBottom = abs(Prntr->Canvas->ClipRect.Bottom);
        int PageBorder = 10;
        int PageClientHight = PageBottom - PageTop - (2* PageBorder);
        int j=0;
        int count = ErrMemo->Lines->Count;
     	Prntr->BeginDoc();                             // Übertragungsbeginn
        for (int i = 0; i<count;i++)
        {
        	if (((height*j*1.5)+height) >  PageClientHight)
            {
              Prntr->NewPage();
              j=0;
              }
  	    	Prntr->Canvas->TextOut(PageBorder,PageBorder+(height*j*1.5),(ErrMemo->Lines->Strings[i]));
            j++;
        }
  		Prntr->EndDoc();
     }
}
//---------------------------------------------------------------------------
void __fastcall TClientForm::SaveStdout1Click(TObject *Sender)
{
	bool DoSave;
     DoSave = SaveDialog1->Execute();
     if (DoSave)
     {
		AnsiString  FileName;
        FileName = SaveDialog1->FileName;
		OutMemo->Lines->SaveToFile(FileName);
    }
}
//---------------------------------------------------------------------------
void __fastcall TClientForm::SaveStderr1Click(TObject *Sender)
{
	bool DoSave;
     DoSave = SaveDialog1->Execute();
     if (DoSave)
     {
		AnsiString  FileName;
        FileName = SaveDialog1->FileName;
		ErrMemo->Lines->SaveToFile(FileName);
    }
}
//---------------------------------------------------------------------------
void __fastcall TClientForm::EchoInput1Click(TObject *Sender)
{
	EchoInput1->Checked=!EchoInput1->Checked;
}
//---------------------------------------------------------------------------
