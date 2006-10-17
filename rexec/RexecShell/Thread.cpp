//---------------------------------------------------------------------------
#include <vcl\vcl.h>
#pragma hdrstop

#include "Thread.h"
#include "client.h"
#include "Main.h"

#define DBG(m)
//#define DBG(m) { form->DebugMemo->Lines->Add(m);}

//---------------------------------------------------------------------------
//   Important: Methods and properties of objects in VCL can only be
//   used in a method called using Synchronize, for example:
//
//      Synchronize(UpdateCaption);
//
//   where UpdateCaption could look like:
//
//      void __fastcall TPollThread::UpdateCaption()
//      {
//        Form1->Caption = "Updated in a thread";
//      }
//---------------------------------------------------------------------------
__fastcall TPollThread::TPollThread(TClientForm *cForm,HANDLE hPipe,TMemo* outMemo)
   : TThread(TRUE)
{
 form =cForm;
 Dest=outMemo;
 eH=hPipe;
 OnTerminate=OnThreadTerminate;
 Resume();
}

void __fastcall TPollThread::OnThreadTerminate(TObject *Sender) {

   DBG("Thread exiting");
   CloseHandle(eH);
   if(!--(form->NumThreads)) {
      form->state=finished;
      form->Caption = (form->Host + " -- finished -- " );
      MainWindow->StartButton->Enabled=true;
   }
   MainWindow->NodesBox->Repaint();
   if(MainWindow->ActiveMDIChild==form) {
     MainWindow->SpeedButton2->Enabled=false;
     form->Kill1->Enabled=false;
   }
   form->StatusBar1->SimpleText="Program exited";
}

//---------------------------------------------------------------------------
void __fastcall TPollThread::Execute()
{
   	char puffer[100];
	DWORD bytesRead=1,index=0;
   //char txt[80];
	ConnectNamedPipe(eH,0);
	DBG("eH connected ");
	DBG("Reading pipe ");
	while(bytesRead) {
		ReadFile(eH,puffer+index,1,&bytesRead,0);
      if(!bytesRead) break;
      //sprintf(txt,"Read Key %d",puffer[index]);
      //DBG(txt);
      if(puffer[index]!=10) {
        if(puffer[index]==13) {//||index>80) {
           puffer[index]='\0';

           text=puffer;
           Synchronize(outputString);

			  index=0;
			  puffer[0]='\0';
        } else index+=bytesRead;
      }

	}
	if(index>0) {
		puffer[index]='\0';
      text=puffer;
      Synchronize(outputString);
	}

}
//---------------------------------------------------------------------------
void __fastcall TPollThread::setWindowTitle() {
     form->Caption = (form->Host + text );
}

void __fastcall TPollThread::outputString() {
  if(Dest->Lines->Count>2000) {
  	Dest->Lines->BeginUpdate();
     for(int i=0;i<10;i++)
      Dest->Lines->Delete(0);
     Dest->Lines->Add(text);
     Dest->Lines->EndUpdate();
  } else Dest->Lines->Add(text);

}
