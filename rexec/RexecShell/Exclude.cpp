//---------------------------------------------------------------------------
#define UNICODE
#include <vcl\vcl.h>
#define NET_API_STATUS   DWORD
#define NET_API_FUNCTION __stdcall
#include <Lmserver.h>
#include <lmapibuf.h>

#include <fstream.h>

#if ((BCBVER > 1))
  //CBuilder5
  #include <stdio.h>
#endif

//#include <dos.h>
#pragma hdrstop

#include "Exclude.h"
#include "NetState.h"
#include "ParForm.h"
//---------------------------------------------------------------------------
#pragma resource "*.dfm"
TExcludeForm *ExcludeForm;
#if (!(BCBVER > 1))
   //CBuilder1
   extern char** _argv;
#endif

//---------------------------------------------------------------------------
__fastcall TExcludeForm::TExcludeForm(TComponent* Owner)
	: TForm(Owner)
{

     char filename[MAX_PATH];
     ifstream s;

     List=new TStringList;
     ExcludedList = new TStringList;
   	 sprintf(filename,"%sExcluded.rsh",_argv[0]);
     s.open(filename);
   	 if(!s.fail()) {
     	while(!s.eof()) {
       		s>>filename;
   	        if(!filename[0]) continue;
            List->Add(filename);
       	}
       	s.close();
   	}

    if(ParWindow->Visible) ParWindow->FormShow(0);
}
//---------------------------------------------------------------------------
void __fastcall TExcludeForm::UpdateServers(TObject *Sender,bool DoRefresh = true)
{
	 DWORD EntriesRead,totalentries,i;
     SERVER_INFO_101 *Sv=0;
     AnsiString Host;

 	 NetServerEnum(0,101,(unsigned char**)&Sv,-1,
        	&EntriesRead,&totalentries,SV_TYPE_NT,0,0);
     HostBox->Items->BeginUpdate();
     HostBox->Items->Clear();
	 for(i=0;i<EntriesRead;i++) {
     	Host=AnsiString(Sv[i].sv101_name).UpperCase();
        if(SelectedBox->Items->IndexOf(Host)<0)
    		HostBox->Items->Add(Host);
     }
     HostBox->Items->EndUpdate();
     if(Sv) NetApiBufferFree(Sv);
     if (DoRefresh)
        Servers->Refresh();
}
//---------------------------------------------------------------------------
void __fastcall TExcludeForm::BitBtn1Click(TObject *Sender)
{
	int i;
    i=0;
   SelectedBox->Items->BeginUpdate();
   HostBox->Items->BeginUpdate();
    while(i<HostBox->Items->Count) {
     if(HostBox->Selected[i]) {
        HostBox->Selected[i]=false;
        SelectedBox->Items->Add(HostBox->Items->Strings[i]);
        HostBox->Items->Delete(i);
     } else i++;
   }
  SelectedBox->Items->EndUpdate();
  HostBox->Items->EndUpdate();
}
//---------------------------------------------------------------------------

void __fastcall TExcludeForm::BitBtn2Click(TObject *Sender)
{
   int i=0;
   SelectedBox->Items->BeginUpdate();
   HostBox->Items->BeginUpdate();
   while(SelectedBox->SelCount) {
        if(SelectedBox->Selected[i]) {
           HostBox->Items->Add(SelectedBox->Items->Strings[i]);
           SelectedBox->Items->Delete(i);
        } else i++;
   }
   SelectedBox->Items->EndUpdate();
   HostBox->Items->EndUpdate();

}
//---------------------------------------------------------------------------
void __fastcall TExcludeForm::HostBoxDragOver(TObject *Sender, TObject *Source, int X,
	int Y, TDragState State, bool &Accept)
{
	if(Source==SelectedBox) Accept=true;
	else Accept=false;
}
//---------------------------------------------------------------------------
void __fastcall TExcludeForm::HostBoxDragDrop(TObject *Sender, TObject *Source, int X,
	int Y)
{
	if(Source != SelectedBox) return;
   	BitBtn2Click(Source);
}
//---------------------------------------------------------------------------
void __fastcall TExcludeForm::SelectedBoxDragOver(TObject *Sender, TObject *Source,
	int X, int Y, TDragState State, bool &Accept)
{
	if(Source==HostBox) Accept=true;
	else Accept=false;
}
//---------------------------------------------------------------------------
void __fastcall TExcludeForm::SelectedBoxDragDrop(TObject *Sender, TObject *Source,
	int X, int Y)
{
	if(Source != HostBox) return;
   	BitBtn1Click(Source);
}
//---------------------------------------------------------------------------
void __fastcall TExcludeForm::SelectedBoxKeyPress(TObject *Sender, char &Key)
{
	if(Key==46) BitBtn2Click(this);	
}
//---------------------------------------------------------------------------
void __fastcall TExcludeForm::Button1Click(TObject *Sender)
{
	List->Assign(SelectedBox->Items);

/*
    for(int i=0;i<List->Count;i++)
	    Servers.Delete(Servers.IndexOf(List->Strings[i].c_str()));

    for(int i=0;i<HostBox->Items->Count;i++)
	    Servers.Refresh(HostBox->Items->Strings[i].c_str());
 */
    Servers->Refresh();
    if(ParWindow->Visible) ParWindow->FormShow(0);

    //Hide();
}
//---------------------------------------------------------------------------
void __fastcall TExcludeForm::Button3Click(TObject *Sender)
{
	ofstream s;
    int res=IDRETRY;
    char filename[MAX_PATH];
    sprintf(filename,"%sExcluded.rsh",_argv[0]);
    /* does not work with CBuilder5 replace with SaveToFile
    s.open(filename);
    while(s.fail()&&res==IDRETRY) {
		res=Application->MessageBox("Could not open file Excluded.rsh",
        "File error",MB_ICONERROR|MB_RETRYCANCEL);
        if(res==IDRETRY) s.open(filename);
    }
    for(int i=0;i<SelectedBox->Items->Count;i++) {
    	s<<SelectedBox->Items->Strings[i]<<endl;
    }*/
    SelectedBox->Items->SaveToFile(filename);
}
//---------------------------------------------------------------------------
void __fastcall TExcludeForm::FormShow(TObject *Sender)
{
     SelectedBox->Items->Assign(List);
     ExcludedList->Duplicates=dupIgnore;
     ExcludedList->Assign(List);
     this->UpdateServers(Sender,false);
}
//---------------------------------------------------------------------------
void __fastcall TExcludeForm::ListBoxDrawItem(TWinControl *Control, int Index,
	TRect &Rect, TOwnerDrawState State)
{
    int Offset = 2;    // default offset
    TColor FC,BC,PC;
    HostData *actHost;
	TListBox *ListBox = dynamic_cast<TListBox*>(Control);
	TCanvas *Canvas = ListBox->Canvas;
    char *ListName = NULL;   //for debugging
    ListName = ListBox->Name.c_str();   //for debugging

	BC=Canvas->Brush->Color;
    PC=Canvas->Pen->Color;
    FC=Canvas->Font->Color;

    if ((ListName == "SelectedBox")   //for debugging
    || (ListBox->Items->Strings[Index] == "PULASKI"))    //for debugging
    {
    PC=Canvas->Pen->Color;      //for debugging
    }

	Canvas->FillRect(Rect);                       // clear rectangle

	// show text
    if (ExcludedList->IndexOf(ListBox->Items->Strings[Index])>=0)
    {
    	//host was excluded
        Canvas->Font->Color=clOlive;
    }
    else
    {
      actHost=(*Servers)[ListBox->Items->Strings[Index].c_str()];
      if(!(actHost)||!actHost->Alive||!(actHost->State) || !(actHost->State->Valid))
    	Canvas->Font->Color=clRed;
      else
    	Canvas->Font->Color=FC;
    }
	Canvas->TextOut(Rect.Left + Offset, Rect.Top, ListBox->Items->Strings[Index]);

    Canvas->Font->Color=FC;
    Canvas->Brush->Color=BC;
    Canvas->Pen->Color=PC;
}
//---------------------------------------------------------------------------
void __fastcall TExcludeForm::RefreshButtonClick(TObject *Sender)
{
	UpdateServers(this);	
}
//---------------------------------------------------------------------------
