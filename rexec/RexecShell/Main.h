//---------------------------------------------------------------------------
#ifndef MainH
#define MainH
#define UNICODE
//---------------------------------------------------------------------------
#include <vcl\Classes.hpp>
#include <vcl\Controls.hpp>
#include <vcl\StdCtrls.hpp>
#include <vcl\Forms.hpp>
#include <vcl\ComCtrls.hpp>
#include <vcl\Dialogs.hpp>
#include <vcl\ExtCtrls.hpp>
#include <vcl\Menus.hpp>
#include <vcl\Buttons.hpp>
#include <vcl\inifiles.hpp>
//---------------------------------------------------------------------------
#include "NetState.h"
#include "PluginManager.h"

#define TRUESTR "TRUE"
#define FALSESTR "FALSE"
class TMainWindow : public TForm
{
__published:	// IDE-managed Components
	TStatusBar *StatusBar1;
	TPanel *Panel1;
	TMainMenu *MainMenu1;
	TMenuItem *Datei1;
	TMenuItem *Auswaehlen;
	TMenuItem *N1;
	TMenuItem *Beenden;
	TPanel *Toolbar;
	TSpeedButton *SpeedButton1;
	TListBox *NodesBox;
	TSpeedButton *StartButton;
	TMenuItem *Window1;
	TMenuItem *Tile1;
	TMenuItem *Cascade1;
   TPanel *Panel2;
   TSpeedButton *SpeedButton2;
   TSpeedButton *SpeedButton3;
   TMenuItem *CloseAll1;
   TFontDialog *FontDialog1;
   TMenuItem *N2;
   TMenuItem *SetFont1;
   TMenuItem *KillAll1;
   TMenuItem *N3;
	TMenuItem *SaveConfig1;
   TMenuItem *LoadConfig1;
   TOpenDialog *OpenDialog1;
   TSaveDialog *SaveDialog1;
   TSpeedButton *SpeedButton4;
   TSpeedButton *SpeedButton5;
	TMenuItem *ShowParTool1;
	TMenuItem *N4;
	TSpeedButton *SpeedButton6;
	TMenuItem *Editexcludelist1;
    TMenuItem *Help1;
    TMenuItem *About1;
    TMenuItem *Clearall1;
	TMenuItem *Run1;
	TMenuItem *Setglobalaccount1;
	TMenuItem *N5;
	TMenuItem *EditIncludelist1;
	TMenuItem *ChangePlugin;
    TSpeedButton *SpeedButtonCloseAll;
	TMenuItem *Info1;
        TMenuItem *N6;
        TMenuItem *RPCEncryption;
        TMenuItem *checkprofilevalidity;
	void __fastcall BeendenClick(TObject *Sender);
	void __fastcall SpeedButton1Click(TObject *Sender);

	
	void __fastcall StartButtonClick(TObject *Sender);
	void __fastcall Tile1Click(TObject *Sender);
	void __fastcall Cascade1Click(TObject *Sender);
	
   
   void __fastcall SpeedButton2Click(TObject *Sender);
   
   
   void __fastcall NodesBoxDblClick(TObject *Sender);
   void __fastcall CloseAll1Click(TObject *Sender);
   void __fastcall FontDialog1Apply(TObject *Sender, HWND Wnd);
   void __fastcall SetFont1Click(TObject *Sender);
   void __fastcall NodesBoxDrawItem(TWinControl *Control, int Index,
   TRect &Rect, TOwnerDrawState State);
   void __fastcall KillAll1Click(TObject *Sender);
   void __fastcall LoadConfig1Click(TObject *Sender);
   void __fastcall SaveConfig1Click(TObject *Sender);
   
	void __fastcall ShowParTool1Click(TObject *Sender);
	void __fastcall Editexcludelist1Click(TObject *Sender);
    void __fastcall About1Click(TObject *Sender);
    void __fastcall Clearall1Click(TObject *Sender);
	
	
	void __fastcall Setglobalaccount1Click(TObject *Sender);
	void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
	void __fastcall FormCreate(TObject *Sender);
	
	void __fastcall FormDestroy(TObject *Sender);
	void __fastcall EditIncludelist1Click(TObject *Sender);
	void __fastcall Window1Click(TObject *Sender);
	
	
	
	
	
	void __fastcall FormShow(TObject *Sender);
	
	void __fastcall Info1Click(TObject *Sender);
        void __fastcall RPCEncryptionClick(TObject *Sender);
        void __fastcall checkprofilevalidityClick(TObject *Sender);
protected:
	void __fastcall ThreadExit(TMessage &Message);
    void __fastcall ThreadStart(TMessage &Message);
    void __fastcall OnPluginChange(TMessage &Message);
   	BEGIN_MESSAGE_MAP
        MESSAGE_HANDLER(REFRESH_START,TMessage,ThreadStart);
		MESSAGE_HANDLER(REFRESH_FINISH,TMessage,ThreadExit);
        MESSAGE_HANDLER(PM_CHANGE,TMessage,OnPluginChange);
   	END_MESSAGE_MAP(TForm)    
private:	// User declarations
	TMenuItem *PlgItem;
    TAccount GlobalAccount;
    int oldPlugin;
	void __fastcall ShowHint(TObject * Sender);
    void __fastcall CreatePluginMenu();
    void __fastcall PluginClick(TObject *Sender);
    BOOL ShownBefore;

    TIniFile *IniFile;

public:		// User declarations
	__fastcall TMainWindow(TComponent* Owner);
   __fastcall ~TMainWindow();
   AnsiString ConfName;

};


//---------------------------------------------------------------------------
extern TMainWindow *MainWindow;
extern CRITICAL_SECTION CS;
extern char name[256];
//---------------------------------------------------------------------------
#endif
