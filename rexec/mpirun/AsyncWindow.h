// AsyncWindow.h: Schnittstelle für die Klasse CAsyncWindow.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ASYNCWINDOW_H__4902F437_263F_11D4_B06F_00104B755369__INCLUDED_)
#define AFX_ASYNCWINDOW_H__4902F437_263F_11D4_B06F_00104B755369__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <Windows.h>
#include <stdio.h>


#define BUFSIZE 2048
class CAsyncWindow  
{
public:
	CAsyncWindow(HANDLE hRemoteProc);
	void AddClient();
	void RemoveClient() {NumConnections -=2;}
	WPARAM MessageLoop();
	void HandleData(UINT message,WPARAM wparam,LPARAM lparam);
	~CAsyncWindow();
	HWND Handle;
private:
    HANDLE hRemoteProc;
    char buf[BUFSIZE];
    int NumConnections;
    static DWORD numInstances;
};

#endif // !defined(AFX_ASYNCWINDOW_H__4902F437_263F_11D4_B06F_00104B755369__INCLUDED_)
