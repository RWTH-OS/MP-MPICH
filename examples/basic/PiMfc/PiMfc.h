// PiMfc.h : Haupt-Header-Datei f�r die Anwendung PIMFC
//

#if !defined(AFX_PIMFC_H__AB674F19_0C65_4E0C_BB68_783EE0023A1D__INCLUDED_)
#define AFX_PIMFC_H__AB674F19_0C65_4E0C_BB68_783EE0023A1D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// Hauptsymbole

/////////////////////////////////////////////////////////////////////////////
// CPiMfcApp:
// Siehe PiMfc.cpp f�r die Implementierung dieser Klasse
//

class CPiMfcApp : public CWinApp
{
public:
	CPiMfcApp();

// �berladungen
	// Vom Klassenassistenten generierte �berladungen virtueller Funktionen
	//{{AFX_VIRTUAL(CPiMfcApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementierung 

	//{{AFX_MSG(CPiMfcApp)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ f�gt unmittelbar vor der vorhergehenden Zeile zus�tzliche Deklarationen ein.

#endif // !defined(AFX_PIMFC_H__AB674F19_0C65_4E0C_BB68_783EE0023A1D__INCLUDED_)
