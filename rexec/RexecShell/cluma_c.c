/* this ALWAYS GENERATED file contains the RPC client stubs */


/* File created by MIDL compiler version 5.01.0164 */
/* at Wed Jan 03 15:42:36 2007
 */
/* Compiler settings for ..\cluma.idl, ..\cluma.acf:
    Os (OptLev=s), W1, Zp8, env=Win32, ms_ext, c_ext
    error checks: allocation ref bounds_check enum stub_data 
*/
//@@MIDL_FILE_HEADING(  )

#include <string.h>
#if defined( _ALPHA_ )
#include <stdarg.h>
#endif

#include "cluma.h"

#define TYPE_FORMAT_STRING_SIZE   285                               
#define PROC_FORMAT_STRING_SIZE   157                               

typedef struct _MIDL_TYPE_FORMAT_STRING
    {
    short          Pad;
    unsigned char  Format[ TYPE_FORMAT_STRING_SIZE ];
    } MIDL_TYPE_FORMAT_STRING;

typedef struct _MIDL_PROC_FORMAT_STRING
    {
    short          Pad;
    unsigned char  Format[ PROC_FORMAT_STRING_SIZE ];
    } MIDL_PROC_FORMAT_STRING;


extern const MIDL_TYPE_FORMAT_STRING __MIDL_TypeFormatString;
extern const MIDL_PROC_FORMAT_STRING __MIDL_ProcFormatString;

/* Standard interface: cluma, ver. 1.0,
   GUID={0xf3d150be,0x4b47,0x11d3,{0xa8,0xe4,0x00,0x10,0x4b,0x75,0x53,0x69}} */



extern RPC_DISPATCH_TABLE cluma_v1_0_DispatchTable;

static const RPC_CLIENT_INTERFACE cluma___RpcClientInterface =
    {
    sizeof(RPC_CLIENT_INTERFACE),
    {{0xf3d150be,0x4b47,0x11d3,{0xa8,0xe4,0x00,0x10,0x4b,0x75,0x53,0x69}},{1,0}},
    {{0x8A885D04,0x1CEB,0x11C9,{0x9F,0xE8,0x08,0x00,0x2B,0x10,0x48,0x60}},{2,0}},
    &cluma_v1_0_DispatchTable,
    0,
    0,
    0,
    0,
    RPC_INTERFACE_HAS_PIPES
    };
RPC_IF_HANDLE cluma_v1_0_c_ifspec = (RPC_IF_HANDLE)& cluma___RpcClientInterface;

extern const MIDL_STUB_DESC cluma_StubDesc;

static RPC_BINDING_HANDLE cluma__MIDL_AutoBindHandle;


/* [fault_status][comm_status] */ error_status_t Ping( 
    /* [in] */ handle_t Binding)
{

    RPC_BINDING_HANDLE _Handle	=	0;
    
    error_status_t _RetVal;
    
    RPC_MESSAGE _RpcMessage;
    
    RPC_STATUS _Status;
    
    MIDL_STUB_MESSAGE _StubMsg;
    
    RpcTryExcept
        {
        RpcTryFinally
            {
            NdrClientInitializeNew(
                          ( PRPC_MESSAGE  )&_RpcMessage,
                          ( PMIDL_STUB_MESSAGE  )&_StubMsg,
                          ( PMIDL_STUB_DESC  )&cluma_StubDesc,
                          0);
            
            
            _Handle = Binding;
            
            
            _StubMsg.BufferLength = 0U;
            NdrGetBuffer( (PMIDL_STUB_MESSAGE) &_StubMsg, _StubMsg.BufferLength, _Handle );
            
            NdrSendReceive( (PMIDL_STUB_MESSAGE) &_StubMsg, (unsigned char __RPC_FAR *) _StubMsg.Buffer );
            
            if ( (_RpcMessage.DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[0] );
            
            _RetVal = *(( error_status_t __RPC_FAR * )_StubMsg.Buffer)++;
            
            }
        RpcFinally
            {
            NdrFreeBuffer( (PMIDL_STUB_MESSAGE) &_StubMsg );
            
            }
        RpcEndFinally
        
        }
    RpcExcept( 1 )
        {
        if(_Status = NdrMapCommAndFaultStatus(( PMIDL_STUB_MESSAGE  )&_StubMsg,( unsigned long __RPC_FAR * )&_RetVal,( unsigned long __RPC_FAR * )&_RetVal,RpcExceptionCode()))
            {
            RpcRaiseException(_Status);
            }
        }
    RpcEndExcept
    return _RetVal;
}


/* [fault_status][comm_status] */ error_status_t R_GetLockStatus( 
    /* [in] */ handle_t Binding,
    /* [ref][out] */ int __RPC_FAR *locked,
    /* [string][out] */ unsigned char __RPC_FAR User[ 255 ])
{

    RPC_BINDING_HANDLE _Handle	=	0;
    
    error_status_t _RetVal;
    
    RPC_MESSAGE _RpcMessage;
    
    RPC_STATUS _Status;
    
    MIDL_STUB_MESSAGE _StubMsg;
    
    RpcTryExcept
        {
        if(!locked)
            {
            RpcRaiseException(RPC_X_NULL_REF_POINTER);
            }
        RpcTryFinally
            {
            NdrClientInitializeNew(
                          ( PRPC_MESSAGE  )&_RpcMessage,
                          ( PMIDL_STUB_MESSAGE  )&_StubMsg,
                          ( PMIDL_STUB_DESC  )&cluma_StubDesc,
                          1);
            
            
            _Handle = Binding;
            
            
            _StubMsg.BufferLength = 0U;
            NdrGetBuffer( (PMIDL_STUB_MESSAGE) &_StubMsg, _StubMsg.BufferLength, _Handle );
            
            NdrSendReceive( (PMIDL_STUB_MESSAGE) &_StubMsg, (unsigned char __RPC_FAR *) _StubMsg.Buffer );
            
            if ( (_RpcMessage.DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[4] );
            
            *locked = *(( int __RPC_FAR * )_StubMsg.Buffer)++;
            
            NdrNonConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                              (unsigned char __RPC_FAR * __RPC_FAR *)&User,
                                              (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[6],
                                              (unsigned char)0 );
            
            _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            _RetVal = *(( error_status_t __RPC_FAR * )_StubMsg.Buffer)++;
            
            }
        RpcFinally
            {
            NdrFreeBuffer( (PMIDL_STUB_MESSAGE) &_StubMsg );
            
            }
        RpcEndFinally
        
        }
    RpcExcept( 1 )
        {
        if(_Status = NdrMapCommAndFaultStatus(( PMIDL_STUB_MESSAGE  )&_StubMsg,( unsigned long __RPC_FAR * )&_RetVal,( unsigned long __RPC_FAR * )&_RetVal,RpcExceptionCode()))
            {
            RpcRaiseException(_Status);
            }
        }
    RpcEndExcept
    return _RetVal;
}


/* [fault_status][comm_status] */ error_status_t R_LockServer( 
    /* [in] */ handle_t Binding)
{

    RPC_BINDING_HANDLE _Handle	=	0;
    
    error_status_t _RetVal;
    
    RPC_MESSAGE _RpcMessage;
    
    RPC_STATUS _Status;
    
    MIDL_STUB_MESSAGE _StubMsg;
    
    RpcTryExcept
        {
        RpcTryFinally
            {
            NdrClientInitializeNew(
                          ( PRPC_MESSAGE  )&_RpcMessage,
                          ( PMIDL_STUB_MESSAGE  )&_StubMsg,
                          ( PMIDL_STUB_DESC  )&cluma_StubDesc,
                          2);
            
            
            _Handle = Binding;
            
            
            _StubMsg.BufferLength = 0U;
            NdrGetBuffer( (PMIDL_STUB_MESSAGE) &_StubMsg, _StubMsg.BufferLength, _Handle );
            
            NdrSendReceive( (PMIDL_STUB_MESSAGE) &_StubMsg, (unsigned char __RPC_FAR *) _StubMsg.Buffer );
            
            if ( (_RpcMessage.DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[0] );
            
            _RetVal = *(( error_status_t __RPC_FAR * )_StubMsg.Buffer)++;
            
            }
        RpcFinally
            {
            NdrFreeBuffer( (PMIDL_STUB_MESSAGE) &_StubMsg );
            
            }
        RpcEndFinally
        
        }
    RpcExcept( 1 )
        {
        if(_Status = NdrMapCommAndFaultStatus(( PMIDL_STUB_MESSAGE  )&_StubMsg,( unsigned long __RPC_FAR * )&_RetVal,( unsigned long __RPC_FAR * )&_RetVal,RpcExceptionCode()))
            {
            RpcRaiseException(_Status);
            }
        }
    RpcEndExcept
    return _RetVal;
}


/* [fault_status][comm_status] */ error_status_t R_UnlockServer( 
    /* [in] */ handle_t Binding)
{

    RPC_BINDING_HANDLE _Handle	=	0;
    
    error_status_t _RetVal;
    
    RPC_MESSAGE _RpcMessage;
    
    RPC_STATUS _Status;
    
    MIDL_STUB_MESSAGE _StubMsg;
    
    RpcTryExcept
        {
        RpcTryFinally
            {
            NdrClientInitializeNew(
                          ( PRPC_MESSAGE  )&_RpcMessage,
                          ( PMIDL_STUB_MESSAGE  )&_StubMsg,
                          ( PMIDL_STUB_DESC  )&cluma_StubDesc,
                          3);
            
            
            _Handle = Binding;
            
            
            _StubMsg.BufferLength = 0U;
            NdrGetBuffer( (PMIDL_STUB_MESSAGE) &_StubMsg, _StubMsg.BufferLength, _Handle );
            
            NdrSendReceive( (PMIDL_STUB_MESSAGE) &_StubMsg, (unsigned char __RPC_FAR *) _StubMsg.Buffer );
            
            if ( (_RpcMessage.DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[0] );
            
            _RetVal = *(( error_status_t __RPC_FAR * )_StubMsg.Buffer)++;
            
            }
        RpcFinally
            {
            NdrFreeBuffer( (PMIDL_STUB_MESSAGE) &_StubMsg );
            
            }
        RpcEndFinally
        
        }
    RpcExcept( 1 )
        {
        if(_Status = NdrMapCommAndFaultStatus(( PMIDL_STUB_MESSAGE  )&_StubMsg,( unsigned long __RPC_FAR * )&_RetVal,( unsigned long __RPC_FAR * )&_RetVal,RpcExceptionCode()))
            {
            RpcRaiseException(_Status);
            }
        }
    RpcEndExcept
    return _RetVal;
}


/* [fault_status][comm_status] */ error_status_t R_GetConsoleUser( 
    /* [in] */ handle_t Binding,
    /* [string][out] */ unsigned char __RPC_FAR User[ 255 ])
{

    RPC_BINDING_HANDLE _Handle	=	0;
    
    error_status_t _RetVal;
    
    RPC_MESSAGE _RpcMessage;
    
    RPC_STATUS _Status;
    
    MIDL_STUB_MESSAGE _StubMsg;
    
    RpcTryExcept
        {
        RpcTryFinally
            {
            NdrClientInitializeNew(
                          ( PRPC_MESSAGE  )&_RpcMessage,
                          ( PMIDL_STUB_MESSAGE  )&_StubMsg,
                          ( PMIDL_STUB_DESC  )&cluma_StubDesc,
                          4);
            
            
            _Handle = Binding;
            
            
            _StubMsg.BufferLength = 0U;
            NdrGetBuffer( (PMIDL_STUB_MESSAGE) &_StubMsg, _StubMsg.BufferLength, _Handle );
            
            NdrSendReceive( (PMIDL_STUB_MESSAGE) &_StubMsg, (unsigned char __RPC_FAR *) _StubMsg.Buffer );
            
            if ( (_RpcMessage.DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[16] );
            
            NdrNonConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                              (unsigned char __RPC_FAR * __RPC_FAR *)&User,
                                              (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[6],
                                              (unsigned char)0 );
            
            _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            _RetVal = *(( error_status_t __RPC_FAR * )_StubMsg.Buffer)++;
            
            }
        RpcFinally
            {
            NdrFreeBuffer( (PMIDL_STUB_MESSAGE) &_StubMsg );
            
            }
        RpcEndFinally
        
        }
    RpcExcept( 1 )
        {
        if(_Status = NdrMapCommAndFaultStatus(( PMIDL_STUB_MESSAGE  )&_StubMsg,( unsigned long __RPC_FAR * )&_RetVal,( unsigned long __RPC_FAR * )&_RetVal,RpcExceptionCode()))
            {
            RpcRaiseException(_Status);
            }
        }
    RpcEndExcept
    return _RetVal;
}


/* [fault_status][comm_status] */ error_status_t R_GetUserData( 
    /* [in] */ handle_t Binding,
    /* [string][in] */ unsigned char __RPC_FAR *DllName,
    /* [size_is][unique][out][in] */ byte __RPC_FAR result[  ],
    /* [in] */ long BufferSize,
    /* [ref][out] */ long __RPC_FAR *ResultSize)
{

    RPC_BINDING_HANDLE _Handle	=	0;
    
    error_status_t _RetVal;
    
    RPC_MESSAGE _RpcMessage;
    
    RPC_STATUS _Status;
    
    MIDL_STUB_MESSAGE _StubMsg;
    
    RpcTryExcept
        {
        if(!DllName)
            {
            RpcRaiseException(RPC_X_NULL_REF_POINTER);
            }
        if(!ResultSize)
            {
            RpcRaiseException(RPC_X_NULL_REF_POINTER);
            }
        RpcTryFinally
            {
            NdrClientInitializeNew(
                          ( PRPC_MESSAGE  )&_RpcMessage,
                          ( PMIDL_STUB_MESSAGE  )&_StubMsg,
                          ( PMIDL_STUB_DESC  )&cluma_StubDesc,
                          5);
            
            
            _Handle = Binding;
            
            
            _StubMsg.BufferLength = 0U + 12U + 11U + 11U;
            NdrConformantStringBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR *)DllName,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[12] );
            
            _StubMsg.MaxCount = BufferSize;
            
            NdrPointerBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR *)result,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[14] );
            
            NdrGetBuffer( (PMIDL_STUB_MESSAGE) &_StubMsg, _StubMsg.BufferLength, _Handle );
            
            NdrConformantStringMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                                         (unsigned char __RPC_FAR *)DllName,
                                         (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[12] );
            
            _StubMsg.MaxCount = BufferSize;
            
            NdrPointerMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                                (unsigned char __RPC_FAR *)result,
                                (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[14] );
            
            _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            *(( long __RPC_FAR * )_StubMsg.Buffer)++ = BufferSize;
            
            NdrSendReceive( (PMIDL_STUB_MESSAGE) &_StubMsg, (unsigned char __RPC_FAR *) _StubMsg.Buffer );
            
            if ( (_RpcMessage.DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[24] );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR * __RPC_FAR *)&result,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[14],
                                  (unsigned char)0 );
            
            _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            *ResultSize = *(( long __RPC_FAR * )_StubMsg.Buffer)++;
            
            _RetVal = *(( error_status_t __RPC_FAR * )_StubMsg.Buffer)++;
            
            }
        RpcFinally
            {
            NdrFreeBuffer( (PMIDL_STUB_MESSAGE) &_StubMsg );
            
            }
        RpcEndFinally
        
        }
    RpcExcept( 1 )
        {
        if(_Status = NdrMapCommAndFaultStatus(( PMIDL_STUB_MESSAGE  )&_StubMsg,( unsigned long __RPC_FAR * )&_RetVal,( unsigned long __RPC_FAR * )&_RetVal,RpcExceptionCode()))
            {
            RpcRaiseException(_Status);
            }
        }
    RpcEndExcept
    return _RetVal;
}

void __RPC_STUB
cluma_R_GetClientName(
    PRPC_MESSAGE _pRpcMessage )
{
    unsigned char ( __RPC_FAR *ClientName )[  ];
    handle_t IDL_handle;
    error_status_t _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    unsigned long __RPC_FAR *size;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &cluma_StubDesc);
    
    IDL_handle = _pRpcMessage->Handle;
    ClientName = 0;
    ( unsigned long __RPC_FAR * )size = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[42] );
            
            size = ( unsigned long __RPC_FAR * )_StubMsg.Buffer;
            _StubMsg.Buffer += sizeof( unsigned long  );
            
            if(_StubMsg.Buffer > _StubMsg.BufferEnd)
                {
                RpcRaiseException(RPC_X_BAD_STUB_DATA);
                }
            }
        RpcExcept( RPC_BAD_STUB_DATA_EXCEPTION_FILTER )
            {
            RpcRaiseException(RPC_X_BAD_STUB_DATA);
            }
        RpcEndExcept
        ClientName = NdrAllocate(&_StubMsg,*size * 1);
        
        _RetVal = R_GetClientName(
                          IDL_handle,
                          *ClientName,
                          size);
        
        _StubMsg.BufferLength = 4U + 11U + 7U;
        _StubMsg.MaxCount = size ? *size : 0;
        
        NdrConformantArrayBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                      (unsigned char __RPC_FAR *)*ClientName,
                                      (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[28] );
        
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        _StubMsg.MaxCount = size ? *size : 0;
        
        NdrConformantArrayMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                                    (unsigned char __RPC_FAR *)*ClientName,
                                    (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[28] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( unsigned long __RPC_FAR * )_StubMsg.Buffer)++ = *size;
        
        *(( error_status_t __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        if ( ClientName )
            _StubMsg.pfnFree( ClientName );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}


/* [fault_status][comm_status] */ error_status_t R_Transfer( 
    /* [in] */ handle_t binding,
    /* [ref][in] */ byte_pipe __RPC_FAR *data)
{

    CLIENT_CALL_RETURN _RetVal;

    
#if defined( _ALPHA_ )
    va_list vlist;
#endif
    
#if defined( _ALPHA_ )
    va_start(vlist,data);
    _RetVal = NdrClientCall2(
                  ( PMIDL_STUB_DESC  )&cluma_StubDesc,
                  (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[54],
                  vlist.a0);
#elif defined( _PPC_ ) || defined( _MIPS_ )

    _RetVal = NdrClientCall2(
                  ( PMIDL_STUB_DESC  )&cluma_StubDesc,
                  (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[54],
                  ( unsigned char __RPC_FAR * )&binding,
                  ( unsigned char __RPC_FAR * )&data);
#else
    _RetVal = NdrClientCall2(
                  ( PMIDL_STUB_DESC  )&cluma_StubDesc,
                  (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[54],
                  ( unsigned char __RPC_FAR * )&binding);
#endif
    return ( error_status_t  )_RetVal.Simple;
    
}


/* [fault_status][comm_status] */ error_status_t R_CreateProcess( 
    /* [in] */ handle_t Binding,
    /* [string][unique][in] */ unsigned char __RPC_FAR *lpApplicationName,
    /* [string][unique][in] */ unsigned char __RPC_FAR *lpCommandLine,
    /* [in] */ unsigned long dwCreationFlags,
    /* [size_is][unique][in] */ byte __RPC_FAR *lpEnvironment,
    /* [in] */ unsigned long EnvSize,
    /* [string][unique][in] */ unsigned char __RPC_FAR *lpCurrentDirectory,
    /* [ref][in] */ R_STARTUPINFO __RPC_FAR *lpStartupInfo,
    /* [ref][out] */ R_PROCESS_INFORMATION __RPC_FAR *lpProcessInformation)
{

    RPC_BINDING_HANDLE _Handle	=	0;
    
    error_status_t _RetVal;
    
    RPC_MESSAGE _RpcMessage;
    
    RPC_STATUS _Status;
    
    MIDL_STUB_MESSAGE _StubMsg;
    
    RpcTryExcept
        {
        if(!lpStartupInfo)
            {
            RpcRaiseException(RPC_X_NULL_REF_POINTER);
            }
        if(!lpProcessInformation)
            {
            RpcRaiseException(RPC_X_NULL_REF_POINTER);
            }
        RpcTryFinally
            {
            NdrClientInitializeNew(
                          ( PRPC_MESSAGE  )&_RpcMessage,
                          ( PMIDL_STUB_MESSAGE  )&_StubMsg,
                          ( PMIDL_STUB_DESC  )&cluma_StubDesc,
                          7);
            
            
            _Handle = Binding;
            
            
            _StubMsg.BufferLength = 0U + 16U + 24U + 11U + 12U + 11U + 20U + 7U;
            NdrPointerBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR *)lpApplicationName,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[56] );
            
            NdrPointerBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR *)lpCommandLine,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[56] );
            
            _StubMsg.MaxCount = EnvSize;
            
            NdrPointerBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR *)lpEnvironment,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[60] );
            
            NdrPointerBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char __RPC_FAR *)lpCurrentDirectory,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[56] );
            
            NdrSimpleStructBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                       (unsigned char __RPC_FAR *)lpStartupInfo,
                                       (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[78] );
            
            NdrGetBuffer( (PMIDL_STUB_MESSAGE) &_StubMsg, _StubMsg.BufferLength, _Handle );
            
            NdrPointerMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                                (unsigned char __RPC_FAR *)lpApplicationName,
                                (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[56] );
            
            NdrPointerMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                                (unsigned char __RPC_FAR *)lpCommandLine,
                                (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[56] );
            
            _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            *(( unsigned long __RPC_FAR * )_StubMsg.Buffer)++ = dwCreationFlags;
            
            _StubMsg.MaxCount = EnvSize;
            
            NdrPointerMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                                (unsigned char __RPC_FAR *)lpEnvironment,
                                (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[60] );
            
            _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            *(( unsigned long __RPC_FAR * )_StubMsg.Buffer)++ = EnvSize;
            
            NdrPointerMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                                (unsigned char __RPC_FAR *)lpCurrentDirectory,
                                (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[56] );
            
            NdrSimpleStructMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                                     (unsigned char __RPC_FAR *)lpStartupInfo,
                                     (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[78] );
            
            NdrSendReceive( (PMIDL_STUB_MESSAGE) &_StubMsg, (unsigned char __RPC_FAR *) _StubMsg.Buffer );
            
            if ( (_RpcMessage.DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[86] );
            
            NdrSimpleStructUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                       (unsigned char __RPC_FAR * __RPC_FAR *)&lpProcessInformation,
                                       (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[150],
                                       (unsigned char)0 );
            
            _RetVal = *(( error_status_t __RPC_FAR * )_StubMsg.Buffer)++;
            
            }
        RpcFinally
            {
            NdrFreeBuffer( (PMIDL_STUB_MESSAGE) &_StubMsg );
            
            }
        RpcEndFinally
        
        }
    RpcExcept( 1 )
        {
        if(_Status = NdrMapCommAndFaultStatus(( PMIDL_STUB_MESSAGE  )&_StubMsg,( unsigned long __RPC_FAR * )&_RetVal,( unsigned long __RPC_FAR * )&_RetVal,RpcExceptionCode()))
            {
            RpcRaiseException(_Status);
            }
        }
    RpcEndExcept
    return _RetVal;
}


/* [fault_status][comm_status] */ error_status_t R_TerminateProcess( 
    /* [in] */ handle_t Binding,
    /* [ref][in] */ R_PROCESS_INFORMATION __RPC_FAR *lpProcessInformation,
    /* [in] */ unsigned long RetValue)
{

    RPC_BINDING_HANDLE _Handle	=	0;
    
    error_status_t _RetVal;
    
    RPC_MESSAGE _RpcMessage;
    
    RPC_STATUS _Status;
    
    MIDL_STUB_MESSAGE _StubMsg;
    
    RpcTryExcept
        {
        if(!lpProcessInformation)
            {
            RpcRaiseException(RPC_X_NULL_REF_POINTER);
            }
        RpcTryFinally
            {
            NdrClientInitializeNew(
                          ( PRPC_MESSAGE  )&_RpcMessage,
                          ( PMIDL_STUB_MESSAGE  )&_StubMsg,
                          ( PMIDL_STUB_DESC  )&cluma_StubDesc,
                          8);
            
            
            _Handle = Binding;
            
            
            _StubMsg.BufferLength = 0U + 0U + 11U;
            NdrSimpleStructBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                       (unsigned char __RPC_FAR *)lpProcessInformation,
                                       (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[150] );
            
            NdrGetBuffer( (PMIDL_STUB_MESSAGE) &_StubMsg, _StubMsg.BufferLength, _Handle );
            
            NdrSimpleStructMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                                     (unsigned char __RPC_FAR *)lpProcessInformation,
                                     (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[150] );
            
            *(( unsigned long __RPC_FAR * )_StubMsg.Buffer)++ = RetValue;
            
            NdrSendReceive( (PMIDL_STUB_MESSAGE) &_StubMsg, (unsigned char __RPC_FAR *) _StubMsg.Buffer );
            
            if ( (_RpcMessage.DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[118] );
            
            _RetVal = *(( error_status_t __RPC_FAR * )_StubMsg.Buffer)++;
            
            }
        RpcFinally
            {
            NdrFreeBuffer( (PMIDL_STUB_MESSAGE) &_StubMsg );
            
            }
        RpcEndFinally
        
        }
    RpcExcept( 1 )
        {
        if(_Status = NdrMapCommAndFaultStatus(( PMIDL_STUB_MESSAGE  )&_StubMsg,( unsigned long __RPC_FAR * )&_RetVal,( unsigned long __RPC_FAR * )&_RetVal,RpcExceptionCode()))
            {
            RpcRaiseException(_Status);
            }
        }
    RpcEndExcept
    return _RetVal;
}


/* [fault_status][comm_status] */ error_status_t R_GetSystemInfo( 
    /* [in] */ handle_t IDL_handle,
    /* [ref][out][in] */ R_MACHINE_INFO __RPC_FAR *lpMachineInfo)
{

    RPC_BINDING_HANDLE _Handle	=	0;
    
    error_status_t _RetVal;
    
    RPC_MESSAGE _RpcMessage;
    
    RPC_STATUS _Status;
    
    MIDL_STUB_MESSAGE _StubMsg;
    
    RpcTryExcept
        {
        if(!lpMachineInfo)
            {
            RpcRaiseException(RPC_X_NULL_REF_POINTER);
            }
        RpcTryFinally
            {
            NdrClientInitializeNew(
                          ( PRPC_MESSAGE  )&_RpcMessage,
                          ( PMIDL_STUB_MESSAGE  )&_StubMsg,
                          ( PMIDL_STUB_DESC  )&cluma_StubDesc,
                          9);
            
            
            _Handle = IDL_handle;
            
            
            _StubMsg.BufferLength = 0U + 0U;
            NdrComplexStructBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                        (unsigned char __RPC_FAR *)lpMachineInfo,
                                        (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[258] );
            
            NdrGetBuffer( (PMIDL_STUB_MESSAGE) &_StubMsg, _StubMsg.BufferLength, _Handle );
            
            NdrComplexStructMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                                      (unsigned char __RPC_FAR *)lpMachineInfo,
                                      (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[258] );
            
            NdrSendReceive( (PMIDL_STUB_MESSAGE) &_StubMsg, (unsigned char __RPC_FAR *) _StubMsg.Buffer );
            
            if ( (_RpcMessage.DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[128] );
            
            NdrComplexStructUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                        (unsigned char __RPC_FAR * __RPC_FAR *)&lpMachineInfo,
                                        (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[258],
                                        (unsigned char)0 );
            
            _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            _RetVal = *(( error_status_t __RPC_FAR * )_StubMsg.Buffer)++;
            
            }
        RpcFinally
            {
            NdrFreeBuffer( (PMIDL_STUB_MESSAGE) &_StubMsg );
            
            }
        RpcEndFinally
        
        }
    RpcExcept( 1 )
        {
        if(_Status = NdrMapCommAndFaultStatus(( PMIDL_STUB_MESSAGE  )&_StubMsg,( unsigned long __RPC_FAR * )&_RetVal,( unsigned long __RPC_FAR * )&_RetVal,RpcExceptionCode()))
            {
            RpcRaiseException(_Status);
            }
        }
    RpcEndExcept
    return _RetVal;
}


/* [fault_status][comm_status] */ error_status_t R_GetProcs( 
    /* [in] */ handle_t Binding)
{

    RPC_BINDING_HANDLE _Handle	=	0;
    
    error_status_t _RetVal;
    
    RPC_MESSAGE _RpcMessage;
    
    RPC_STATUS _Status;
    
    MIDL_STUB_MESSAGE _StubMsg;
    
    RpcTryExcept
        {
        RpcTryFinally
            {
            NdrClientInitializeNew(
                          ( PRPC_MESSAGE  )&_RpcMessage,
                          ( PMIDL_STUB_MESSAGE  )&_StubMsg,
                          ( PMIDL_STUB_DESC  )&cluma_StubDesc,
                          10);
            
            
            _Handle = Binding;
            
            
            _StubMsg.BufferLength = 0U;
            NdrGetBuffer( (PMIDL_STUB_MESSAGE) &_StubMsg, _StubMsg.BufferLength, _Handle );
            
            NdrSendReceive( (PMIDL_STUB_MESSAGE) &_StubMsg, (unsigned char __RPC_FAR *) _StubMsg.Buffer );
            
            if ( (_RpcMessage.DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[0] );
            
            _RetVal = *(( error_status_t __RPC_FAR * )_StubMsg.Buffer)++;
            
            }
        RpcFinally
            {
            NdrFreeBuffer( (PMIDL_STUB_MESSAGE) &_StubMsg );
            
            }
        RpcEndFinally
        
        }
    RpcExcept( 1 )
        {
        if(_Status = NdrMapCommAndFaultStatus(( PMIDL_STUB_MESSAGE  )&_StubMsg,( unsigned long __RPC_FAR * )&_RetVal,( unsigned long __RPC_FAR * )&_RetVal,RpcExceptionCode()))
            {
            RpcRaiseException(_Status);
            }
        }
    RpcEndExcept
    return _RetVal;
}

void __RPC_STUB
cluma_R_ProcEnumCallback(
    PRPC_MESSAGE _pRpcMessage )
{
    handle_t IDL_handle;
    unsigned char __RPC_FAR *Owner;
    error_status_t _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    unsigned long id;
    unsigned char __RPC_FAR *name;
    RPC_STATUS _Status;
    
    ((void)(_Status));
    NdrServerInitializeNew(
                          _pRpcMessage,
                          &_StubMsg,
                          &cluma_StubDesc);
    
    IDL_handle = _pRpcMessage->Handle;
    ( unsigned char __RPC_FAR * )name = 0;
    ( unsigned char __RPC_FAR * )Owner = 0;
    RpcTryFinally
        {
        RpcTryExcept
            {
            if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[136] );
            
            id = *(( unsigned long __RPC_FAR * )_StubMsg.Buffer)++;
            
            NdrConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&name,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[12],
                                           (unsigned char)0 );
            
            NdrConformantStringUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                           (unsigned char __RPC_FAR * __RPC_FAR *)&Owner,
                                           (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[12],
                                           (unsigned char)0 );
            
            if(_StubMsg.Buffer > _StubMsg.BufferEnd)
                {
                RpcRaiseException(RPC_X_BAD_STUB_DATA);
                }
            }
        RpcExcept( RPC_BAD_STUB_DATA_EXCEPTION_FILTER )
            {
            RpcRaiseException(RPC_X_BAD_STUB_DATA);
            }
        RpcEndExcept
        
        _RetVal = R_ProcEnumCallback(
                             IDL_handle,
                             id,
                             name,
                             Owner);
        
        _StubMsg.BufferLength = 4U;
        _StubMsg.BufferLength += 16;
        
        _pRpcMessage->BufferLength = _StubMsg.BufferLength;
        
        _Status = I_RpcGetBuffer( _pRpcMessage ); 
        if ( _Status )
            RpcRaiseException( _Status );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *) _pRpcMessage->Buffer;
        
        *(( error_status_t __RPC_FAR * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)((long)_StubMsg.Buffer - (long)_pRpcMessage->Buffer);
    
}


/* [fault_status][comm_status] */ error_status_t R_KillProcess( 
    /* [in] */ handle_t Binding,
    /* [in] */ unsigned long id)
{

    RPC_BINDING_HANDLE _Handle	=	0;
    
    error_status_t _RetVal;
    
    RPC_MESSAGE _RpcMessage;
    
    RPC_STATUS _Status;
    
    MIDL_STUB_MESSAGE _StubMsg;
    
    RpcTryExcept
        {
        RpcTryFinally
            {
            NdrClientInitializeNew(
                          ( PRPC_MESSAGE  )&_RpcMessage,
                          ( PMIDL_STUB_MESSAGE  )&_StubMsg,
                          ( PMIDL_STUB_DESC  )&cluma_StubDesc,
                          11);
            
            
            _Handle = Binding;
            
            
            _StubMsg.BufferLength = 0U + 4U;
            NdrGetBuffer( (PMIDL_STUB_MESSAGE) &_StubMsg, _StubMsg.BufferLength, _Handle );
            
            *(( unsigned long __RPC_FAR * )_StubMsg.Buffer)++ = id;
            
            NdrSendReceive( (PMIDL_STUB_MESSAGE) &_StubMsg, (unsigned char __RPC_FAR *) _StubMsg.Buffer );
            
            if ( (_RpcMessage.DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[150] );
            
            _RetVal = *(( error_status_t __RPC_FAR * )_StubMsg.Buffer)++;
            
            }
        RpcFinally
            {
            NdrFreeBuffer( (PMIDL_STUB_MESSAGE) &_StubMsg );
            
            }
        RpcEndFinally
        
        }
    RpcExcept( 1 )
        {
        if(_Status = NdrMapCommAndFaultStatus(( PMIDL_STUB_MESSAGE  )&_StubMsg,( unsigned long __RPC_FAR * )&_RetVal,( unsigned long __RPC_FAR * )&_RetVal,RpcExceptionCode()))
            {
            RpcRaiseException(_Status);
            }
        }
    RpcEndExcept
    return _RetVal;
}


/* [fault_status][comm_status] */ error_status_t R_ShutDown( 
    /* [in] */ handle_t IDL_handle,
    /* [in] */ long restart)
{

    RPC_BINDING_HANDLE _Handle	=	0;
    
    error_status_t _RetVal;
    
    RPC_MESSAGE _RpcMessage;
    
    RPC_STATUS _Status;
    
    MIDL_STUB_MESSAGE _StubMsg;
    
    RpcTryExcept
        {
        RpcTryFinally
            {
            NdrClientInitializeNew(
                          ( PRPC_MESSAGE  )&_RpcMessage,
                          ( PMIDL_STUB_MESSAGE  )&_StubMsg,
                          ( PMIDL_STUB_DESC  )&cluma_StubDesc,
                          12);
            
            
            _Handle = IDL_handle;
            
            
            _StubMsg.BufferLength = 0U + 4U;
            NdrGetBuffer( (PMIDL_STUB_MESSAGE) &_StubMsg, _StubMsg.BufferLength, _Handle );
            
            *(( long __RPC_FAR * )_StubMsg.Buffer)++ = restart;
            
            NdrSendReceive( (PMIDL_STUB_MESSAGE) &_StubMsg, (unsigned char __RPC_FAR *) _StubMsg.Buffer );
            
            if ( (_RpcMessage.DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[150] );
            
            _RetVal = *(( error_status_t __RPC_FAR * )_StubMsg.Buffer)++;
            
            }
        RpcFinally
            {
            NdrFreeBuffer( (PMIDL_STUB_MESSAGE) &_StubMsg );
            
            }
        RpcEndFinally
        
        }
    RpcExcept( 1 )
        {
        if(_Status = NdrMapCommAndFaultStatus(( PMIDL_STUB_MESSAGE  )&_StubMsg,( unsigned long __RPC_FAR * )&_RetVal,( unsigned long __RPC_FAR * )&_RetVal,RpcExceptionCode()))
            {
            RpcRaiseException(_Status);
            }
        }
    RpcEndExcept
    return _RetVal;
}


error_status_t R_CreateProcessNoUser( 
    /* [in] */ handle_t IDL_handle,
    /* [string][unique][in] */ unsigned char __RPC_FAR *lpApplicationName,
    /* [string][unique][in] */ unsigned char __RPC_FAR *lpCommandLine,
    /* [in] */ unsigned long dwCreationFlags,
    /* [size_is][unique][in] */ byte __RPC_FAR *lpEnvironment,
    /* [in] */ unsigned long EnvSize,
    /* [string][unique][in] */ unsigned char __RPC_FAR *lpCurrentDirectory,
    /* [ref][in] */ R_STARTUPINFO __RPC_FAR *lpStartupInfo,
    /* [ref][out] */ R_PROCESS_INFORMATION __RPC_FAR *lpProcessInformation)
{

    RPC_BINDING_HANDLE _Handle	=	0;
    
    error_status_t _RetVal;
    
    RPC_MESSAGE _RpcMessage;
    
    MIDL_STUB_MESSAGE _StubMsg;
    
    if(!lpStartupInfo)
        {
        RpcRaiseException(RPC_X_NULL_REF_POINTER);
        }
    if(!lpProcessInformation)
        {
        RpcRaiseException(RPC_X_NULL_REF_POINTER);
        }
    RpcTryFinally
        {
        NdrClientInitializeNew(
                          ( PRPC_MESSAGE  )&_RpcMessage,
                          ( PMIDL_STUB_MESSAGE  )&_StubMsg,
                          ( PMIDL_STUB_DESC  )&cluma_StubDesc,
                          13);
        
        
        _Handle = IDL_handle;
        
        
        _StubMsg.BufferLength = 0U + 16U + 24U + 11U + 12U + 11U + 20U + 7U;
        NdrPointerBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR *)lpApplicationName,
                              (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[56] );
        
        NdrPointerBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR *)lpCommandLine,
                              (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[56] );
        
        _StubMsg.MaxCount = EnvSize;
        
        NdrPointerBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR *)lpEnvironment,
                              (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[60] );
        
        NdrPointerBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char __RPC_FAR *)lpCurrentDirectory,
                              (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[56] );
        
        NdrSimpleStructBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                   (unsigned char __RPC_FAR *)lpStartupInfo,
                                   (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[78] );
        
        NdrGetBuffer( (PMIDL_STUB_MESSAGE) &_StubMsg, _StubMsg.BufferLength, _Handle );
        
        NdrPointerMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                            (unsigned char __RPC_FAR *)lpApplicationName,
                            (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[56] );
        
        NdrPointerMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                            (unsigned char __RPC_FAR *)lpCommandLine,
                            (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[56] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( unsigned long __RPC_FAR * )_StubMsg.Buffer)++ = dwCreationFlags;
        
        _StubMsg.MaxCount = EnvSize;
        
        NdrPointerMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                            (unsigned char __RPC_FAR *)lpEnvironment,
                            (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[60] );
        
        _StubMsg.Buffer = (unsigned char __RPC_FAR *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( unsigned long __RPC_FAR * )_StubMsg.Buffer)++ = EnvSize;
        
        NdrPointerMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                            (unsigned char __RPC_FAR *)lpCurrentDirectory,
                            (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[56] );
        
        NdrSimpleStructMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                                 (unsigned char __RPC_FAR *)lpStartupInfo,
                                 (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[78] );
        
        NdrSendReceive( (PMIDL_STUB_MESSAGE) &_StubMsg, (unsigned char __RPC_FAR *) _StubMsg.Buffer );
        
        if ( (_RpcMessage.DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[86] );
        
        NdrSimpleStructUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                   (unsigned char __RPC_FAR * __RPC_FAR *)&lpProcessInformation,
                                   (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[150],
                                   (unsigned char)0 );
        
        _RetVal = *(( error_status_t __RPC_FAR * )_StubMsg.Buffer)++;
        
        }
    RpcFinally
        {
        NdrFreeBuffer( (PMIDL_STUB_MESSAGE) &_StubMsg );
        
        }
    RpcEndFinally
    
    return _RetVal;
}


static const COMM_FAULT_OFFSETS cluma_CommFaultOffsets[] = 
{
#ifdef _X86_
	{ -1, -1 },	/* x86 Offsets for Ping */
#endif
#if defined(_MIPS_) || defined(_PPC_)
	{ -1, -1 },	/* MIPS, PPC Offsets for Ping */
#endif
#ifdef _ALPHA_
	{ -1, -1 },	/* Alpha Offsets for Ping */
#endif
#ifdef _X86_
	{ -1, -1 },	/* x86 Offsets for R_GetLockStatus */
#endif
#if defined(_MIPS_) || defined(_PPC_)
	{ -1, -1 },	/* MIPS, PPC Offsets for R_GetLockStatus */
#endif
#ifdef _ALPHA_
	{ -1, -1 },	/* Alpha Offsets for R_GetLockStatus */
#endif
#ifdef _X86_
	{ -1, -1 },	/* x86 Offsets for R_LockServer */
#endif
#if defined(_MIPS_) || defined(_PPC_)
	{ -1, -1 },	/* MIPS, PPC Offsets for R_LockServer */
#endif
#ifdef _ALPHA_
	{ -1, -1 },	/* Alpha Offsets for R_LockServer */
#endif
#ifdef _X86_
	{ -1, -1 },	/* x86 Offsets for R_UnlockServer */
#endif
#if defined(_MIPS_) || defined(_PPC_)
	{ -1, -1 },	/* MIPS, PPC Offsets for R_UnlockServer */
#endif
#ifdef _ALPHA_
	{ -1, -1 },	/* Alpha Offsets for R_UnlockServer */
#endif
#ifdef _X86_
	{ -1, -1 },	/* x86 Offsets for R_GetConsoleUser */
#endif
#if defined(_MIPS_) || defined(_PPC_)
	{ -1, -1 },	/* MIPS, PPC Offsets for R_GetConsoleUser */
#endif
#ifdef _ALPHA_
	{ -1, -1 },	/* Alpha Offsets for R_GetConsoleUser */
#endif
#ifdef _X86_
	{ -1, -1 },	/* x86 Offsets for R_GetUserData */
#endif
#if defined(_MIPS_) || defined(_PPC_)
	{ -1, -1 },	/* MIPS, PPC Offsets for R_GetUserData */
#endif
#ifdef _ALPHA_
	{ -1, -1 },	/* Alpha Offsets for R_GetUserData */
#endif
#ifdef _X86_
	{ -1, -1 },	/* x86 Offsets for R_Transfer */
#endif
#if defined(_MIPS_) || defined(_PPC_)
	{ -1, -1 },	/* MIPS, PPC Offsets for R_Transfer */
#endif
#ifdef _ALPHA_
	{ -1, -1 },	/* Alpha Offsets for R_Transfer */
#endif
#ifdef _X86_
	{ -1, -1 },	/* x86 Offsets for R_CreateProcess */
#endif
#if defined(_MIPS_) || defined(_PPC_)
	{ -1, -1 },	/* MIPS, PPC Offsets for R_CreateProcess */
#endif
#ifdef _ALPHA_
	{ -1, -1 },	/* Alpha Offsets for R_CreateProcess */
#endif
#ifdef _X86_
	{ -1, -1 },	/* x86 Offsets for R_TerminateProcess */
#endif
#if defined(_MIPS_) || defined(_PPC_)
	{ -1, -1 },	/* MIPS, PPC Offsets for R_TerminateProcess */
#endif
#ifdef _ALPHA_
	{ -1, -1 },	/* Alpha Offsets for R_TerminateProcess */
#endif
#ifdef _X86_
	{ -1, -1 },	/* x86 Offsets for R_GetSystemInfo */
#endif
#if defined(_MIPS_) || defined(_PPC_)
	{ -1, -1 },	/* MIPS, PPC Offsets for R_GetSystemInfo */
#endif
#ifdef _ALPHA_
	{ -1, -1 },	/* Alpha Offsets for R_GetSystemInfo */
#endif
#ifdef _X86_
	{ -1, -1 },	/* x86 Offsets for R_GetProcs */
#endif
#if defined(_MIPS_) || defined(_PPC_)
	{ -1, -1 },	/* MIPS, PPC Offsets for R_GetProcs */
#endif
#ifdef _ALPHA_
	{ -1, -1 },	/* Alpha Offsets for R_GetProcs */
#endif
#ifdef _X86_
	{ -1, -1 },	/* x86 Offsets for R_KillProcess */
#endif
#if defined(_MIPS_) || defined(_PPC_)
	{ -1, -1 },	/* MIPS, PPC Offsets for R_KillProcess */
#endif
#ifdef _ALPHA_
	{ -1, -1 },	/* Alpha Offsets for R_KillProcess */
#endif
#ifdef _X86_
	{ -1, -1 },	/* x86 Offsets for R_ShutDown */
#endif
#if defined(_MIPS_) || defined(_PPC_)
	{ -1, -1 },	/* MIPS, PPC Offsets for R_ShutDown */
#endif
#ifdef _ALPHA_
	{ -1, -1 },	/* Alpha Offsets for R_ShutDown */
#endif
	{ -2, -2 }
};


static const MIDL_STUB_DESC cluma_StubDesc = 
    {
    (void __RPC_FAR *)& cluma___RpcClientInterface,
    MIDL_user_allocate,
    MIDL_user_free,
    &cluma__MIDL_AutoBindHandle,
    0,
    0,
    0,
    0,
    __MIDL_TypeFormatString.Format,
    1, /* -error bounds_check flag */
    0x20000, /* Ndr library version */
    0,
    0x50100a4, /* MIDL Version 5.1.164 */
    cluma_CommFaultOffsets,
    0,
    0,  /* notify & notify_flag routine table */
    1,  /* Flags */
    0,  /* Reserved3 */
    0,  /* Reserved4 */
    0   /* Reserved5 */
    };

static RPC_DISPATCH_FUNCTION cluma_table[] =
    {
    cluma_R_GetClientName,
    cluma_R_ProcEnumCallback,
    0
    };
RPC_DISPATCH_TABLE cluma_v1_0_DispatchTable = 
    {
    2,
    cluma_table
    };

#if !defined(__RPC_WIN32__)
#error  Invalid build platform for this stub.
#endif



static const MIDL_PROC_FORMAT_STRING __MIDL_ProcFormatString =
    {
        0,
        {
			0x4e,		/* FC_IN_PARAM_BASETYPE */
			0xf,		/* FC_IGNORE */
/*  2 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x10,		/* FC_ERROR_STATUS_T */
/*  4 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0xf,		/* FC_IGNORE */
/*  6 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/*  8 */	NdrFcShort( 0x2 ),	/* Type Offset=2 */
/* 10 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 12 */	NdrFcShort( 0x6 ),	/* Type Offset=6 */
/* 14 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x10,		/* FC_ERROR_STATUS_T */
/* 16 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0xf,		/* FC_IGNORE */
/* 18 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 20 */	NdrFcShort( 0x6 ),	/* Type Offset=6 */
/* 22 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x10,		/* FC_ERROR_STATUS_T */
/* 24 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0xf,		/* FC_IGNORE */
/* 26 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 28 */	NdrFcShort( 0xa ),	/* Type Offset=10 */
/* 30 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 32 */	NdrFcShort( 0xe ),	/* Type Offset=14 */
/* 34 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 36 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 38 */	NdrFcShort( 0x2 ),	/* Type Offset=2 */
/* 40 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x10,		/* FC_ERROR_STATUS_T */
/* 42 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0xf,		/* FC_IGNORE */
/* 44 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 46 */	NdrFcShort( 0x1c ),	/* Type Offset=28 */
/* 48 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 50 */	NdrFcShort( 0x26 ),	/* Type Offset=38 */
/* 52 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x10,		/* FC_ERROR_STATUS_T */

	/* Procedure R_Transfer */

/* 54 */	0x0,		/* 0 */
			0x68,		/* Old Flags:  comm or fault/decode */
/* 56 */	NdrFcLong( 0x0 ),	/* 0 */
/* 60 */	NdrFcShort( 0x6 ),	/* 6 */
#ifndef _ALPHA_
/* 62 */	NdrFcShort( 0xc ),	/* x86, MIPS, PPC Stack size/offset = 12 */
#else
			NdrFcShort( 0x18 ),	/* Alpha Stack size/offset = 24 */
#endif
/* 64 */	0x32,		/* FC_BIND_PRIMITIVE */
			0x0,		/* 0 */
#ifndef _ALPHA_
/* 66 */	NdrFcShort( 0x0 ),	/* x86, MIPS, PPC Stack size/offset = 0 */
#else
			NdrFcShort( 0x0 ),	/* Alpha Stack size/offset = 0 */
#endif
/* 68 */	NdrFcShort( 0x0 ),	/* 0 */
/* 70 */	NdrFcShort( 0x8 ),	/* 8 */
/* 72 */	0xc,		/* Oi2 Flags:  has return, has pipes, */
			0x2,		/* 2 */

	/* Parameter binding */

/* 74 */	NdrFcShort( 0x10c ),	/* Flags:  pipe, in, simple ref, */
#ifndef _ALPHA_
/* 76 */	NdrFcShort( 0x4 ),	/* x86, MIPS, PPC Stack size/offset = 4 */
#else
			NdrFcShort( 0x8 ),	/* Alpha Stack size/offset = 8 */
#endif
/* 78 */	NdrFcShort( 0x30 ),	/* Type Offset=48 */

	/* Parameter data */

/* 80 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
#ifndef _ALPHA_
/* 82 */	NdrFcShort( 0x8 ),	/* x86, MIPS, PPC Stack size/offset = 8 */
#else
			NdrFcShort( 0x10 ),	/* Alpha Stack size/offset = 16 */
#endif
/* 84 */	0x10,		/* FC_ERROR_STATUS_T */
			0x0,		/* 0 */

	/* Return value */

/* 86 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0xf,		/* FC_IGNORE */
/* 88 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 90 */	NdrFcShort( 0x38 ),	/* Type Offset=56 */
/* 92 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 94 */	NdrFcShort( 0x38 ),	/* Type Offset=56 */
/* 96 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 98 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 100 */	NdrFcShort( 0x3c ),	/* Type Offset=60 */
/* 102 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 104 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 106 */	NdrFcShort( 0x38 ),	/* Type Offset=56 */
/* 108 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 110 */	NdrFcShort( 0x4a ),	/* Type Offset=74 */
/* 112 */	
			0x51,		/* FC_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 114 */	NdrFcShort( 0x92 ),	/* Type Offset=146 */
/* 116 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x10,		/* FC_ERROR_STATUS_T */
/* 118 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0xf,		/* FC_IGNORE */
/* 120 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 122 */	NdrFcShort( 0xa0 ),	/* Type Offset=160 */
/* 124 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 126 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x10,		/* FC_ERROR_STATUS_T */
/* 128 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0xf,		/* FC_IGNORE */
/* 130 */	
			0x50,		/* FC_IN_OUT_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 132 */	NdrFcShort( 0xa4 ),	/* Type Offset=164 */
/* 134 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x10,		/* FC_ERROR_STATUS_T */
/* 136 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0xf,		/* FC_IGNORE */
/* 138 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 140 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 142 */	NdrFcShort( 0xa ),	/* Type Offset=10 */
/* 144 */	
			0x4d,		/* FC_IN_PARAM */
#ifndef _ALPHA_
			0x1,		/* x86, MIPS & PPC Stack size = 1 */
#else
			0x2,		/* Alpha Stack size = 2 */
#endif
/* 146 */	NdrFcShort( 0xa ),	/* Type Offset=10 */
/* 148 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x10,		/* FC_ERROR_STATUS_T */
/* 150 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0xf,		/* FC_IGNORE */
/* 152 */	0x4e,		/* FC_IN_PARAM_BASETYPE */
			0x8,		/* FC_LONG */
/* 154 */	0x53,		/* FC_RETURN_PARAM_BASETYPE */
			0x10,		/* FC_ERROR_STATUS_T */

			0x0
        }
    };

static const MIDL_TYPE_FORMAT_STRING __MIDL_TypeFormatString =
    {
        0,
        {
			NdrFcShort( 0x0 ),	/* 0 */
/*  2 */	
			0x11, 0xc,	/* FC_RP [alloced_on_stack] [simple_pointer] */
/*  4 */	0x8,		/* FC_LONG */
			0x5c,		/* FC_PAD */
/*  6 */	
			0x26,		/* FC_CSTRING */
			0x5c,		/* FC_PAD */
/*  8 */	NdrFcShort( 0xff ),	/* 255 */
/* 10 */	
			0x11, 0x8,	/* FC_RP [simple_pointer] */
/* 12 */	
			0x22,		/* FC_C_CSTRING */
			0x5c,		/* FC_PAD */
/* 14 */	
			0x12,		/* FC_UP */
			0x0,		/* 0 */
/* 16 */	NdrFcShort( 0x2 ),	/* Offset= 2 (18) */
/* 18 */	
			0x1b,		/* FC_CARRAY */
			0x0,		/* 0 */
/* 20 */	NdrFcShort( 0x1 ),	/* 1 */
/* 22 */	0x28,		/* Corr desc:  parameter, FC_LONG */
			0x0,		/*  */
#ifndef _ALPHA_
/* 24 */	NdrFcShort( 0xc ),	/* x86, MIPS, PPC Stack size/offset = 12 */
#else
			NdrFcShort( 0x18 ),	/* Alpha Stack size/offset = 24 */
#endif
/* 26 */	0x1,		/* FC_BYTE */
			0x5b,		/* FC_END */
/* 28 */	
			0x1b,		/* FC_CARRAY */
			0x0,		/* 0 */
/* 30 */	NdrFcShort( 0x1 ),	/* 1 */
/* 32 */	0x29,		/* Corr desc:  parameter, FC_ULONG */
			0x54,		/* FC_DEREFERENCE */
#ifndef _ALPHA_
/* 34 */	NdrFcShort( 0x8 ),	/* x86, MIPS, PPC Stack size/offset = 8 */
#else
			NdrFcShort( 0x10 ),	/* Alpha Stack size/offset = 16 */
#endif
/* 36 */	0x2,		/* FC_CHAR */
			0x5b,		/* FC_END */
/* 38 */	
			0x11, 0x8,	/* FC_RP [simple_pointer] */
/* 40 */	0x8,		/* FC_LONG */
			0x5c,		/* FC_PAD */
/* 42 */	
			0x11, 0x0,	/* FC_RP */
/* 44 */	NdrFcShort( 0x4 ),	/* Offset= 4 (48) */
/* 46 */	0x1,		/* FC_BYTE */
			0x5c,		/* FC_PAD */
/* 48 */	0xb5,		/* FC_PIPE */
			0x0,		/* 0 */
/* 50 */	NdrFcShort( 0xfffffffc ),	/* Offset= -4 (46) */
/* 52 */	NdrFcShort( 0x1 ),	/* 1 */
/* 54 */	NdrFcShort( 0x1 ),	/* 1 */
/* 56 */	
			0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 58 */	
			0x22,		/* FC_C_CSTRING */
			0x5c,		/* FC_PAD */
/* 60 */	
			0x12, 0x0,	/* FC_UP */
/* 62 */	NdrFcShort( 0x2 ),	/* Offset= 2 (64) */
/* 64 */	
			0x1b,		/* FC_CARRAY */
			0x0,		/* 0 */
/* 66 */	NdrFcShort( 0x1 ),	/* 1 */
/* 68 */	0x29,		/* Corr desc:  parameter, FC_ULONG */
			0x0,		/*  */
#ifndef _ALPHA_
/* 70 */	NdrFcShort( 0x14 ),	/* x86, MIPS, PPC Stack size/offset = 20 */
#else
			NdrFcShort( 0x28 ),	/* Alpha Stack size/offset = 40 */
#endif
/* 72 */	0x1,		/* FC_BYTE */
			0x5b,		/* FC_END */
/* 74 */	
			0x11, 0x0,	/* FC_RP */
/* 76 */	NdrFcShort( 0x2 ),	/* Offset= 2 (78) */
/* 78 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 80 */	NdrFcShort( 0x48 ),	/* 72 */
/* 82 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 84 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 86 */	NdrFcShort( 0x4 ),	/* 4 */
/* 88 */	NdrFcShort( 0x4 ),	/* 4 */
/* 90 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 92 */	
			0x22,		/* FC_C_CSTRING */
			0x5c,		/* FC_PAD */
/* 94 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 96 */	NdrFcShort( 0x8 ),	/* 8 */
/* 98 */	NdrFcShort( 0x8 ),	/* 8 */
/* 100 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 102 */	
			0x22,		/* FC_C_CSTRING */
			0x5c,		/* FC_PAD */
/* 104 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 106 */	NdrFcShort( 0xc ),	/* 12 */
/* 108 */	NdrFcShort( 0xc ),	/* 12 */
/* 110 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 112 */	
			0x22,		/* FC_C_CSTRING */
			0x5c,		/* FC_PAD */
/* 114 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 116 */	NdrFcShort( 0x44 ),	/* 68 */
/* 118 */	NdrFcShort( 0x44 ),	/* 68 */
/* 120 */	0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 122 */	
			0x22,		/* FC_C_CSTRING */
			0x5c,		/* FC_PAD */
/* 124 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 126 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 128 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 130 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 132 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 134 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 136 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 138 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 140 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 142 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 144 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 146 */	
			0x11, 0x4,	/* FC_RP [alloced_on_stack] */
/* 148 */	NdrFcShort( 0x2 ),	/* Offset= 2 (150) */
/* 150 */	
			0x15,		/* FC_STRUCT */
			0x3,		/* 3 */
/* 152 */	NdrFcShort( 0x10 ),	/* 16 */
/* 154 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 156 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 158 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 160 */	
			0x11, 0x0,	/* FC_RP */
/* 162 */	NdrFcShort( 0xfffffff4 ),	/* Offset= -12 (150) */
/* 164 */	
			0x11, 0x0,	/* FC_RP */
/* 166 */	NdrFcShort( 0x5c ),	/* Offset= 92 (258) */
/* 168 */	
			0x26,		/* FC_CSTRING */
			0x5c,		/* FC_PAD */
/* 170 */	NdrFcShort( 0x64 ),	/* 100 */
/* 172 */	
			0x1d,		/* FC_SMFARRAY */
			0x0,		/* 0 */
/* 174 */	NdrFcShort( 0x80 ),	/* 128 */
/* 176 */	0x2,		/* FC_CHAR */
			0x5b,		/* FC_END */
/* 178 */	
			0x15,		/* FC_STRUCT */
			0x3,		/* 3 */
/* 180 */	NdrFcShort( 0x90 ),	/* 144 */
/* 182 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 184 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 186 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 188 */	NdrFcShort( 0xfffffff0 ),	/* Offset= -16 (172) */
/* 190 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 192 */	
			0x15,		/* FC_STRUCT */
			0x3,		/* 3 */
/* 194 */	NdrFcShort( 0x18 ),	/* 24 */
/* 196 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 198 */	0x6,		/* FC_SHORT */
			0x6,		/* FC_SHORT */
/* 200 */	0x6,		/* FC_SHORT */
			0x38,		/* FC_ALIGNM4 */
/* 202 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 204 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 206 */	
			0x1f,		/* FC_SMVARRAY */
			0x3,		/* 3 */
/* 208 */	NdrFcShort( 0x190 ),	/* 400 */
/* 210 */	NdrFcShort( 0x64 ),	/* 100 */
/* 212 */	NdrFcShort( 0x4 ),	/* 4 */
/* 214 */	0x9,		/* Corr desc: FC_ULONG */
			0x0,		/*  */
/* 216 */	NdrFcShort( 0xfffc ),	/* -4 */
/* 218 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 220 */	
			0x1f,		/* FC_SMVARRAY */
			0x3,		/* 3 */
/* 222 */	NdrFcShort( 0x190 ),	/* 400 */
/* 224 */	NdrFcShort( 0x64 ),	/* 100 */
/* 226 */	NdrFcShort( 0x4 ),	/* 4 */
/* 228 */	0x9,		/* Corr desc: FC_ULONG */
			0x0,		/*  */
/* 230 */	NdrFcShort( 0xfe6c ),	/* -404 */
/* 232 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 234 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 236 */	NdrFcShort( 0x424 ),	/* 1060 */
/* 238 */	NdrFcShort( 0x0 ),	/* 0 */
/* 240 */	NdrFcShort( 0x0 ),	/* Offset= 0 (240) */
/* 242 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 244 */	NdrFcShort( 0xffffff12 ),	/* Offset= -238 (6) */
/* 246 */	0x38,		/* FC_ALIGNM4 */
			0x8,		/* FC_LONG */
/* 248 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 250 */	NdrFcShort( 0xffffffd4 ),	/* Offset= -44 (206) */
/* 252 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 254 */	NdrFcShort( 0xffffffde ),	/* Offset= -34 (220) */
/* 256 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 258 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 260 */	NdrFcShort( 0x530 ),	/* 1328 */
/* 262 */	NdrFcShort( 0x0 ),	/* 0 */
/* 264 */	NdrFcShort( 0x0 ),	/* Offset= 0 (264) */
/* 266 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 268 */	NdrFcShort( 0xffffff9c ),	/* Offset= -100 (168) */
/* 270 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 272 */	NdrFcShort( 0xffffffa2 ),	/* Offset= -94 (178) */
/* 274 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 276 */	NdrFcShort( 0xffffffac ),	/* Offset= -84 (192) */
/* 278 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 280 */	NdrFcShort( 0xffffffd2 ),	/* Offset= -46 (234) */
/* 282 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */

			0x0
        }
    };
