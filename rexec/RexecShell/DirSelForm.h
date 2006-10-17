//---------------------------------------------------------------------------
#ifndef DirSelFormH
#define DirSelFormH
//---------------------------------------------------------------------------
#include <vcl\Classes.hpp>
#include <vcl\Controls.hpp>
#include <vcl\StdCtrls.hpp>
#include <vcl\Forms.hpp>
#include <vcl\FileCtrl.hpp>
//---------------------------------------------------------------------------
class TDirSel : public TForm
{
__published:	// IDE-verwaltete Komponenten
	TDirectoryListBox *DirList;
	TDriveComboBox *DriveComboBox1;
	TButton *Button1;
	TButton *Button2;
private:	// Benutzer-Deklarationen
public:		// Benutzer-Deklarationen
	__fastcall TDirSel(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern TDirSel *DirSel;
//---------------------------------------------------------------------------
#endif
