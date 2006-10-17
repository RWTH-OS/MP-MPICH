//---------------------------------------------------------------------------
#include <vcl\vcl.h>
#include <fstream.h>
#include <dos.h>
#pragma hdrstop

#include "Include.h"
#include "NetState.h"
#include "ParForm.h"
//---------------------------------------------------------------------------
#pragma resource "*.dfm"
TIncludeForm *IncludeForm;
//---------------------------------------------------------------------------
__fastcall TIncludeForm::TIncludeForm(TComponent* Owner)
	: TForm(Owner)
{
     /*char filename[MAX_PATH];
     ifstream s;

     List=new TStringList;
   	 sprintf(filename,"%sIncluded.rsh",_argv[0]);
     s.open(filename);
   	 if(!s.fail()) {
     	while(!s.eof()) {
       		s>>filename;
   	        if(!filename[0]) continue;
            List->Add(filename);
       	}
       	s.close();
   	}
    Disabled = false;
    if(ParWindow->Visible) ParWindow->FormShow(0); */
}
//---------------------------------------------------------------------------
void __fastcall TIncludeForm::FormShow(TObject *Sender)
{
	SelectedBox->Items->Assign(List);
}
//---------------------------------------------------------------------------
void __fastcall TIncludeForm::Button1Click(TObject *Sender)
{
	AnsiString Name = HostEdit->Text.Trim().UpperCase();
	if(Name.Length()>0 && SelectedBox->Items->IndexOf(Name)<0 &&
       Servers->IndexOf(Name.c_str())<0) {
    	SelectedBox->Items->Add(Name);
        HostEdit->Text="";
    }
}
//---------------------------------------------------------------------------
void __fastcall TIncludeForm::Button2Click(TObject *Sender)
{
	List->Assign(SelectedBox->Items);
    if(ParWindow->Visible) ParWindow->FormShow(0);
}
//---------------------------------------------------------------------------
void __fastcall TIncludeForm::Button3Click(TObject *Sender)
{
	ofstream s;
    int res=IDRETRY;
    char filename[MAX_PATH];
    sprintf(filename,"%sIncluded.rsh",_argv[0]);
    /* does not work with CBuilder5 replace with SaveToFile
    s.open(filename);
    while(s.fail()&&res==IDRETRY) {
		res=Application->MessageBox("Could not open file Included.rsh",
        "File error",MB_ICONERROR|MB_RETRYCANCEL);
        if(res==IDRETRY) s.open(filename);
    }
    for(int i=0;i<SelectedBox->Items->Count;i++) {
    	s<<SelectedBox->Items->Strings[i]<<endl;
    }
    */
    SelectedBox->Items->SaveToFile(filename);
}
//---------------------------------------------------------------------------
void __fastcall TIncludeForm::SelectedBoxDblClick(TObject *Sender)
{
	int i=0;
    if(Disabled) {
    	Application->MessageBox("Cannot modify include list while refresh in progess","Message",
        						 MB_OK|MB_ICONWARNING);
        return;
    }

   SelectedBox->Items->BeginUpdate();
   while(SelectedBox->SelCount) {
        if(SelectedBox->Selected[i]) {
           Servers->Delete(Servers->IndexOf(SelectedBox->Items->Strings[i].c_str()));
           SelectedBox->Items->Delete(i);
        } else i++;
   }
   SelectedBox->Items->EndUpdate();
}
//---------------------------------------------------------------------------
void __fastcall TIncludeForm::SelectedBoxKeyDown(TObject *Sender, WORD &Key,
	TShiftState Shift)
{
	if(Key==46) SelectedBoxDblClick(this);
}
//---------------------------------------------------------------------------
void __fastcall TIncludeForm::FormCreate(TObject *Sender)
{
	char filename[MAX_PATH];
     ifstream s;

     List=new TStringList;
   	 sprintf(filename,"%sIncluded.rsh",_argv[0]);
     s.open(filename);
   	 if(!s.fail()) {
     	while(!s.eof()) {
       		s>>filename;
   	        if(!filename[0]) continue;
            List->Add(filename);
       	}
       	s.close();
   	}
    Disabled = false;
    if(ParWindow->Visible) ParWindow->FormShow(0);
    
	Servers->RegisterClientWindow(Handle);
}

void __fastcall TIncludeForm::ParData(TMessage Message) {
	switch(Message.Msg) {
    case ENUM_START:
    case PAR_DATA:
    case REFRESH_START:
    	Disabled = true;
        Button1->Enabled = false;
        break;
	default: Disabled = false;
     		 Button1->Enabled = true;
    }
}
//---------------------------------------------------------------------------