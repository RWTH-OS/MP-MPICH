//---------------------------------------------------------------------------
#ifndef ExcludeH
#define ExcludeH
//---------------------------------------------------------------------------
#include <vcl\Classes.hpp>
#include <vcl\Controls.hpp>
#include <vcl\StdCtrls.hpp>
#include <vcl\Forms.hpp>
#include <vcl\Buttons.hpp>
//---------------------------------------------------------------------------
class TExcludeForm : public TForm
{
__published:	// IDE-verwaltete Komponenten
	TGroupBox *GroupBox1;
	TListBox *SelectedBox;
	TBitBtn *BitBtn1;
	TBitBtn *BitBtn2;
	TListBox *HostBox;
	TButton *RefreshButton;
	TLabel *Label1;
	TButton *Button1;
	TButton *Button2;
	TButton *Button3;
	void __fastcall UpdateServers(TObject *Sender,bool DoRefresh);
	void __fastcall BitBtn1Click(TObject *Sender);
	void __fastcall BitBtn2Click(TObject *Sender);
	void __fastcall HostBoxDragOver(TObject *Sender, TObject *Source, int X, int Y,
	TDragState State, bool &Accept);
	void __fastcall HostBoxDragDrop(TObject *Sender, TObject *Source, int X, int Y);
	void __fastcall SelectedBoxDragOver(TObject *Sender, TObject *Source, int X,
	int Y, TDragState State, bool &Accept);
	void __fastcall SelectedBoxDragDrop(TObject *Sender, TObject *Source, int X,
	int Y);
	void __fastcall SelectedBoxKeyPress(TObject *Sender, char &Key);
	void __fastcall Button1Click(TObject *Sender);
	void __fastcall Button3Click(TObject *Sender);
	
	void __fastcall FormShow(TObject *Sender);
	void __fastcall ListBoxDrawItem(TWinControl *Control, int Index, TRect &Rect,
	TOwnerDrawState State);

	
	void __fastcall RefreshButtonClick(TObject *Sender);
private:	// Benutzer-Deklarationen
		TStringList *ExcludedList;
public:		// Benutzer-Deklarationen
		TStringList *List;

	__fastcall TExcludeForm(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern TExcludeForm *ExcludeForm;
//---------------------------------------------------------------------------
#endif
