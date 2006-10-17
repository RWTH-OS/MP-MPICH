// PiMfc.cpp : Legt das Klassenverhalten für die Anwendung fest.
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
// CPiMfcApp

BEGIN_MESSAGE_MAP(CPiMfcApp, CWinApp)
	//{{AFX_MSG_MAP(CPiMfcApp)
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPiMfcApp Konstruktion

CPiMfcApp::CPiMfcApp()
{
}

/////////////////////////////////////////////////////////////////////////////
// Das einzige CPiMfcApp-Objekt

CPiMfcApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CPiMfcApp Initialisierung


int myargc;

LPTSTR * myargv, pcline;

BOOL CPiMfcApp::InitInstance()
{
	// Standardinitialisierung

#ifdef _AFXDLL
	Enable3dControls();			// Diese Funktion bei Verwendung von MFC in gemeinsam genutzten DLLs aufrufen
#else
	Enable3dControlsStatic();	// Diese Funktion bei statischen MFC-Anbindungen aufrufen
#endif

    int NumArgs,lauf;
    size_t len;
	LPTSTR lpCommandline;
    LPWSTR *lpwCommandline;

    //get commandline-string
	lpCommandline = GetCommandLine();
     //convert #0-separated commandline-string to array of strings
    lpwCommandline = CommandLineToArgvW(GetCommandLineW(),&NumArgs );

    //convert unicode to ansi strings in array
    myargc = NumArgs;
    myargv = (LPTSTR*) calloc (sizeof(LPTSTR),myargc);

    for (lauf = 0;lauf < NumArgs; lauf++)
    {
        len = wcslen(lpwCommandline[lauf]);
        if (len >0)
        {
            myargv[lauf] = (LPTSTR) calloc(sizeof(char),len+1);
            len = wcstombs(myargv[lauf],lpwCommandline[lauf],len+1);
        }

    }


    
	MPI_Init(&myargc,&myargv);
    MPI_Comm_size(MPI_COMM_WORLD,&numprocs);
	MPI_Comm_rank(MPI_COMM_WORLD,&myid);

	
	if (myid == 0)
	{
		//show dialog form
		CPiMfcDlg dlg;
		m_pMainWnd = &dlg;
		int nResponse = dlg.DoModal();
		if (nResponse == IDOK)
		{
		}
		else if (nResponse == IDCANCEL)
		{
		}
	}
	else
	{
	  //run process invisible
	  int calctype = START_CALCULATION;
	  while(calctype != EXIT_PROGRAM)
	  {	
		  //wait for button click of master process
		  MPI_Bcast(&calctype, 1, MPI_INT, 0, MPI_COMM_WORLD);
		  switch(calctype)
		  {
		  case EXIT_PROGRAM: 
			  {
				  break;
			  }
		  case CALCULATE_INTEGRATING:
			  {
			  	  int n = 0;
				  calc_integrating(n);
			  }
		  };
	  }		
	}
	MPI_Finalize();

   
	//return FALSE to close the application
	return FALSE;
}
