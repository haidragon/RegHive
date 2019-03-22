#pragma once

#include <windows.h>

//////////////////////////////////////////////////////////////////////////

typedef hash_set<char*, hash<char*>, str_equal_to> mystrhash_hive ;


//////////////////////////////////////////////////////////////////////////

BOOL 
RefreshSubtree_Total (
	IN HTREEITEM hfc, 
	IN HKEY hk,
	IN const char *kname
	) ;

int 
RefreshSubtree_hive (
	IN HTREEITEM hti, 
	IN HKEY hk,
	IN const char *kname
	) ; 

VOID
RefreshSubtree_hive_intenal (
	IN struct hive *hdesc, 
	IN HTREEITEM hti, 
	IN char* szFatherPath,
	IN int* count_subkeys,
	mystrhash_hive had_keys
	) ;

int 
RefreshSubtree_mix (
	IN HTREEITEM hti, 
	IN HKEY hk, 
	IN const char *kname 
	) ; 

//////////////////////////////////////////////////////////////////////////
