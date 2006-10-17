//---------------------------------------------------------------------------
#include <vcl\vcl.h>
//#include <stdio.h>
//#include <string.h>
#pragma hdrstop

#include "LoginData.h"
#include "RexecClient.h"
#include "IndividualConfig.h"
//---------------------------------------------------------------------------
#pragma resource "*.dfm"
TLoginDlg *LoginDlg;
char *MakeErrorMessage(DWORD ErrorId);
/*
static BOOL WINAPI EnumCallBack(char *Account, char *User, char* Domain, char *password, void *Sender) {
	TListView *ListView;
    TListItem *NewItem;
    AnsiString D(Domain);
    AnsiString Name;
    Name = Domain;
    Name += "/";
    Name += User;
    char *pwd;
    ListView=(TListView*)Sender;
    NewItem=ListView->Items->Add();
    if(!NewItem) return TRUE;
	NewItem->Caption=Name;
    NewItem->SubItems->Add(User);
    NewItem->SubItems->Add(D.UpperCase());
    pwd=new char[strlen(password)+1];
    strcpy(pwd,password);
    NewItem->Data=pwd;
    return TRUE;
}
*/
//---------------------------------------------------------------------------
__fastcall TLoginDlg::TLoginDlg(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TLoginDlg::FormCreate(TObject *Sender)
{
    Config = new TIConfigDlg(this);
    Config->PageControl1->Parent = Panel2;
    Config->PageControl1->Align = alClient;
    Config->PageControl1->ActivePage=Config->AccSheet;
    Config->PageControl1->Pages[0]->TabVisible = false;
    GetActualUser(LoginUser,LoginDomain);
}
//---------------------------------------------------------------------------
    void TLoginDlg::SetLoginData(HostData *Host) {
    	if(!Host || !Host->Account) return;
        Config->SetConfig(Host,INDIVIDUAL);
    }
//---------------------------------------------------------------------------
    void TLoginDlg::GetLoginData(HostData *Host) {
		TCred *Creds;
        TAccount A;
        if(!Host) return;

        Creds = new TCred;
        Config->GetAccount(Creds);
        GetActualUser(LoginUser,LoginDomain);
        if(LoginUser==Creds->GetName() && LoginDomain == Creds->GetDomain())
        	SetHostAccount(Host,0);
        else {
        	strcpy(A.User,Creds->GetName().c_str());
    		strcpy(A.Password,Creds->GetPassword().c_str());
		    if(Creds->GetDomain()!=AnsiString("LOCAL"))
   				strcpy(A.Domain,Creds->GetDomain().c_str());
	    	else
   				strcpy(A.Domain,Host->Name);
	    	SetHostAccount(Host,&A);
        }
        delete Creds;
    }
//---------------------------------------------------------------------------

void __fastcall TLoginDlg::FormDestroy(TObject *Sender)
{
	if(Config) delete Config;
}
//---------------------------------------------------------------------------
void __fastcall TLoginDlg::OKButtonClick(TObject *Sender)
{
 	TAccountIndex id;
	Config->PageControl1Change(0);
    AccountManager.SetActualAccount(Config->NameEdit->Text.Trim().UpperCase(),
    								Config->DomainCombo->Text.Trim().UpperCase());
}
//---------------------------------------------------------------------------
void __fastcall TLoginDlg::FormShow(TObject *Sender)
{
    Config->PageControl1->ActivePage=Config->AccSheet;//Si
	Config->Refresh2Click(0);
}
//---------------------------------------------------------------------------
