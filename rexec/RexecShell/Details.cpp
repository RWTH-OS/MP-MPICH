//---------------------------------------------------------------------------
#include <vcl\vcl.h>
#pragma hdrstop

#include <malloc.h>
#include "Details.h"
#include "NetState.h"
#include "PluginManager.h"
#include "ProcWindow.h"

#if (BCBVER > 1)
   //CBuilder5
   #include <stdio.h>
#endif


//---------------------------------------------------------------------------
#pragma resource "*.dfm"
TDetailForm *DetailForm;
extern "C" char *GetHostAccountName(HostData *Server,char *buffer,DWORD size);
//---------------------------------------------------------------------------
__fastcall TDetailForm::TDetailForm(TComponent* Owner)
	: TForm(Owner)
{
}

extern char *MakeErrorMessage(DWORD ErrorId);
void TDetailForm::CreateDesc(HostData *Data) {
	char *pos;
    char *UserString;
    DWORD StringSize=0,i;
    UserString=0;
    char User[256],IP[20];
    R_MACHINE_INFO *MInfo;
    AnsiString txt;
    TListItem *Item;
    PlgDesc *ActivePlugin = PluginManager.GetActualPlugin();

    ActHost= Data;
    if(!Data->State) TabSheet2->TabVisible = false;
    else TabSheet2->TabVisible = true;
	Memo1->Lines->Clear();
    Caption = AnsiString("Details for host ")+AnsiString(Data->Name);
    if(!Data || !Data->State) {
    	Memo1->Lines->Add("No state information available");
        Memo1->Lines->Add("");
		if(Data->LastError != ERROR_SUCCESS) {
        	Memo1->Lines->Add("Last connection error:");
            Memo1->Lines->Add(MakeErrorMessage(Data->LastError));
        }
        OsInfo->Visible = false;
        HwPanel->Visible = false;
        AccountPanel->Visible = false;
        IPList->Visible = false;
    	return;
    } else {
        AccountPanel->Visible = true;
    }

    if(!Data->State->Configuration) {
   	    OsInfo->Visible = false;
        HwPanel->Visible = false;
    } else {
		MInfo = (R_MACHINE_INFO*)(Data->State->Configuration);
		MessageLabel->Caption = (char*)MInfo->ServerString;
//        Memo1->Lines->Add((char*)MInfo->ServerString);
    	NoProcs->Caption = AnsiString((int)MInfo->HW.dwNumberOfProcessors);
        PhysMem->Caption = AnsiString((int)MInfo->HW.dwTotalPhysMem/1024)+" KB";
        switch(MInfo->HW.wProcessorArchitecture) {
        case PROCESSOR_ARCHITECTURE_INTEL :
        		ProcType->Caption = "x86 Family "; break;
        case PROCESSOR_ARCHITECTURE_ALPHA :
        		ProcType->Caption = "Alpha Family "; break;
        case PROCESSOR_ARCHITECTURE_MIPS :
        		ProcType->Caption = "MPIS Family "; break;
        case PROCESSOR_ARCHITECTURE_PPC :
        		ProcType->Caption = "PowerPC Family "; break;
        default: ProcType->Caption = "Unknown Family "; break;
        }
		ProcType->Caption = ProcType->Caption+
            AnsiString((int)MInfo->HW.wProcessorLevel)+" Model ";
		ProcType->Caption = ProcType->Caption+
            AnsiString((int)(MInfo->HW.wProcessorRevision>>8))+" Stepping ";
				ProcType->Caption = ProcType->Caption+
            AnsiString((int)(MInfo->HW.wProcessorRevision&0xFF))+
            "  (~";
        ProcType->Caption = ProcType->Caption+
            AnsiString((int)(MInfo->HW.Mhz))+" MHz)" ;

        switch(MInfo->OS.dwPlatformId) {
        case VER_PLATFORM_WIN32_NT:
        	OsLabel->Caption = "Windows NT ";
            OsLabel->Caption = OsLabel->Caption +
            	AnsiString((int)MInfo->OS.dwMajorVersion)+AnsiString(".")+
                AnsiString((int)MInfo->OS.dwMinorVersion);
            break;

        case VER_PLATFORM_WIN32_WINDOWS:
			if(!MInfo->OS.dwMinorVersion)
	            OsLabel->Caption = "Windows 95 ";
            else
    	        OsLabel->Caption = "Windows 98 ";
            break;
        default:
        	OsLabel->Caption = "Win32 "; break;
        }
		OsLabel->Caption = OsLabel->Caption+AnsiString("  (Build ")+
        	AnsiString((int)(MInfo->OS.dwBuildNumber));
            if(MInfo->OS.szCSDVersion[0])
           		OsLabel->Caption = OsLabel->Caption+AnsiString(": ")+
                				  AnsiString((char*)MInfo->OS.szCSDVersion);
            OsLabel->Caption = OsLabel->Caption+")";
        OsInfo->Visible = true;
        HwPanel->Visible = true;
    }

    if(ActivePlugin->Convert) {
    	ActivePlugin->Convert(Data->State,UserString,&StringSize);
        if(StringSize) {
        	UserString=(char*)alloca(StringSize);
            ActivePlugin->Convert(Data->State,UserString,&StringSize);
        }
    }

    ConsoleUser->Caption = Data->State->ConsoleUser;
    if(Data->State->Locked)
    	LockedAccount->Caption = Data->State->LockedBy;
    else
    	LockedAccount->Caption = "";
    ConnectAccount->Caption = GetHostAccountName(Data,User,256);
    if(UserString) {
        Memo1->Lines->Add("");
    	pos=strchr(UserString,'\n');
    	while(pos) {
        	*pos=0;
    		Memo1->Lines->Add(UserString);
        	UserString=pos+1;
        	pos=strchr(UserString,'\n');
    	}
    	Memo1->Lines->Add(UserString);
    }

    //MInfo->IP.NumEntries = 0;
    if(!MInfo->IP.NumEntries) {
    	IPList->Visible=false;
    } else {
        IPList->Visible=true;
	    for(i=0;i<MInfo->IP.NumEntries;++i) {
    		sprintf(IP,"%d.%d.%d.%d",ANET(MInfo->IP.IPS[i]),
        	                         BNET(MInfo->IP.IPS[i]),
            	                     CNET(MInfo->IP.IPS[i]),
                	                 HPART(MInfo->IP.IPS[i]));


		    Item = IPList->Items->Add();
		    Item->Caption = AnsiString(IP);
            sprintf(IP,"%.2lf",(double)MInfo->IP.Speeds[i]/1.e6);
		    Item->SubItems->Add(IP);
    	}
    }
}
//---------------------------------------------------------------------------
void __fastcall TDetailForm::Button1Click(TObject *Sender)
{
	Close();
}
//---------------------------------------------------------------------------
void __fastcall TDetailForm::FormClose(TObject *Sender, TCloseAction &Action)
{
	Action=caFree;	
}
//---------------------------------------------------------------------------
void __fastcall TDetailForm::FormCreate(TObject *Sender)
{
	PWindow = new TProcForm(this);
    PWindow->Visible = false;
    PWindow->Panel1->Parent = TabSheet2;
    PWindow->Button1->OnClick = Button1->OnClick;
}
//---------------------------------------------------------------------------
void __fastcall TDetailForm::PageControl1Change(TObject *Sender)
{
	if(PageControl1->ActivePage == TabSheet2 && ActHost->State) {
   		GetHostProcs(PWindow->Handle,ActHost);
    }
}
//---------------------------------------------------------------------------
void __fastcall TDetailForm::FormDestroy(TObject *Sender)
{
	PWindow->Panel1->Parent = PWindow;
	PWindow->Close();
}
//---------------------------------------------------------------------------
