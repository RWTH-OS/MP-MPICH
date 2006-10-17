// PiMfcDlg.h : Header-Datei
//

#if !defined(AFX_PIMFCDLG_H__D7B6D685_69AE_48FD_B55F_E9A30AE0526B__INCLUDED_)
#define AFX_PIMFCDLG_H__D7B6D685_69AE_48FD_B55F_E9A30AE0526B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CPiMfcDlg Dialogfeld

class CPiMfcDlg : public CDialog
{
// Konstruktion
public:
	CPiMfcDlg(CWnd* pParent = NULL);	// Standard-Konstruktor

// Dialogfelddaten
	//{{AFX_DATA(CPiMfcDlg)
	enum { IDD = IDD_PIMFC_DIALOG };
	CEdit	m_Edit_Pi;
	CStatic	m_Static_erg;
	//}}AFX_DATA

	// Vom Klassenassistenten generierte Überladungen virtueller Funktionen
	//{{AFX_VIRTUAL(CPiMfcDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV-Unterstützung
	//}}AFX_VIRTUAL

// Implementierung
protected:
	HICON m_hIcon;

	// Generierte Message-Map-Funktionen
	//{{AFX_MSG(CPiMfcDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnButton1();
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // !defined(AFX_PIMFCDLG_H__D7B6D685_69AE_48FD_B55F_E9A30AE0526B__INCLUDED_)
