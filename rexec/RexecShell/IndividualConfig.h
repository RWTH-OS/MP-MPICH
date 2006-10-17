//---------------------------------------------------------------------------
#ifndef IndividualConfigH
#define IndividualConfigH
//---------------------------------------------------------------------------
#include <vcl\Classes.hpp>
#include <vcl\Controls.hpp>
#include <vcl\StdCtrls.hpp>
#include <vcl\Forms.hpp>
#include <vcl\ExtCtrls.hpp>
#include <vcl\ComCtrls.hpp>
#include <vcl\Buttons.hpp>
#include <vcl\Menus.hpp>
#include <vcl\Dialogs.hpp>
//---------------------------------------------------------------------------
#include "cluma.h"
#include "..\mpirun\plugins\Plugin.h"
#include "NetState.h"
#include "PluginManager.h"
#include "AccountManager.h"

enum ConfMode {INDIVIDUAL=0,GLOBAL=1};

class TIConfigDlg : public TForm
{
__published:	// IDE-verwaltete Komponenten
	TPanel *Panel1;
	TButton *Button1;
	TButton *Button2;
	TPanel *Panel2;
	TPageControl *PageControl1;
	TTabSheet *BasicSheet;
	TLabel *Label1;
	TLabel *Label2;
	TLabel *Label3;
	TLabel *Label4;
	TEdit *Program;
	TEdit *COptions;
	TEdit *WDir;
	TComboBox *PluginCombo;
	TCheckBox *Lock;
	TTabSheet *AccSheet;
	TLabel *Label7;
	TLabel *Label8;
	TLabel *Label9;
	TLabel *Label10;
	TEdit *NameEdit;
	TListBox *AccountList;
	TEdit *PasswordEdit;
	TComboBox *DomainCombo;
	TBitBtn *StoreAccButton;
	TBitBtn *DelAccButton;
	TTabSheet *EnvSheet;
	TLabel *Label11;
	TLabel *Label12;
	TLabel *Label13;
	TListBox *VarList;
	TEdit *VarNameEdit;
	TEdit *VarValEdit;
	TBitBtn *StoreVarButton;
	TBitBtn *DelVarButton;
	TOpenDialog *OpenDialog;
	TPopupMenu *PopupMenu1;
	TMenuItem *Refresh1;
	TMenuItem *N1;
	TMenuItem *Selectitem1;
	TMenuItem *Deleteitem1;
	TBitBtn *ConfPl;
	TBitBtn *BitBtn1;
	TBitBtn *BitBtn2;
	TCheckBox *LoadEnv;
        TTabSheet *AdvSheet;
        TCheckBox *CheckBox1;
        TCheckBox *CheckBox2;
        TCheckBox *CheckBox3;
        TBitBtn *BitBtn3;
        TLabel *Label5;
        TEdit *Edit1;
        TLabel *Label6;
        TComboBox *PriorityCombo;
        TBitBtn *SetAccountBtn;
        TLabel *Label14;
        TEdit *EPluginParams;
	void __fastcall SpeedButton1Click(TObject *Sender);
	void __fastcall PluginComboChange(TObject *Sender);
	void __fastcall ConfPlClick(TObject *Sender);
	void __fastcall EditChange(TObject *Sender);
	void __fastcall EditEnter(TObject *Sender);
	void __fastcall DomainComboExit(TObject *Sender);
	void __fastcall AccountListEnter(TObject *Sender);
	void __fastcall DelAccButtonClick(TObject *Sender);
	void __fastcall StoreAccButtonClick(TObject *Sender);
	void __fastcall Select2Click(TObject *Sender);
	void __fastcall Refresh2Click(TObject *Sender);
	
	void __fastcall ListMouseDown(TObject *Sender, TMouseButton Button,
	TShiftState Shift, int X, int Y);
	void __fastcall FormCreate(TObject *Sender);
	void __fastcall FormDestroy(TObject *Sender);
	void __fastcall LockClick(TObject *Sender);
	void __fastcall AccountListExit(TObject *Sender);
	
	void __fastcall Changed(TObject *Sender);
	void __fastcall BitBtn2Click(TObject *Sender);
	
	
	void __fastcall PageControl1Change(TObject *Sender);
	
	void __fastcall AccountListDrawItem(TWinControl *Control, int Index,
	TRect &Rect, TOwnerDrawState State);
	
        void __fastcall SetAccountBtnClick(TObject *Sender);

        
    protected:
    void __fastcall ThreadExit(TMessage &Message);
    void __fastcall ThreadStart(TMessage &Message);
    void __fastcall OnPluginChange(TMessage &Message);
   	BEGIN_MESSAGE_MAP
        MESSAGE_HANDLER(REFRESH_START,TMessage,ThreadStart);
		MESSAGE_HANDLER(REFRESH_FINISH,TMessage,ThreadExit);
        MESSAGE_HANDLER(PM_CHANGE,TMessage,OnPluginChange);
    END_MESSAGE_MAP(TForm);
private:	// Benutzer-Deklarationen
	ConfMode ActMode;
    HostData *ActConfig;
    void __fastcall SetDomain(char *d);
    void __fastcall LookupAccount();
    HWND stHandle;
    int OldIndex;
    bool ProgChanged,DirChanged,CommandlineChanged,NameChanged;
    bool PasswordChanged,DomainChanged,PluginParamsChanged;
public:		// Benutzer-Deklarationen
	__fastcall TIConfigDlg(TComponent* Owner);
   	virtual __fastcall ~TIConfigDlg();
    void __fastcall SetConfig(HostData *Data,ConfMode mode);
    void __fastcall GetConfig(HostData *Data);
    void __fastcall SetAccount(TCred *);
    void __fastcall GetAccount(TCred *);
};
//---------------------------------------------------------------------------
extern TIConfigDlg *IConfigDlg;
//---------------------------------------------------------------------------
#endif
