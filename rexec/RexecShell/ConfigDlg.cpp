//---------------------------------------------------------------------------
#define UNICODE
#include <vcl\vcl.h>

#include <fstream.h>
#include <dos.h>
#include <malloc.h>

#pragma hdrstop

#include "Main.h"
#include "ConfigDlg.h"
#include "LoginData.h"
#include "..\mpirun\plugins\Plugin.h"
#include "Details.h"
#include "Parform.h"
#include "rexecclient.h"
#include "IndividualConfig.h"
#include "FileFunctions.h"
#include "RexHelp.h"

//---------------------------------------------------------------------------
#pragma resource "*.dfm"




TConfDlg *ConfDlg;
//PlgDesc *ActivePlugin;

AnsiString str;

char *MakeErrorMessage(DWORD ErrorId);

//---------------------------------------------------------------------------
__fastcall TConfDlg::TConfDlg(TComponent* Owner)
   : TForm(Owner)
{
	/*ActivePlugin=0;
    UpdateSelState = false;  */
}
//---------------------------------------------------------------------------
// The message handlers
//---------------------------------------------------------------------------
void __fastcall TConfDlg::ThreadExit(TMessage &Message) {
    if(Message.Msg==REFRESH_FINISH) {
		HostBox->Cursor=crDefault;
    	AddBtn->Enabled=true;
    	RefreshButton->Enabled=true;
        HostBox->Enabled = true;
        if(Incomplete) {
            HostBox->Clear();
        	Servers->EnumData(Handle);
        } else {
        	RemoveInvalidHosts();
            if(UpdateSelState) {
	            UpdateSelState = false;
                UpdateParams();
            }
        }
    } else {
    	Incomplete=false;
        RemoveInvalidHosts();
        if(UpdateSelState) {
	    	UpdateSelState = false;
            UpdateParams();
        }
    }
   	if(HostBox->Items->Count) HostBox->Selected[0]=true;

}

void __fastcall TConfDlg::ThreadStart(TMessage &Message) {
	if(Message.Msg==REFRESH_START) {
    	HostBox->Clear();
		HostBox->Cursor=crHourGlass;
    	AddBtn->Enabled=false;
        RefreshButton->Enabled=false;
        HostBox->Enabled = false;
    }
}

void __fastcall TConfDlg::ParData(TMessage &Message) {
    HostData *Data;
    char Account[256];
    int index;
    Data=(HostData*)Message.LParam;

    index=HostBox->Items->IndexOf(Data->Name);
   	HostBox->Items->BeginUpdate();
    if(index<0) {
    	HostBox->Items->Add(Data->Name);
    }

    //if(Data && Data->State) {
    if(Data && Data->State && ActivePlugin) {
	    if(ActivePlugin->NewData)
		   	Data->State->Valid = ActivePlugin->NewData(Data);
	    else
    		Data->State->Valid = TRUE;
    }

	if(Data->State) Data->Alive=Data->State->Valid;
    else {
    	Data->Alive=FALSE;
        HostBox->Items->EndUpdate();
    	return;
    }


    if(Data->State->Locked) {
    	Data->Alive &=
        (stricmp(Data->State->LockedBy,GetHostAccountName(Data,Account,256))==0);
    }
    HostBox->Items->EndUpdate();
}

void __fastcall TConfDlg::OnGetFont(TMessage &Message) {
	Message.Result=(long)(Font->Handle);
}

void __fastcall TConfDlg::OnPluginChange(TMessage &Message) {
	int i;
    CHostRef *Host;
	ActivePlugin = PluginManager.GetActualPlugin();

    FreeContext(&GlobalConfig);
    //if(Message.WParam) {
	    for(i=0;i<SelectedBox->Items->Count;++i) {
    		Host = (CHostRef *)SelectedBox->Items->Objects[i];
        	FreeContext(*Host);
    	}

    	for(i=0;i<LastList->Count;++i) {
    		Host = (CHostRef *)LastList->Objects[i];
        	FreeContext(*Host);
    	}
    //}

    if(Message.WParam) {
    	UpdateSelState = true;
    	RefreshButtonClick(0);
    } else  {
    	Servers->EnumData(Handle);
        UpdateParams();
        HostBox->Refresh();
    }

}

//---------------------------------------------------------------------------
// TConfDlg members
//---------------------------------------------------------------------------

TStrings * __fastcall TConfDlg::GetHostList() {
	return LastList;
}

//---------------------------------------------------------------------------
void __fastcall TConfDlg::FormCreate(TObject *Sender)
{
	ActivePlugin=0;
    UpdateSelState = false;
    
	Incomplete=false;
    Servers->RegisterClientWindow(Handle);
    PluginManager.RegisterClientWindow(Handle);
    memset(&GlobalConfig,0,sizeof(GlobalConfig));
	Config = new TIConfigDlg(this);
    Config->PageControl1->Parent = Panel3;
    Config->PageControl1->Align = alClient;
    Config->GetConfig(&GlobalConfig);
    ActivePlugin = PluginManager.GetActualPlugin();
    LastList = new TStringList;
    LastList->Sorted=false;
}

static void ListObjectCmpDel(TStrings *l1,TStrings *l2,int index,bool both) {
	TObject *ref;
    int id;
	ref = l1->Objects[index];
    if(ref) {
    	id = l2->IndexOfObject(ref);
        if(both && id>=0) l2->Delete(id);
      	l1->Objects[index]=0;
       	if(both || id<0) delete ref;
    }
   	l1->Delete(index);
}

inline void __fastcall TConfDlg::RemoveSelHost(int index,bool both) {
	ListObjectCmpDel(SelectedBox->Items,LastList,index,both);
}

void TConfDlg::RemoveInvalidHosts() {
	int i=0,count;
    HostData* actData;
    count = SelectedBox->Items->Count;
    while(i<SelectedBox->Items->Count) {
    	actData=(*Servers)[SelectedBox->Items->Strings[i].c_str()];
        if(!actData||!actData->Alive||!(actData->State)||!actData->State->Valid) {
        	RemoveSelHost(i,true);
        }
        else i++;
    }
    // Probably we changed the selected hosts.
    // So refresh the information for the plugin...
    if(count != SelectedBox->Items->Count) {
    	UpdateParams();
        UpdateSelState = false;
    }
}


//---------------------------------------------------------------------------
void __fastcall TConfDlg::FormActivate(TObject *Sender)
{
    CHostRef *host;
    Config->SetConfig(&GlobalConfig,GLOBAL);
    Config->Refresh2Click(this);
	// Remove the configuration data from hosts
    // That have a copy of the global config.
   	for(int i =0;i<LastList->Count;++i) {
    	host = (CHostRef*)(LastList->Objects[i]);
        if(host) host->RemoveConfigRefs();
    }

    SelectedBox->Items->Assign(LastList);
    if(!Servers->Refreshing) {
		Incomplete=false;
    	HostBox->Clear();
    	Servers->EnumData(Handle);
    } else {
    	Incomplete=true;
    }
}
//---------------------------------------------------------------------------
void __fastcall TConfDlg::SelectedBoxDragOver(TObject *Sender, TObject *Source,
   int X, int Y, TDragState State, bool &Accept)
{
   if(Source==HostBox) Accept=true;
   else Accept=false;
}
//---------------------------------------------------------------------------
void __fastcall TConfDlg::SelectedBoxDragDrop(TObject *Sender, TObject *Source,
   int X, int Y)
{
   if(Source != HostBox) return;
   AddBtnClick(Source);

}
//---------------------------------------------------------------------------
void __fastcall TConfDlg::AddBtnClick(TObject *Sender)
{
   int i,lastSelected=-1,nextSelected = -1;
   HostData *actHost;
   for (i=0;i<HostBox->Items->Count;i++) {
     if(HostBox->Selected[i]) {
        actHost=(*Servers)[HostBox->Items->Strings[i].c_str()]; //get Host with Name c_str()
    	if(!actHost||!actHost->Alive||!(actHost->State) ||!(actHost->State->Valid))
           	return;
        lastSelected=i;
        HostBox->Selected[i]=false;
        SelectedBox->Items->AddObject(HostBox->Items->Strings[i],new CHostRef(CopyHost(0,actHost)));
     }
   }

  //if(lastSelected==i-1) lastSelected=-1;
  //HostBox->Selected[lastSelected+1]=true;

  	 //set next valid host as selected
     i = lastSelected;
     do {
          i = (i + 1) % HostBox->Items->Count;
          actHost=(*Servers)[HostBox->Items->Strings[i].c_str()]; //get Host with Name c_str()
    	  if(actHost&&actHost->Alive&&(actHost->State) &&(actHost->State->Valid))
           	nextSelected = i;
     } while (nextSelected == -1);
     HostBox->Selected[nextSelected]=true;
  
  UpdateParams();
}
//---------------------------------------------------------------------------
void __fastcall TConfDlg::RemoveBtnClick(TObject *Sender)
{
   int i=0,LastRemoved=0;
   SelectedBox->Items->BeginUpdate();
   while(SelectedBox->SelCount) {
        if(SelectedBox->Selected[i]) {
           RemoveSelHost(i,false);
           LastRemoved = i;
        } else i++;
   }

   if(SelectedBox->Items->Count<=LastRemoved)
   	LastRemoved=0;
   if(SelectedBox->Items->Count>0)
   		SelectedBox->Selected[LastRemoved] = true;
   SelectedBox->Items->EndUpdate();
   UpdateParams();
   Servers->EnumData(Handle);
}
//---------------------------------------------------------------------------

void TConfDlg::UpdateParams() {
	DWORD size=SelectedBox->Items->Count;
    int index;
    DWORD NewIndex,i;
    HostData **Hosts;
    CHostRef *actHost;

  if(ActivePlugin->SelectedChange) {
  	Hosts=(HostData**)malloc(sizeof(HostData*)*size);
    index=0;
    for(i=0;i<size;i++)  {
        actHost=(CHostRef*)SelectedBox->Items->Objects[i];
    	Hosts[index++]=(HostData*)*actHost;
    }

    NewIndex=index;
  	ActivePlugin->SelectedChange(Servers->GetList(),Servers->Count(),
        						 Hosts,&NewIndex,&GlobalConfig);
    if(NewIndex != (DWORD)(abs(index))) {
	    SelectedBox->Items->BeginUpdate();
        index = 0;
		for(i=0; i<NewIndex;++i) {
	        actHost=(CHostRef*)SelectedBox->Items->Objects[index];
            if((HostData*)*actHost != Hosts[i])
                RemoveSelHost(index,false);
            else
                ++index;
        }
        while((DWORD)(abs(SelectedBox->Items->Count)) > NewIndex)
            RemoveSelHost(NewIndex,false);
        SelectedBox->Items->EndUpdate();
    }

    if(!Incomplete) HostBox->Refresh();
    free(Hosts);
  }
}
//---------------------------------------------------------------------------
void __fastcall TConfDlg::HostBoxDragOver(TObject *Sender, TObject *Source,
   int X, int Y, TDragState State, bool &Accept)
{
   if(Source==SelectedBox) Accept=true;
   else Accept=false;
}
//---------------------------------------------------------------------------
void __fastcall TConfDlg::HostBoxDragDrop(TObject *Sender, TObject *Source,
   int X, int Y)
{
   if(Source != SelectedBox) return;
   RemoveBtnClick(Source);
}
//---------------------------------------------------------------------------

void __fastcall TConfDlg::SelectedBoxKeyDown(TObject *Sender, WORD &Key,
   TShiftState Shift)
{
   if(Key==46) RemoveBtnClick(this);
}
//---------------------------------------------------------------------------
BOOL TConfDlg::LoadConfig(const AnsiString &File) {
	HANDLE file;
    DWORD type, fileversion;
    HostData *actData;
    char PlgName[30]="NONE",*N = PlgName;
    int i;
    bool ignoreContext=false;

    file = CreateFileA(File.c_str(),GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,
                     FILE_FLAG_SEQUENTIAL_SCAN,0);
    if(file == INVALID_HANDLE_VALUE) {
  	    Application->MessageBox("Cannot open file!","Error",MB_OK|MB_ICONERROR);
	    return FALSE;
    }
    type = ReadType(file);
    if(type != MAGIC) {
    	CloseHandle(file);
        Application->MessageBox("Invalid file format!",
                                "Error",MB_OK|MB_ICONERROR);
        return FALSE;
    }
    fileversion = ReadType(file);
    if(fileversion != FILE_VERSION) {

        i = Application->MessageBox("Wrong file version! Try anyway?","Error",MB_YESNO|MB_ICONERROR);
        if (i == ID_NO)  {
          CloseHandle(file);
          return FALSE;
          }
    }

    // This should not happen...
    while(SelectedBox->Items->Count) {
       	RemoveSelHost(0,true);
    }
    for(i=0;i<LastList->Count;++i) {
    	delete LastList->Objects[i];
        LastList->Objects[i] = 0;
    }
    LastList->Clear();

    actData = &GlobalConfig;
    FreeProcData(actData);
    SetHostAccount(actData,0);

	do {
    	type = ReadType(file);
        switch(type) {
        case TYPE_CONFIG: ReadConfigData(file,actData,fileversion);
                            if(ignoreContext) FreeContext(actData);
        					break;
		case TYPE_ACCOUNT: ReadAccount(file,actData);
        					
         					break;
        case TYPE_ACCOUNT_ENCRYPTED: ReadAccountEncrypted(file,actData); break;
		case TYPE_HOST: actData = 0;
                        ReadHost(file,&actData);
                        if(actData) {
                        	CopyStateData(actData,(*Servers)[actData->Name]);
                        	LastList->AddObject(actData->Name,new CHostRef(actData));
                        }
                        break;
        case TYPE_PLUGIN: ReadString(file,&N);
        			      if(!PluginManager.SetActualPlugin(PlgName))
                          	ignoreContext = true;
                          break;

		case TYPE_VERSION:
        case TYPE_GLOBAL:
		case TYPE_NONE:
        default: break;
        }
    } while(type != TYPE_NONE);
    CloseHandle(file);
    return TRUE;
}

void TConfDlg::SaveConfig(const AnsiString &File) {

	HANDLE file;
    DWORD type,Written;
    int i;
    CHostRef *host;
    AnsiString FailedName="NONE";
    file = CreateFileA(File.c_str(),GENERIC_WRITE,0,0,CREATE_ALWAYS,
                     FILE_FLAG_SEQUENTIAL_SCAN,0);
    if(file == INVALID_HANDLE_VALUE) {
  	    Application->MessageBox("Cannot create file!","Error",MB_OK|MB_ICONERROR);
	    return;
    }

    // Remove the configuration from hosts
    // That do not have an individual configuration
    for(i =0;i<LastList->Count;++i) {
    	host = (CHostRef*)(LastList->Objects[i]);
        if(host) host->RemoveConfigRefs();
    }

    FailedName="MAGIC";
    type = MAGIC;
    if(!WriteFile(file,&type,sizeof(DWORD),&Written,0)) goto Remove;
    FailedName="FILE_VERSION";
    type = FILE_VERSION;
    if(!WriteFile(file,&type,sizeof(DWORD),&Written,0))  goto Remove;
    FailedName="Plugin";
    if(!WritePlugin(file,PluginManager.GetActualPlugin())) goto Remove;
    FailedName = "WriteConfigData";
    if(!WriteConfigData(file,&GlobalConfig)) goto Remove;
    FailedName="WriteHost";
	for(i=0;i<LastList->Count;++i) {
    	if(!WriteHost(file,(HostData*)*(CHostRef*)LastList->Objects[i]))
        	goto Remove;
    }
    //SI: 13.06.05 Write Account at last position
    //SI: 14.06.05 save without Write Account possible
    FailedName="WriteAccount";
    //if(!WriteAccount(file,&GlobalConfig)) goto Remove;
    if(!WriteAccount(file,&GlobalConfig))
      Application->MessageBox("Could not save account information in config!","Error",MB_ICONERROR|MB_OK);;

    type = TYPE_NONE;
    WriteFile(file,&type,sizeof(DWORD),&Written,0);
   	CloseHandle(file);

    for(i=0;i<LastList->Count;++i)  {
    	host=(CHostRef*)LastList->Objects[i];
        // If the host has no individual configuration then
        // set a reference to the global config.
        host->SetConfigRef(&GlobalConfig);
    }
    return;
Remove:
	CloseHandle(file);
    DeleteFileA(File.c_str());
    FailedName = FailedName + " failed. Could not save config!";
    Application->MessageBox(FailedName.c_str(),"Error",MB_ICONERROR|MB_OK);
	return;
}

void __fastcall TConfDlg::RefreshButtonClick(TObject *Sender)
{

    TMessage m;
    m.Msg=REFRESH_START;
    Incomplete=true;
    if(!Servers->Refresh()) {
    	ThreadStart(m);
    } else Incomplete=false;
}
//---------------------------------------------------------------------------
void __fastcall TConfDlg::HostBoxDrawItem(TWinControl *Control, int Index,
	TRect &Rect, TOwnerDrawState State)
{
	int Offset = 2;// default offset
    TColor FC,BC,PC;
	TListBox *ListBox = dynamic_cast<TListBox*>(Control);
	TCanvas *Canvas = ListBox->Canvas;
    BC=Canvas->Brush->Color;
    PC=Canvas->Pen->Color;
    Canvas->Brush->Color=clWindow;
    HostData *actHost;

	// display the text
   FC=Canvas->Font->Color;
   if(!Servers->Refreshing) {
	   actHost=(*Servers)[ListBox->Items->Strings[Index].c_str()];
	   	if(!(actHost)||!actHost->Alive||!(actHost->State) || !(actHost->State->Valid))
   			Canvas->Font->Color=clRed;
   		else
   			Canvas->Font->Color=clWindowText;
    } else Canvas->Font->Color=clGrayText;

   if((ListBox->Focused() || AddBtn->Focused()) && State.Contains(odSelected)) {
   		Canvas->Brush->Color=Canvas->Font->Color;
        Canvas->Font->Color=clHighlightText;
   }
   Canvas->Pen->Color=Canvas->Brush->Color;
   Canvas->FillRect(Rect);                       // clear the rectangle
   Canvas->TextOut(Rect.Left + Offset, Rect.Top, ListBox->Items->Strings[Index]);
   Canvas->Font->Color=FC;
   Canvas->Brush->Color=BC;
   Canvas->Pen->Color=PC;
}
//---------------------------------------------------------------------------

void __fastcall TConfDlg::SelectedBoxDrawItem(TWinControl *Control, int Index,
	TRect &Rect, TOwnerDrawState State)
{
	int Offset = 2;// default offset
    TColor FC,BC,PC;
	TListBox *ListBox = dynamic_cast<TListBox*>(Control);
	TCanvas *Canvas = ListBox->Canvas;
    BC=Canvas->Brush->Color;
    PC=Canvas->Pen->Color;
    FC = Canvas->Font->Color;
    Canvas->Brush->Color=clWindow;
    CHostRef *actHost;
    TFontStyles FS;


    FS = Canvas->Font->Style;
    actHost=(CHostRef *)ListBox->Items->Objects[Index];
    if((*actHost)->ProcData || (*actHost)->Account)
   		Canvas->Font->Style = FS<<fsBold;

	Canvas->Font->Color=clWindowText;
   	if((ListBox->Focused() || RemoveBtn->Focused()) && State.Contains(odSelected)) {
   		Canvas->Brush->Color=Canvas->Font->Color;
        Canvas->Font->Color=clHighlightText;
   	}
   	Canvas->Pen->Color=Canvas->Brush->Color;
   	Canvas->FillRect(Rect);                       // clear the rectangle
   	Canvas->TextOut(Rect.Left + Offset, Rect.Top, ListBox->Items->Strings[Index]);
   	Canvas->Font->Color=FC;
   	Canvas->Brush->Color=BC;
   	Canvas->Pen->Color=PC;
   	Canvas->Font->Style=FS;
}
//---------------------------------------------------------------------------
void __fastcall TConfDlg::FormDestroy(TObject *Sender)
{
	delete Config;
    Servers->RemoveClientWindow(Handle);
    PluginManager.RemoveClientWindow(Handle);
}
//---------------------------------------------------------------------------
void __fastcall TConfDlg::Refresh1Click(TObject *Sender)
{
	int index=HostBox->ItemIndex;
    if(index<0) return;
	Servers->Refresh(HostBox->Items->Strings[index].c_str());
    FormActivate(0);
}
//---------------------------------------------------------------------------
void __fastcall TConfDlg::ShowDetails1Click(TObject *Sender)
{
	HostData *actData;
    TDetailForm *DetailForm;
	int index=HostBox->ItemIndex;
    if(index<0) return;
    actData=(*Servers)[HostBox->Items->Strings[index].c_str()];
    if(!actData) return;
    DetailForm = new TDetailForm(this);
    DetailForm->CreateDesc(actData);
	DetailForm->Show();
}
//---------------------------------------------------------------------------
void __fastcall TConfDlg::PopupMenu1Popup(TObject *Sender)
{
	HostData *actData;
    int index=HostBox->ItemIndex;

    //if(index<0||(!(actData=(*Servers)[HostBox->Items->Strings[index].c_str()])))

    actData = NULL;

    if (index >= 0)
        actData=(*Servers)[HostBox->Items->Strings[index].c_str()];

    if(index<0||(!actData)) {
        Select1->Enabled=false;
        ShowDetails1->Enabled=false;

    } else {
        Select1->Enabled= (actData->Alive && actData->State && actData->State->Valid);
        ShowDetails1->Enabled=true;
    }

}

//---------------------------------------------------------------------------
void __fastcall TConfDlg::SpeedButton2Click(TObject *Sender)
{
	ParWindow->Show();
}
//---------------------------------------------------------------------------
void __fastcall TConfDlg::HostBoxMouseDown(TObject *Sender, TMouseButton Button,
	TShiftState Shift, int X, int Y)
{
	int id;
	POINT p={X,Y};
    TListBox *Box;
    Box = (TListBox*)Sender;
	if(Button == mbRight) {
    	id=Box->ItemAtPos(p,true);
        if(id>=0) {
        	for (int i=0;i<Box->Items->Count;++i)
			    Box->Selected[i]= false;
        	Box->Selected[id]=true;
        }
    }
}

static void MergeAccounts(TAccount **dst,TAccount *src) {
	if(!dst || !*dst) return;

    if(!src)  {
    	memset(*dst,0,sizeof(TAccount));
    	free(*dst);
        *dst = 0;
    	return;
    }

    if(strcmp((*dst)->User,src->User)) (*dst)->User[0]=0;
    if(strcmp((*dst)->Domain,src->Domain) )(*dst)->Domain[0]=0;
    if(strcmp((*dst)->Password,src->Password)) (*dst)->Password[0]=0;
}

static void MergeProcData(HostData *dst,HostData *src) {
	TProcessData *s,*d;
	if(!dst || !dst->ProcData) return;

    if(!src->ProcData)  {
    	FreeProcData(dst);
    	return;
    }
    s = src->ProcData;
    d = dst->ProcData;

    if( (d->Executable && !s->Executable)||
        (d->Executable && s->Executable && strcmp(d->Executable,s->Executable)))
    	 ProcStrRemove(&d->Executable,&d->ExeSize);

	if( (d->WorkingDir && !s->WorkingDir)||
        (d->WorkingDir && s->WorkingDir && strcmp(d->WorkingDir,s->WorkingDir)))
    	 ProcStrRemove(&d->WorkingDir,&d->WDSize);

	if( (d->UserOptions && !s->UserOptions)||
        (d->UserOptions && s->UserOptions && strcmp(d->UserOptions,s->UserOptions)))
    	 ProcStrRemove(&d->UserOptions,&d->OptSize);

        if( (d->PluginOptions && !s->PluginOptions)||
        (d->PluginOptions && s->PluginOptions && strcmp(d->PluginOptions,s->PluginOptions)))
    	 ProcStrRemove(&d->PluginOptions,&d->PluginOptSize);


	if( (d->Environment && !d->Environment) ||
     	(d->Environment && s->Environment && (d->EnvSize != s->EnvSize)))
        FreeEnvironment(dst);

    if(d->LockIt != s->LockIt)
    	d->LockIt = 2;

    if(d->LoadProfile != s->LoadProfile)
    	d->LoadProfile = 2;
}


void __fastcall TConfDlg::CreateTmpConfig(HostData *tmpConfig) {
	HostData *other,def;
    int index,count;

    memset(tmpConfig,0,sizeof(HostData));
    memset(&def,0,sizeof(HostData));
    index = 0;
	while(index<SelectedBox->Items->Count && !SelectedBox->Selected[index])
    	++index;
    Config->GetConfig(&def);


    count = SelectedBox->SelCount-1;
    other = (HostData*)*(CHostRef*)SelectedBox->Items->Objects[index];
    if(!other->ProcData)
    	CopyProcData(tmpConfig,&def);
    else
    	CopyProcData(tmpConfig,other);
    if(!other->Account)
    	SetHostAccount(tmpConfig,def.Account);
    else
	    SetHostAccount(tmpConfig,other->Account);
    if(!count) {
    	CopyStateData(tmpConfig,other);
    } else {
    	++index;
    	for(;count>0;++index) {
    		if(!SelectedBox->Selected[index]) continue;
        	--count;
        	other = (HostData*)*(CHostRef*)SelectedBox->Items->Objects[index];
        	if(!other->Account) other = &def;
	        MergeAccounts(&tmpConfig->Account,other->Account);
        	if(!other->ProcData) other = &def;
    		MergeProcData(tmpConfig,other);
    	}
    }
    SetHostAccount(&def,0);
    FreeProcData(&def);

}
//---------------------------------------------------------------------------
void __fastcall TConfDlg::SelectedBoxDblClick(TObject *Sender)
{
	TIConfigDlg *IConfig;
    HostData tmpConfig,*actHost;
    int count,index;
    count = SelectedBox->SelCount;
    if(!count) return;
    CreateTmpConfig(&tmpConfig);

    IConfig = new TIConfigDlg(this);
	IConfig->SetConfig(&tmpConfig,INDIVIDUAL);
    if(IConfig->ShowModal() == mrOk) {
		//IConfig->GetConfig(&tmpConfig);
	    for(index=0;count>0;++index) {
    		if(!SelectedBox->Selected[index]) continue;
        	--count;
            actHost = (HostData*)*(CHostRef*)SelectedBox->Items->Objects[index];
            if(!actHost->ProcData) CopyProcData(actHost,&GlobalConfig);
            if(!actHost->Account) SetHostAccount(actHost,GlobalConfig.Account);
            IConfig->GetConfig(actHost);
        }
    }
    SetHostAccount(&tmpConfig,0);
    FreeProcData(&tmpConfig);

    delete IConfig;

}
//---------------------------------------------------------------------------
void __fastcall TConfDlg::OKBtnClick(TObject *Sender)
{
     char errcodestr[6];
     //try to log on actual computer with chosen account to test if it is valid


    /*
    //set given account
    AccountManager.SetActualAccount(Config->NameEdit->Text.Trim().UpperCase(),
    								Config->DomainCombo->Text.Trim().UpperCase());//Si

    //copied from main.cpp to use given account for servers
    TLoginDlg *Login;  //Si
    Login = new TLoginDlg(this);
		Servers->Lock();
    	for(int i=0;i<Servers->Count();i++) {
           Login->GetLoginData((*Servers)[i]);
    	}
        Servers->Unlock();
        Servers->Refresh();
    delete Login;
    */

	HostData **Hosts;
	int size,j,i;
    DWORD index,dw;
    CHostRef *actHost;

    index=0;
	Config->GetConfig(&GlobalConfig);
    while(LastList->Count)
    	ListObjectCmpDel(LastList,SelectedBox->Items,0,false);

    LastList->Assign(SelectedBox->Items);
    SelectedBox->Items->Clear();

    size = LastList->Count;
   	for(i=0;i<size;++i)  {
    	actHost=(CHostRef*)LastList->Objects[i];
        // If the host has no individual configuration then
        // set a reference to the global config.
        actHost->SetConfigRef(&GlobalConfig);
    }

    if(ActivePlugin->DlgClose) {
	  	Hosts=(HostData**)alloca(sizeof(HostData*)*size);
    	for(i=0;i<size;++i)  {
        	actHost=(CHostRef*)LastList->Objects[i];
            EmptyEnvironment(*actHost);
    		Hosts[index++]=(HostData*)*actHost;
    	}
		ActivePlugin->DlgClose(Servers->GetList(),Servers->Count(),Hosts,&index,&GlobalConfig);

        // Resort the list according to the order created by the plugin
        for(dw=0;dw<index;++dw) {
        	for(j=dw;j<size;++j) {
            	actHost = (CHostRef*)LastList->Objects[j];
                if(((HostData*)*actHost) == Hosts[dw]) {
	                if((DWORD)j!=dw) LastList->Exchange(j,dw);
                	break;
                }
            }
        }
        // Remove all remaining hosts from the list...
        for(i=index;i<size;++i) {
        	actHost = (CHostRef*)LastList->Objects[i];
            delete actHost;
        }
    }

}
//---------------------------------------------------------------------------
void __fastcall TConfDlg::CancelBtnClick(TObject *Sender)
{
	CHostRef *actHost;
    HostData **Hosts;
    int size,i;
    DWORD index =0;
   // Discard all object references
   // that are no longer needed...
   while(SelectedBox->Items->Count)
    	ListObjectCmpDel(SelectedBox->Items,LastList,0,false);

   for(int i=0;i<LastList->Count;++i)  {
    	actHost=(CHostRef*)LastList->Objects[i];
        // If the host has no individual configuration then
        // set a reference to the global config.
        actHost->SetConfigRef(&GlobalConfig);
   }

   // Now re-create the commandline,
   // since we deleted it when we opened the dialog.
   if(ActivePlugin->DlgClose) {
   		size = LastList->Count;
	  	Hosts=(HostData**)alloca(sizeof(HostData*)*size);
    	for(i=0;i<size;++i)  {
        	actHost=(CHostRef*)LastList->Objects[i];
            EmptyEnvironment(*actHost);
    		Hosts[index++]=(HostData*)*actHost;
    	}
		ActivePlugin->DlgClose(Servers->GetList(),Servers->Count(),Hosts,&index,&GlobalConfig);
   }

}
//---------------------------------------------------------------------------

void __fastcall TConfDlg::PopupMenu2Popup(TObject *Sender)
{
	int i;
    CHostRef * actHost;
    bool check = true;
    if(SelectedBox->SelCount<1) {
      Configure_item->Enabled = false;
      Defaultconfig_item->Enabled = false;
      Defaultconfig_item->Checked = false;
      Remove_item->Enabled = false;
      return;
    }
    Configure_item->Enabled = true;
	Remove_item->Enabled = true;
    for(i=0;i<SelectedBox->Items->Count;++i) {
    	if(!SelectedBox->Selected[i]) continue;
    	actHost=(CHostRef *)SelectedBox->Items->Objects[i];
    	if((*actHost)->ProcData || (*actHost)->Account) {
        	check = false;
            break;
        }
    }
   Defaultconfig_item->Checked = check;
   Defaultconfig_item->Enabled = !check;
}
//---------------------------------------------------------------------------
void __fastcall TConfDlg::Defaultconfig_itemClick(TObject *Sender)
{
	int i;
    CHostRef * actHost;
    for(i=0;i<SelectedBox->Items->Count;++i) {
    	if(!SelectedBox->Selected[i]) continue;
    	actHost=(CHostRef *)SelectedBox->Items->Objects[i];
		if((*actHost)->ProcData)
	        FreeProcData(*actHost);
		if((*actHost)->Account)
	        SetHostAccount(*actHost,0);
    }
    SelectedBox->Invalidate();
}
//---------------------------------------------------------------------------
void __fastcall TConfDlg::AddBtnEnter(TObject *Sender)
{
	HostBox->Invalidate();
}
//---------------------------------------------------------------------------
void __fastcall TConfDlg::RemoveBtnEnter(TObject *Sender)
{
	SelectedBox->Invalidate();
}
//---------------------------------------------------------------------------
void __fastcall TConfDlg::FormClose(TObject *Sender, TCloseAction &Action)
{
	Config->PageControl1Change(0);	
}
//---------------------------------------------------------------------------
