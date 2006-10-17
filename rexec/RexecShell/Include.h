//---------------------------------------------------------------------------
#ifndef IncludeH
#define IncludeH
//---------------------------------------------------------------------------
#include <vcl\Classes.hpp>
#include <vcl\Controls.hpp>
#include <vcl\StdCtrls.hpp>
#include <vcl\Forms.hpp>

#include "NetState.h"
//---------------------------------------------------------------------------
class TIncludeForm : public TForm
{
__published:	// IDE-verwaltete Komponenten
	TListBox *SelectedBox;
	TEdit *HostEdit;
	TButton *Button1;
	TButton *Button2;
	TButton *Button3;
	TLabel *Label1;
	TButton *Button4;
	void __fastcall FormShow(TObject *Sender);
	void __fastcall Button1Click(TObject *Sender);
	void __fastcall Button2Click(TObject *Sender);
	void __fastcall Button3Click(TObject *Sender);
	void __fastcall SelectedBoxDblClick(TObject *Sender);
	
	void __fastcall SelectedBoxKeyDown(TObject *Sender, WORD &Key,
	TShiftState Shift);
	void __fastcall FormCreate(TObject *Sender);
protected:
	void __fastcall ParData(TMessage Message);
   	BEGIN_MESSAGE_MAP
    	MESSAGE_HANDLER(PAR_DATA,TMessage,ParData);
        MESSAGE_HANDLER(ENUM_START,TMessage,ParData);
		MESSAGE_HANDLER(ENUM_FINISH,TMessage,ParData);
        MESSAGE_HANDLER(REFRESH_START,TMessage,ParData);
		MESSAGE_HANDLER(REFRESH_FINISH,TMessage,ParData);
   	END_MESSAGE_MAP(TForm)
private:	// Benutzer-Deklarationen
	bool Disabled;
public:		// Benutzer-Deklarationen
TStringList *List;
	__fastcall TIncludeForm(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern TIncludeForm *IncludeForm;
//---------------------------------------------------------------------------
#endif
