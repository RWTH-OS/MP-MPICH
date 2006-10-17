//---------------------------------------------------------------------------
#include <vcl\vcl.h>
#pragma hdrstop

#include "cluma.h"
#include "ProcWindow.h"
//---------------------------------------------------------------------------
#pragma resource "*.dfm"
extern char *MakeErrorMessage(DWORD ErrorId);

TProcForm *ProcForm;

int __stdcall CmpId(TListItem *I1,TListItem* I2,int param) {
	if(I1->Caption.ToInt() < I2->Caption.ToInt()) return -1;
    if(I1->Caption.ToInt() > I2->Caption.ToInt()) return 1;
    else return 0;
}

int __stdcall CmpName(TListItem *I1,TListItem* I2,int param) {
	return strcmp(I1->SubItems->Strings[param].c_str(),
                  I2->SubItems->Strings[param].c_str());
}



//---------------------------------------------------------------------------
__fastcall TProcForm::TProcForm(TComponent* Owner)
	: TForm(Owner)
{
#if ((BCBVER > 1))
  //CBuilder5
  Compare = (PFNLVCOMPARE)CmpId;
#else
  //CBuilder1
  Compare = (TLVCompare)CmpId;
#endif

}

//---------------------------------------------------------------------------

void __fastcall TProcForm::OnProcEnumStart(TMessage &Message) {
	if(Message.Msg == RC_PROC_START) {
		ListView->Items->BeginUpdate();
		ListView->Items->Clear();
        ActHost = (HostData*)Message.LParam;
        Caption = AnsiString("Processes on ")+ActHost->Name;
    } else {
    	ListView->CustomSort(Compare,SubIndex);
    	ListView->Items->EndUpdate();
    }
}

void __fastcall TProcForm::OnProcEnum(TMessage &Message) {
	Enum_t *p;
	TListItem *Item;
    p = (Enum_t*)Message.LParam;
    Item = ListView->Items->Add();
    Item->Caption = AnsiString((int)p->ID);
    Item->SubItems->Add(p->Name);
    if(*p->Owner) Item->SubItems->Add(p->Owner);
}

void __fastcall TProcForm::FormClose(TObject *Sender, TCloseAction &Action)
{
 	Action = caFree;	
}
//---------------------------------------------------------------------------
void __fastcall TProcForm::Button1Click(TObject *Sender)
{
	Close();	
}
//---------------------------------------------------------------------------
void __fastcall TProcForm::RefreshButtonClick(TObject *Sender)
{
	GetHostProcs(Handle,ActHost);
}
//---------------------------------------------------------------------------
void __fastcall TProcForm::KillButtonClick(TObject *Sender)
{
	TListItem *Item;
    int id;
    DWORD res;

  	Item=ListView->ItemFocused;
    if(!Item) return;
    id = Item->Caption.ToInt();
    res = KillProcess(ActHost,id);
	if( res != ERROR_SUCCESS)
    	Application->MessageBox(MakeErrorMessage(res),"Kill failed",MB_OK|MB_ICONERROR);
    else ListView->Items->Delete(Item->Index);
}
//---------------------------------------------------------------------------
void __fastcall TProcForm::ListViewChange(TObject *Sender, TListItem *Item,
	TItemChange Change)
{
	if(Change==ctState) {
    	if(ListView->ItemFocused != NULL)
        	KillButton->Enabled = true;
        else
        	KillButton->Enabled = false;
    }
}
//---------------------------------------------------------------------------
void __fastcall TProcForm::ListViewColumnClick(TObject *Sender,
	TListColumn *Column)
{
#if ((BCBVER > 1))
  //CBuilder5
  if(Column->Index == 0)
      Compare = (PFNLVCOMPARE)CmpId;
  else
      Compare = (PFNLVCOMPARE)CmpName;
#else
  //CBuilder1
  if(Column->Index == 0)
      Compare = (TLVCompare)CmpId;
  else
      Compare = (TLVCompare)CmpName;
#endif

    SubIndex = Column->Index-1;
    ListView->CustomSort(Compare,SubIndex);
}
//---------------------------------------------------------------------------
