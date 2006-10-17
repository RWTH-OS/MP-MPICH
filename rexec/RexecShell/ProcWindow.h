//---------------------------------------------------------------------------
#ifndef ProcWindowH
#define ProcWindowH
//---------------------------------------------------------------------------
#include <vcl\Classes.hpp>
#include <vcl\Controls.hpp>
#include <vcl\StdCtrls.hpp>
#include <vcl\Forms.hpp>
#include <vcl\ExtCtrls.hpp>
#include <vcl\ComCtrls.hpp>

#include "RexecClient.h"
#include "..\mpirun\plugins\Plugin.h"
//---------------------------------------------------------------------------
class TProcForm : public TForm
{
__published:	// IDE-verwaltete Komponenten
	TPanel *Panel1;
	TPanel *Panel2;
	TButton *KillButton;
	TButton *RefreshButton;
	TButton *Button1;
	TListView *ListView;
	void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
	void __fastcall Button1Click(TObject *Sender);
	
	void __fastcall RefreshButtonClick(TObject *Sender);
	void __fastcall KillButtonClick(TObject *Sender);
	
	void __fastcall ListViewChange(TObject *Sender, TListItem *Item,
	TItemChange Change);
	void __fastcall ListViewColumnClick(TObject *Sender, TListColumn *Column);
	
protected:
	void __fastcall OnProcEnum(TMessage &Message);
    void __fastcall OnProcEnumStart(TMessage &Message);
   	BEGIN_MESSAGE_MAP
        MESSAGE_HANDLER(RC_PROC,TMessage,OnProcEnum);
        MESSAGE_HANDLER(RC_PROC_START,TMessage,OnProcEnumStart);
        MESSAGE_HANDLER(RC_PROC_END,TMessage,OnProcEnumStart);
    END_MESSAGE_MAP(TForm)
private:	// Benutzer-Deklarationen
	HostData *ActHost;
#if ((BCBVER > 1))
  //CBuilder5
  PFNLVCOMPARE Compare;
#else
  //CBuilder1
  TLVCompare Compare;
#endif

    int SubIndex;
public:		// Benutzer-Deklarationen
	__fastcall TProcForm(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern TProcForm *ProcForm;
//---------------------------------------------------------------------------
#endif
