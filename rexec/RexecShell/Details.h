//---------------------------------------------------------------------------
#ifndef DetailsH
#define DetailsH
//---------------------------------------------------------------------------
#include <vcl\Classes.hpp>
#include <vcl\Controls.hpp>
#include <vcl\StdCtrls.hpp>
#include <vcl\Forms.hpp>
#include <vcl\ExtCtrls.hpp>
#include <vcl\ComCtrls.hpp>
#pragma hdrstop

#include "cluma.h"
#include "..\mpirun\plugins\Plugin.h"
#include "ProcWindow.h"
//---------------------------------------------------------------------------
class TDetailForm : public TForm
{
__published:	// IDE-verwaltete Komponenten
	TPageControl *PageControl1;
	TTabSheet *TabSheet1;
	TPanel *OsInfo;
	TPanel *Panel1;
	TButton *Button1;
	TPanel *Panel2;
	TMemo *Memo1;
	TPanel *HwPanel;
	TLabel *Label1;
	TBevel *Bevel1;
	TLabel *NoProcs;
	TLabel *Label2;
	TBevel *Bevel2;
	TLabel *ProcType;
	TLabel *Label3;
	TBevel *Bevel3;
	TLabel *PhysMem;
	TPanel *AccountPanel;
	TLabel *Label4;
	TLabel *Label5;
	TBevel *Bevel4;
	TBevel *Bevel5;
	TLabel *ConsoleUser;
	TLabel *ConnectAccount;
	TLabel *Label6;
	TBevel *Bevel6;
	TLabel *LockedAccount;
	TTabSheet *TabSheet2;
	TLabel *MessageLabel;
	TLabel *OsLabel;
	TListView *IPList;
	void __fastcall Button1Click(TObject *Sender);
	void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
	
	
	void __fastcall FormCreate(TObject *Sender);
	void __fastcall PageControl1Change(TObject *Sender);
	void __fastcall FormDestroy(TObject *Sender);
        
private:	// Benutzer-Deklarationen
	HostData *ActHost;
    TProcForm *PWindow;
public:		// Benutzer-Deklarationen
	__fastcall TDetailForm(TComponent* Owner);
    void CreateDesc(HostData *Data);
};
//---------------------------------------------------------------------------
//extern TDetailForm *DetailForm;
//---------------------------------------------------------------------------
#endif
