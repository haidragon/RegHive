#pragma once

#include <windows.h>

//////////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////////

int
AddSubKey_hive (
	IN TVINSERTSTRUCT tvins,
	IN char *s
	) ;

VOID
AddSubKey_hive_intenal (
	IN struct hive *hdesc, 
	IN char* szSubKeyPath,
	IN TVINSERTSTRUCT tvins
	) ;

BOOL
check_if_thisKey_is_exclusive (
	IN struct hive *hdesc, 
	IN char* szSubKeyPath,
	IN char* szKeyName
	) ;

int
DeleteSubKey_hive (
	IN char *s
	) ;

int
DeleteAllValue_hive (
	IN char *s
	) ;

int
DeleteSelectedValue_hive (
	IN char *s
	) ;

int
ModifyValueName_hive (
	IN char *s,
	IN char *newname,
	IN char *oldname
	) ;

int
CreateValue_hive (
	IN char *s,
	IN char *ValueName,
	IN char *ValueData,
	IN int ValueType
	) ;

int
ModifyValueData_hive (
	IN char *s,
	IN char *ValueName,
	IN char *ValueContext,
	IN int ValueType
	) ;

//////////////////////////////////////////////////////////////////////////
