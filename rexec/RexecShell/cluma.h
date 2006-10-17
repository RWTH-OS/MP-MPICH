
#pragma warning( disable: 4049 )  /* more than 64k source lines */

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 5.03.0280 */
/* at Fri Dec 08 14:25:29 2000
 */
/* Compiler settings for ..\cluma.idl, ..\cluma.acf:
    Os (OptLev=s), W1, Zp8, env=Win32 (32b run), ms_ext, c_ext
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
//@@MIDL_FILE_HEADING(  )


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 440
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __cluma_h__
#define __cluma_h__

/* Forward Declarations */ 

#ifdef __cplusplus
extern "C"{
#endif 

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

#ifndef __cluma_INTERFACE_DEFINED__
#define __cluma_INTERFACE_DEFINED__

/* interface cluma */
/* [explicit_handle][version][uuid] */ 

typedef /* [public][public] */ struct __MIDL_cluma_0001
    {
    /* [ref][string] */ unsigned char ServerString[ 100 ];
    struct 
        {
        unsigned long dwMajorVersion;
        unsigned long dwMinorVersion;
        unsigned long dwBuildNumber;
        unsigned long dwPlatformId;
        unsigned char szCSDVersion[ 128 ];
        }	OS;
    struct 
        {
        unsigned long dwNumberOfProcessors;
        unsigned long dwActiveProcessorMask;
        unsigned short wProcessorArchitecture;
        unsigned short wProcessorLevel;
        unsigned short wProcessorRevision;
        unsigned long Mhz;
        unsigned long dwTotalPhysMem;
        }	HW;
    struct 
        {
        /* [ref][string] */ unsigned char Hostname_ip[ 255 ];
        unsigned long NumEntries;
        /* [length_is] */ unsigned long IPS[ 100 ];
        /* [length_is] */ unsigned long Speeds[ 100 ];
        }	IP;
    }	R_MACHINE_INFO;

typedef /* [public][public] */ struct __MIDL_cluma_0005
    {
    unsigned long cb;
    /* [unique][string] */ unsigned char __RPC_FAR *lpReserved;
    /* [unique][string] */ unsigned char __RPC_FAR *lpDesktop;
    /* [unique][string] */ unsigned char __RPC_FAR *lpTitle;
    unsigned long dwX;
    unsigned long dwY;
    unsigned long dwXSize;
    unsigned long dwYSize;
    unsigned long dwXCountChars;
    unsigned long dwYCountChars;
    unsigned long dwFillAttribute;
    unsigned long dwFlags;
    unsigned short wShowWindow;
    unsigned short cbReserved2;
    unsigned long lpReserved2;
    long hStdInput;
    long hStdOutput;
    long hStdError;
    /* [unique][string] */ unsigned char __RPC_FAR *lpPassword;
    }	R_STARTUPINFO;

typedef /* [public][public][public] */ struct __MIDL_cluma_0006
    {
    long hProcess;
    long hThread;
    unsigned long dwProcessId;
    unsigned long dwThreadId;
    }	R_PROCESS_INFORMATION;

typedef /* [public] */ struct __MIDL_cluma_0007
    {
    unsigned long ID;
    /* [ref][string] */ unsigned char __RPC_FAR *name;
    }	R_PROC_ENUM_T;

typedef struct pipe_byte_pipe
    {
    void (__RPC_FAR * pull) (
        char __RPC_FAR * state,
        byte __RPC_FAR * buf,
        unsigned long esize,
        unsigned long __RPC_FAR * ecount );
    void (__RPC_FAR * push) (
        char __RPC_FAR * state,
        byte __RPC_FAR * buf,
        unsigned long ecount );
    void (__RPC_FAR * alloc) (
        char __RPC_FAR * state,
        unsigned long bsize,
        byte __RPC_FAR * __RPC_FAR * buf,
        unsigned long __RPC_FAR * bcount );
    char __RPC_FAR * state;
    } 	byte_pipe;

/* [fault_status][comm_status] */ error_status_t Ping( 
    /* [in] */ handle_t Binding);

/* [fault_status][comm_status] */ error_status_t R_GetLockStatus( 
    /* [in] */ handle_t Binding,
    /* [ref][out] */ int __RPC_FAR *locked,
    /* [string][out] */ unsigned char __RPC_FAR User[ 255 ]);

/* [fault_status][comm_status] */ error_status_t R_LockServer( 
    /* [in] */ handle_t Binding);

/* [fault_status][comm_status] */ error_status_t R_UnlockServer( 
    /* [in] */ handle_t Binding);

/* [fault_status][comm_status] */ error_status_t R_GetConsoleUser( 
    /* [in] */ handle_t Binding,
    /* [string][out] */ unsigned char __RPC_FAR User[ 255 ]);

/* [fault_status][comm_status] */ error_status_t R_GetUserData( 
    /* [in] */ handle_t Binding,
    /* [string][in] */ unsigned char __RPC_FAR *DllName,
    /* [size_is][unique][out][in] */ byte __RPC_FAR result[  ],
    /* [in] */ long BufferSize,
    /* [ref][out] */ long __RPC_FAR *ResultSize);

/* [fault_status][comm_status][callback] */ error_status_t R_GetClientName( 
    /* [in] */ handle_t IDL_handle,
    /* [size_is][ref][out] */ unsigned char __RPC_FAR ClientName[  ],
    /* [ref][out][in] */ unsigned long __RPC_FAR *size);

/* [fault_status][comm_status] */ error_status_t R_Transfer( 
    /* [in] */ handle_t binding,
    /* [ref][in] */ byte_pipe __RPC_FAR *data);

/* [fault_status][comm_status] */ error_status_t R_CreateProcess( 
    /* [in] */ handle_t Binding,
    /* [string][unique][in] */ unsigned char __RPC_FAR *lpApplicationName,
    /* [string][unique][in] */ unsigned char __RPC_FAR *lpCommandLine,
    /* [in] */ unsigned long dwCreationFlags,
    /* [size_is][unique][in] */ byte __RPC_FAR *lpEnvironment,
    /* [in] */ unsigned long EnvSize,
    /* [string][unique][in] */ unsigned char __RPC_FAR *lpCurrentDirectory,
    /* [ref][in] */ R_STARTUPINFO __RPC_FAR *lpStartupInfo,
    /* [ref][out] */ R_PROCESS_INFORMATION __RPC_FAR *lpProcessInformation);

/* [fault_status][comm_status] */ error_status_t R_TerminateProcess( 
    /* [in] */ handle_t Binding,
    /* [ref][in] */ R_PROCESS_INFORMATION __RPC_FAR *lpProcessInformation,
    /* [in] */ unsigned long RetValue);

/* [fault_status][comm_status] */ error_status_t R_GetSystemInfo( 
    /* [in] */ handle_t IDL_handle,
    /* [ref][out][in] */ R_MACHINE_INFO __RPC_FAR *lpMachineInfo);

/* [fault_status][comm_status] */ error_status_t R_GetProcs( 
    /* [in] */ handle_t Binding);

/* [fault_status][comm_status][callback] */ error_status_t R_ProcEnumCallback( 
    /* [in] */ handle_t IDL_handle,
    /* [in] */ unsigned long id,
    /* [ref][string][in] */ unsigned char __RPC_FAR *name,
    /* [ref][string][in] */ unsigned char __RPC_FAR *Owner);

/* [fault_status][comm_status] */ error_status_t R_KillProcess( 
    /* [in] */ handle_t Binding,
    /* [in] */ unsigned long id);

/* [fault_status][comm_status] */ error_status_t R_ShutDown( 
    /* [in] */ handle_t IDL_handle,
    /* [in] */ long restart);


extern RPC_IF_HANDLE cluma_v1_0_c_ifspec;
extern RPC_IF_HANDLE cluma_v1_0_s_ifspec;
#endif /* __cluma_INTERFACE_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


