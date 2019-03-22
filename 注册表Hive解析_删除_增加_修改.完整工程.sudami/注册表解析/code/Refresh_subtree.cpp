/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2009/02/06
* MODULE : Refresh_subtree.cpp 
*
* Description:
*   
*   该模块负责刷新树形控件的所有内容                     
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
#include "Refresh_subtree.h"

//////////////////////////////////////////////////////////////////////////

extern HWND			ListW, TreeW	;
extern PHIVE_UNION	g_phive_union	;
extern struct hive	*g_pCurrentHive	;

BOOL g_cantRefreshHive = FALSE ;
HKEY g_hk ;
char* g_szTmp_for_refresh = NULL;

/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                  刷新树形控件的所有内容	                  +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--

BOOL 
RefreshSubtree_Total (
	IN HTREEITEM hfc, 
	IN HKEY hk,
	IN const char *kname
	) 
{
	char szxxname[4096] = "" ;

	g_hk = hk ;
	if ( (0 == stricmp( kname, "HKEY_CLASSES_ROOT" ))
		|| 0 == stricmp( kname, "HKEY_LOCAL_MACHINE\\SOFTWARE\\Classes" )
	   )
	{
		//
		// 考虑到刷新这么大的一个根键,用解析hive的方式耗时太长,故换回常规API的方法
		//
// 		g_pCurrentHive = g_phive_union.pHive_HKLM_SOFTWARE ;
// 		if ( !g_phive_union.pHive_HKLM_SOFTWARE ) { 
// 			g_phive_union.pHive_HKLM_SOFTWARE = My_openHive( HIVE_SOFTWARE, HMODE_RW );
// 		} 
// 	
// 		sprintf( szxxname, "%s", "HKEY_LOCAL_MACHINE\\SOFTWARE\\Classes" );
// 		kname = szxxname;
// 
// 		RefreshSubtree_hive( hfc, szxxname );
// 		return g_cantRefreshHive ;

		RefreshSubtree_normal( hfc, hk, kname );
		return TRUE ;
	}

	//
	// 是从根键 HKEY_LOCAL_MACHINE 或 HKEY_USERS 下刷新的. 分配内存备份名字,
	//
	g_szTmp_for_refresh = (char *)HEAP_ALLOC( 0x1000 ); 

	RefreshSubtree_mix( hfc, hk, kname );

	HEAP_FREE( g_szTmp_for_refresh ); // 释放内存

	return g_cantRefreshHive ;
}


int 
RefreshSubtree_hive (
	IN HTREEITEM hti, 
	IN HKEY hk,
	IN const char *kname
	) 
{
	char buf[4096] ; //tmp?
	int n, knl, nkofs ;
	TVITEM tvi, tvi1;
	HTREEITEM hfc, hfcold;
	mystrhash_hive had_keys;
	char szSubKeyPath[2048] = "" ;

	knl = strlen( kname );
	hfc = TreeView_GetChild( TreeW, hti );

	// 先通过树形控件,得到旧的元素,保存于hash表中
	for( n = 0; hfc; n++ ) 
	{
		tvi.mask		= TVIF_HANDLE | TVIF_TEXT | TVIF_STATE ;
		tvi.hItem		= hfc ;
		tvi.stateMask	= TVIS_EXPANDED ;
		tvi.pszText		= buf ; 
		tvi.cchTextMax	= 4095 ;

		// 获取当前子键的名字,保存与tvi.pszText指向的buf中
		if ( !TreeView_GetItem(TreeW, &tvi) ) { ErrMsgDlgBox("RefreshSubtree"); return 1; }
		
		bool is_deleted = false;
		bool is_exp		= !!( tvi.state & TVIS_EXPANDED ) ;
		char *curname	= (char*)malloc( knl + strlen(buf) + 2 );
		strcpy( curname, kname );
		curname[knl] = '\\'; 
		strcpy( curname + knl + 1, buf );

		bool is_deleted_normalAPI = TRUE;
		HKEY sk ;
		
		// 先用普通API打开测试
		if ( RegOpenKeyEx( hk, buf, 0, 0, &sk ) == ERROR_SUCCESS 
			|| (RegOpenKeyEx( hk, buf, 0, KEY_ENUMERATE_SUB_KEYS | KEY_EXECUTE, &sk ) 
			== ERROR_SUCCESS)
		   ) 
		{
			// 虽然用hive解析出现了Bug,没有找到这2个子键,但用常规API还是找到了
			is_deleted_normalAPI = FALSE ;
			
			// 同样插入到hash表中去
		}
		
		// 检查该键是否存在
		*szSubKeyPath = 0;

		//
		// 哎呀,若是 HKEY_USERS\\下的子键,该函数会修改参数二(curname)的值. 
		// 在其中修改时发现会引起其他未知错误,索性在此先备份一下.
		//
		memset( g_szTmp_for_refresh, 0, sizeof(g_szTmp_for_refresh) );
		strcpy( g_szTmp_for_refresh, curname ); // 备份
		check_is_hive_open( &g_phive_union, curname, szSubKeyPath, 0 );
		nkofs = trav_path( g_pCurrentHive, g_pCurrentHive->rootofs + 4, szSubKeyPath, TRAV_PATH_WANT_NK );
		strcpy( curname, g_szTmp_for_refresh ); // 恢复

		if ( !nkofs ) 
		{ 
			is_deleted = TRUE ; // 没找到这个键,表明它已经被删除了.标记一下

			//
			// 调试时发现, 在对 HKEY_CURRENT_USER根键下的子键进行刷新时, 有2个子键一直找不到 --
			// "SessionInformation" 和 "Volatile Environment", 只在刷新里面存在这种情况,若是点+
			// 展开,是能找到并正确显示的,目前仍然不清楚出错原因. 所以这里只能用常规API再弥补一下.
			// 哎,code写的一团糟啊. sudami 2009/02/07
			// 原来这2个键本身就是特殊键,在系统自带的regedit.exe里面也不能进行重命名和删除操作
			//
			if ( FALSE == is_deleted_normalAPI )
			{
				is_deleted = FALSE ;
			}

		} else { // 该键仍然存在

			DWORD has_subkeys;
			if ( is_exp ) { // 若该键已经展开,则要递归遍历
				has_subkeys = RefreshSubtree_hive( hfc, sk, curname );
			} else {

				// 查看该子键是否存在子键,若有获取子键个数
				has_subkeys = get_Key_s_subkey_Counts ( 
									g_pCurrentHive, 
									szSubKeyPath, 
									g_pCurrentHive->rootofs + 4, 
									TRAV_PATH_WANT_NK );

				if ( 0 == has_subkeys )	{ has_subkeys = (DWORD)-1; }
			}

			if ( has_subkeys != (DWORD)-1 ) 
			{
				tvi1.mask		= TVIF_HANDLE | TVIF_CHILDREN ;
				tvi1.hItem		= hfc ;
				tvi1.cChildren	= has_subkeys != 0 ;
				TreeView_SetItem( TreeW, &tvi1 ); // 若该子键有子键,设置下
			}
		} // end -- if( nkofs )

		// 若打开了Reg句柄, 关闭之
		if ( FALSE == is_deleted_normalAPI ) { RegCloseKey(sk); }

		// 找到头儿了都没找到这个Buffer,表明是新的
		if ( (FALSE == is_deleted) &&  ( had_keys.find(buf) == had_keys.end() ) ) { 
			had_keys.insert( strdup(buf) ); // 把查找到的展开的子键名插入到hash表中
		}
		
		free( curname );
		if ( is_deleted ) { hfcold = hfc ; }
		hfc = TreeView_GetNextSibling( TreeW, hfc );
		if (is_deleted) { TreeView_DeleteItem( TreeW, hfcold ); }
	}

	DWORD count_subkeys = had_keys.size();
	
	// 开始刷新,若有新元素,则插入子键到树形控件中
	check_is_hive_open( &g_phive_union, (char*)kname, szSubKeyPath, 0 );
	RefreshSubtree_hive_intenal ( 
		g_pCurrentHive, 
		hti, 
		(char*)szSubKeyPath, 
		(int*)&count_subkeys, 
		had_keys );

	// 收尾工作,清理hash表
	mystrhash_hive::iterator i = had_keys.begin();
	while(i != had_keys.end()) {
		char *c = (*i);
		i++;
		delete []c;
	}
	had_keys.clear();

	return count_subkeys;
}


VOID
RefreshSubtree_hive_intenal (
	IN struct hive *hdesc, 
	IN HTREEITEM hti, 
	IN char* szFatherPath,
	IN int* count_subkeys,
	IN mystrhash_hive had_keys
	)
{
	int nkofs = 0, count = 0, countri = 0;
	char valb[2046] = "";
	char szSubKeyPath[2048] = "" ;
	TVINSERTSTRUCT tvins;
	struct nk_key *key ;
	struct ex_data ex;

	*szSubKeyPath = 0;
	nkofs = trav_path( hdesc, hdesc->rootofs + 4, szFatherPath, TRAV_PATH_WANT_NK );
	if ( !nkofs ) 
	{
		printf("nk_ls: Key <%s> not found\n",szFatherPath);
		return;
	}
	nkofs += 4;
	
	key = (struct nk_key *)(hdesc->buffer + nkofs);
	if (key->id != 0x6b6e)
	{
		printf("Error: Not a 'nk' node!\n");
		return;
	}

	if (key->no_subkeys)
	{
		count = 0;
		printf("-------- SubKey Lists --------\n");
		ex.name = (char *)HEAP_ALLOC( 0x1000 ); // 不管怎样,先分配块大内存,供每次遍历存放名字用

		while( (ex_next_n( hdesc, nkofs, &count, &countri, &ex ) > 0) ) 
		{
			if ( count == 1 ) 
			{
				tvins.hParent		 = hti ;
				tvins.hInsertAfter	 = TVI_SORT ;
				tvins.item.mask		 = TVIF_TEXT | TVIF_CHILDREN | TVIF_STATE ;
				tvins.item.state	 = 0 ;
				tvins.item.stateMask = TVIS_EXPANDEDONCE ;
			}
			
			tvins.item.pszText   = ex.name ; // 子键的名字
			if ( had_keys.find( ex.name ) == had_keys.end() ) 
			{
				// 在旧的hash表中没有找到这个子键的名字,表明是新插入的.赶紧加到树形控件中
				DWORD has_subkeys = get_Key_s_subkey_Counts ( 
					hdesc, 
					"", 
					ex.nkoffs + 4, 
					TRAV_PATH_WANT_NK );
				
				tvins.item.cChildren = has_subkeys != 0;
				TreeView_InsertItem( TreeW, &tvins ); // 新增之~
				++(*count_subkeys);
			}
		}

		HEAP_FREE( ex.name );
	}

	return ;
}


//////////////////////////////////////////////////////////////////////////


typedef hash_set<char*, hash<char*>, str_equal_to> mystrhash_mix;

int 
RefreshSubtree_mix (
	IN HTREEITEM hti, 
	IN HKEY hk, 
	IN const char *kname 
	) 
{
	char buf[4096];//tmp?
	char szSubKeyPath[2048] = "" ;
	int n;
	int knl = strlen(kname);
	TVITEM tvi, tvi1;
	HTREEITEM hfc = TreeView_GetChild(TreeW, hti), hfcold;
	mystrhash_mix had_keys;
	for(n = 0; hfc; n++) 
	{
		HKEY sk;
		tvi.mask=TVIF_HANDLE | TVIF_TEXT | TVIF_STATE;
		tvi.hItem=hfc;
		tvi.stateMask = TVIS_EXPANDED;
		tvi.pszText = buf, tvi.cchTextMax = 4095;

		// 获取当前子键的名字,保存与tvi.pszText指向的buf中
		if (!TreeView_GetItem(TreeW, &tvi)) {ErrMsgDlgBox("RefreshSubtree"); return 1;}
		
		bool is_exp = !!(tvi.state & TVIS_EXPANDED);
		char *curname = (char*)malloc(knl + strlen(buf) + 2);
		strcpy(curname, kname); curname[knl] = '\\'; strcpy(curname + knl + 1, buf);
		
		bool is_deleted = false;

		if (RegOpenKeyEx(hk, buf, 0, KEY_ENUMERATE_SUB_KEYS | KEY_EXECUTE, &sk) == ERROR_SUCCESS) 
		{
			DWORD has_subkeys;
			if (is_exp) 
			{
				if ( (0 == stricmp( curname, "HKEY_LOCAL_MACHINE\\HARDWARE" )) 
					|| (0 == stricmp( curname, "HKEY_CURRENT_CONFIG\\Software" )) 
					|| (0 == stricmp( curname, "HKEY_CURRENT_CONFIG\\System" )) 
					) 
				{
					// 是虚拟键,用正常API枚举刷新
					has_subkeys = RefreshSubtree_normal(hfc, sk, curname );

				} else {
					// OK! 已经枚举到了实际的hive了.用解析hive的方式刷新之
					has_subkeys = RefreshSubtree_hive( hfc, sk, curname );
				}
				
			} else {
				if (RegQueryInfoKey(sk, 0, 0, 0, &has_subkeys, 0, 0, 0, 0, 0, 0, 0))
					has_subkeys = (DWORD)-1;
			}
			RegCloseKey(sk);

			if (has_subkeys != (DWORD)-1) {
				tvi1.mask = TVIF_HANDLE | TVIF_CHILDREN;
				tvi1.hItem = hfc, tvi1.cChildren = has_subkeys != 0;
				TreeView_SetItem(TreeW, &tvi1); 
			}
			if (had_keys.find(buf) == had_keys.end()) {
				had_keys.insert(strdup(buf));
			}
		} else {
			if (RegOpenKeyEx(hk, buf, 0, 0, &sk) == ERROR_SUCCESS) { //??
				RegCloseKey(sk);
			} else {
				is_deleted = true;
			}
		}
		free(curname);
		if (is_deleted) hfcold = hfc;
		hfc = TreeView_GetNextSibling(TreeW, hfc);
		if (is_deleted) TreeView_DeleteItem(TreeW, hfcold);
	}

	DWORD count_subkeys = had_keys.size();
	DWORD sns = MAX_PATH + 1;
	char *sn = (char*)malloc(sns);
	FILETIME ft;
	TVINSERTSTRUCT tvins;

	// 普通方式刷新
	for(n = 0;; n++) 
	{
		sns = MAX_PATH + 1;
		LONG rv = RegEnumKeyEx(hk, n, sn, &sns, 0, 0,0, &ft);
		if (rv != 0) break;

		if (n == 0)
		{
			tvins.hParent = hti, tvins.hInsertAfter = TVI_SORT;
			tvins.item.mask =  TVIF_TEXT | TVIF_CHILDREN | TVIF_STATE;
			tvins.item.state=0, tvins.item.stateMask=TVIS_EXPANDEDONCE;
			tvins.item.pszText = sn;
		}

		if (had_keys.find(sn) == had_keys.end()) 
		{
			HKEY sk;
			DWORD has_subkeys = 0;
			if (!RegOpenKeyEx(hk, sn, 0, KEY_EXECUTE, &sk)) {
				RegQueryInfoKey(sk, 0, 0, 0, &has_subkeys, 0, 0, 0, 0, 0, 0, 0);
				RegCloseKey(sk);
			}
			tvins.item.cChildren = has_subkeys != 0;
			TreeView_InsertItem(TreeW, &tvins);
			count_subkeys++;
		}
	}
	free(sn);
	
	// 清理工作
	mystrhash_mix::iterator i = had_keys.begin();
	while(i != had_keys.end()) {
		char *c = (*i);
		i++;
		delete []c;
	}
	had_keys.clear();
	return count_subkeys;
}