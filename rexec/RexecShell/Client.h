//---------------------------------------------------------------------------
#ifndef ClientH
#define ClientH
#define UNICODE
//---------------------------------------------------------------------------
#include <vcl\Classes.hpp>
#include <vcl\Controls.hpp>
#include <vcl\StdCtrls.hpp>
#include <vcl\Forms.hpp>
#include <vcl\ComCtrls.hpp>
#include <vcl\ExtCtrls.hpp>
#include <vcl\Menus.hpp>
#include <vcl\Dialogs.hpp>
//---------------------------------------------------------------------------
#pragma hdrstop

#include "RexecClient.h"
enum TaskStates {running,killing,finished,init,error};

class CHostRef;
class TClientForm : public TForm
{
__published:	// IDE-managed Components
	TStatusBar *StatusBar1;
   TMainMenu *ClientMenu;
   TMenuItem *Task1;
   TMenuItem *Kill1;
	TGroupBox *OutBox;
   TMemo *OutMemo;
   TGroupBox *ErrBox;
   TMemo *ErrMemo;
   TMenuItem *ShowStderr1;
   TMenuItem *N1;
    TMenuItem *N2;
    TMenuItem *ClearOutput1;
	TMenuItem *Edit1;
	TMenuItem *PrintStdout1;
	TMenuItem *PrintStderr1;
	TPrintDialog *PrintDialog1;
	TMenuItem *N3;
	TMenuItem *SaveStdout1;
	TMenuItem *SaveStderr1;
	TSaveDialog *SaveDialog1;
	TMenuItem *EchoInput1;
	void __fastcall FormCloseQuery(TObject *Sender, bool &CanClose);
	void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
	void __fastcall OutMemoKeyPress(TObject *Sender, char &Key);

   void __fastcall Kill1Click(TObject *Sender);
   
   void __fastcall OutMemoEnter(TObject *Sender);
   void __fastcall ShowStderr1Click(TObject *Sender);


   void __fastcall ErrBoxMouseDown(TObject *Sender, TMouseButton Button,
   TShiftState Shift, int X, int Y);
   void __fastcall ErrBoxMouseUp(TObject *Sender, TMouseButton Button,
   TShiftState Shift, int X, int Y);
   void __fastcall ErrBoxMouseMove(TObject *Sender, TShiftState Shift, int X,
   int Y);
   void __fastcall FormCreate(TObject *Sender);
	void __fastcall FormResize(TObject *Sender);

	void __fastcall FormDestroy(TObject *Sender);
    void __fastcall ClearOutput1Click(TObject *Sender);
	
	void __fastcall PrintStdout1Click(TObject *Sender);
	void __fastcall PrintStderr1Click(TObject *Sender);
	void __fastcall SaveStdout1Click(TObject *Sender);
	void __fastcall SaveStderr1Click(TObject *Sender);
	void __fastcall EchoInput1Click(TObject *Sender);
private:	// User declarations
//friend LRESULT CALLBACK ThreadMsgProc(HWND me,UINT MSG,WPARAM WParam,LPARAM LParam);
protected:
    void __fastcall Finished();
	void __fastcall PipeData(TMessage Message);
    void __fastcall InitFinished(TMessage Message);
    void __fastcall KillFailed(TMessage Msg);
	BEGIN_MESSAGE_MAP
     MESSAGE_HANDLER(IN_DATA,TMessage,PipeData);
     MESSAGE_HANDLER(ERR_DATA,TMessage,PipeData);
     MESSAGE_HANDLER(INIT_FINISH,TMessage,InitFinished);
     MESSAGE_HANDLER(KILL_FAILED,TMessage,KillFailed);
   END_MESSAGE_MAP(TForm)
public:		// User declarations
	//AnsiString Host;
   void Execute(CHostRef *r);
	__fastcall TClientForm(TComponent* Owner);
    __fastcall ~TClientForm();
   void applyFont();
   bool local,closed,Splitting,complete[2];
   HANDLE inH,hRemoteProc;
   volatile TaskStates state;
   DWORD Connections;
   static TFont* MemoFont;
   double ratio;
   int pos;
   CHostRef *Ref;
   //CRITICAL_SECTION ClientCS;
   //HostData *Server;
   TStringList *Strings;

};
//---------------------------------------------------------------------------
extern TClientForm *ClientForm;


//---------------------------------------------------------------------------
#endif
