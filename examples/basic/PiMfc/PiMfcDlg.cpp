// PiMfcDlg.cpp : Implementierungsdatei
//

#include "stdafx.h"
#include "PiMfc.h"
#include "PiMfcDlg.h"
#include "calculation.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPiMfcDlg Dialogfeld

CPiMfcDlg::CPiMfcDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPiMfcDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPiMfcDlg)
		// HINWEIS: Der Klassenassistent fügt hier Member-Initialisierung ein
	//}}AFX_DATA_INIT
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CPiMfcDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPiMfcDlg)
	DDX_Control(pDX, IDC_EDIT_PI, m_Edit_Pi);
	DDX_Control(pDX, IDC_STATIC_ERG, m_Static_erg);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CPiMfcDlg, CDialog)
	//{{AFX_MSG_MAP(CPiMfcDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, OnButton1)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPiMfcDlg Nachrichten-Handler

BOOL CPiMfcDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetIcon(m_hIcon, TRUE);			// Großes Symbol verwenden
	SetIcon(m_hIcon, FALSE);		// Kleines Symbol verwenden
	
	// ZU ERLEDIGEN: Hier zusätzliche Initialisierung einfügen
	
	return TRUE;  // Geben Sie TRUE zurück, außer ein Steuerelement soll den Fokus erhalten
}

// Wollen Sie Ihrem Dialogfeld eine Schaltfläche "Minimieren" hinzufügen, benötigen Sie 
//  den nachstehenden Code, um das Symbol zu zeichnen. Für MFC-Anwendungen, die das 
//  Dokument/Ansicht-Modell verwenden, wird dies automatisch für Sie erledigt.

void CPiMfcDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // Gerätekontext für Zeichnen

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Symbol in Client-Rechteck zentrieren
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Symbol zeichnen
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

HCURSOR CPiMfcDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}
 
void CPiMfcDlg::OnButton1() 
{
	CString hstr;
	int n = 0, calctype = CALCULATE_INTEGRATING;
	double pi = 0;
	double startwtime, endwtime;

    
	m_Edit_Pi.GetWindowText(hstr);
   
    n = abs(atoi(hstr));
	if (n<1)
		n=1;
	
	//wake up remote processes
	MPI_Bcast(&calctype, 1, MPI_INT, 0, MPI_COMM_WORLD);

    startwtime = MPI_Wtime();
	pi = calc_integrating (n);
	endwtime = MPI_Wtime();
  

	hstr.Format("number processes = %i,  n = %i, PI = %f\nwall clock time = %f\n"
		,numprocs,n,pi,endwtime-startwtime);
		
	m_Static_erg.SetWindowText(hstr);
}

void CPiMfcDlg::OnCancel() 
{
	int calctype = EXIT_PROGRAM;
	
	MPI_Bcast(&calctype, 1, MPI_INT, 0, MPI_COMM_WORLD);
	
	CDialog::OnCancel();
}
