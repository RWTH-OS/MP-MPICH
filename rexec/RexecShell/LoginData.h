//---------------------------------------------------------------------------
#ifndef LoginDataH
#define LoginDataH
//---------------------------------------------------------------------------
#include <vcl\Classes.hpp>
#include <vcl\Controls.hpp>
#include <vcl\StdCtrls.hpp>
#include <vcl\Forms.hpp>
#include <vcl\ComCtrls.hpp>
#include <vcl\Menus.hpp>
#include <vcl\ExtCtrls.hpp>
//---------------------------------------------------------------------------

#pragma hdrstop
#include "cluma.h"
#include "..\mpirun\plugins\Plugin.h"
#include "IndividualConfig.h"

class TLoginDlg : public TForm
{
__published:	// IDE-verwaltete Komponenten
	TPanel *Panel1;
        TButton *OKButton;
        TButton *CancelButton;
	TPanel *Panel2;
	void __fastcall FormCreate(TObject *Sender);
	
	
	
	

	
	

	
	
	
	void __fastcall FormDestroy(TObject *Sender);
	void __fastcall OKButtonClick(TObject *Sender);
	void __fastcall FormShow(TObject *Sender);
private:	// Benutzer-Deklarationen
	TIConfigDlg *Config;
    AnsiString LoginUser,LoginDomain;
public:		// Benutzer-Deklarationen
	__fastcall TLoginDlg(TComponent* Owner);
    void SetLoginData(HostData *Host);
    void GetLoginData(HostData *Host);
    AnsiString Account,Domain,Passwd,LastStored,AccountName;
    int Index;
};
//---------------------------------------------------------------------------
extern TLoginDlg *LoginDlg;
//---------------------------------------------------------------------------
#endif
