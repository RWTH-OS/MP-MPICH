//---------------------------------------------------------------------------
#ifndef MsgBoxH
#define MsgBoxH
//---------------------------------------------------------------------------
#include <vcl\Classes.hpp>
#include <vcl\Controls.hpp>
#include <vcl\StdCtrls.hpp>
#include <vcl\Forms.hpp>
#include <vcl\Buttons.hpp>

/* definition of Button types */
#define BtnOk 1
#define BtnCancel 2
#define BtnYes 4
#define BtnNo 8
#define BtnHelp 16
#define BtnClose 32
#define BtnAbort 64
#define BtnRetry 128
#define BtnIgnore 256
#define BtnAll 512

//---------------------------------------------------------------------------
class TMsgDlg : public TForm
{
__published:	// IDE-verwaltete Komponenten
        TBitBtn *YesBtn;
        TBitBtn *NoBtn;
        TBitBtn *AllBtn;
        TBitBtn *OkBtn;
        TBitBtn *CancelBtn;
        TLabel *LabelText;
        TBitBtn *AbortBtn;
        TBitBtn *RetryBtn;
        TBitBtn *IgnoreBtn;
        TBitBtn *CloseBtn;
        TBitBtn *HelpBtn;
private:	// Benutzer-Deklarationen
public:		// Benutzer-Deklarationen
        __fastcall TMsgDlg(TComponent* Owner);
};
//---------------------------------------------------------------------------
//extern TMsgDlg *MsgDlg;
int MyMsgBox(AnsiString DlgCaption, AnsiString DlgText,WORD Buttons);
//---------------------------------------------------------------------------
#endif
