#pragma once

#include <windows.h>
#include <map>

#include "struct.h"
using namespace std;

//////////////////////////////////////////////////////////////////////////

typedef struct _ALL_SYSTEM_HANDLE_INFORMATION {
	DWORD Count;
	SYSTEM_HANDLE_INFORMATION Handles[1] ;
} ALL_SYSTEM_HANDLE_INFORMATION, *PALL_SYSTEM_HANDLE_INFORMATION;

//
// 容器/*std::*/map相关的结构体
//
struct RegHiveRootKey // hive文件路径
{
	char strRegHiveRootKey[255] ; //捆绑进来
	
	friend bool operator<(const RegHiveRootKey &ls,const RegHiveRootKey &rs) 
	{
		return ( 
			(strcmp( ls.strRegHiveRootKey ,rs.strRegHiveRootKey ) < 0) 
			);
	}
};


struct RegHiveFileItem // 注册表根键 & 关联的hive句柄
{
	char strRegFilePath[255] ; //捆绑进来
	HANDLE hRegFile ;
	
	friend bool operator<(const RegHiveFileItem &ls,const RegHiveFileItem &rs) 
	{
		return ( 
			(strcmp( ls.strRegFilePath ,rs.strRegFilePath ) < 0) 
			|| ( (DWORD)(ls.hRegFile) < (DWORD)(rs.hRegFile) ) 
			);
	}
};


typedef struct _QueryFileName {
	DWORD nFileNameBufLen ;
	PVOID pFileNameBuf ;
	HANDLE hFile ;
	BOOL bFileNameOk ;
} QueryFileName, *PQueryFileName ;


typedef struct _memory_dump {
	HANDLE hFileMemory ;   // system进程中对应的hive文件句柄
	char* strHiveRegPath ; // 对应的注册表路径 eg. \REGISTRY\MACHINE\SAM
} MEMORY_DUMP, *PMEMORY_DUMP ;


typedef ULONG (WINAPI *pNtQuerySystemInformation)( ULONG, PVOID, ULONG, PULONG );
typedef ULONG (WINAPI *pZwQueryInformationFile)( HANDLE, PVOID, PVOID, ULONG, ULONG );
typedef ULONG (WINAPI *pZwQueryObject)( HANDLE, ULONG, PVOID, ULONG, PULONG );
typedef ULONG (WINAPI *pZwTerminateThread)( HANDLE, LONG );

//////////////////////////////////////////////////////////////////////////

 


//////////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
extern "C" {
#endif

BOOL 
Init_hive_analyse (
	/*std::*/map<RegHiveRootKey, RegHiveFileItem> &mapRegHiveFile
	) ;

BOOL 
Init_hive_nt_fun_from_ntdll (
	) ;

BOOL 
get_system_pid (
	) ;

BOOL 
get_file_ojbect_type_number (
	IN PALL_SYSTEM_HANDLE_INFORMATION pAHI, 
	IN HANDLE hNul
	) ;

PALL_SYSTEM_HANDLE_INFORMATION 
get_system_handle_table (
	) ;

BOOL 
get_reg_to_hive_file (
	IN /*std::*/map<RegHiveRootKey, RegHiveFileItem> &mapRegHiveFile
	) ;

DWORD 
WINAPI 
parse_file_handle_to_device_file_name (
	IN void *lpParam
	) ;

BOOL 
is_our_need (
	IN PCHAR strFilePath,
	IN HANDLE hFile, 
	IN /*std::*/map<RegHiveRootKey, RegHiveFileItem> &mapRegHiveFile
	) ;

BOOL 
duplicate_hive_file_handle (
	IN PALL_SYSTEM_HANDLE_INFORMATION pAHI, 
	IN /*std::*/map<RegHiveRootKey, RegHiveFileItem> &mapRegHiveFile
	) ;

VOID 
del_reg_hive_file_handle_null (
	IN /*std::*/map<RegHiveRootKey, RegHiveFileItem> &mapRegHiveFile
	) ;

VOID
Display_RegHive_in_map (
	IN /*std::*/map<RegHiveRootKey, RegHiveFileItem> &mapRegHiveFile
	) ;

BOOL
map_or_read_file (
	IN char *filename,
	IN struct hive *hdesc
	) ;

struct hive *My_openHive(char *filename, int mode);
void My_closeHive(struct hive *hdesc);
int My_writeHive(struct hive *hdesc);

HANDLE
find_RegHive_handle_in_map (
	IN char* strHiveRootPath,
	IN /*std::*/map<RegHiveRootKey, RegHiveFileItem> &mapRegHiveFile
	) ;

void RaiseToDebugP() ;

//////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif