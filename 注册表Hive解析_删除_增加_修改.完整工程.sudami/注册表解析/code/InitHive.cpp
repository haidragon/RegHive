/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2009/02/15 [15:2:2009 - 14:57]
* MODULE : D:\Program\R0\Coding\RegHive\code\ParaseHive_new\mfc_test\code\InitHive.cpp
* 
* Description:
*   初始化Hive的模块                      
*
***
* Copyright (c) 2008 - 2010 sudami.
* Freely distributable in source or binary for noncommercial purposes.
* TAKE IT EASY,JUST FOR FUN.
*
****************************************************************************************/

#include <windows.h>
#include <Tlhelp32.h>
#include <process.h>
#include <tchar.h>
#include <stdlib.h>
#include <map>

#include "struct.h"
#include "ntreg.h"
#include "InitHive.h"


//////////////////////////////////////////////////////////////////////////


DWORD g_Sytem_Pid				= 0 ;	// System进程Id
DWORD g_FileHandleObjectType	= 0 ;	// 文件句柄的对象类型标号


pNtQuerySystemInformation ZwQuerySystemInformation ;
pZwQueryInformationFile ZwQueryInformationFile;
pZwQueryObject ZwQueryObject;
pZwTerminateThread ZwTerminateThread;

//////////////////////////////////////////////////////////////////////////


BOOL 
Init_hive_nt_fun_from_ntdll (
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2009/01/31 

Routine Description:
  分析Hive文件需要的3个Nt函数地址

Return Value:
  成功返回TRUE,失败返回FALSE   

--*/
{
	BOOL bResult = FALSE	;
	HMODULE hMod			;
	
	hMod = GetModuleHandle(_T("ntdll.dll"));
	if(NULL != hMod)
	{
		ZwQuerySystemInformation = (pNtQuerySystemInformation)GetProcAddress(hMod, "ZwQuerySystemInformation");
		ZwQueryInformationFile = (pZwQueryInformationFile)GetProcAddress(hMod, "ZwQueryInformationFile");
		ZwQueryObject = (pZwQueryObject)GetProcAddress(hMod, "ZwQueryObject");
		ZwTerminateThread = (pZwTerminateThread)GetProcAddress(hMod, "ZwTerminateThread");
		
		if(NULL != ZwQuerySystemInformation && NULL != ZwQueryInformationFile && NULL != ZwQueryObject && NULL != ZwTerminateThread)
		{
			bResult = TRUE;
		}
	}
	
	return bResult;
}


BOOL 
get_system_pid (
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2009/01/31 

Routine Description:
  获取System进程的Pid

Return Value:
  成功返回TRUE,失败返回FALSE  

--*/
{
	BOOL bResult = FALSE	;
	HANDLE hToolhelp		;
	PROCESSENTRY32 pi		;
	
	hToolhelp = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
	if( INVALID_HANDLE_VALUE != hToolhelp )
	{
		pi.dwSize = sizeof( PROCESSENTRY32 );
		if( FALSE != Process32First( hToolhelp, &pi ) )
		{
			do 
			{
				if( 0 == _tcsicmp( pi.szExeFile, _T("System") ) )
				{
					g_Sytem_Pid = pi.th32ProcessID ;
					bResult = TRUE ;
					break;
				}

				memset( &pi, 0, sizeof( PROCESSENTRY32 ) );
				pi.dwSize = sizeof( PROCESSENTRY32 );
			} while( FALSE != Process32Next( hToolhelp, &pi ) );
		}
		
		CloseHandle( hToolhelp );
	}
	
	return bResult ;
}


BOOL 
get_file_ojbect_type_number (
	IN PALL_SYSTEM_HANDLE_INFORMATION pAHI, 
	IN HANDLE hNul
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2009/01/31 

Routine Description:
  获取文件句柄的ObjectTypeNumber

Arguments:
  pAHI - 系统句柄信息
  hNul - Nul句柄

Return Value:
  成功返回TRUE,失败返回FALSE  

--*/
{
	BOOL bResult = FALSE			;
	DWORD nCount, nIndex, nPid		;
	PSYSTEM_HANDLE_INFORMATION pSHI ;
	
	nPid	= GetCurrentProcessId( );
	nCount	= pAHI->Count ;
	pSHI	= (PSYSTEM_HANDLE_INFORMATION) pAHI->Handles ;

	for( nIndex = 0; nIndex < nCount; nIndex++, pSHI++ )
	{
		if( nPid == pSHI->ProcessId )
		{
			if( (USHORT)hNul == pSHI->Handle )
			{
				g_FileHandleObjectType = (DWORD) pSHI->ObjectTypeNumber ;
				bResult = TRUE ;
				break ;
			}
		}
	}

	return bResult;
}


PALL_SYSTEM_HANDLE_INFORMATION 
get_system_handle_table (
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2009/01/31 

Routine Description:
  获取系统句柄表

Return Value:
  成功返回系统句柄表存放内存指针,否则NULL 

--*/
{
	NTSTATUS status								= (NTSTATUS) 0 ;
	DWORD nNeedMemSize							= 0		;
	PALL_SYSTEM_HANDLE_INFORMATION pResult		= NULL	;
	ALL_SYSTEM_HANDLE_INFORMATION ashiTemp[2]	= { 0 }	;

#define STATUS_INFO_LENGTH_MISMATCH      ((NTSTATUS)0xC0000004)

	status = ZwQuerySystemInformation (
		SystemHandleInformation,
		(void *)&ashiTemp,
		sizeof(ALL_SYSTEM_HANDLE_INFORMATION),
		&nNeedMemSize );

	if( !NT_SUCCESS(status) && STATUS_INFO_LENGTH_MISMATCH != status ) 
	{
		printf( "FAILED_WITH_STATUS,NtQuerySystemInformation,Status:0x%08lx",status );
		return pResult;
	}
	
	do 
	{
		nNeedMemSize += 0x10000 ; // 加大点内存

		free( pResult );
		pResult = (PALL_SYSTEM_HANDLE_INFORMATION) malloc( nNeedMemSize );
		if(NULL == pResult)
		{
			free( pResult );
			pResult = NULL ;
			break ;
		}

		status = ZwQuerySystemInformation (
			SystemHandleInformation, 
			(void *)pResult, 
			nNeedMemSize, 
			NULL) ;

		if( !NT_SUCCESS(status) && STATUS_INFO_LENGTH_MISMATCH != status ) 
		{
			printf( "FAILED_WITH_STATUS,NtQuerySystemInformation,Status:0x%08lx",status );
			free( pResult );
			pResult = NULL ;
			break ;
		}

	} while( status == STATUS_INFO_LENGTH_MISMATCH );
	

	return pResult;
}


BOOL 
get_reg_to_hive_file (
	IN /*std::*/map<RegHiveRootKey, RegHiveFileItem> &mapRegHiveFile
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2009/01/31 

Routine Description:
  枚举 HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\hivelist
  下的Hive文件名和注册表的对应情况

Arguments:
  mapRegHiveFile - 保存注册表和Hive对应关系

Return Value:
  成功TRUE,否则FALSE

--*/
{
	BOOL bResult = FALSE;
	HKEY hKey ;
	DWORD nIndex, nType, nResult = 0 ;
	DWORD nValueNameLen = MAX_PATH;
	DWORD nValueLen = MAX_PATH * sizeof(TCHAR);
	TCHAR strValueName[MAX_PATH];
	TCHAR strValue[MAX_PATH];
	RegHiveRootKey regHiveRootKey ;
	RegHiveFileItem regHiveFileItem ;

	nResult =  (DWORD) RegOpenKeyEx (
		HKEY_LOCAL_MACHINE,
		_T( "SYSTEM\\CurrentControlSet\\Control\\hivelist" ),
		0, 
		KEY_READ,
		&hKey );

	if( ERROR_SUCCESS != nResult ) 
	{
		dprintf( "get_reg_to_hive_file() RegOpenKeyEx FAILED: %d\n", nResult );
		return bResult ;
	}
	
	//枚举其下的注册表值
	for( nIndex = 0; TRUE; nIndex++ )
	{
		nResult = (DWORD) RegEnumValue (
			hKey, nIndex, strValueName, &nValueNameLen, 
			0, &nType, (LPBYTE)strValue, &nValueLen ) ;

		if( ERROR_SUCCESS != nResult ) 
		{
			dprintf( "get_reg_to_hive_file() RegEnumValue FAILED: %d\n", nResult );
			return bResult ;
		}
		
		if(REG_SZ == nType)
		{
			if(0 != _tcslen(strValue))
			{
				bResult = TRUE;
				
				regHiveFileItem.hRegFile = NULL; // xx
				_tcsncpy( regHiveFileItem.strRegFilePath, strValue, MAX_PATH );
				regHiveFileItem.strRegFilePath[MAX_PATH - 1] = _T('\0') ;

				_tcsncpy( regHiveRootKey.strRegHiveRootKey, strValueName, MAX_PATH );
				regHiveRootKey.strRegHiveRootKey[MAX_PATH - 1] = _T('\0') ;
				
				mapRegHiveFile.insert (
					/*std::*/map<RegHiveRootKey, RegHiveFileItem>::value_type(regHiveRootKey, regHiveFileItem)
					);
			}
		}
		
		nValueNameLen = MAX_PATH ;
		nValueLen = MAX_PATH * sizeof(TCHAR) ;
	}
	
	RegCloseKey(hKey);

	return bResult ;
}


DWORD 
WINAPI 
parse_file_handle_to_device_file_name (
	IN void *lpParam
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2009/01/31 

Routine Description:
  根据句柄信息,判定这个句柄对应对象的名称

Arguments:
  lpParam - 指向QueryFileName的指针

Return Value:

--*/
{
	BOOL bResult = FALSE;
	PQueryFileName pQueryFileName = (PQueryFileName)lpParam;
	NTSTATUS ntstatus ;
	IO_STATUS_BLOCK ioStatus ;
	FILE_NAME_INFORMATION * pFNI ;
	UNICODE_STRING *pUnicodeString ;
	DWORD nLen;

	pFNI = (PFILE_NAME_INFORMATION)pQueryFileName->pFileNameBuf;
	pFNI->FileNameLength = (pQueryFileName->nFileNameBufLen - sizeof(FILE_NAME_INFORMATION)) >> 1;

	// 如果拒绝访问,ZwQueryInformationFile会被挂起
	ntstatus = ZwQueryInformationFile (
		pQueryFileName->hFile, 
		&ioStatus, 
		pFNI, 
		pQueryFileName->nFileNameBufLen,
		FileNameInformation );

	if( STATUS_SUCCESS == ntstatus )
	{
		memset( pQueryFileName->pFileNameBuf, 0, pQueryFileName->nFileNameBufLen );

		ntstatus = ZwQueryObject (
			pQueryFileName->hFile,
			ObjectNameInformation, 
			(void *)pQueryFileName->pFileNameBuf, 
			pQueryFileName->nFileNameBufLen, 
			NULL );

		if( STATUS_SUCCESS == ntstatus )
		{
			pUnicodeString = (UNICODE_STRING *)pQueryFileName->pFileNameBuf ;
			nLen = pUnicodeString->Length ;
			if( 0 != nLen && pUnicodeString->Buffer )
			{
				printf( "Name:%ws\n", pUnicodeString->Buffer );

				bResult = TRUE;
			}
		}
	}
	
	pQueryFileName->bFileNameOk = bResult;
	
	return 0;
}


BOOL 
is_our_need (
	IN PCHAR strFilePath,
	IN HANDLE hFile, 
	IN /*std::*/map<RegHiveRootKey, RegHiveFileItem> &mapRegHiveFile
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2009/01/31 

Routine Description:
  文件句柄路径是否我们要的

Arguments:
  strFilePath - 文件句柄路径
  hFile - 文件句柄
  mapRegHiveFile - 保存注册表和Hive对应关系

Return Value:
  如果是我们要的TRUE,否则一律FALSE

--*/
{
	BOOL bResult = FALSE;
	/*std::*/map<RegHiveRootKey, RegHiveFileItem>::iterator iter;

	if( NULL == strFilePath ) { return bResult; }

	for( iter = mapRegHiveFile.begin(); iter != mapRegHiveFile.end(); iter++ )
	{

		if( 0 == stricmp( strFilePath, iter->second.strRegFilePath ) )
		{
			bResult = TRUE ;
			iter->second.hRegFile = hFile ;
			break;
		}
	}

	return bResult;
}


PVOID 
get_FileObjectName_from_handle (
	HANDLE hObject
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2009/01/31 

Routine Description:
  调用 ZwQueryInformationFile,ZwQueryObject获取文件句柄对应的文件路径

Arguments:
  hObject - 句柄(文件,设备,进程 ...)

Return Value:

--*/
{
	PVOID		pvBuffer = NULL;
	DWORD		dwLengthRet = 0;
	NTSTATUS	status = 0;
	UNICODE_STRING* pUnicodeString; 
	IO_STATUS_BLOCK ioStatus ;
	char sztmp[4096] = "" ;

	status = ZwQueryInformationFile (
		hObject, 
		&ioStatus, 
		(PVOID)&sztmp, 
		sizeof(sztmp),
		FileNameInformation );
	
	if( STATUS_SUCCESS != status ) { return pvBuffer; }
	
	status = ZwQueryObject( hObject, ObjectNameInformation, NULL, 0, &dwLengthRet );
	if( !NT_SUCCESS(status) && STATUS_INFO_LENGTH_MISMATCH != status ) 
	{
		printf( "get_FileObjectName_from_handle() ZwQueryObject() FAILED,Status:0x%08lx",status );
		return pvBuffer;
	}

	do 
	{
		dwLengthRet += 0x10000 ; // 加大点内存
		
		free( pvBuffer );
		pvBuffer = malloc( dwLengthRet );
		if(NULL == pvBuffer)
		{
			free( pvBuffer );
			pvBuffer = NULL ;
			break ;
		}
		
		status = ZwQueryObject( hObject, ObjectNameInformation, pvBuffer, dwLengthRet, &dwLengthRet );
		
		if( !NT_SUCCESS(status) && STATUS_INFO_LENGTH_MISMATCH != status ) 
		{
			printf( "FAILED_WITH_STATUS,NtQuerySystemInformation,Status:0x%08lx",status );
			free( pvBuffer );
			pvBuffer = NULL ;
			break ;
		}
		
	} while( status == STATUS_INFO_LENGTH_MISMATCH );
	
	pUnicodeString = (UNICODE_STRING *)pvBuffer ;
	if( 0 != pUnicodeString->Length && pUnicodeString->Buffer )
	{
	//	printf( "Name:%ws\n", pUnicodeString->Buffer );
		return pUnicodeString;
	}

	return NULL ;
}


void RaiseToDebugP()
{
    HANDLE hToken;
    HANDLE hProcess = GetCurrentProcess();
    if ( OpenProcessToken(hProcess, TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken) )
    {
        TOKEN_PRIVILEGES tkp;
        if ( LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &tkp.Privileges[0].Luid) )
        {
            tkp.PrivilegeCount = 1;
            tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
            
            BOOL bREt = AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, NULL, 0) ;
        }
        CloseHandle(hToken);
    }    
}


BOOL 
duplicate_hive_file_handle (
	IN PALL_SYSTEM_HANDLE_INFORMATION pAHI, 
	IN /*std::*/map<RegHiveRootKey, RegHiveFileItem> &mapRegHiveFile
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2009/01/31 

Routine Description:
  复制注册表文件句柄到本进程

Arguments:
  pAHI - 系统句柄信息
  mapRegHiveFile - 保存注册表和Hive对应关系

Return Value:
  成功TRUE,否则FALSE

--*/
{
	BOOL bResult = FALSE ;
	BOOL bIsOurNeed ;
	DWORD nCount, nIndex ;
	HANDLE hSystemProcess = NULL;
	HANDLE hCurrentHandle, hResultHandle;
	PSYSTEM_HANDLE_INFORMATION pSHI;

// 	if( FALSE == open_system_process( &hSystemProcess ) ) 
	{
		RaiseToDebugP() ;
		hSystemProcess = OpenProcess( PROCESS_DUP_HANDLE, FALSE, g_Sytem_Pid );
	}
	
	if( NULL == hSystemProcess ) 
	{ 
		OutputDebugString( 
			"duplicate_hive_file_handle() OpenProcess() Failed,打开系统进程没权限\n"
			);
		return bResult;
	}
	
	nCount	= pAHI->Count	;
	pSHI	= pAHI->Handles	;
	hCurrentHandle = GetCurrentProcess();
	
	for( nIndex = 0; nIndex < nCount; nIndex++, pSHI++ )
	{
		if( (g_Sytem_Pid == pSHI->ProcessId)
			&& (g_FileHandleObjectType == (DWORD) pSHI->ObjectTypeNumber)
		  )
		{
			//是System进程中的File类型句柄
			if( FALSE == \
				DuplicateHandle(
					hSystemProcess,
					(HANDLE)pSHI->Handle, 
					hCurrentHandle,
					&hResultHandle, 
					0, 
					FALSE, 
					DUPLICATE_SAME_ACCESS )
			   )
			{
				continue ; 
			}
			
			// 复制成功, 开始用驱动来获取句柄路径
			bIsOurNeed = FALSE;
// 			if(true == query_my_handle_path((DWORD)hResultHandle, strBuf, sizeof(strBuf)))
// 			{
// 				bIsOurNeed = is_our_need( queryFileName.pFileNameBuf, hResultHandle, mapRegHiveFile );
// 			}
// 			else
			{
				char tmpName[4096] = "" ;
				UNICODE_STRING* pObjectName = (UNICODE_STRING*)get_FileObjectName_from_handle( hResultHandle );
				
				if ( NULL != pObjectName )
				{
					sprintf( tmpName, "%ws", pObjectName->Buffer );
					bIsOurNeed = is_our_need( tmpName, hResultHandle, mapRegHiveFile );
					free( pObjectName );
				}
			}
			
			if( FALSE == bIsOurNeed )	// 不是我们需要的就关闭复制过来的句柄
			{
				CloseHandle( hResultHandle );
			}
			else
			{
				bResult = TRUE;	// 设置成功标志
			}
			
		}
	}
	
	CloseHandle( hSystemProcess );
	return bResult;
}


VOID 
del_reg_hive_file_handle_null (
	IN /*std::*/map<RegHiveRootKey, RegHiveFileItem> &mapRegHiveFile
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2009/01/31 

Routine Description:
  规整下注册表和Hive文件的对应关系,把文件句柄为NULL的干掉

Arguments:
  mapRegHiveFile - 保存注册表和Hive对应关系

Return Value:

--*/
{
	/*std::*/map<RegHiveRootKey, RegHiveFileItem>::iterator iter;
	/*std::*/map<RegHiveRootKey, RegHiveFileItem>::iterator iterTemp;
	
	for( iter = mapRegHiveFile.begin(); iter != mapRegHiveFile.end(); )
	{
		if( NULL != iter->second.hRegFile ) {
			iter++;
		} else {
			iterTemp = iter;
			iter++;
			mapRegHiveFile.erase( iterTemp );
		}
	}
}


VOID
Display_RegHive_in_map (
	IN /*std::*/map<RegHiveRootKey, RegHiveFileItem> &mapRegHiveFile
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2009/02/01 

Routine Description:
  把最终得到的注册表hive句柄和对应的路径显示出来

Arguments:
  mapRegHiveFile - 保存注册表和Hive对应关系

Return Value:

--*/
{
	/*std::*/map<RegHiveRootKey, RegHiveFileItem>::iterator iter ;

	for( iter = mapRegHiveFile.begin(); iter != mapRegHiveFile.end(); iter++ )
	{
		printf ( 
			"注册表根键:%s\n注册表HIVE路径:%s\n关联句柄:0x%08lx\n\n",
			iter->first.strRegHiveRootKey,
			iter->second.strRegFilePath,
			iter->second.hRegFile
			);
	}

	return ;
}


HANDLE
find_RegHive_handle_in_map (
	IN char* strHiveRootPath,
	IN /*std::*/map<RegHiveRootKey, RegHiveFileItem> &mapRegHiveFile
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2009/02/01 

Routine Description:
  在map容器中找到 比如根键 "\\REGISTRY\\MACHINE\\SAM" 对应的句柄

Arguments:
  strHiveRootPath - eg. "\\REGISTRY\\MACHINE\\SAM"
  mapRegHiveFile - 保存注册表和Hive对应关系

Return Value:
  
--*/
{
	/*std::*/map<RegHiveRootKey, RegHiveFileItem>::iterator iter ;

	if ( NULL == strHiveRootPath ) { return 0; }

	for( iter = mapRegHiveFile.begin(); iter != mapRegHiveFile.end(); iter++ )
	{
		if ( 0 == strcmp( iter->first.strRegHiveRootKey, strHiveRootPath ) )
		{
			return iter->second.hRegFile ;
		}
	}

	return 0 ;
}


//////////////////////////////////////////////////////////////////////////

BOOL g_bHive_Initied = FALSE ; 

BOOL 
Init_hive_analyse (
	/*std::*/map<RegHiveRootKey, RegHiveFileItem> &mapRegHiveFile
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2009/02/01 

Routine Description:
  初始化R3解析hive,准备好一切东西

Arguments:
  mapRegHiveFile - 保存注册表和Hive对应关系

Return Value:

--*/
{
	BOOL bResult = FALSE;
	HANDLE hFile;
	PALL_SYSTEM_HANDLE_INFORMATION pAHI;
	
	mapRegHiveFile.clear();
	
	// 获取3个Nt函数地址
	if( FALSE == Init_hive_nt_fun_from_ntdll() ) { return bResult; }	

	// 获取System进程Id
	if( FALSE == get_system_pid() ) { return bResult; }

	// 产生一个文件句柄
	hFile = CreateFile(_T("NUL"), GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, 0);	
	if( INVALID_HANDLE_VALUE == hFile ) { return bResult; }

	pAHI = get_system_handle_table( );	// 获取系统句柄表
	if( NULL == pAHI ) { return bResult; }
				
	// 获取文件句柄的对象类型标号
	if( FALSE == get_file_ojbect_type_number( pAHI, hFile ) ) { return bResult; }	
					
	// 获取注册表和Hive文件对应关系
	if( FALSE == get_reg_to_hive_file( mapRegHiveFile ) ) { return bResult; }	
	
	// 复制注册表文件句柄到本进程
	if( FALSE == duplicate_hive_file_handle( pAHI, mapRegHiveFile ) ) { return bResult; }
	
	// 规整下注册表和Hive文件的对应关系,把文件句柄为NULL的干掉
	del_reg_hive_file_handle_null( mapRegHiveFile );
	bResult = TRUE;

	// 打印收集的注册表hive和关联句柄等信息
	Display_RegHive_in_map( mapRegHiveFile );

	// 收尾工作
	free(pAHI);
	pAHI = NULL;	
	CloseHandle(hFile);

	if(FALSE == bResult) { mapRegHiveFile.clear(); }
	g_bHive_Initied = bResult ; 
	return bResult;
}



BOOL
map_or_read_file (
	IN char *filename,
	IN struct hive *hdesc
	)
{
	DWORD szread ;
	PCHAR pTMP = NULL ; 
	HANDLE hMap = NULL, htmpFile = NULL, hFileMemory = NULL ;

	if ( NULL == filename ) { return FALSE ; }

	hdesc->state		= 0		;
	hdesc->buffer		= NULL	;
	hdesc->bMapped		= FALSE ;
	hdesc->hFileMemory	= NULL	;
	hdesc->filename		= str_dup( filename );

	if ( '\\' == *filename )
	{
		// 表明是要从内存中去解析hive
		/*std::*/map<RegHiveRootKey, RegHiveFileItem> mapRegHiveFile ;
		
		Init_hive_analyse( mapRegHiveFile );
		hFileMemory = find_RegHive_handle_in_map( filename, mapRegHiveFile );
		if ( NULL == hFileMemory )
		{
			//
			// 清理工作
			//
			return FALSE ;
		}

		FlushFileBuffers( hFileMemory );

		hdesc->hFileMemory	= hFileMemory ;
		hdesc->size = GetFileSize( hFileMemory,  NULL );
		hMap = CreateFileMapping( hFileMemory, NULL, /*PAGE_READONLY |*/ PAGE_READWRITE, 0, 0, 0 );
		if( NULL == hMap ) { return FALSE ; }
		
		pTMP = (PCHAR)MapViewOfFileEx( hMap, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0, NULL );
		if ( NULL == pTMP ) { return FALSE ; }
		
		hdesc->buffer = pTMP;	
		hdesc->bMapped = TRUE ;
		return TRUE ;

	} 
	else // 是读其他hive文件
	{
		htmpFile = CreateFile(hdesc->filename,
			GENERIC_READ,
			0,
			NULL,
			OPEN_ALWAYS,
			FILE_ATTRIBUTE_NORMAL,
			NULL);
		
		if (htmpFile == INVALID_HANDLE_VALUE) { return FALSE ; }
		
		// Read the whole file
		hdesc->size = GetFileSize( htmpFile,  NULL );
		ALLOC( hdesc->buffer, 1, hdesc->size );
		ReadFile( htmpFile, (void *)hdesc->buffer, hdesc->size, &szread, NULL );
		CloseHandle( htmpFile );
		return TRUE ;
	}

	return FALSE ;
}


struct hive *
My_openHive (
	IN char *filename,
	IN int mode
	)
{
	BOOL bResult = FALSE ;
	struct hive *hdesc;
	int vofs;
	unsigned long pofs;
	char *c;
	struct hbin_page *p;
	struct regf_header *hdr;
	
	int verbose = (mode & HMODE_VERBOSE);
	CREATE(hdesc,struct hive,1);
	
	bResult = map_or_read_file( filename, hdesc );
	if ( FALSE == bResult )
	{ 
		My_closeHive( hdesc );
		return NULL ;
	}
	
	// Now run through file, tallying all pages
	// NOTE/KLUDGE: Assume first page starts at offset 0x1000
	pofs = 0x1000;
	hdr = (struct regf_header *)hdesc->buffer;
	if (hdr->id != 0x66676572) 
	{
		printf("openHive(%s): File does not seem to be a registry hive!\n",filename);
		return NULL ;
	}
	for (c = hdr->name; *c && (c < hdr->name + 64); c += 2) 
		putchar(*c);
	
	printf( "\n" );
	hdesc->rootofs = hdr->ofs_rootkey + 0x1000;
	while (pofs < hdesc->size) 
	{
#ifdef LOAD_DEBUG
		if (verbose) 
			hexdump(hdesc->buffer,pofs,pofs+0x20,1);
#endif
		p = (struct hbin_page *)(hdesc->buffer + pofs);
		if (p->id != 0x6E696268) 
		{
			printf("Page at 0x%lx is not 'hbin', assuming file contains garbage at end",pofs);
			break;
		}
		
		hdesc->pages++;
#ifdef LOAD_DEBUG
		if (verbose) 
			printf("\n###### Page at 0x%0lx has size 0x%0lx, next at 0x%0lx ######\n",pofs,p->len_page,p->ofs_next);
#endif
		
		if (p->ofs_next == 0) 
		{
#ifdef LOAD_DEBUG
			if (verbose) 
				printf("openhive debug: bailing out.. pagesize zero!\n");
#endif
			return(hdesc);
		}
		
#if 0
		
		if (p->len_page != p->ofs_next)
		{
#ifdef LOAD_DEBUG
			if (verbose) 
				printf("openhive debug: len & ofs not same. HASTA!\n");
#endif
			exit(0);
			
		}
		
#endif
		vofs = pofs + 0x20; /* Skip page header */
		
#if 1
		while (vofs-pofs < p->ofs_next)
		{
			vofs += parse_block(hdesc,vofs,verbose);
		}
#endif
		pofs += p->ofs_next;
		
	}

	return(hdesc);
}



void My_closeHive(struct hive *hdesc)
{
	
	// FREE(hdesc->filename);
	FlushFileBuffers( hdesc->hFileMemory );

	if ( hdesc->bMapped ) {
		UnmapViewOfFile( hdesc->buffer );
	} else {
		FREE( hdesc->buffer );
	}
	
	FREE( hdesc );
}


int My_writeHive(struct hive *hdesc)
{
	
	HANDLE hFile;
	DWORD dwBytesWritten;
	hFile = CreateFile("C:\\tmp2.hiv",  
		GENERIC_WRITE,                // open for writing
		0,                            // do not share
		NULL,                         // no security
		CREATE_ALWAYS,                  // open or create
		FILE_ATTRIBUTE_NORMAL,        // normal file
		NULL);
	if(hFile == INVALID_HANDLE_VALUE)
	{      
		printf("Can't open dump file");
		return 0;
	}
	WriteFile(hFile, hdesc->buffer, hdesc->size,&dwBytesWritten, NULL);
	if(dwBytesWritten != hdesc->size)
	{
		printf("WriteHive error\n");
	}
	CloseHandle(hFile);
	return 0;
}

