/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2009/02/05 
* MODULE : Add_Delete_subkey_value.cpp 
*
* Description:
*   
*   该模块负责 增加/删除 "子键" & "内容"                     
*
***
* Copyright (c) 2008 - 2010 sudami.
* Freely distributable in source or binary for noncommercial purposes.
* TAKE IT EASY,JUST FOR FUN.
*
****************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <ctype.h>
#include <windows.h>
#include <commctrl.h>
#include <time.h>
#include <Shlobj.h>
#include <map>

#include "ntreg.h"
#include "InitHive.h"
#include "regedit.h"
#include "List_SubKey_and_Value.h"

#include "Add_Delete_subkey_value.h"

//////////////////////////////////////////////////////////////////////////

extern HWND			ListW, TreeW	;
extern PHIVE_UNION	g_phive_union	;
extern struct hive* g_pCurrentHive	;
extern HTREEITEM	currentitem_tv	;
extern char*		currentitem		;



/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                    给父键增加一个子键                     +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--

int
AddSubKey_hive (
	TVINSERTSTRUCT tvins,
	IN char *s
	)
{
	char szSubKeyPath[2048] = "" ;

	int nRtnType = check_is_hive_open( &g_phive_union, s, szSubKeyPath, 1 );
	if ( ReturnType_OK != nRtnType ) { return nRtnType ; }
	OutputDebugString((char*)g_pCurrentHive->filename);

	// 通过解析hive来枚举增加一个子键
	AddSubKey_hive_intenal( g_pCurrentHive, szSubKeyPath, tvins );
	FlushFileBuffers( g_pCurrentHive->hFileMemory ); // 使用函数FlushFileBuffers来把RAM里的数据保存到硬盘里

	return ReturnType_OK ;
}


VOID
AddSubKey_hive_intenal (
	IN struct hive *hdesc, 
	IN char* szSubKeyPath,
	IN TVINSERTSTRUCT tvins
	)
{
	int n = 0 ;
	char *pKeyFullpath, szTmpKeyname[255] ;

	tvins.item.mask		 = TVIF_CHILDREN | TVIF_STATE ;
	tvins.item.hItem	 = currentitem_tv ; 
	tvins.item.cChildren = 1 ;
	tvins.item.state	 = 0 ;
	tvins.item.stateMask = TVIS_EXPANDEDONCE ;

	TreeView_SetItem( TreeW, &tvins.item );
	TreeView_Expand( TreeW, currentitem_tv, TVE_EXPAND );

	// 新建一个子键时,找到唯一的临时名给它
	do 
	{
		sprintf( szTmpKeyname, "New key #%03i", n++ );
		if ( TRUE == check_if_thisKey_is_exclusive( hdesc, szSubKeyPath, szTmpKeyname ) ) {
			break ;
		}

	} while ( n < 20 );

	pKeyFullpath = (char*) malloc( strlen(currentitem) + strlen(szTmpKeyname) + 5 );
	sprintf( pKeyFullpath, "%s\\%s", currentitem, szTmpKeyname );
	free( currentitem ); currentitem = pKeyFullpath ;

	// 调用hive解析函数,增加这个子键
	if ( 0 == *szSubKeyPath ) {
		sprintf( szSubKeyPath, "%s", "\\" );
	}

	if ( FALSE == add_key_whatever( hdesc, szSubKeyPath, szTmpKeyname ) ) {
		return ;
	}

	// 在tree控件中表现出来
	tvins.hParent		 = currentitem_tv ;
	tvins.hInsertAfter	 = TVI_SORT ;
	tvins.item.mask		 = TVIF_CHILDREN | TVIF_STATE | TVIF_TEXT ;
	tvins.item.state	 = 0 ; 
	tvins.item.stateMask = 0xFFFF ;
	tvins.item.cChildren = 1 ;
	tvins.item.pszText	 = szTmpKeyname ;
	tvins.item.cChildren = 0 ;
	
	currentitem_tv = TreeView_InsertItem( TreeW, &tvins );
	
	TreeView_EnsureVisible( TreeW, currentitem_tv );
	TreeView_EditLabel( TreeW, currentitem_tv );
	TreeView_Select( TreeW, currentitem_tv, TVGN_CARET );

	return ;
}


BOOL
check_if_thisKey_is_exclusive (
	IN struct hive *hdesc, 
	IN char* szSubKeyPath,
	IN char* szKeyName
	)
{
	struct nk_key *key;
	int nkofs;
	struct ex_data ex;
	int count = 0, countri = 0;

	if ( NULL == szKeyName ) { return FALSE ; }

	// 遍历该父键的所有子键,查看传进来的这个名字是否是新的
	nkofs = trav_path( hdesc, hdesc->rootofs + 4, szSubKeyPath, TRAV_PATH_WANT_NK );
	if ( !nkofs ) 
	{
		printf( "nk_ls: Key <%s> not found\n", szSubKeyPath );
		return FALSE ;
	}
	nkofs += 4;
	
	key = (struct nk_key *)( hdesc->buffer + nkofs );
	if (key->id != 0x6b6e)
	{
		printf("Error: Not a 'nk' node!\n");
		return FALSE ;
	}
	
	if (key->no_subkeys)
	{
		count = 0;
		printf("-------- SubKey Lists --------\n");
		ex.name = (char *)HEAP_ALLOC( 0x1000 ); // 不管怎样,先分配块大内存,供每次遍历存放名字用
		
		while( (ex_next_n( hdesc, nkofs, &count, &countri, &ex ) > 0) ) 
		{
			if ( 0 == stricmp( szSubKeyPath, ex.name ) )
			{
				HEAP_FREE( ex.name );
				return FALSE ;
			}
		}
		
		HEAP_FREE( ex.name );
	}

	return TRUE ;
}




/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                    删除父键的一个子键                     +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--

int
DeleteSubKey_hive (
	IN char *s
	)
{
	char szSubKeyPath[2048] = "" ;

	int nRtnType = check_is_hive_open( &g_phive_union, s, szSubKeyPath, 1 );
	if ( ReturnType_OK != nRtnType ) { return nRtnType ; }
	OutputDebugString((char*)g_pCurrentHive->filename);

	// 通过解析hive来枚举删除一个子键
	if ( FALSE == del_key_whatever( g_pCurrentHive, szSubKeyPath ) ) { return -1 ; }
	FlushFileBuffers( g_pCurrentHive->hFileMemory ); // 使用函数FlushFileBuffers来把RAM里的数据保存到硬盘里
	
	FREE( currentitem );
	TreeView_DeleteItem( TreeW, currentitem_tv );

	return ReturnType_OK ;
}



/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                    删除父键的指定键值                     +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


int
DeleteAllValue_hive (
	IN char *s
	)
{
	char szSubKeyPath[2048] = "" ;
	int  n					= -1 ;

	int nRtnType = check_is_hive_open( &g_phive_union, s, szSubKeyPath, 1 );
	if ( ReturnType_OK != nRtnType ) { return nRtnType ; }
	OutputDebugString((char*)g_pCurrentHive->filename);

	// 通过解析hive来枚举删除一个子键
	if ( FALSE == del_allValues_whatever( g_pCurrentHive, szSubKeyPath ) ) { return -1 ; }
	FlushFileBuffers( g_pCurrentHive->hFileMemory ); // 使用函数FlushFileBuffers来把RAM里的数据保存到硬盘里
	
	FREE( currentitem );

	while( ( n = ListView_GetNextItem( ListW, n, LVNI_SELECTED ) ) >= 0 ) 
	{
		if ( ListView_DeleteItem(ListW, n) )
		{
			n-- ;
		}
	}

	return ReturnType_OK ;
}



int
DeleteSelectedValue_hive (
	IN char *s
	)
{
	char szSubKeyPath[2048] = ""   ;
	PCHAR pTmpBuffer		= NULL ;
	int  n					= -1   ;
	
	int nRtnType = check_is_hive_open( &g_phive_union, s, szSubKeyPath, 1 );
	if ( ReturnType_OK != nRtnType ) { return nRtnType ; }
	OutputDebugString((char*)g_pCurrentHive->filename);
	
	pTmpBuffer = (char*)HEAP_ALLOC( 0x1000 );
	n = -1 ;
	while( ( n = ListView_GetNextItem( ListW, n, LVNI_SELECTED ) ) >= 0 ) 
	{
		*pTmpBuffer = 0 ;
		ListView_GetItemText( ListW, n, 0, pTmpBuffer, 0x1000 );

		// 通过解析hive来枚举删除一个子键的指定Value
		if ( TRUE == del_value_whatever( g_pCurrentHive, szSubKeyPath, pTmpBuffer ) 
			&& ListView_DeleteItem( ListW, n ) 
		   )
		{
			n--;
		}
	}
	
	FlushFileBuffers( g_pCurrentHive->hFileMemory ); // 使用函数FlushFileBuffers来把RAM里的数据保存到硬盘里
	return ReturnType_OK ;
}



/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                    给父键的增加指定键值                     +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


int
ModifyValueName_hive (
	IN char *s,
	IN char *newname,
	IN char *oldname
	)
{
	void *data ;
	int  type = -1 ;
	int nkofs ;
	char szSubKeyPath[2048] = "" ;

	// 合法性检查
	if ( 0 == stricmp( newname, oldname ) ) { return ReturnType_OK ; }

	int nRtnType = check_is_hive_open( &g_phive_union, s, szSubKeyPath, 1 );
	if ( ReturnType_OK != nRtnType ) { return nRtnType ; }
	OutputDebugString((char*)g_pCurrentHive->filename);

	//
	nkofs = trav_path( g_pCurrentHive, g_pCurrentHive->rootofs + 4, szSubKeyPath, TRAV_PATH_WANT_NK );
	if ( !nkofs ) { return -1; }
	nkofs += 4 ;

	// 得到旧键的类型 & 内容
	type = get_val_type( g_pCurrentHive, nkofs, oldname );
	if (type == -1) { return -1; }

//	data = (void *)get_val_data( g_pCurrentHive, nkofs, oldname, 0 );
	data = (void *)getValueData_mfc( g_pCurrentHive, nkofs, oldname );
	
	// 通过解析hive来给父键的增加指定键值
	add_value_whatever( 
		g_pCurrentHive, 
		szSubKeyPath, 
		newname, 
		type, 
		data == NULL ? "" : (char*)data );

	HEAP_FREE( data );

	// 删除旧的Value
	del_value_whatever( g_pCurrentHive, szSubKeyPath, oldname );
	FlushFileBuffers( g_pCurrentHive->hFileMemory ); // 使用函数FlushFileBuffers来把RAM里的数据保存到硬盘里

	return ReturnType_OK ;
}


int
CreateValue_hive (
	IN char *s,
	IN char *ValueName,
	IN char *ValueData,
	IN int ValueType
	)
{
	int nkofs ;
	char szSubKeyPath[2048] = "" ;
	
	// 合法性检查
	if ( (NULL == ValueName) || (ValueType < 1)  ) { return -1 ; }
	
	int nRtnType = check_is_hive_open( &g_phive_union, s, szSubKeyPath, 1 );
	if ( ReturnType_OK != nRtnType ) { return nRtnType ; }
	OutputDebugString((char*)g_pCurrentHive->filename);
	
	nkofs = trav_path( g_pCurrentHive, g_pCurrentHive->rootofs + 4, szSubKeyPath, TRAV_PATH_WANT_NK );
	if ( !nkofs ) { return -1; }
	nkofs += 4 ;
	
	// 通过解析hive来给父键的增加指定键值
	add_value_whatever( 
		g_pCurrentHive, 
		szSubKeyPath, 
		ValueName, 
		ValueType, 
		ValueData == NULL ? "\0\0\0" : ValueData );
	
	FlushFileBuffers( g_pCurrentHive->hFileMemory ); // 使用函数FlushFileBuffers来把RAM里的数据保存到硬盘里
	
	return ReturnType_OK ;
}



int
ModifyValueData_hive (
	IN char *s,
	IN char *ValueName,
	IN char *ValueContext,
	IN int ValueType
	)
{
	int nkofs, nCounts ;
	struct keyvala *kr ;
	char szTmp[256] ;
	WCHAR szWtmp[256] ;
	char szSubKeyPath[2048] = "" ;
	
	// 合法性检查
	if ( (NULL == ValueName) || (ValueType < 1)  ) { return -1 ; }
	
	int nRtnType = check_is_hive_open( &g_phive_union, s, szSubKeyPath, 1 );
	if ( ReturnType_OK != nRtnType ) { return nRtnType ; }
	OutputDebugString((char*)g_pCurrentHive->filename);
	
	nkofs = trav_path( g_pCurrentHive, g_pCurrentHive->rootofs + 4, szSubKeyPath, TRAV_PATH_WANT_NK );
	if ( !nkofs ) { return -1; }
	nkofs += 4 ;
	
	// 通过解析hive来给父键的增加指定键值
	if ( REG_DWORD == ValueType ) {
		put_dword( g_pCurrentHive, nkofs, ValueName, ValueContext );
	
	} else {

		nCounts = 0 ;
		kr = (keyvala*) HEAP_ALLOC( sizeof(int) + sizeof(int) );
	//	ALLOC( (char*)kr, 1, sizeof(int) + sizeof(int) );
		
		strncpy( szTmp, ValueContext, 256 );
		while ( *ValueContext )
		{
			nCounts++;
			++ValueContext ;
			
			if ( 0 == *ValueContext ) { break ; }
		}
		
		// 字符型的要注意~~
		MByteToWChar( szTmp, szWtmp, 256 ); // 要变成宽字符~
		kr->len = nCounts*2 ;
		kr->data = (PVOID)szWtmp;
		
		put_buf2val_sz( g_pCurrentHive, kr, nkofs, ValueName, ValueType );
	//	FREE(kr);
		HEAP_FREE( kr );
	}
	
	FlushFileBuffers( g_pCurrentHive->hFileMemory ); // 使用函数FlushFileBuffers来把RAM里的数据保存到硬盘里
	
	return ReturnType_OK ;
}


//////////////////////////////////////////////////////////////////////////