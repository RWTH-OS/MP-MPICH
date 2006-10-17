#include <windows.h>
#include <winioctl.h>
#include <stdio.h>
#include "SCIExt.h"
#include "readini.h"

 

#define SCI_TYPE 60000
#define _SISCI_INOUT_ARGUMENT 3
#define _DIS_MAKBF(value,width,offset) (((value) & (width)) << (offset))
#define _SISCI_SET_ARGUMENT_SIZE(size)     _DIS_MAKBF((size) ,0xff,16)
#define _SISCI_INOUT(type) _SISCI_SET_ARGUMENT(_SISCI_INOUT_ARGUMENT, sizeof(type))
#define _SISCI_MKCTL_CODE(code,arg) CTL_CODE(SCI_TYPE, (0x900 + (code)), METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_GET_COMPILATION_REV  _SISCI_MKCTL_CODE(0x1c,  _SISCI_INOUT(DriverInfo))
#define IOCTL_GET_REV              _SISCI_MKCTL_CODE(0x1d,  _SISCI_INOUT(DriverInfo))
#define IOCTL_QUERY_ADAPTER        _SISCI_MKCTL_CODE(0x1f,  _SISCI_INOUT(QueryAdapter))   

/*#define IOCTL_GET_COMPILATION_REV \
	CTL_CODE(SCI_TYPE,  0x932, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_GET_REV \
	CTL_CODE(SCI_TYPE,  0x921, METHOD_BUFFERED, FILE_ANY_ACCESS)


#define IOCTL_QUERY_ADAPTER \
    CTL_CODE(SCI_TYPE,  0x939, METHOD_BUFFERED, FILE_ANY_ACCESS)*/




#define SCI_QUERY_MASK              0x10000000
#define SCI_QUERY_ADAPTER_MASK      0x20000000
#define SCI_QUERY_SYSTEM_MASK       0x40000000

typedef enum {

    _INTERNAL_Q_VENDORID                          = (0x0001 | SCI_QUERY_MASK),
    _INTERNAL_Q_API                               = (0x0002 | SCI_QUERY_MASK),
    _INTERNAL_Q_ADAPTER                           = (0x0003 | SCI_QUERY_MASK),
    _INTERNAL_Q_SYSTEM                            = (0x0004 | SCI_QUERY_MASK)

} sci_query_command_t;



typedef enum {

    _INTERNAL_Q_ADAPTER_DMA_SIZE_ALIGNMENT        = (0x0001 | SCI_QUERY_ADAPTER_MASK),
    _INTERNAL_Q_ADAPTER_DMA_OFFSET_ALIGNMENT      = (0x0002 | SCI_QUERY_ADAPTER_MASK),
    _INTERNAL_Q_ADAPTER_DMA_MTU                   = (0x0003 | SCI_QUERY_ADAPTER_MASK),
    _INTERNAL_Q_ADAPTER_SUGGESTED_MIN_DMA_SIZE    = (0x0004 | SCI_QUERY_ADAPTER_MASK),
    _INTERNAL_Q_ADAPTER_SUGGESTED_MIN_BLOCK_SIZE  = (0x0005 | SCI_QUERY_ADAPTER_MASK),
    _INTERNAL_Q_ADAPTER_CARD_TYPE                 = (0x0006 | SCI_QUERY_ADAPTER_MASK),
    _INTERNAL_Q_ADAPTER_SERIAL_NUMBER             = (0x0007 | SCI_QUERY_ADAPTER_MASK),
    _INTERNAL_Q_ADAPTER_NODEID                    = (0x0008 | SCI_QUERY_ADAPTER_MASK),
    _INTERNAL_Q_ADAPTER_NUMBER_OF_STREAMS         = (0x0009 | SCI_QUERY_ADAPTER_MASK),
    _INTERNAL_Q_ADAPTER_CONFIGURED                = (0x000A | SCI_QUERY_ADAPTER_MASK),
    _INTERNAL_Q_ADAPTER_LINK_OPERATIONAL          = (0x000B | SCI_QUERY_ADAPTER_MASK),
    _INTERNAL_Q_ADAPTER_HW_LINK_STATUS_IS_OK      = (0x000C | SCI_QUERY_ADAPTER_MASK),
    _INTERNAL_Q_ADAPTER_NUMBER                    = (0x000D | SCI_QUERY_ADAPTER_MASK),
    _INTERNAL_Q_ADAPTER_INSTANCE_NUMBER           = (0x000E | SCI_QUERY_ADAPTER_MASK),
    _INTERNAL_Q_ADAPTER_FIRMWARE_OK               = (0x000F | SCI_QUERY_ADAPTER_MASK),
    _INTERNAL_Q_ADAPTER_CONNECTED_TO_SWITCH       = (0x0010 | SCI_QUERY_ADAPTER_MASK),
    _INTERNAL_Q_ADAPTER_LOCAL_SWITCH_TYPE         = (0x0011 | SCI_QUERY_ADAPTER_MASK),
    _INTERNAL_Q_ADAPTER_LOCAL_SWITCH_PORT_NUMBER  = (0x0012 | SCI_QUERY_ADAPTER_MASK),
    _INTERNAL_Q_ADAPTER_CONNECTED_TO_EXPECTED_SWITCH_PORT 
                                                  = (0x0013 | SCI_QUERY_ADAPTER_MASK),
    _INTERNAL_Q_ADAPTER_STREAM_BUFFER_SIZE        = (0x0014 | SCI_QUERY_ADAPTER_MASK),
    _INTERNAL_Q_ADAPTER_ATT_PAGE_SIZE             = (0x0015 | SCI_QUERY_ADAPTER_MASK),
    _INTERNAL_Q_ADAPTER_ATT_NUMBER_OF_ENTRIES     = (0x0016 | SCI_QUERY_ADAPTER_MASK),
    _INTERNAL_Q_ADAPTER_ATT_AVAILABLE_ENTRIES     = (0x0017 | SCI_QUERY_ADAPTER_MASK)

} sci_query_adapter_subcommand_t;



static const LPCTSTR SciName = "\\\\.\\SISCI";
static NodeData Data;
static BOOL Init = FALSE;

typedef struct {
    unsigned int error;
    union {
	DriverInfo Driver;
	QueryAdapter Adapter;
    } Data;
} IoctlData;

static void sciIoctl(HANDLE hSCIDevice,        /* pointer to device of interest */
                         unsigned int dwIoControlCode, /* control code of operation to perform */
                         void *lpInOutBuffer,        /* pointer to input or output data */
			 DWORD DataBufferSize,
                         DWORD * error)         /* pointer to error code */
{               
    //unsigned int      DataBufferSize;
    int        result;
    unsigned int      retbyte;
    IoctlData Buffer;

    *error = 0;

    memcpy(&Buffer.Data, lpInOutBuffer, DataBufferSize);
    {
        OVERLAPPED overlapped;
        overlapped.hEvent = CreateEvent(NULL,TRUE,FALSE,NULL);
        if (overlapped.hEvent != NULL) {
            result = DeviceIoControl(hSCIDevice, dwIoControlCode, &Buffer,
                     DataBufferSize + sizeof(unsigned int), &Buffer,
                     DataBufferSize + sizeof(unsigned int),
                    &retbyte,&overlapped);

            /* if io is pending then wait for it */
            if (!result && GetLastError() == ERROR_IO_PENDING) {
                result = GetOverlappedResult(hSCIDevice,&overlapped,&retbyte,TRUE);
            }

            CloseHandle(overlapped.hEvent);

            /* check error code */
            if (!result) {
                *error = GetLastError();
            } else {
                *error = Buffer.error;
            }

        } else {
            *error = GetLastError();
        }
    }
    memcpy(lpInOutBuffer, &Buffer.Data, DataBufferSize);
}


static HANDLE SCIOpen()
{
    HANDLE            handle;
    unsigned int      i = 0;
    DWORD			  error;
   
    do {            /* Find an available file for this open */
        char            name[100];
        sprintf(name, "%s%u", SciName, i);
        handle = CreateFile(name,
                    GENERIC_READ | GENERIC_WRITE,
                    0,
                    NULL,
                    OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL,
                    NULL);
//	printdbg(("SCIOpen i %d dev %s err %d\n",i, name, handle));
        i++;
    } while (handle == INVALID_HANDLE_VALUE && i < 100);

    /* Initialize reference counter */

    if(handle != INVALID_HANDLE_VALUE) {   
		sciIoctl(handle, (unsigned int) IOCTL_GET_REV, &Data.Driver,
			 sizeof(Data.Driver),&error);
		if (error != 0) {
			/* SISCI Api may have changed */
			/* Because SCIOpen did not fail, we pass */
			/* "SCI installed, driver unknown" */
			/* to enable useage by ch_smi plugin */
			Data.Driver.irm_revision = (DWORD) 99;
			Data.Driver.sc_revision = (DWORD) 99;
			strcpy(Data.Driver.sc_revision_string, 
			  "SCI installed, driver unknown");
		}
    }

	

    return handle;
    
}  /* end of sciOpen */

static
DWORD _queryNodeId(HANDLE fd,DWORD AdapterNo)
{

    DWORD error;
    QueryAdapter queryAd;
    QueryAdapter_old queryAd_old;
    /*
     * We need to do an IOCTL to the core.
     * Initialize the ioctl struct.
     */
    queryAd.adapterNo = AdapterNo;    
    queryAd.command   = _INTERNAL_Q_ADAPTER_NODEID;
    queryAd.data      = 0;   
    queryAd.flags     = 0;

    sciIoctl(fd, (unsigned int) IOCTL_QUERY_ADAPTER, &queryAd, sizeof(queryAd),&error);           
    if(!error)
	return queryAd.data;

    queryAd_old.adapterNo = AdapterNo;    
    queryAd_old.command   = _INTERNAL_Q_ADAPTER_NODEID;
    queryAd_old.data      = 0;   
    queryAd_old.flags     = 0;

    sciIoctl(fd, (unsigned int) IOCTL_QUERY_ADAPTER, &queryAd_old, sizeof(queryAd_old),&error);           
    if(!error)
	return queryAd_old.data;
    else return (DWORD)-1;
}

__declspec(dllexport) BOOL WINAPI _QueryUserData(void *SciId, DWORD *size) {
    if(!size) return FALSE;
    
    if(!Init) {
	*size = 0;
	return TRUE;
    }
    if(*size<sizeof(Data)||!SciId) {
	*size = sizeof(Data);
	return FALSE;
    }
    *size = sizeof(Data);
    memcpy(SciId,&Data,*size);
    return TRUE;
}

static DWORD GetAdapters(HANDLE fd) {
    DWORD res;
    
    Data.NumAdapters = 0;
    do {
	res = _queryNodeId(fd,Data.NumAdapters);
	if(res != (DWORD)-1) 
	    Data.Ids[Data.NumAdapters++]=res;
    }while(res != (DWORD)-1 && Data.NumAdapters <MAXADAPTERS);
    
    if ((res == (DWORD)-1) && (Data.NumAdapters == 0)) {
	/* SISCI Api may have changed */
	/* Because SCIOpen did not fail, we assume 1 Adapter, SCI-ID 99 */
	/* to enable useage by ch_smi plugin */
	Data.Ids[Data.NumAdapters++] = 99;
    }
    return Data.NumAdapters;
}

BOOL WINAPI DllMain(
  HINSTANCE hinstDLL,  // handle to DLL module
  DWORD fdwReason,     // reason for calling function
  LPVOID lpvReserved   // reserved
  ) {
    HANDLE fd;
	SciInfo_t SciInfo;
	unsigned int i;

    if(fdwReason == DLL_PROCESS_ATTACH) {
	memset(&Data,0,sizeof(Data));

	/* if ini file exists */
	if (ReadIniFile("c:\\temp\\SciExt.ini", &SciInfo) == ERR_SCIINFO_OK) {
	    /* overide all information specified by driver */
		/* with data from inifile */

		/* We do no real query, set dummy data */
		/* "SCI installed, driver unknown" */
		/* to enable useage by ch_smi plugin */
		Data.Driver.irm_revision = (DWORD) 99;
		Data.Driver.sc_revision = (DWORD) 99;
		strcpy(Data.Driver.sc_revision_string, 
		  "SCI installed, driver unknown, information set by ini-file");

		Data.NumAdapters = SciInfo.NumAdapter;
		for (i=0; i<Data.NumAdapters; i++) {
			Data.Ids[i] = SciInfo.SciId[i];
			Data.Subnets[i] = SciInfo.SciRing[i];
		}

		Init = TRUE;
		return(TRUE);
	}	

	
	fd = SCIOpen();
	if(fd == INVALID_HANDLE_VALUE) return TRUE;
	//Data.NodeId = _queryNodeId(fd);
	GetAdapters(fd);
	Init = TRUE;
	CloseHandle(fd);
    }

    return TRUE;
}
