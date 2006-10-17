//---------------------------------------------------------------------------
#ifndef MainformH
#define MainformH
//---------------------------------------------------------------------------
#include <vcl\Classes.hpp>
#include <vcl\Controls.hpp>
#include <vcl\StdCtrls.hpp>
#include <vcl\Forms.hpp>
#include <vcl\ComCtrls.hpp>
#include <vcl\ExtCtrls.hpp>
#include <vcl\Buttons.hpp>
#include <vcl\Menus.hpp>
//---------------------------------------------------------------------------

#include "NetState.h"
#include <ImgList.hpp>

class TParWindow : public TForm
{
__published:	// IDE-verwaltete Komponenten
	TListView *ListView;
	TImageList *ImageList1;
	TPanel *Panel1;
	TSpeedButton *QuitButton;
	TSpeedButton *LockButton;
	TSpeedButton *UnlockButton;
	TSpeedButton *RefreshButton;
	TPopupMenu *PopupMenu1;
	TMenuItem *LockUnlockServer;
	TMenuItem *Refresh1;
	TTimer *Timer1;
	TSpeedButton *TimerButton;
	TSpeedButton *StickButton;
	TPanel *Panel2;
	TProgressBar *ProgressBar;
	TMenuItem *N1;
	TMenuItem *Showdetails1;
	TMenuItem *SetAccount1;
	TMainMenu *MainMenu1;
	TMenuItem *Server1;
	TMenuItem *View1;
	TMenuItem *Refresh2;
	TMenuItem *Locked1;
	TMenuItem *SetAccount2;
	TMenuItem *Showdetails2;
	TMenuItem *N2;
	TMenuItem *Exit1;
	TMenuItem *Stayontop1;
	TMenuItem *Processes1;
	TMenuItem *Processes2;
	TMenuItem *RefreshSelected;
	TMenuItem *N3;
	TMenuItem *Restart1;
	TMenuItem *ShutDown1;
	TTimer *RebootTimer;
	void __fastcall QuitButtonClick(TObject *Sender);
	
	void __fastcall RefreshButtonClick(TObject *Sender);
	
	
	
	
	void __fastcall UnlockButtonClick(TObject *Sender);
	void __fastcall LockButtonClick(TObject *Sender);

	void __fastcall ListViewChange(TObject *Sender, TListItem *Item,
	TItemChange Change);
	void __fastcall LockUnlockServerClick(TObject *Sender);

	void __fastcall FormShow(TObject *Sender);
	void __fastcall StickButtonClick(TObject *Sender);
	void __fastcall FormCreate(TObject *Sender);
	void __fastcall Showdetails1Click(TObject *Sender);
	void __fastcall SetAccount1Click(TObject *Sender);
	void __fastcall Stayontop1Click(TObject *Sender);
	void __fastcall Processes1Click(TObject *Sender);
	void __fastcall PopupMenu1Popup(TObject *Sender);
	void __fastcall RefreshSelectedClick(TObject *Sender);
	void __fastcall Restart1Click(TObject *Sender);
	void __fastcall ShutDown1Click(TObject *Sender);
	void __fastcall RebootTimerTimer(TObject *Sender);
private:	// Benutzer-Deklarationen
protected:
	void __fastcall TParWindow::ThreadExit(TMessage Message);
    void __fastcall TParWindow::ThreadStart(TMessage Message);
    void __fastcall TParWindow::ParData(TMessage Message);
   	BEGIN_MESSAGE_MAP
    	MESSAGE_HANDLER(PAR_DATA,TMessage,ParData);
        MESSAGE_HANDLER(ENUM_START,TMessage,ThreadStart);
		MESSAGE_HANDLER(ENUM_FINISH,TMessage,ThreadExit);
        MESSAGE_HANDLER(REFRESH_START,TMessage,ThreadStart);
		MESSAGE_HANDLER(REFRESH_FINISH,TMessage,ThreadExit);
   	END_MESSAGE_MAP(TForm)
public:		// Benutzer-Deklarationen
	__fastcall TParWindow(TComponent* Owner);
    LONG LastSize;
    bool Incomplete;
};
//---------------------------------------------------------------------------
extern TParWindow *ParWindow;
//---------------------------------------------------------------------------
#endif
