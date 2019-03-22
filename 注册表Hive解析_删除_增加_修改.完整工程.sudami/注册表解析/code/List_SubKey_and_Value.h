#pragma once

#include <windows.h>

//////////////////////////////////////////////////////////////////////////

typedef enum _check_is_hive_open_ReturnType
{
    ReturnType_IsNoting,
	ReturnType_IsRootKey,
	ReturnType_ERRO,
	ReturnType_OK
} check_is_hive_open_ReturnType ;


//////////////////////////////////////////////////////////////////////////

int
GetUserName (
	IN char* szOutUserName
	) ;

int
check_is_hive_open (
	IN struct _phive_union *pHive_nion,
	IN char *s,
	IN char *subKeyPath,
	IN int checkType // 0 ÊÇListValue; 1 ÊÇListSubKey
	) ; 
	
char*
GetStringEnd (
	IN char* ptr
	) ;

char* 
hexdump_mfc (
	IN char *hbuf, 
	IN int start, 
	IN int stop, 
	IN int ascii
	) ;

char* 
getValueData_mfc (
	IN struct hive *hdesc,
	IN int nkofs, 
	IN char *path
	) ;

void
ListValues_hive_intenal (
	IN struct hive *hdesc, 
	IN char* szSubKeyPath
	) ;

int
ListValues_hive (
	IN HWND hwnd,
	IN char *s
	) ;


int
ListSubKeys_hive (
	IN HTREEITEM hItem,
	TVINSERTSTRUCT tvins,
	IN char *s
	) ;

VOID
nk_ls_ListSubKeys (
	IN struct hive *hdesc, 
	IN char* szSubKeyPath,
	IN HTREEITEM hItem,
	IN TVINSERTSTRUCT tvins
	) ;

int
get_Key_s_subkey_Counts (
	IN struct hive *hdesc, 
	IN char *path, 
	IN int vofs,
	IN int type
	) ;

//////////////////////////////////////////////////////////////////////////
