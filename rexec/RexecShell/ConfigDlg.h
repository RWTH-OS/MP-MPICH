//---------------------------------------------------------------------------
#ifndef ConfigDlgH
#define ConfigDlgH

#define UNICODE
//---------------------------------------------------------------------------
#include <vcl\Classes.hpp>
#include <vcl\Controls.hpp>
#include <vcl\StdCtrls.hpp>
#include <vcl\Forms.hpp>
#include <vcl\Buttons.hpp>
#include <vcl\Dialogs.hpp>
#include <vcl\Menus.hpp>
#include <vcl\ComCtrls.hpp>
#include <vcl\ExtCtrls.hpp>

#include "NetState.h"
#include "..\mpirun\plugins\Plugin.h"
#include "client.h"
#include "PluginManager.h"
#include "IndividualConfig.h"
#include "Environment.h"

//---------------------------------------------------------------------------
//class TClientForm;

class CHostRef:public TObject {
public:
    CHostRef() {Host=0; Window=0;ProcRef=AccountRef=false;}
    CHostRef(HostData &S) {Host=&S;Window=0;ProcRef=AccountRef=false;}
    CHostRef(HostData *S) {Host=S;Window=0;ProcRef=AccountRef=false;}
    virtual __fastcall ~CHostRef() {
    	if(Window) Window->Close();
        ProcRef=AccountRef=false;
        FreeHost(Host);
    }
    char *GetEnvironment() {
    	if(Host && Host->ProcData) return Host->ProcData->Environment;
        else return 0;
    }
    DWORD GetEnvSize() {
    	if(Host && Host->ProcData) return Host->ProcData->EnvSize;
        else return 0;
    }
	void PutEnvString(char *Name,char *Value) {if(Host) ::PutEnvString(Host,Name,Value);}
    char *GetEnvString(char *Name,char *Value,DWORD *BufSize){
    	if(Host) return ::GetEnvString(Host,Name,Value,BufSize);
        else return 0;
    }
    TClientForm *GetWindow() {return Window;}
    void SetWindow(TClientForm *W) {Window=W;}
   	void RemoveWindow() {Window=0;}
   	void CloseWindow() {if(Window) Window->Close();}
    void ShowWindow() {if(Window) Window->Show();}

    void SetConfig(HostData *src) {
    	CopyProcData(Host,src);
        SetHostAccount(Host,src->Account);
        ProcRef=AccountRef=false;
    }
    void SetConfigRef(HostData *src) {
    	if(!Host) return;
        if(!Host->ProcData) {
        	CopyProcData(Host,src);
            ProcRef = true;
        } else if(!Host->ProcData->Executable)
            ProcSetString(&Host->ProcData->Executable,
                          src->ProcData->Executable,
                          &Host->ProcData->ExeSize);

        
        if(!Host->Account) {
        	SetHostAccount(Host,src->Account);
            AccountRef = true;
        }
    }

    void RemoveConfigRefs() {
    	if(!Host) return;
        if(ProcRef) {
        	FreeProcData(Host);
            ProcRef=false;
        }
        if(AccountRef) {
        	SetHostAccount(Host,0);
            AccountRef=false;
        }

    }
    bool IsValidConfig() {
    	return (Host && Host->ProcData && Host->Account &&
                Host->ProcData->Executable && Host->Account->User[0] &&
                Host->Account->Domain[0]);
    }
	HostData &operator*() {return *Host; }
	HostData *operator->() {return Host;}
    operator HostData*() {return Host;}
    HostData *operator=(HostData *S) {Host=S; return S;}
    HostData &operator=(HostData &S) {Host=&S; return S;}

private:
	CHostRef(CHostRef&);
   	CHostRef &operator=(CHostRef&);
    HostData *Host;
    TClientForm *Window;
    bool ProcRef,AccountRef;
};

class TConfDlg : public TForm
{
__published:	// IDE-managed Components
   TOpenDialog *OpenDialog;
	TPopupMenu *PopupMenu1;
	TMenuItem *ShowDetails1;
	TMenuItem *N1;
	TMenuItem *Refresh1;
	TMenuItem *Select1;
	TPanel *Panel1;
	TButton *OKBtn;
	TButton *CancelBtn;
	TPanel *Panel2;
	TGroupBox *GroupBox1;
	TSpeedButton *SpeedButton2;
	TLabel *Label5;
	TLabel *Label6;
	TListBox *SelectedBox;
	TBitBtn *AddBtn;
	TBitBtn *RemoveBtn;
	TListBox *HostBox;
	TBitBtn *RefreshButton;
	TPanel *Panel3;
	TPopupMenu *PopupMenu2;
	TMenuItem *Configure_item;
	TMenuItem *N2;
	TMenuItem *Remove_item;
	TMenuItem *Defaultconfig_item;
   void __fastcall FormActivate(TObject *Sender);
   void __fastcall SelectedBoxDragOver(TObject *Sender, TObject *Source, int X,
   int Y, TDragState State, bool &Accept);
   void __fastcall SelectedBoxDragDrop(TObject *Sender, TObject *Source, int X,
   int Y);
   void __fastcall AddBtnClick(TObject *Sender);
   void __fastcall RemoveBtnClick(TObject *Sender);
   void __fastcall HostBoxDragOver(TObject *Sender, TObject *Source, int X,
   int Y, TDragState State, bool &Accept);
   void __fastcall HostBoxDragDrop(TObject *Sender, TObject *Source, int X,
   int Y);
   
   
   
   
   void __fastcall SelectedBoxKeyDown(TObject *Sender, WORD &Key,
   TShiftState Shift);
	
	void __fastcall FormCreate(TObject *Sender);
	void __fastcall RefreshButtonClick(TObject *Sender);
	void __fastcall HostBoxDrawItem(TWinControl *Control, int Index, TRect &Rect,
	TOwnerDrawState State);
	void __fastcall FormDestroy(TObject *Sender);
	void __fastcall Refresh1Click(TObject *Sender);
	void __fastcall ShowDetails1Click(TObject *Sender);
	void __fastcall PopupMenu1Popup(TObject *Sender);
	void __fastcall SpeedButton2Click(TObject *Sender);
	void __fastcall HostBoxMouseDown(TObject *Sender, TMouseButton Button,
	TShiftState Shift, int X, int Y);
	

	void __fastcall SelectedBoxDblClick(TObject *Sender);
	void __fastcall OKBtnClick(TObject *Sender);
	void __fastcall CancelBtnClick(TObject *Sender);
	
	
	void __fastcall SelectedBoxDrawItem(TWinControl *Control, int Index,
	TRect &Rect, TOwnerDrawState State);
	void __fastcall PopupMenu2Popup(TObject *Sender);
	
	
	void __fastcall Defaultconfig_itemClick(TObject *Sender);
	void __fastcall AddBtnEnter(TObject *Sender);
	void __fastcall RemoveBtnEnter(TObject *Sender);
	
	void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
private:	// User declarations
	TIConfigDlg *Config;
    PlgDesc *ActivePlugin;
    TStringList *LastList;
protected:
	void __fastcall ThreadExit(TMessage &Message);
    void __fastcall ThreadStart(TMessage &Message);
    void __fastcall ParData(TMessage &Message);
    void __fastcall OnGetFont(TMessage &Message);
    void __fastcall OnPluginChange(TMessage &Message);
   	BEGIN_MESSAGE_MAP
    	MESSAGE_HANDLER(PAR_DATA,TMessage,ParData);
        MESSAGE_HANDLER(ENUM_START,TMessage,ThreadStart);
		MESSAGE_HANDLER(ENUM_FINISH,TMessage,ThreadExit);
        MESSAGE_HANDLER(REFRESH_START,TMessage,ThreadStart);
		MESSAGE_HANDLER(REFRESH_FINISH,TMessage,ThreadExit);
        MESSAGE_HANDLER(WM_GETFONT,TMessage,OnGetFont);
        MESSAGE_HANDLER(PM_CHANGE,TMessage,OnPluginChange);
   	END_MESSAGE_MAP(TForm)

    void RemoveInvalidHosts();
    void __fastcall RemoveSelHost(int index,bool both);
    void __fastcall CreateTmpConfig(HostData *tmpConfig);
public:		// User declarations
   __fastcall TConfDlg(TComponent* Owner);
   void UpdateParams();
   BOOL LoadConfig(const AnsiString &File);
   void SaveConfig(const AnsiString &File);
   TStrings * __fastcall GetHostList();
   //wchar_t Hostname[20];
   HostData GlobalConfig;
   bool Incomplete,UpdateSelState;

};
//---------------------------------------------------------------------------
extern TConfDlg *ConfDlg;
//---------------------------------------------------------------------------
#endif
