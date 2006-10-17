//---------------------------------------------------------------------------
#ifndef ThreadH
#define ThreadH
//---------------------------------------------------------------------------
#include <vcl\Classes.hpp>
//---------------------------------------------------------------------------

class TClientForm;

class TPollThread : public TThread
{
private:
protected:
   void __fastcall Execute();
public:
   __fastcall TPollThread(TClientForm *cForm,HANDLE hPipe,TMemo* outMemo);
   void __fastcall setWindowTitle();
   void __fastcall outputString();
   void __fastcall OnThreadTerminate(TObject *Sender);
   TMemo *Dest;
   HANDLE eH;
   AnsiString text;
   TClientForm *form;
};
//---------------------------------------------------------------------------
#endif
 