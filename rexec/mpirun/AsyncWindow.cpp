// AsyncWindow.cpp: Implementierung der Klasse CAsyncWindow.
//
//////////////////////////////////////////////////////////////////////

#pragma warning (disable : 4786)

#include "AsyncWindow.h"
#include "rexecClient.h"


//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////



DWORD CAsyncWindow::numInstances=0;


extern "C" {

LRESULT CALLBACK WndProc(HWND hwnd,UINT message,WPARAM wparam,LPARAM lparam) {
    
    CAsyncWindow *Window;
    switch(message) {
    case WM_DESTROY: PostQuitMessage(0); return 0;
    case IN_DATA: 
    case ERR_DATA:
	
	Window = (CAsyncWindow*)GetWindowLong(hwnd,GWL_USERDATA);
	Window->HandleData(message,wparam,lparam);	
	return 0;
    }
    return DefWindowProc(hwnd,message,wparam,lparam);
}

}

void CAsyncWindow::HandleData(UINT message,WPARAM wparam,LPARAM lparam) {
    int res;
    FILE *out;
    bool finish;
    
    if(WSAGETSELECTEVENT(lparam) == FD_CLOSE) 
		finish = true;
    else 
		finish=false;

    out = (message == ERR_DATA)?stderr:stdout;

    res = recv(wparam,buf,BUFSIZE-1,0);
    while(res>0) {
      buf[res]=0;
	  fprintf(out,buf);
      if(finish)
	    res = recv(wparam,buf,BUFSIZE-1,0);
      else 
		res =0;
    }  
    if(finish && !(--NumConnections)) DestroyWindow(Handle);
    return;
}




CAsyncWindow::CAsyncWindow(HANDLE hRemoteProc)
{

    WNDCLASS wc;

    if(!numInstances) {
	memset(&wc,0,sizeof(wc));
	wc.lpfnWndProc = WndProc;
	wc.lpszClassName = TEXT("AsyncNtrexecWindow");
	wc.hInstance = GetModuleHandle(0);
	if(!RegisterClass(&wc)) {
	    fprintf(stderr,"RegisterClass failed: %d",GetLastError());
	    return ;
	}
    }
    numInstances++;
    NumConnections=0;
    this->hRemoteProc = hRemoteProc;
    Handle = CreateWindow(TEXT("AsyncNtrexecWindow"),"AsyncWindow",WS_OVERLAPPEDWINDOW,0,0,10,10,
	                  NULL,NULL,GetModuleHandle(0),NULL);
    SetWindowLong(Handle,GWL_USERDATA,(LONG)this);
    
    
}

WPARAM CAsyncWindow::MessageLoop() {
    MSG msg;
    if(!NumConnections) return 0;
    while(GetMessage(&msg,NULL,0,0)) {
	TranslateMessage(&msg);
	DispatchMessage(&msg);
    }
    return msg.wParam;
}

CAsyncWindow::~CAsyncWindow()
{
    if(!--numInstances)
	UnregisterClass(TEXT("AsyncNtrexecWindow"),GetModuleHandle(0));
}

void CAsyncWindow::AddClient() {
    NumConnections += 2;
}
