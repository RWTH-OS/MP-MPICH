//---------------------------------------------------------------------------

#if (!(BCBVER > 1))
	//CBuilder1
	#include <vcl\shlobj.hpp>
#else
	//CBuilder5
    #define NO_WIN32_LEAN_AND_MEAN
    #include  "shlobj.h"
#endif

#include <vcl\vcl.h>

#pragma hdrstop                  

#include "IndividualConfig.h"
#include "PluginManager.h"
#include "Environment.h"
#include "RexecClient.h"
#include "ConfigDlg.h"                
#include "DirSelForm.h"
#include "AccountManager.h"
#include "LoginData.h"
#include "RexHelp.h"

//---------------------------------------------------------------------------
#pragma resource "*.dfm"
TIConfigDlg *IConfigDlg;
char *MakeErrorMessage(DWORD ErrorId);
        

//---------------------------------------------------------------------------
// Static helpers
//---------------------------------------------------------------------------
/*
static BOOL WINAPI
AccCallBack(char *Account, char *User, char* Domain, char *Password, void *Sender) {
	TStrings *items = (TStrings*)(Sender);
    int id;
    AnsiString Name;
    Name = Domain;
    Name += "/";
    Name += User;
    id = items->IndexOf(Name);
    if(id<0) {
    	id=items->Add(Name);
    } else delete items->Objects[id];
    items->Objects[id]=new TCred(User,Domain,Password);
    return TRUE;
}

*/
//---------------------------------------------------------------------------
// Message handlers
//---------------------------------------------------------------------------

void __fastcall TIConfigDlg::ThreadExit(TMessage &Message) {
	PlgDesc *plg;
	PluginCombo->Enabled = true;
    plg = PluginManager.GetActualPlugin();
    ConfPl->Enabled = (plg && plg->Configure);
}

void __fastcall TIConfigDlg::ThreadStart(TMessage &Message) {
 	PluginCombo->Enabled = false;
    ConfPl->Enabled = false;
}

void __fastcall TIConfigDlg::OnPluginChange(TMessage &Message) {
	PluginCombo->ItemIndex = Message.LParam;
    PluginComboChange(PluginCombo);
}

//---------------------------------------------------------------------------
// Normal members
//---------------------------------------------------------------------------

__fastcall TIConfigDlg::TIConfigDlg(TComponent* Owner)
	: TForm(Owner)
{
   	OldIndex = -1;
   	ProgChanged=false;
	DirChanged=false;
    CommandlineChanged = false;
	NameChanged = false;
    PasswordChanged = false;
	DomainChanged = false;
}

__fastcall TIConfigDlg::~TIConfigDlg() {

}

//---------------------------------------------------------------------------
void __fastcall TIConfigDlg::FormCreate(TObject *Sender)
{
    PlgDesc *plg;
    TCred *ActCreds;
	stHandle = Handle;
	Servers->RegisterClientWindow(stHandle);
	PluginManager.RegisterClientWindow(stHandle);

    ActConfig = (HostData*)malloc(sizeof(HostData));
    memset(ActConfig,0,sizeof(HostData));

    PluginCombo->Items->Assign(PluginManager.GetPluginList());

    plg = PluginManager.GetActualPlugin();
    ConfPl->Enabled = (plg && plg->Configure);
    PageControl1->ActivePage = BasicSheet;
    PageControl1->Pages[2]->TabVisible = false;
    PageControl1->Pages[3]->TabVisible = false;
    DomainCombo->Items->Add("LOCAL");
    PriorityCombo->ItemIndex=PriorityCombo->Items->IndexOf("NORMAL_PRIORITY_CLASS");
    //Refresh2Click(this);
	ActCreds = AccountManager.GetActualAccount();
    if(ActCreds) {
    	NameEdit->Text=ActCreds->GetName();
        SetDomain(ActCreds->GetDomain().c_str());
        PasswordEdit->Text = ActCreds->GetPassword();
    }
    /*
    if(AccountList->Items->Count > 0) {
	    LookupAccount();
    }
    // Set the initial values for the account tab
    // To be changed later...

    char UName[256],Domain[256];
    DWORD Size=256;

    GetEnvironmentVariableA("USERDOMAIN",Domain,256);
    GetUserNameA(UName,&Size);
    NameEdit->Text=UName;

    SetDomain(Domain);

    if(AccountList->Items->Count > 0) {
	    LookupAccount();
    }
    */
}

void __fastcall TIConfigDlg::SetDomain(char *d) {
	AnsiString dm;
	int id;
    dm = d;
    if(dm.Trim().IsEmpty()) id = -1;
    else {
	    dm = dm.Trim().UpperCase();
    	id = DomainCombo->Items->IndexOf(dm);
    	if(id<0)
    		id = DomainCombo->Items->Add(dm);
    }
    DomainChanged |= (DomainCombo->ItemIndex!= id);
	DomainCombo->ItemIndex= id;
}

void __fastcall TIConfigDlg::SetAccount(TCred *Cred) {
	NameEdit->Text = Cred->GetName();
    PasswordEdit->Text = Cred->GetPassword();
    SetDomain(Cred->GetDomain().c_str());
}

void __fastcall TIConfigDlg::GetAccount(TCred *Cred) {
	Cred->GetName()= NameEdit->Text.Trim().UpperCase();
	Cred->GetPassword()= PasswordEdit->Text.Trim() ;
    Cred->GetDomain() = DomainCombo->Text.Trim().UpperCase();
}

void __fastcall TIConfigDlg::SetConfig(HostData *Data,ConfMode mode) {
	ActMode = mode;
    if(!Data) return;
    CopyProcData(ActConfig,Data);
    CopyStateData(ActConfig,Data);
    if(Data->ProcData) {
	    Program->Text = (Data->ProcData->Executable?Data->ProcData->Executable:"");
   	 	WDir->Text = (Data->ProcData->WorkingDir?Data->ProcData->WorkingDir:"");
    	COptions->Text = (Data->ProcData->UserOptions?Data->ProcData->UserOptions:"");
        EPluginParams->Text = (Data->ProcData->PluginOptions?Data->ProcData->PluginOptions:"");
    	Lock->State = (TCheckBoxState)Data->ProcData->LockIt;
        LoadEnv->State = (TCheckBoxState)Data->ProcData->LoadProfile;
    } else {
    	Program->Text = "";
   	 	WDir->Text = "";
    	COptions->Text = "";
        EPluginParams->Text = "";
    	Lock->State = cbGrayed;
        LoadEnv->State = cbGrayed;
    }
    if(Data->Account) {
    	NameEdit->Text = Data->Account->User;
        PasswordEdit->Text = Data->Account->Password;
        SetDomain(Data->Account->Domain);
    } else {
    	NameEdit->Text = "";
        DomainCombo->ItemIndex=-1;
        PasswordEdit->Text = "";
    }
    // And the environment settings
    // Not yet implemented...

    // Here are state settings...

    //si new process priority
    PriorityCombo->ItemIndex=PriorityCombo->Items->IndexOf("NORMAL_PRIORITY_CLASS");
    if (Data && Data->ProcData)
    {
      if (Data->ProcData->PriorityClass == IDLE_PRIORITY_CLASS)
        PriorityCombo->ItemIndex=PriorityCombo->Items->IndexOf("IDLE_PRIORITY_CLASS");
      if (Data->ProcData->PriorityClass == HIGH_PRIORITY_CLASS)
        PriorityCombo->ItemIndex=PriorityCombo->Items->IndexOf("HIGH_PRIORITY_CLASS");
      if (Data->ProcData->PriorityClass == REALTIME_PRIORITY_CLASS)
        PriorityCombo->ItemIndex=PriorityCombo->Items->IndexOf("REALTIME_PRIORITY_CLASS");
    }

    PageControl1->ActivePage = BasicSheet;
    PluginCombo->ItemIndex = PluginManager.GetActualIndex();
    PluginCombo->Enabled = (mode != INDIVIDUAL);
    if(mode ==GLOBAL) Lock->Caption = "&Lock nodes";
    else Lock->Caption = "&Lock node";
	if(!Data->Account) //needed for CBuilder5
      LookupAccount();
    else 
	  if(!Data->Account->Password[0])
	    LookupAccount();
    if (Data->ProcData != 0) //Si
    {
        ProgChanged=(Data->ProcData->Executable!=0);
        DirChanged=(Data->ProcData->WorkingDir !=0);
        CommandlineChanged = (Data->ProcData->UserOptions !=0);
        PluginParamsChanged = (Data->ProcData->PluginOptions!=0);
    }
    else
    {
    	ProgChanged=false;
        DirChanged=false;
        CommandlineChanged = false;
        PluginParamsChanged = false;
    }
	NameChanged = false; //si
    if (Data->Account != NULL)   //si
        NameChanged = (Data->Account->User[0] !=0);
    PasswordChanged = !PasswordEdit->Text.IsEmpty();
	DomainChanged = (DomainCombo->ItemIndex>=0);
}

void __fastcall TIConfigDlg::GetConfig(HostData *Data) {
	TProcessData *d;
    if(!Data->ProcData) {
    	Data->ProcData = (TProcessData*)malloc(sizeof(TProcessData));
        memset(Data->ProcData,0,sizeof(TProcessData));
    }


    d = Data->ProcData;
    if (ActConfig)  //si
    {
      if(ActConfig->ProcData && ActConfig->ProcData->Context) {
    	if(d->ContextSize!= ActConfig->ProcData->ContextSize) {
        	d->Context = realloc(d->Context,ActConfig->ProcData->ContextSize);
        }
        memcpy(d->Context,ActConfig->ProcData->Context,ActConfig->ProcData->ContextSize);
    	d->ContextSize = ActConfig->ProcData->ContextSize;
      }
    }
    if(ProgChanged)
		ProcSetString(&d->Executable,Program->Text.Trim().c_str(),&d->ExeSize);
	if(DirChanged)
	   	ProcSetString(&d->WorkingDir,WDir->Text.Trim().c_str(),&d->WDSize);
	if(CommandlineChanged)
	    ProcSetString(&d->UserOptions,COptions->Text.Trim().c_str(),&d->OptSize);
        if(PluginParamsChanged)
	    ProcSetString(&d->PluginOptions,EPluginParams->Text.Trim().c_str(),&d->PluginOptSize);

	d->LockIt = Lock->State;
    d->LoadProfile = LoadEnv->State;
    //si new process priority
    d->PriorityClass = NORMAL_PRIORITY_CLASS; //default value
    if (PriorityCombo->Text == "IDLE_PRIORITY_CLASS" )
      d->PriorityClass = IDLE_PRIORITY_CLASS;
    if (PriorityCombo->Text == "HIGH_PRIORITY_CLASS")
      d->PriorityClass = HIGH_PRIORITY_CLASS;
    if (PriorityCombo->Text == "REALTIME_PRIORITY_CLASS")
      d->PriorityClass = REALTIME_PRIORITY_CLASS;

    if( NameChanged || DomainChanged || PasswordChanged) {
	    if(!Data->Account) {
        	Data->Account = (TAccount*)malloc(sizeof(TAccount));
			Data->Account->User[0]=0;
            Data->Account->Domain[0]=0;
            Data->Account->Password[0]=0;
        }
    	if(NameChanged)
    		strcpy(Data->Account->User,NameEdit->Text.Trim().c_str());
		if(DomainChanged)
		    strcpy(Data->Account->Domain,DomainCombo->Text.c_str());
    	if(PasswordChanged)
 	    	strcpy(Data->Account->Password,PasswordEdit->Text.c_str());
    }
    // Environment is missing :-(
    CopyProcData(ActConfig,Data);
    CopyStateData(ActConfig,Data);
}

void __fastcall TIConfigDlg::LookupAccount() {
	TCred *Cred;
    TAccountIndex i;
    AnsiString AccName;

	if(!PasswordEdit->Text.IsEmpty()) return;
    //AnsiString Name = NameEdit->Text.Trim().UpperCase();
    //AnsiString Domain = DomainCombo->Text.Trim().UpperCase();

    i=AccountManager.FindAccount(NameEdit->Text.Trim(),DomainCombo->Text.Trim());
    Cred =AccountManager.GetAccount(i,AccName);
    if(Cred) {
    	PasswordEdit->Text = Cred->GetPassword();
        PasswordChanged = true;
    }
    /*
    for(int i=0;i<AccountList->Items->Count;++i) {
    	Cred = (TCred*)AccountList->Items->Objects[i];
        if(!Cred) continue;
        if(Cred->GetName().UpperCase() == Name && Cred->GetDomain().UpperCase() == Domain)  {
        	PasswordEdit->Text = Cred->GetPassword();
            PasswordChanged = true;
        }
    }
     */
}

//---------------------------------------------------------------------------
void __fastcall TIConfigDlg::SpeedButton1Click(TObject *Sender)
{
     char *bs;
     char d[MAX_PATH];
     DWORD len = MAX_PATH;
     if(OpenDialog->Execute()) {
       if(OpenDialog->FileName[2] == ':') {
	       strcpy(d,OpenDialog->FileName.SubString(1,2).c_str());
    	   if(WNetGetConnection(d,d,&len) != NO_ERROR)
	    	   strcpy(d,OpenDialog->FileName.c_str());
	       else {
    	   		strcat(d,OpenDialog->FileName.SubString(3,
                OpenDialog->FileName.Length()-2).c_str());
           }
       	} else strcpy(d,OpenDialog->FileName.c_str());
       Program->Text=d;
       bs=strrchr(d,'\\');
       if(bs) {
          *bs=0;
          WDir->Text=d;
       }
     }	
}
//---------------------------------------------------------------------------
void __fastcall TIConfigDlg::PluginComboChange(TObject *Sender)
{
	PlgDesc *plg;
    FreeContext(ActConfig);
	PluginManager.SetActualPlugin(PluginCombo->ItemIndex);
    plg = PluginManager.GetActualPlugin();
    ConfPl->Enabled = (plg && plg->Configure);
}

//---------------------------------------------------------------------------
void __fastcall TIConfigDlg::ConfPlClick(TObject *Sender)
{   PlgDesc *ActivePlugin = PluginManager.GetActualPlugin();
	HWND parentH;

	if(ActivePlugin && ActivePlugin->Configure) {
   	    if(ActMode==GLOBAL) parentH = ConfDlg->Handle;
        else parentH = Handle;
    	ActivePlugin->Configure(Application->Handle,parentH,ActConfig,ActMode);
    }
}
//---------------------------------------------------------------------------
void __fastcall TIConfigDlg::EditChange(TObject *Sender)
{
	StoreAccButton->Enabled = (!DomainCombo->Text.Trim().IsEmpty() &&
       							!NameEdit->Text.Trim().IsEmpty());
	Changed(Sender);
}
//---------------------------------------------------------------------------
void __fastcall TIConfigDlg::EditEnter(TObject *Sender)
{

    EditChange(Sender);
}
//---------------------------------------------------------------------------
void __fastcall TIConfigDlg::DomainComboExit(TObject *Sender)
{
	SetDomain(DomainCombo->Text.c_str());
}
//---------------------------------------------------------------------------
void __fastcall TIConfigDlg::AccountListEnter(TObject *Sender)
{
	//AccountList->ItemIndex = OldIndex;
	DelAccButton->Enabled=(AccountList->ItemIndex >=0);
        StoreAccButton->Enabled=false;
}
//---------------------------------------------------------------------------
void __fastcall TIConfigDlg::AccountListExit(TObject *Sender)
{
	//OldIndex = AccountList->ItemIndex;
    DelAccButton->Enabled= (AccountList->ItemIndex >=0);
//    AccountList->ItemIndex = -1;
}
//---------------------------------------------------------------------------
void __fastcall TIConfigDlg::DelAccButtonClick(TObject *Sender)
{
	int id = AccountList->ItemIndex;
    if(id<0) return;

    OldIndex = AccountList->ItemIndex;
	if(!AccountManager.DeleteAccount(AccountList->Items->Strings[id].c_str())) {
   	  Application->MessageBox(MakeErrorMessage(GetLastError()),"Delete failed",
                              MB_ICONERROR|MB_OK);
   } else AccountList->Items->Delete(id);
   if(!AccountList->Items->Count) OldIndex = -1;
   else if(OldIndex >= AccountList->Items->Count) OldIndex = 0;
   AccountList->ItemIndex = OldIndex;
}
//---------------------------------------------------------------------------
void __fastcall TIConfigDlg::StoreAccButtonClick(TObject *Sender)
{
	AnsiString AccName;

    //int id;
    if(DomainCombo->Text.Trim().IsEmpty() ||
       NameEdit->Text.Trim().IsEmpty()) return;

    AccName=DomainCombo->Text.Trim()+"/"+NameEdit->Text.Trim();
    AccountManager.StoreAccount(AccName.c_str(),
                     NameEdit->Text.Trim().c_str(),
                     DomainCombo->Text.c_str(),
                     PasswordEdit->Text.c_str());


    Refresh2Click(0);
/*{

     AccountList->Items->BeginUpdate();
     id = AccountList->Items->IndexOf(AccName.c_str());
     if(id>=0) {
           delete AccountList->Items->Objects[id];
           AccountList->Items->Delete(id);
     }
     AccCallBack(AccName.c_str(),
                   NameEdit->Text.Trim().c_str(),
                   DomainCombo->Text.c_str(),
                   PasswordEdit->Text.c_str(),AccountList->Items);
    }
    AccountList->Items->EndUpdate();
*/
}
//---------------------------------------------------------------------------
void __fastcall TIConfigDlg::Select2Click(TObject *Sender)
{
    int id = AccountList->ItemIndex;
    DelAccButton->Enabled=(id >=0);
    StoreAccButton->Enabled=false;
    TCred *Cred;    
    if(id>=0) {
       Cred = (TCred*)AccountList->Items->Objects[id];
       PasswordEdit->Text=Cred->GetPassword();
       NameEdit->Text=Cred->GetName();
       id=DomainCombo->Items->IndexOf(Cred->GetDomain());
       if(id<0)
               id=DomainCombo->Items->Add(Cred->GetDomain());
       DomainCombo->ItemIndex=id;
       StoreAccButton->Enabled=!Cred->Persistent;
    }
}
//---------------------------------------------------------------------------
void __fastcall TIConfigDlg::Refresh2Click(TObject *Sender)
{
   TAccountIndex i;
    TCred *C;
    AnsiString Name;
   	i = AccountManager.GetFirstIndex();
    AccountList->Items->BeginUpdate();
   	AccountList->Items->Clear();
    do {
    	C = AccountManager.GetAccount(i,Name);
        if(C && !C->IsDeleted()) {
        	AccountList->Items->AddObject(Name,C);
            if(DomainCombo->Items->IndexOf(C->GetDomain()) <0)
	            DomainCombo->Items->Add(C->GetDomain());
        }
        ++i;
    } while(C);
    AccountList->Items->EndUpdate();
    AccountList->ItemIndex=-1;
    DelAccButton->Enabled=false;
}
//---------------------------------------------------------------------------
void __fastcall TIConfigDlg::ListMouseDown(TObject *Sender, TMouseButton Button,
	TShiftState Shift, int X, int Y)
{
	POINT p={X,Y};
    TListBox *Box;
    int id;
    Box = (TListBox*)Sender;
	if(Button == mbRight) {
    	id=Box->ItemAtPos(p,true);
        if(id>=0) Box->ItemIndex=id;
    }	
}
//---------------------------------------------------------------------------
void __fastcall TIConfigDlg::FormDestroy(TObject *Sender)
{
    PluginManager.RemoveClientWindow(Handle);
    Servers->RemoveClientWindow(Handle);
    FreeHost(ActConfig);
}
//---------------------------------------------------------------------------
void __fastcall TIConfigDlg::LockClick(TObject *Sender)
{
	TCheckBox* S = (TCheckBox*)Sender;
	if(S->State == cbGrayed) S->State = cbChecked;
}

//---------------------------------------------------------------------------
void __fastcall TIConfigDlg::Changed(TObject *Sender)
{
	if(Sender == Program) ProgChanged=true;
    else if(Sender == WDir) DirChanged=true;
    else if(Sender == COptions) CommandlineChanged = true;
    else if(Sender == EPluginParams) PluginParamsChanged = true;
    else if(Sender == NameEdit) NameChanged = true;
    else if(Sender == PasswordEdit) PasswordChanged = true;
    else if(Sender == DomainCombo) DomainChanged = true;
}

int CALLBACK
#if (!(BCBVER > 1))
	//CBuilder1
	BrowseCallbackProc(HWND hwnd,int uMsg,LPARAM lp, LPARAM pData) {
#else
	//CBuilder5
    BrowseCallbackProc(HWND hwnd,UINT uMsg,LPARAM lp, LPARAM pData) {
#endif

      //char szDir[MAX_PATH];
         switch(uMsg) {
            case BFFM_INITIALIZED: {
                  // WParam is TRUE since you are passing a path.
                  // It would be FALSE if you were passing a pidl.
                  SendMessage(hwnd,BFFM_SETSELECTION,TRUE,pData);
               break;
            }
            /*
            case BFFM_SELCHANGED: {
               // Set the status window to the currently selected path.
               if (SHGetPathFromIDList((PItemIDList) lp ,szDir)) {
                  SendMessage(hwnd,BFFM_SETSTATUSTEXT,0,(LPARAM)szDir);
               }
               break;
            }
            */
            default:
               break;
         }
         return 0;
      }


//---------------------------------------------------------------------------
void __fastcall TIConfigDlg::BitBtn2Click(TObject *Sender)
{
	TBrowseInfo bi;
    char szDir[MAX_PATH];
    PItemIDList pidl;
#if (!(BCBVER > 1))
	//CBuilder1
	Ole2::IMalloc *pMalloc;
#else
	//CBuilder5
    //namespace Ole2 defined in sysdefs.h for CBuilder 1  not longer defined  for CBuilder5
    IMalloc *pMalloc;
#endif



    AnsiString StartDir;

    StartDir = WDir->Text;
    if(StartDir.IsEmpty())
    	if(!GetCurrentDirectory(MAX_PATH,szDir))
	    	StartDir = "C:\\";
        else StartDir = szDir;
#if (!(BCBVER > 1))
	//CBuilder1
	if (SUCCEEDED(SHGetMalloc(pMalloc))) {
#else
	//CBuilder5
    if (SUCCEEDED(SHGetMalloc(&pMalloc))) {
#endif
    	ZeroMemory(&bi,sizeof(bi));
        if(ActMode==GLOBAL)
	        bi.hwndOwner = ConfDlg->Handle;
        else
        	bi.hwndOwner = Handle;
        bi.pszDisplayName = 0;
        bi.pidlRoot = 0;
        bi.ulFlags = BIF_RETURNONLYFSDIRS|BIF_RETURNFSANCESTORS ;
        bi.lpfn = BrowseCallbackProc;
        bi.lParam = (LPARAM)StartDir.c_str();
        bi.lpszTitle = "Please select a working directory";
#if (!(BCBVER > 1))
		//CBuilder1
		pidl = SHBrowseForFolder(bi);
#else
		//CBuilder5
    	pidl = SHBrowseForFolder(&bi);
#endif

        if (pidl) {
        	if (SHGetPathFromIDList(pidl,szDir)) {
            	WDir->Text = szDir;
            }
        }
        pMalloc->Free(pidl); pMalloc->Release();
    }


}
//---------------------------------------------------------------------------
void __fastcall TIConfigDlg::PageControl1Change(TObject *Sender)
{
	TCred *Creds;
    TAccountIndex i;
    AnsiString NName;
    if(NameEdit->Text.Trim().IsEmpty() ||
       DomainCombo->Text.Trim().IsEmpty() ||
       PasswordEdit->Text.Trim().IsEmpty()) return;
	if(PageControl1->ActivePage != AccSheet || !Sender) {
    	i = AccountManager.FindAccount(NameEdit->Text.Trim(),DomainCombo->Text.Trim());
        Creds = AccountManager.GetAccount(i,NName);
        if(Creds) {
        	if(Creds->GetPassword().IsEmpty())
            	Creds->GetPassword() = PasswordEdit->Text.Trim();
        } else {
	        Creds = new TCred(NameEdit->Text.c_str(),
                        DomainCombo->Text.c_str(),
                        PasswordEdit->Text.c_str());
            NName = DomainCombo->Text.Trim()+AnsiString("/")+NameEdit->Text.Trim();
        	AccountManager.InsertAccount(NName,Creds);
        }
    }
    Refresh2Click(0);
}
//---------------------------------------------------------------------------
void __fastcall TIConfigDlg::AccountListDrawItem(TWinControl *Control,
	int Index, TRect &Rect, TOwnerDrawState State)
{
int Offset = 2;// default offset
    TColor FC,BC,PC;
	TListBox *ListBox = dynamic_cast<TListBox*>(Control);
	TCanvas *Canvas = ListBox->Canvas;
    BC=Canvas->Brush->Color;
    PC=Canvas->Pen->Color;
    FC = Canvas->Font->Color;
    Canvas->Brush->Color=clWindow;
    TCred *cred,*actCred;
    TFontStyles FS;


    FS = Canvas->Font->Style;
   	actCred = AccountManager.GetActualAccount();
    cred=(TCred *)ListBox->Items->Objects[Index];
    if(actCred && *actCred == *cred)
   		Canvas->Font->Style = FS<<fsBold;

    if(!cred->Persistent)
		Canvas->Font->Style = Canvas->Font->Style<<fsItalic;

	Canvas->Font->Color=clWindowText;
   	if(ListBox->Focused() && State.Contains(odSelected)) {
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

void __fastcall TIConfigDlg::SetAccountBtnClick(TObject *Sender)
{
   if (CheckUserAccountOnChange)
   {
    if (!IsValidUser(NameEdit->Text.c_str(),DomainCombo->Text.c_str(),
    	PasswordEdit->Text.c_str() ))
    {
      //wrong password
      DWORD errcode = GetLastError();
      if (errcode == 1326)
      {
      	//Invalid Username or Password
        Application->MessageBox(
        "The actual account (domain/username/password) is not accepted by the local host. Please check if it's valid for the remote machines.",
        "WARNING!", MB_OK);
      }
      /*else
      {
        itoa(errcode,errcodestr,10);
        strcat(errcodestr,": LogonUser failed");
        Application->MessageBox(errcodestr
        ,
        "WARNING!", MB_OK);
        }  */
     }
    } //CheckUserAccountOnChange

  //set given account
    AccountManager.SetActualAccount(NameEdit->Text.Trim().UpperCase(),
                                    DomainCombo->Text.Trim().UpperCase());//Si
             
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
    AccountList->Repaint();   
}
//---------------------------------------------------------------------------
