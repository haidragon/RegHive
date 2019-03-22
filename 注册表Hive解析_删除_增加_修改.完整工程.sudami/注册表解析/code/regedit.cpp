  /***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2009/02/05 
* MODULE : regedit.cpp  
*
* Description:
*   
*   总模块,负责显示界面,相应消息                     
*
***
* Copyright (c) 2008 - 2010 sudami.
* Freely distributable in source or binary for noncommercial purposes.
* TAKE IT EASY,JUST FOR FUN.
*
****************************************************************************************/

#include "regedit.h"
#include "resource.h"
#include <time.h>
#include <Shlobj.h>
#include <map>

#include "regsavld.h"
#include "ntreg.h"
#include "InitHive.h"
#include "List_SubKey_and_Value.h"
#include "Add_Delete_subkey_value.h"
#include "Refresh_subtree.h"

//////////////////////////////////////////////////////////////////////////


HINSTANCE hInst;
HWND MainWindow, SbarW, hwndToolTip, TreeW, ListW, LastFocusedW, RplProgrDlg = 0;
HCURSOR EWcur;
HIMAGELIST imt;

HIMAGELIST m_hilTree;
HICON hicon ;
HBITMAP hImage;

//////////////////////////////////////////////////////////////////////////


struct hive *g_pCurrentHive = NULL;

PHIVE_UNION g_phive_union ;

//////////////////////////////////////////////////////////////////////////

char szClsName[] = "sudami's Regedit Hive Parase Demo";
char szWndName[] = "RegHive V1.02";
bool has_rest_priv = true, has_back_priv = true;
bool co_initialized = false;
volatile bool rr_connecting = false;

int xw=10,yw=10,dxw=600,dyw=400,xTree=150,xName=400,xType=20,xData=150;
bool sDat=0, sVal=0, sKeys=0, sMatch=0;
DWORD Settings[16];
DWORD SbarHeight;
bool onWpos;
int xWpos;
char *currentitem=NULL;
HTREEITEM currentitem_tv=NULL;
HBITMAP img_up,img_down;
HFONT Cour12;
HPEN ThickPen;
HMENU theFavMenu = NULL, theFileMenu = NULL, theEditMenu = NULL;
struct favitem_t { char *name, *key, *value, *comment; };
vector<favitem_t> favItems ;

#define REFAVPATH "Software\\Microsoft\\Windows\\CurrentVersion\\Applets\\Regedit\\Favorites"
#define MYCOMP "My Computer\\"
#define MYCOMPLEN 12

int dragitno = 0;
HIMAGELIST dragiml = 0;
bool is_dragging = false, is_key_dragging = false; 
bool prev_candrop = false, could_ever_drop = false;
HTREEITEM prevdhlti = 0;
HCURSOR curs_arr = 0, curs_no = 0;
HICON regsmallicon = 0;
time_t prevdhltibtm = 0;
// TreeView_SetItemState is documented as 5.80, but it doesn't use anything!!!
#ifndef TreeView_SetItemState
#define TreeView_SetItemState(hwndTV, hti, data, _mask) \
{ TVITEM _ms_TVi;\
	_ms_TVi.mask = TVIF_STATE; \
	_ms_TVi.hItem = hti; \
	_ms_TVi.stateMask = _mask;\
	_ms_TVi.state = data;\
	SNDMSG((hwndTV), TVM_SETITEM, 0, (LPARAM)(TV_ITEM FAR *)&_ms_TVi);\
}
#endif

char *mvcpkeyfrom = 0, *mvcpkeyto = 0;
BYTE mvcp_move = 1;


HTREEITEM HKCR,HKCU,HKLM,HKUS,HKCC,HKDD,HKPD;


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                段落间隔 -- 输入你的模块功能               +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


int WINAPI WinMain (HINSTANCE hTI, HINSTANCE, LPSTR lpszargs, int nWinMode)
{
	HWND hwnd;
	MSG msg;
	WNDCLASS wcl;
	hInst=hTI;
	
	
	if (FindWindow(szClsName,NULL)) {MessageBeep(MB_OK);return 0;}//?
	InitCommonControls();
	
	wcl.hInstance=hTI;
	wcl.lpszClassName=szClsName;
	wcl.lpfnWndProc=WindowProc;
	wcl.style=CS_HREDRAW | CS_VREDRAW;
	wcl.hIcon=LoadIcon (hTI,"RegEdit");
	wcl.hCursor=LoadCursor(NULL,IDC_SIZEWE);//LoadCursor (NULL, IDC_ARROW);
	wcl.lpszMenuName="MainMenu"; // 这里就把菜单注册了
	wcl.cbClsExtra=0;
	wcl.cbWndExtra=0;
	wcl.hbrBackground=(HBRUSH)GetStockObject (LTGRAY_BRUSH);
	if (!RegisterClass (&wcl)) return 0;
	
	
	wcl.lpszClassName="MyHexEdit";
	wcl.lpfnWndProc=MyHexEditProc;
	wcl.hIcon=NULL, wcl.hCursor=NULL;
	wcl.lpszMenuName=NULL;
	wcl.cbWndExtra=16;
	wcl.hbrBackground=(HBRUSH)GetStockObject(WHITE_BRUSH);
	if (!RegisterClass (&wcl)) return 0;
	
	EWcur=LoadCursor(NULL,IDC_SIZEWE);
	imt=ImageList_LoadBitmap(hTI,(char*)IDB_TYPES,16,0,CLR_NONE);
	img_up=LoadBitmap(hTI,"ARROWUP");
	img_down=LoadBitmap(hTI,"ARROWDOWN");
	Cour12=CreateFont(16,8,0,0,FW_LIGHT,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,FIXED_PITCH | FF_DONTCARE, "Courier New");
	ThickPen=CreatePen(PS_SOLID,3,RGB(0,0,0));
	
	curs_no = LoadCursor(NULL, IDC_NO);
	if (!curs_no) ErrMsgDlgBox("LoadCursor(NULL, IDC_NO)");
	curs_arr = LoadCursor(NULL, IDC_ARROW);
	if (!curs_arr) ErrMsgDlgBox("LoadCursor(NULL, IDC_ARROW)");
	regsmallicon = LoadIcon(hInst, "REGSMALL");
	
	
	Beep(300,10);
	LoadSettings();
	hwnd=CreateWindow (szClsName, szWndName,
		WS_OVERLAPPEDWINDOW,
		xw,
		yw,
		dxw, //Width
		dyw, //Height
		HWND_DESKTOP,
		NULL,
		hTI,
		NULL
		);
	MainWindow=hwnd;
	
	ShowWindow(hwnd,nWinMode);
	UpdateWindow(hwnd);
	
	theFavMenu = GetSubMenu(GetMenu(hwnd), 3);
	theFileMenu = GetSubMenu(GetMenu(hwnd), 0);
	theEditMenu = GetSubMenu(GetMenu(hwnd), 1);
	
	if (EnablePrivilege_NT(0, SE_BACKUP_NAME)) {
		has_back_priv = false;
		//ErrMsgDlgBox(SE_BACKUP_NAME);
	}
	if (EnablePrivilege_NT(0, SE_RESTORE_NAME)) {
		has_rest_priv = false;
		//ErrMsgDlgBox(SE_RESTORE_NAME);
	}
	EnablePrivilege_NT(0, SE_SHUTDOWN_NAME); //may be...
	
	while (GetMessage (&msg,NULL,0,0)) {
		TranslateMessage (&msg);
		DispatchMessage(&msg);
	}
	//CloseHandle(imt);
	DeleteObject(img_up); DeleteObject(img_down);
	DeleteObject(Cour12); DeleteObject(ThickPen);
	if (co_initialized) CoUninitialize();
	return 0;
}

LRESULT CALLBACK WindowProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	//HDC hdc;
	int n, k, i ;
	char *s, ss[40] ;
	HKEY hk ;
	TVINSERTSTRUCT tvins = { 0 };
	LVCOLUMN lvcol ;
	HMENU pum, mn1 ;
	
	switch (msg) {

	case WM_CHAR : 
		
		if ( (TCHAR)wParam == VK_TAB && !is_dragging ) 
		{
			// 若当前没有拖拽,且用户按下了table键,则在树形控件和列表控件之间转换
			if ( LastFocusedW == TreeW ) {
				LastFocusedW = ListW ;
			} else {
				LastFocusedW = TreeW ;
			}

			SetFocus( LastFocusedW ); // 设置当前焦点
		}

		// 若当前处于树形控件的拖拽状态,但用户按下了ESC键,则取消拖拽
		if ( (TCHAR)wParam == VK_ESCAPE && is_dragging ) {
			ValuesEndDrag(hwnd, false);
		}

		break;
		

	case WM_COMMAND :

		if ( LOWORD(wParam) >= 41000 && LOWORD(wParam) < 41100 ) // 收藏夹菜单
		{ 
			int it = LOWORD(wParam) - 41000 ;
			if ( it >= favItems.size() ) break ;

			char *k0 = favItems[it].key; fchar key;
			FavKeyName2ShortName( k0, key );
			HTREEITEM ti = ShowItemByKeyName( TreeW, key );

			if (ti && favItems[it].value) 
			{
				if ( !SelectItemByValueName( ListW, favItems[it].value ) )
					SetFocus(LastFocusedW = ListW);
			}

			break;
		}

		switch (LOWORD (wParam)) {
		case IDM_EXIT:
			//SaveSettings();
			DestroyWindow(hwnd);
			break;
			
		case IDM_SETTINGS: // 弹出 "设置" 对话框
		//	DialogBox( hInst, "SETTINGS", hwnd, DialogSettings );
			break;
			
		case IDM_ABOUT:	// 弹出 "关于" 对话框
			DialogBox( hInst, "ABOUT", hwnd, DialogAbout );
			break;
			
		case IDM_EDIT_FIND:
		case IDM_SEARCHREPLACE: // 查找,替换 太复杂了.不必用Hive去做了.
			if (RplProgrDlg) { SetFocus(RplProgrDlg); break; }
			DoSearchAndReplace( hwnd, false, LOWORD(wParam) == IDM_EDIT_FIND );
			break;
			
		case IDM_FINDNEXT:
			DoSearchAndReplaceNext( hwnd );
			break;
			
		case ID_REGISTRY_LOADHIVE: // 调用 RegLoadKey 函数 加载hive文件
			{
				load_hive_dialog_data d;
				achar ci = currentitem;
				char *c = strchr(ci, '\\'); if (c) *c = 0;
				d.root_key_name = ci;
				if ( DialogBoxParam(hInst,"LOADHIVE",hwnd,DialogLDH,(LONG)&d) ) 
				{
					hk = rkeys.KeyByName(d.root_key_name);
					if(DWORD LE=RegLoadKey(hk, d.subkey_name, d.fname)) {
						ErrMsgDlgBox("RegLoadKey", LE);
					}
				}
			}
			break;
			
		case IDM_REGISTRY_UNLOADHIVE: { // 调用 RegUnLoadKey 函数 卸载hive文件
			achar ci = currentitem;
			char *c = strchr(ci, '\\'); if (!c) break;
			*c++ = 0;
			hk = rkeys.KeyByName(ci);
			if (DWORD LE=RegUnLoadKey(hk, c)) {
				ErrMsgDlgBox("RegUnLoadKey", LE);
			} else
				SendMessage(hwnd,WM_COMMAND,340,0);
									  }
			break;
			
		case ID_EDIT_SAVEKEYTOFILE:  // 调用 RegSaveKey 函数保存成hive文件
			{
				save_key_dialog_data d;
				d.key_name = currentitem;
				if (DialogBoxParam(hInst,"SAVEKEY",hwnd,DialogSVK,(LONG)&d)) 
				{
					hk = GetKeyByName(d.key_name, KEY_READ);
					if(DWORD LE = RegSaveKey(hk, d.fname, NULL)) {
						ErrMsgDlgBox("RegSaveKey", LE);
					} 
					RegCloseKey(hk);
				}
			}
			break;
			
		case ID_EDIT_LOADKEYFROMFILE:  // 调用 RegRestoreKey 函数载入现有的hive文件
			{
				load_key_dialog_data d;
				d.key_name = currentitem;
				d.force = d.nolazy = d.refresh = d.volatil = false;
				if (DialogBoxParam(hInst,"LOADKEY",hwnd,DialogLDK,(LONG)&d)) 
				{
					hk = GetKeyByName(d.key_name, KEY_READ);
					DWORD flag = (d.force? REG_FORCE_RESTORE : 0) | (d.nolazy? REG_NO_LAZY_FLUSH : 0)
						| (d.refresh? REG_REFRESH_HIVE : 0) | (d.volatil? REG_WHOLE_HIVE_VOLATILE : 0);
					if(DWORD LE=RegRestoreKey(hk, d.fname, flag)) {
						ErrMsgDlgBox("RegRestoreKey", LE);
					}
					RegCloseKey(hk);
				}
			}
			break;
			
		case IDM_FAV_GOTO: { // 收藏菜单中,显示"goto"按钮
			fchar key;
			if (DialogBoxParam(hInst,"GOTOKEY",hwnd,DialogGotoKey,(LONG)&key)) {
				ShowItemByKeyName(TreeW, key);
			}
						   }
			break;
			
		case IDM_FAV_ADDKEY: { // 收藏菜单中, 添加选项
			fchar key_and_title[4]; key_and_title[0].c = strdup(currentitem);
			SuggestTitleForKey(key_and_title[0] /*key*/, key_and_title[1] /*title*/);
			i = ListView_GetNextItem(ListW, -1, LVNI_FOCUSED);
			if (i >= 0) {
				DWORD ns = 0;
				GetLVItemText(ListW, i, key_and_title[2].c, ns);
			}
			if (DialogBoxParam(hInst,"ADDFAVKEY",hwnd,DialogAddFavKey,(LONG)&key_and_title)) {
				AddKeyToFavorites(key_and_title[1], key_and_title[0], key_and_title[2], key_and_title[3]);
			}
							 }
			break;
			
		case IDM_FAV_EDIT: {
			bool is_already_there = !stricmp(currentitem, "HKEY_CURRENT_USER\\"REFAVPATH);
			ShowItemByKeyName(TreeW, "HKEY_CURRENT_USER\\"REFAVPATH);
			LastFocusedW=ListW;
			SetFocus(LastFocusedW);
			if (!is_already_there && ListView_GetItemCount(ListW) > 0)  //And select first item (if any)
				ListView_SetItemState(ListW, 0, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
						   }
			break;
			
		case 340: { // 刷新 F5
			HTREEITEM hfc = TreeView_GetChild(TreeW, 0);
			for(n = 0; hfc; n++) 
			{
				char *kname = GetKeyNameByItem(TreeW, hfc);
				hk = GetKeyByName(kname, KEY_ENUMERATE_SUB_KEYS);
				TVITEM tvi;
				tvi.mask=TVIF_HANDLE | TVIF_STATE, tvi.hItem=hfc, tvi.stateMask = TVIS_EXPANDED;
				if (!TreeView_GetItem(TreeW, &tvi)) {ErrMsgDlgBox("RefreshSubtree_normal"); return 1;}
				bool is_exp = !!(tvi.state & TVIS_EXPANDED);

				if ((HANDLE)hk != INVALID_HANDLE_VALUE && is_exp) //?!
				{
				//	RefreshSubtree_normal( hfc, hk, kname );

					// 刷新2次. 一次有时不能完全显示改变情况,和hive内存缓存有关吧
					RefreshSubtree_Total( hfc, hk, kname );
					RefreshSubtree_Total( hfc, hk, kname );
				}
				free(kname);
				RegCloseKey(hk);
				hfc = TreeView_GetNextSibling(TreeW, hfc);
			}
		//	if (hfc || n < 7) MessageBox(0, "????", "340", MB_OK);
				  }
			break;
			
		case 320:
			if (!ListView_GetSelectedCount(ListW)) break;
			i=ListView_GetNextItem(ListW,-1,/*LVNI_BELOW |*/ LVNI_FOCUSED);
			goto e201dbl;
			break;
			
		case 321: // Delete value(s)
			k = ListView_GetSelectedCount( ListW );
			if ( currentitem && k ) 
			{
				NMLVKEYDOWN nmdel;
				nmdel.hdr.code		= LVN_KEYDOWN	;
				nmdel.hdr.idFrom	= 201			;
				nmdel.hdr.hwndFrom	= ListW			;
				nmdel.wVKey			= VK_DELETE		;
				nmdel.flags			= 0				;

				SendMessage( hwnd, WM_NOTIFY, 201, (LPARAM)&nmdel );
			}
			break;

		case 322: // Rename value
			k=ListView_GetNextItem(ListW,-1,LVNI_FOCUSED);
			if (k>=0) ListView_EditLabel(ListW,k);
			break;
			
		case 323://move value(s)
		case 324://copy value(s)
			ValuesAskMoveOrCopy(hwnd, (int)(LOWORD (wParam) == 323) | 2, 0);
			break;
			
		case 329: if (currentitem) TreeView_Expand(TreeW,currentitem_tv,TVE_COLLAPSE | TVE_COLLAPSERESET);
			break;
		case 330: if (currentitem) {
			tvins.item.mask=TVIF_CHILDREN | TVIF_STATE;
			tvins.item.hItem=currentitem_tv; tvins.item.cChildren=1;
			tvins.item.state=0, tvins.item.stateMask=TVIS_EXPANDEDONCE;
			TreeView_SetItem(TreeW,&tvins.item);
			TreeView_Expand(TreeW,currentitem_tv,TVE_EXPAND);
				  }
			break;
		case 332: // 删除子键
			if (currentitem) {
				NMTVKEYDOWN nmdel;
				nmdel.hdr.code=TVN_KEYDOWN, nmdel.hdr.idFrom=200, nmdel.hdr.hwndFrom=TreeW;
				nmdel.wVKey=VK_DELETE, nmdel.flags=0;
				SendMessage(hwnd,WM_NOTIFY,200,(LPARAM)&nmdel);
			}
			break;
		case 333://rename key
			TreeView_EnsureVisible(TreeW,currentitem_tv);
			TreeView_EditLabel(TreeW,currentitem_tv);
			TreeView_Select(TreeW,currentitem_tv,TVGN_CARET);
			break;
			
		case 334://move key
		case 335://copy key
			KeyAskMoveOrCopy(hwnd, LOWORD (wParam) == 334, 0);
			break;
			
		case 336://copy key name
		case 337://copy key name (short)
			if (*currentitem) {
				if (!OpenClipboard(hwnd)) break;
				bool full = LOWORD (wParam) == 336;
				const char *fknr, *fknt;
				int len = strlen(currentitem) + 1;
				if (full) {
					fknr = rkeys.ShortName2LongName(currentitem, &fknt);
					if (fknr) len = strlen(fknr) + strlen(fknt) + 1 + (*fknt != 0);
					else full = false;
				}
				HGLOBAL g = GlobalAlloc(GMEM_MOVEABLE, len);
				if (g) {
					char *c = (char*)GlobalLock(g);
					if (c) {
						if (full) 
							if (*fknt) sprintf(c, "%s\\%s", fknr, fknt);
							else strcpy(c, fknr);
							else strcpy(c, currentitem);
							GlobalUnlock(g);
							EmptyClipboard();
							SetClipboardData(CF_TEXT, g);
					}
				}
				CloseClipboard(); 
			}
			break;
			
		case 300: // 给父键创建一个子键. 此时创建的一定是个临时的键,因为创建后会有个重命名操作
				  // 所以这里就用正常的方式创建,重命名的时候再调用Hive方式来增加
			if ( !currentitem ) break;
 		// 	AddSubKey_hive( tvins, currentitem ); // 暂时屏蔽掉用hive方式增加键值的方式

			if (!currentitem) break;
			hk=GetKeyByName(currentitem,KEY_CREATE_SUB_KEY);
			if((HANDLE)hk==INVALID_HANDLE_VALUE) {
				MessageBox(hwnd,"Can't create subkeys for this key",currentitem,0);
				break;
			}

			tvins.item.mask=TVIF_CHILDREN | TVIF_STATE;
			tvins.item.hItem=currentitem_tv; tvins.item.cChildren=1;
			tvins.item.state=0, tvins.item.stateMask=TVIS_EXPANDEDONCE;
			TreeView_SetItem( TreeW, &tvins.item );
			TreeView_Expand( TreeW, currentitem_tv, TVE_EXPAND );
			n=0;

			do {
				HKEY hk1;
				sprintf(ss,"New key #%03i",n++);
				if (RegCreateKeyEx(hk,ss,NULL,"regedt",REG_OPTION_NON_VOLATILE,
					KEY_QUERY_VALUE,NULL,&hk1,(DWORD*)&k)!=ERROR_SUCCESS) k=-1;
				else RegCloseKey(hk1);

			} while(k==REG_OPENED_EXISTING_KEY);

			if (k==-1) {RegCloseKey(hk); break;}

			s=(char*)malloc(strlen(currentitem)+strlen(ss)+5);
			sprintf(s,"%s\\%s",currentitem,ss);
			free(currentitem); currentitem=s;
			//SendMessage(hwnd,WM_COMMAND,340,1);
			RegCloseKey(hk);

			tvins.hParent=currentitem_tv, tvins.hInsertAfter=TVI_SORT;
			tvins.item.mask=TVIF_CHILDREN | TVIF_STATE | TVIF_TEXT, tvins.item.state=0; 
			tvins.item.stateMask=0xFFFF, tvins.item.cChildren=1;
			tvins.item.pszText=ss, tvins.item.cChildren=0;

			currentitem_tv = TreeView_InsertItem(TreeW,&tvins);

			TreeView_EnsureVisible(TreeW,currentitem_tv);
			TreeView_EditLabel(TreeW,currentitem_tv);
			TreeView_Select(TreeW,currentitem_tv,TVGN_CARET);

			break;
			
		case 301:case 302:case 303:case 304:case 305:case 306:case 307: // Create new value
		case 308:case 309:case 310:case 311:case 312:
			if (!currentitem) break;
			hk=GetKeyByName(currentitem,KEY_SET_VALUE | KEY_QUERY_VALUE);
			if((HANDLE)hk==INVALID_HANDLE_VALUE) {
				MessageBox(hwnd,"Can't create values for this key",currentitem,0);
				break;
			}
			n=0;
			do {
				sprintf(ss,"New Value #%03i",n++);
			} while(RegQueryValueEx(hk,ss,NULL,NULL,NULL,NULL)==ERROR_SUCCESS);
			n--;
			k = GetRegValueType(LOWORD(wParam)-300,12);
			SetFocus( ListW );

			//
			// 用hive方式创建新Value
			//
			if ( ReturnType_OK != CreateValue_hive( currentitem, ss, "\0\0\0", k ) ) 
			{ 
				// 用普通方式创建新Value
				RegSetValueEx(hk,ss,NULL,k,(BYTE*)"\0\0\0",GetMinRegValueSize(k));
				RegCloseKey(hk);
			}
			 
			// 创建新Value后,重新列举一下
			ListValues_hive( hwnd,currentitem );
			{
				LVFINDINFO finf;
				finf.flags=LVFI_STRING, finf.psz=ss;
				k=ListView_FindItem(ListW,-1,&finf);
				if (k>=0) ListView_EditLabel(ListW,k);
			}
			break;
			
		case 351:case 352:case 353:case 354:case 355:case 356:case 357:
		case 358:case 359:case 360:case 361:case 362:
			if (currentitem && lParam) { // Change value type: lParam=name
				char *vn,*name=(char*)lParam,*vd,*vdp;
				DWORD vknl,vkdl,type;
				LVFINDINFO finf;
				LVITEM item;
				hk=GetKeyByName(currentitem,KEY_QUERY_VALUE | KEY_SET_VALUE);
				if ((HANDLE)hk==INVALID_HANDLE_VALUE) return 0;
				if (RegQueryInfoKey(hk,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
					&vknl,&vkdl,NULL,NULL)!=ERROR_SUCCESS) return 0;
				vn=(char*)malloc(++vknl+20); vd=(char*)malloc(++vkdl+1);
				if (RegQueryValueEx(hk,name,NULL,&type,(BYTE*)vd,&vkdl)!=ERROR_SUCCESS ||
					RegSetValueEx(hk,name,NULL,
					type=GetRegValueType(LOWORD(wParam)-350,type),
					(BYTE*)vd,vkdl)!=ERROR_SUCCESS) {
					RegCloseKey(hk);
					free(vn); free(vd);
					SetFocus(ListW);
					return 0;
				}
				SetFocus(ListW);
				RegCloseKey(hk);
				finf.flags=LVFI_STRING, finf.psz=name;
				n=ListView_FindItem(ListW,-1,&finf);
				vdp=(char*)malloc(max((int)vkdl*3,32));
				if (n>=0) {
					item.iItem=n, item.mask=LVIF_IMAGE, item.iSubItem=0;
					item.stateMask=0, item.iImage=ValueTypeIcon(type);
					ListView_SetItem(ListW,&item);
					GetValueDataString(vd,vdp,vkdl,type);
					item.mask=LVIF_TEXT, item.iSubItem=1;
					item.pszText=vdp;
					ListView_SetItem(ListW,&item);
				}
				free(vn); free(vd); free(vdp);
			}
			break;
	}
	break;
	
  case WM_CREATE:
	  //MyFnt=CreateFont(16,8,0,0,FW_SEMIBOLD,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH | FF_DONTCARE, "Arial Cyr");
	  //MyFnS=CreateFont(16,8,0,0,FW_LIGHT,FALSE,FALSE,TRUE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH | FF_DONTCARE, "Arial Cyr");
	  //DlgThinFnt=CreateFont(15,5,0,0,FW_LIGHT,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH | FF_DONTCARE, "Arial Cyr");
	  //Sans8=CreateFont(16,7,0,0,FW_LIGHT,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH | FF_DONTCARE, "MS Sans Serif");
	  
	  SbarW=CreateStatusWindow(WS_CHILD | WS_VISIBLE | WS_BORDER | SBARS_SIZEGRIP, "Root",hwnd,1001);
	  hwndToolTip = CreateWindowEx( 0, TOOLTIPS_CLASS, NULL, WS_POPUP | TTS_ALWAYSTIP,
		  CW_USEDEFAULT, CW_USEDEFAULT, 20, 20, hwnd, NULL, hInst, NULL);
	  if (!hwndToolTip) {
		  MessageBeep(MB_OK);
	  }
	  else SendMessage(hwndToolTip, TTM_SETMAXTIPWIDTH, 0, 300);
	  {
		  RECT SbarRect;
		  SendMessage(SbarW,SB_GETRECT,0,(LPARAM)&SbarRect);
		  SbarHeight=SbarRect.bottom-SbarRect.top;
	  }
	  TreeW = CreateWindowEx (  // 创建树形控件
		  WS_EX_CLIENTEDGE, WC_TREEVIEW, "none", 
		  WS_CHILD | WS_VISIBLE | TVS_HASLINES | TVS_HASBUTTONS | TVS_LINESATROOT |
		  TVS_EDITLABELS | TVS_SHOWSELALWAYS | WS_TABSTOP,
		  0, 0, xTree, dyw - SbarHeight, hwnd, (HMENU)200, hInst, NULL );

	  if (!TreeW) ErrMsgDlgBox("TreeView");
	  LastFocusedW=TreeW;

	  ListW = CreateWindowEx ( // 创建列表
		  WS_EX_CLIENTEDGE, WC_LISTVIEW, "none", 
		  WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SHAREIMAGELISTS | WS_TABSTOP |
		  LVS_EDITLABELS | LVS_SORTASCENDING,
		  xTree + 3, 0, dxw - xTree - 3, dyw - SbarHeight, hwnd, (HMENU)201, hInst, NULL );

	  if (!ListW) ErrMsgDlgBox( "SysListView32" );

	  ListView_SetImageList( ListW, imt, LVSIL_SMALL );

	  // 树形图标初始化
// 	  m_hilTree = ImageList_LoadBitmap( hInst, MAKEINTRESOURCE(IDI_ICON_A), 16, 0, RGB(255,0,255) );

// 	  hImage = ::LoadBitmap( hInst, MAKEINTRESOURCE(IDI_ICON_A) ); 
// 	  m_hilTree = ImageList_Create(16, 15, ILC_MASK, 9, 1); 
// 	  ImageList_Add( m_hilTree, hImage, NULL); 

	  m_hilTree = ImageList_Create( 18, 18, ILC_MASK | ILC_COLOR32, 9, 1 ); 
	  hicon = LoadIcon( hInst, MAKEINTRESOURCE(IDI_ICON_FOLDER_CLOSE) );
	  ImageList_AddIcon( m_hilTree, hicon );
	  TreeView_SetImageList ( TreeW, m_hilTree, TVSIL_NORMAL);

	  // TreeView Setup
	  tvins.hParent=TVI_ROOT, tvins.hInsertAfter=TVI_LAST;
	  tvins.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_CHILDREN | TVIF_STATE | TVIF_TEXT;
	  tvins.item.iImage = 0 ;
	  tvins.item.iSelectedImage = 5 ;
	  tvins.item.state=TVIS_BOLD;//|TVIS_EXPANDED ; 
	  tvins.item.stateMask=0xFFFF, tvins.item.cChildren=1;

	  tvins.item.pszText="HKEY_CLASSES_ROOT";
	  HKCR=TreeView_InsertItem(TreeW,&tvins);

	  tvins.item.pszText="HKEY_CURRENT_USER";
	  HKCU=TreeView_InsertItem(TreeW,&tvins);
	  tvins.item.pszText="HKEY_LOCAL_MACHINE";
	  HKLM=TreeView_InsertItem(TreeW,&tvins);
	  tvins.item.pszText="HKEY_USERS";
	  HKUS=TreeView_InsertItem(TreeW,&tvins);
	  tvins.item.pszText="HKEY_CURRENT_CONFIG";
 	  HKCC=TreeView_InsertItem(TreeW,&tvins);

// 	  tvins.item.pszText="HKDD";//Win95
// 	  HKCC=TreeView_InsertItem(TreeW,&tvins);
// 	  tvins.item.pszText="HKPD";//WinNT
// 	  HKCC=TreeView_InsertItem(TreeW,&tvins);

	  // ListView Setup
	  lvcol.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH, lvcol.fmt=LVCFMT_LEFT;

	  lvcol.iSubItem=0, lvcol.pszText="Name", lvcol.cx=xName;
	  if (ListView_InsertColumn(ListW,0,&lvcol)==-1) ErrMsgDlgBox("ListView");

// 	  lvcol.iSubItem=1, lvcol.pszText="Type", lvcol.cx=xType;
// 	  if (ListView_InsertColumn(ListW,1,&lvcol)==-1) ErrMsgDlgBox("ListView");

	  lvcol.iSubItem=2, lvcol.pszText="Data", lvcol.cx=xData;
	  if (ListView_InsertColumn(ListW,2,&lvcol)==-1) ErrMsgDlgBox("ListView");

	  SetFocus(TreeW);
	  break;
	  
  case WM_CLOSE:
	  //SaveSettings();
	  if (RplProgrDlg) { SetFocus(RplProgrDlg); break; }
	  DestroyWindow(hwnd);
	  break;
	  
  case WM_DESTROY:
	  //DeleteObject(MyFnt); DeleteObject(MyFnS);
	  //DeleteObject(DlgThinFnt); //DeleteObject(Sans8);
	  PostQuitMessage (0);
	  break;
	  
  case WM_INITMENUPOPUP:
	  if ((HMENU)wParam == theFavMenu) {
		  AddFavoritesToMenu((HMENU)wParam);
		  return 0;
	  }
	  break;
	  
  case WM_NOTIFY:
	  k=((LPNMHDR)lParam)->idFrom;
	  switch (((LPNMHDR)lParam)->code) {
		  unsigned short wVKey;
	  case TTN_NEEDTEXT:
		  break;
		  
	  case TVN_SELCHANGING:
		  if (is_dragging) ValuesEndDrag(hwnd, false);
		  return 0; // 都返回0,表示永远放行.
/*
举个例子:
当点击Tree控件时，EditView中会显示一些数据，供用户浏览和更改。如果用户只是浏览，
那么当用户点击Tree中其他Item时，系统不弹出提示框；如果用户更改了Edit中的内容，
并且在没有保存的情况下试图选择Tree中的另外一个Item，则系统会弹出YESNOCANCEL消息框，
提示是否保存这个内容。如果选择CANCEL，则应该终止用户刚才进行的选择，并使原来选择的
那个Item保持高亮显示。Obviously，这需要对TVN_SELCHANGING消息进行响应。
   Returns TRUE to prevent the selection from changing. 
*/
		  break;

	  case TVN_SELCHANGED:
		  if ( k==200 ) // 是树形控件在接收消息,即用户点击了一个子键,要
		  {				// 在列表控件中显示该子键相应的值
			  // 得到当前的位置(eg. HKEY_LOCAL_MACHINE\SOFTWARE\360Safe)
			  // 该函数负责申请内存存放全路径,指针s指向该内存
			  s = GetKeyNameByItem( TreeW, currentitem_tv=((LPNMTREEVIEW)lParam)->itemNew.hItem );
			  if (currentitem) { free( currentitem ); }
			  currentitem = s ;
			  SendMessage( SbarW, SB_SETTEXT, 0, (LPARAM)s ); // 改变状态栏的信息
			  ListValues_hive( hwnd, s ); // 列举当前键下的所有键值
		  }
		  break;
		  
	  case TVN_ITEMEXPANDING: // 展开某一键的所有子键
		  if ( 
			  k==200 && ((LPNMTREEVIEW)lParam)->action==TVE_EXPAND 
			  && 0 == (((LPNMTREEVIEW)lParam)->itemNew.state & TVIS_EXPANDED)
			 )
		  {
			  // 若树形控件接收到展开命令,且该子键确实没有展开
			  s = GetKeyNameByItem( TreeW, ((LPNMTREEVIEW)lParam)->itemNew.hItem );
			  int rtnKeyType = ListSubKeys_hive ( 
				  ((LPNMTREEVIEW)lParam)->itemNew.hItem,
				  tvins,
				  s );

			  if ( ReturnType_IsRootKey != rtnKeyType ) { return 0; }

			  //
			  // 是虚拟键,需要调用常规API显示出来即可
			  //
			  char s[512],ss[512],sss[512];
			  DWORD d=512;
			  FILETIME lwt;
			  HKEY hks;
			  HTREEITEM hdel=TreeView_GetChild(TreeW,((LPNMTREEVIEW)lParam)->itemNew.hItem);
			  sss[0]=0;
			  if (hdel)
			  {
				  tvins.item.mask=TVIF_TEXT, tvins.item.pszText=sss, tvins.item.cchTextMax=512;
				  tvins.item.hItem=hdel;
				  TreeView_GetItem(TreeW,&tvins.item);
			  }

			  tvins.hParent=((LPNMTREEVIEW)lParam)->itemNew.hItem;
			  tvins.hInsertAfter=TVI_LAST;
			  tvins.item.mask=TVIF_CHILDREN | TVIF_STATE | TVIF_TEXT;
			  tvins.item.state=0; 
			  tvins.item.stateMask=0xFFFF;
			  tvins.item.cChildren=1;

			  hk=GetKeyByItem(TreeW,tvins.hParent,KEY_ENUMERATE_SUB_KEYS);
			  tvins.item.pszText=s;
			  n=0;
			  while( RegEnumKeyEx(hk,n++,s,&d,NULL,NULL,NULL,&lwt)==ERROR_SUCCESS )
			  {
				  RegOpenKey(hk,s,&hks);
				  tvins.item.cChildren=(RegEnumKey(hks,0,ss,512)==ERROR_SUCCESS);
				  CloseKey_NHC(hks);
				  if (strcmp(sss,s)) TreeView_InsertItem(TreeW,&tvins);
				  d=512;
			  }
			  CloseKey_NHC(hk);
			  TreeView_SortChildren(TreeW,((LPNMTREEVIEW)lParam)->itemNew.hItem,0);
			  //Beep(600,10);

			  return 0;
		  }
		  break;
		  
	  case TVN_ITEMEXPANDED:
		  if( k==200 && ((LPNMTREEVIEW)lParam)->action==TVE_COLLAPSE ) 
		  {
			  TreeView_Expand (
				  TreeW,
				  ((LPNMTREEVIEW)lParam)->itemNew.hItem,
				  TVE_COLLAPSE | TVE_COLLAPSERESET );
			  // The TreeView_Expand macro causes the TVN_ITEMEXPANDING and TVN_ITEMEXPANDED
			  // messages to be generated if the item being expanded does not have the
			  // TVIS_EXPANDEDONCE state bit set. Using TVE_COLLAPSE and TVE_COLLAPSERESET 
			  // with TreeView_Expand will cause the TVIS_EXPANDEDONCE state to be reset. 
			  // 也就是没展开/关闭一次子键,都要重置一下 TVIS_EXPANDEDONCE 这个标志位
			  // 
		  }
		  break;
		  
	  case TVN_BEGINLABELEDIT: // 表明用户要更改子键的名字.若是根键,则不可更改
		  if ( k == 200 ) 
		  {
			  // 0 allows 1 prevents
			  if( !CanKeyBeRenamed( TreeW, ((LPNMTVDISPINFO)lParam)->item.hItem ) ) { 
				  return 1 ;
			  }

			  return 0;
		  }
		  break;

	  case TVN_ENDLABELEDIT: // 更改子键的名字. 不提供Hive方式增加子键
		  if ( k==200 ) 
		  {
			  fchar s, d; char *c;
			  int n;

			  // 合法性检查
			  if ( ((LPNMTVDISPINFO)lParam)->item.pszText == NULL ) return 0;
			  if (!CanKeyBeRenamed(TreeW,((LPNMTVDISPINFO)lParam)->item.hItem)) return 0;//Cancel

			  // s指针指向的是旧值, 即重命名前的该键全路径
			  s = GetKeyNameByItem(TreeW,((LPNMTVDISPINFO)lParam)->item.hItem);
			  d = (char*)malloc(strlen(s)+strlen(((LPNMTVDISPINFO)lParam)->item.pszText)+10);
			  strcpy( d, s ); 
			  c = strrchr( d, '\\' );
			  if ( !c ) { 
				  return 0 ; //Must never happen!
			  }

			  // c此时指向的是新命名的键的名字,非全路径
			  strcpy( c+1, ((LPNMTVDISPINFO)lParam)->item.pszText ); 		  
			  
			  extern bool flag_merge_chosen; flag_merge_chosen = false;
			  if ( !IsSubKey( s, d ) || !IsSubKey( d, s ) ) 
			  {	
				  n = MoveKey( s, d ); // 这里负责重命名.好复杂.如用Hive来做,应该比较简单~

			  } else {
				  n = 0 ;
			  }

			  if ( n == 1 && !strcmp( s, currentitem ) ) {
				  free( currentitem );
				  currentitem = d.c; 
				  d.c = 0;
			  }

			  if ( n == 1 && (strchr(c + 1, '\\') || flag_merge_chosen) ) {
				  SendMessage( hwnd, WM_COMMAND, 340, 0 ); // 刷新之
			  }

			  return n;
		  }
		  break;
		  
	  case LVN_BEGINLABELEDIT: // 将要更改value(s)
		  if (k==201) 
		  {		// 0 放行; 1 拒绝 
			  hk=GetKeyByName(currentitem,KEY_QUERY_VALUE | KEY_SET_VALUE);
			  if ((HANDLE)hk==INVALID_HANDLE_VALUE) return 1;
			  RegCloseKey(hk);
			  return 0;
		  }
		  break;

	  case LVN_ENDLABELEDIT: // 更改value(s)在图形界面上已经结束.到这里进行统一管理
		  if ( k == 201 )
		  {
			  // 更改子键Value的名字
			  NMLVDISPINFO *di=(NMLVDISPINFO*)lParam;
			  if (!di->item.pszText) return 0;
			  hk=GetKeyByName(currentitem,KEY_QUERY_VALUE | KEY_SET_VALUE);
			  if ((HANDLE)hk==INVALID_HANDLE_VALUE) return 0;

			  fchar oldname(malloc(2048)); *oldname=0;
			  ListView_GetItemText(ListW,di->item.iItem,0,oldname.c,2048);//!
			  char *newname = di->item.pszText;

			  //
			  // hive方式更改Value
			  // 
			  if ( ReturnType_OK == ModifyValueName_hive( currentitem, newname, oldname ) ) { return 1 ; }

			  //
			  // 普通方式更改Value
			  //
			  int rv = RenameKeyValue(hk, hk, newname, oldname);
			  RegCloseKey(hk);
			  return !rv;
		  }
		  break;
		  
	  case TVN_KEYDOWN: // 删除树形控件中指定的子键
		  wVKey = ((LPNMTVKEYDOWN)lParam)->wVKey;

		// #define VK_TAB 0x09
		  if (wVKey == 9) {
			  LastFocusedW=ListW;
			  SetFocus(LastFocusedW);
			  return 1;
		  }
		  if (wVKey == VK_F5) {
			  SendMessage(hwnd,WM_COMMAND,340,0);
			  return 1;
		  }
		  if (wVKey == VK_F3) {
			  DoSearchAndReplaceNext(hwnd);
			  return 1;
		  }
		  if ((wVKey == 'H' || wVKey == 'F') && (GetKeyState(VK_CONTROL) & 0x8000) != 0) {
			  DoSearchAndReplace(hwnd, false, wVKey == 'F');
			  return 1;
		  }
		  if (wVKey == 27 && is_dragging) ValuesEndDrag(hwnd, false);
		  if (wVKey == VK_DELETE && currentitem)
		  {
			  // Start Delete SubKey ...
			  if ( MessageBox(hwnd,"A you sure you want to delete this key?",
				  currentitem,MB_ICONQUESTION | MB_OKCANCEL) == IDOK
				 ) 
			  {
				  if ( !CanDeleteThisKey( currentitem, true ) ) return 1;

				  //
				  // 调用hive方式删除子键
				  //
				  if ( ReturnType_OK == DeleteSubKey_hive( currentitem ) ) return 1;
				 
				  //
				  // 若用hive方式删除失败,换用普通方式删~
				  //
				  hk=GetKeyByName(currentitem,KEY_QUERY_VALUE | KEY_SET_VALUE | 
					  KEY_ENUMERATE_SUB_KEYS | KEY_CREATE_SUB_KEY);
				  if ((HANDLE)hk==INVALID_HANDLE_VALUE) {
					  MessageBox(hwnd,currentitem,"Access denied:",MB_ICONWARNING);
					  return 1;
				  }

				  k=DeleteAllSubkeys(hk);
				  RegCloseKey(hk);
				  if (k!=1 || DeleteKeyByName(currentitem)!=ERROR_SUCCESS) {
					  MessageBox(hwnd,"Could not entirely delete this key",currentitem,MB_ICONWARNING);
					  SendMessage(hwnd,WM_COMMAND,340,0);
					  return 1;
				  }
				 
				  return 1;
			  }
		  }
		  break;

	  case LVN_ITEMCHANGING: 
		  if (is_dragging) return 1;
		  return 0;
		  
	  case LVN_KEYDOWN: // 删除该键的指定键值(values)
		  wVKey = ((LPNMLVKEYDOWN)lParam)->wVKey;
		  if (wVKey == 9) {
			  LastFocusedW=TreeW;
			  SetFocus(LastFocusedW);
			  return 1;
		  }
		  if (wVKey == VK_F3) {
			  DoSearchAndReplaceNext(hwnd);
			  return 1;
		  }
		  if ((wVKey == 'H' || wVKey == 'F') && (GetKeyState(VK_CONTROL) & 0x8000) != 0) {
			  DoSearchAndReplace(hwnd, false, wVKey == 'F');
			  return 1;
		  }
		  if (wVKey == 27 && is_dragging) ValuesEndDrag(hwnd, false);
		  if ( wVKey == VK_DELETE && currentitem ) 
		  {
			  k = ListView_GetSelectedCount( ListW );
			  if ( k && MessageBox(hwnd,"A you sure you want to delete these value(s)?",
				  currentitem,MB_ICONQUESTION | MB_OKCANCEL)==IDOK
				 ) 
			  {
				  //
				  // 删除该子键的指定键值 Hive方式
				  //
			//	  if ( ReturnType_OK == DeleteAllValue_hive( currentitem ) ) return 1;	
				  if ( ReturnType_OK == DeleteSelectedValue_hive( currentitem ) ) return 1;	

				  //
				  // 普通方式
				  //
				  DWORD ns,ds; char *s;
				  hk=GetKeyByName(currentitem,KEY_QUERY_VALUE | KEY_SET_VALUE);
				  if ((HANDLE)hk==INVALID_HANDLE_VALUE) {
					  MessageBox(hwnd,currentitem,"Access denied:",MB_ICONWARNING);
					  return 1;
				  }

				  if (RegQueryInfoKey(hk,NULL,NULL,NULL,NULL,NULL,NULL,NULL,&ns,&ds,NULL,NULL)!=
					  ERROR_SUCCESS) {
					  RegCloseKey(hk);
					  return 1;
				  }

				  if (!k) return 1;//???
				  s = (char*)malloc( ns +=32 );
				  n = -1 ;
				  while( ( n = ListView_GetNextItem( ListW, n, LVNI_SELECTED ) ) >= 0 ) 
				  {
					  *s=0;
					  ListView_GetItemText( ListW, n, 0, s, ns-1 );
					  if ( RegDeleteValue(hk,s)==ERROR_SUCCESS && ListView_DeleteItem(ListW,n) )
					  {
						  n--, k-- ;
					  }
				  }

				  free(s);
				  if (k!=0) {
					  MessageBox(hwnd,"Could not delete all specified values",currentitem,MB_ICONWARNING);
					  RegCloseKey(hk);
					  return 1;
				  }
				  //SendMessage(hwnd,WM_COMMAND,340,0);???
				  RegCloseKey(hk);
				  return 1;
			  }

			  return 1;
		  }
		  break;

	  case NM_RETURN:
		  if ( k!=201 ) break;
		  if (!ListView_GetSelectedCount(ListW)) break;
		  i=ListView_GetNextItem(ListW,-1,/*LVNI_BELOW |*/ LVNI_FOCUSED);
		  goto e201dbl;

	  case NM_DBLCLK: // 编辑修改Value的内容,这里要用到hive
		  if ( k==201 ) 
		  {
			  i=((LPNMLISTVIEW)lParam)->iItem;
e201dbl:
			  if ( i < 0 ) break;
			  DWORD ns=0;
			  val_ed_dialog_data dp;
			  dp.keyname = currentitem;
			  GetLVItemText(ListW, i, dp.name, ns);
			  if (dp.EditValue(hwnd)) {
				  break;
			  }
			  fchar vdp(malloc(max((int)dp.newdata.l * 3, 32)));
			  
			  LVITEM item;
			  item.iItem=i, item.stateMask=0;
			  item.mask=LVIF_TEXT, item.iSubItem=1;
			  item.pszText=vdp.c;
			  GetValueDataString(dp.newdata,vdp,dp.newdata.l,dp.type);
			  ListView_SetItem(ListW,&item);
		  }
		  break;
		  
	  case NM_RCLICK: // 用户单击鼠标右键,弹出菜单
		  if (k==200) 
		  {
			  // 是树形控件的右键菜单
			  TVHITTESTINFO hti;
			  int CanCreate=0, CanChange,cnc;
			  GetTWHit(&hti);
			  DWORD mp=GetMessagePos();
			  if (hti.flags & TVHT_ONITEM) {
				  s=GetKeyNameByItem(TreeW,currentitem_tv=hti.hItem);//Just for the case
				  if (currentitem) free(currentitem);
				  currentitem=s;
				  TreeView_SelectItem(TreeW,hti.hItem);
				  HKEY hk=GetKeyByName(currentitem,KEY_QUERY_VALUE | KEY_SET_VALUE);
				  if ((HANDLE)hk==INVALID_HANDLE_VALUE) {
					  hk=GetKeyByName(currentitem,KEY_QUERY_VALUE);
					  if ((HANDLE)hk==INVALID_HANDLE_VALUE) break; //Crash
					  CanChange=0;
				  } else CanChange=1;
				  pum=CreatePopupMenu();
				  cnc=CanChange?0:MF_GRAYED;
				  CanCreate=cnc;//!?
				  tvins.item.hItem=hti.hItem, tvins.item.mask=TVIF_CHILDREN | TVIF_STATE;
				  tvins.item.stateMask=TVIS_EXPANDED | TVIS_EXPANDEDONCE;
				  TreeView_GetItem(TreeW,&tvins.item);
				  MENUITEMINFO mii;
				  mii.cbSize=sizeof(mii); mii.fMask=MIIM_STATE | MIIM_TYPE | MIIM_ID;
				  mii.fType=MFT_STRING, mii.wID=330-((tvins.item.state&TVIS_EXPANDED)!=0);
				  mii.dwTypeData=(tvins.item.state&TVIS_EXPANDED)?"Collapse":"Expand";
				  mii.fState=MFS_DEFAULT | ((tvins.item.cChildren==0) ? MFS_DISABLED:0);
				  InsertMenuItem(pum,mii.wID,0,&mii);
				  mn1=TypeMenu(300,0,CanCreate);
				  AppendMenu(pum,MF_POPUP,(DWORD)mn1,"&New");
				  AppendMenu(pum,MF_STRING | cnc,331,"&Find...");
				  AppendMenu(pum,MF_SEPARATOR,-1,"");
				  AppendMenu(pum,MF_STRING | cnc,332,"&Delete");
				  AppendMenu(pum,MF_STRING | cnc,333,"&Rename");
// 				  AppendMenu(pum,MF_STRING | cnc,334,"&Move to...");
// 				  AppendMenu(pum,MF_STRING,335,"Cop&y to...");
				  AppendMenu(pum,MF_SEPARATOR,-1,"");
				  AppendMenu(pum,MF_STRING,336,"&Copy key name (full)");
				  AppendMenu(pum,MF_STRING,337,"Copy key name (&short)");
				  cnc=TrackPopupMenu(pum,TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RETURNCMD | TPM_RIGHTBUTTON,
					  LOWORD(mp),HIWORD(mp),0,hwnd,NULL);
				  DestroyMenu(pum);
				  RegCloseKey(hk);
				  if (cnc) SendMessage(hwnd,WM_COMMAND,cnc,0);
			  }
		  } else if ( k == 201 ) {

			  // 是列表控件的右键菜单	
			  LVHITTESTINFO hti;
			  GetLWHit( &hti );
			  DWORD mp=GetMessagePos(),type,type_n,ns;
			  int CanCreate=MF_GRAYED*0, CanChange,cnc;
			  char *name=NULL;
			  if (currentitem) 
			  {
				  HKEY hk=GetKeyByName(currentitem,KEY_QUERY_VALUE | KEY_SET_VALUE);
				  if ((HANDLE)hk==INVALID_HANDLE_VALUE) {
					  hk=GetKeyByName(currentitem,KEY_QUERY_VALUE);
					  if ((HANDLE)hk==INVALID_HANDLE_VALUE) break; //Crash
					  CanChange=0;
				  } else CanChange=1;
				  if (RegQueryInfoKey(hk,NULL,NULL,NULL,NULL,NULL,NULL,NULL,&ns,NULL,NULL,NULL)!=
					  ERROR_SUCCESS) {
					  RegCloseKey(hk);
					  break;
				  }
				  pum=CreatePopupMenu();
				  cnc=CanChange?0:MF_GRAYED;
				  CanCreate=cnc;//!?
				  if (hti.flags & LVHT_ONITEM) {
					  HMENU mn2;
					  name=(char*)malloc(ns+=30); *name=0;
					  ListView_GetItemText(ListW,hti.iItem,0,name,ns);
					  RegQueryValueEx(hk,name,NULL,&type,NULL,NULL);
					  type_n=GetTypeMnuNo(type);
					  mn2=TypeMenu(350,type_n,cnc);
					  MENUITEMINFO mii;
					  mii.cbSize=sizeof(mii); mii.fMask=MIIM_STATE | MIIM_TYPE | MIIM_ID;
					  mii.fType=MFT_STRING, mii.dwTypeData="&Modify";
					  mii.fState=MFS_DEFAULT, mii.wID=320;
					  InsertMenuItem(pum,320,0,&mii);
					  AppendMenu(pum,MF_POPUP,(DWORD)mn2,"&Change Type");
					  AppendMenu(pum,MF_SEPARATOR,-1,"");
					  AppendMenu(pum,MF_STRING | cnc,321,"&Delete");
					  AppendMenu(pum,MF_STRING | cnc,322,"&Rename");
// 					  AppendMenu(pum,MF_STRING | cnc,323,"Move &to...");
// 					  AppendMenu(pum,MF_STRING,324,"Cop&y to...");
					  AppendMenu(pum,MF_SEPARATOR,-1,"");
					  mn1=TypeMenu(300,0,CanCreate);
					  AppendMenu(pum,MF_POPUP,(DWORD)mn1,"&New");
				  } else {
					  mn1=TypeMenu(300,0,CanCreate);
					  AppendMenu(pum,MF_POPUP,(DWORD)mn1,"&New");
				  }
				  cnc=TrackPopupMenu(pum,TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RETURNCMD | TPM_RIGHTBUTTON,
					  LOWORD(mp),HIWORD(mp),0,hwnd,NULL);
				  DestroyMenu(pum);
				  RegCloseKey(hk);
				  if (cnc) SendMessage(hwnd,WM_COMMAND,cnc,(LPARAM)name);
				  if (name) free(name);
			  }
		  }
		  return 0;
		  break;
		  
	case NM_SETFOCUS:
		if ( k == 200 ) LastFocusedW = TreeW ;
		else if ( k == 201 ) LastFocusedW = ListW ;
		break;
	
		// 拖拽功能太没必要了,略去
//     case LVN_BEGINDRAG:
// 		void ValuesBeginDrag(HWND hwnd, LPARAM lParam);
// 		ValuesBeginDrag(hwnd, lParam);
// 		break;
// 		
//     case TVN_BEGINDRAG:
// 		void KeyBeginDrag(HWND hwnd, LPARAM lParam);
// 		KeyBeginDrag(hwnd, lParam);
// 		break;
		
	default: return 0;
    } // end of WM_NOTIFY
	break;
	
	// 拖拽功能太没必要了,略去
  case WM_LBUTTONDOWN: 
// 	  if (wParam==MK_LBUTTON && LOWORD(lParam)>=xTree && LOWORD(lParam)<xTree+3) 
// 	  {
// 		  onWpos=true,xWpos=LOWORD(lParam)-xTree;
// 		  SetCapture(hwnd);
// 	  }
	  break;
	  
  case WM_RBUTTONDOWN:
// 	  if (is_dragging) ValuesEndDrag(hwnd, false);
	  break;
	  
  case WM_LBUTTONUP:
// 	  if (onWpos) 
// 	  {
// 		  onWpos=false;
// 		  ReleaseCapture();
// 		  if (LOWORD(lParam)>5 && LOWORD(lParam)<dxw-5) 
// 		  {
// 			  xTree=LOWORD(lParam)+xWpos;
// 			  SetWindowPos(TreeW,HWND_TOP,0,0,min(dxw,xTree),dyw-SbarHeight,0);
// 			  SetWindowPos(ListW,HWND_TOP,xTree+3,0,dxw-xTree-3,dyw-SbarHeight,0);
// 		  }
// 	  }
// 	  if (is_dragging) ValuesEndDrag(hwnd, true);
	  break;
	  
  case WM_MOUSEMOVE:
	  {
		  MSG ttmsg;
		  ttmsg.hwnd = hwnd;
		  ttmsg.message = msg;
		  ttmsg.wParam = wParam;
		  ttmsg.lParam = lParam;
		  GetCursorPos(&ttmsg.pt);
		  ttmsg.time = GetMessageTime();
		  SendMessage(hwndToolTip, TTM_RELAYEVENT, 0, (LPARAM)&ttmsg);
	  }
	  if (onWpos && LOWORD(lParam)>5 && LOWORD(lParam)<dxw-5) {
		  int xtr1 = LOWORD(lParam)+xWpos;
		  if (xTree == xtr1) { Sleep(1); break; }
		  xTree = xtr1;
		  SetWindowPos(TreeW,HWND_TOP,0,0,min(dxw,xTree),dyw-SbarHeight,0);
		  SetWindowPos(ListW,HWND_TOP,xTree+3,0,dxw-xTree-3,dyw-SbarHeight,0);
	  }
// 	  if (is_dragging) {
// 		  void ValuesContinueDrag(HWND hwnd, LPARAM lParam);
// 		  ValuesContinueDrag(hwnd, lParam);
// 	  }
	  break;
	  
  case WM_SIZE:
	  SendMessage(SbarW,WM_SIZE,wParam,lParam);
	  dxw=LOWORD(lParam),dyw=HIWORD(lParam);
	  SetWindowPos(TreeW,HWND_TOP,0,0,min(dxw,xTree),dyw-SbarHeight,0);
	  SetWindowPos(ListW,HWND_TOP,xTree+3,0,dxw-xTree-3,dyw-SbarHeight,0);
	  //if (wParam==SIZE_MINIMIZED) ShowWindow(hwnd,SW_HIDE);
	  break;
	  
  case WM_SETFOCUS:
	  SetFocus(LastFocusedW);
	  break;
	  
//   case WM_CAPTURECHANGED:
// 	  if (onWpos) onWpos = false;
// 	  if (is_dragging) ValuesEndDrag(hwnd, false);
// 	  break;
	  
  default:
	  return DefWindowProc (hwnd,msg,wParam,lParam);
	  
  }
  return 0;
}

typedef hash_set<char*, hash<char*>, str_equal_to> mystrhash;

int 
RefreshSubtree_normal (
	IN HTREEITEM hti, 
	IN HKEY hk, 
	IN const char *kname
	) 
{
	char buf[4096];//tmp?
	int n;
	int knl = strlen(kname);
	TVITEM tvi, tvi1;
	HTREEITEM hfc = TreeView_GetChild(TreeW, hti), hfcold;
	mystrhash had_keys;
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
		if (RegOpenKeyEx(hk, buf, 0, KEY_ENUMERATE_SUB_KEYS | KEY_EXECUTE, &sk) == ERROR_SUCCESS) {
			DWORD has_subkeys;
			if (is_exp) 
				has_subkeys = RefreshSubtree_normal(hfc, sk, curname );
			else {
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
	for(n = 0;; n++) {
		sns = MAX_PATH + 1;
		LONG rv = RegEnumKeyEx(hk, n, sn, &sns, 0, 0,0, &ft);
		if (rv != 0) break;
		if (n == 0) {
			tvins.hParent = hti, tvins.hInsertAfter = TVI_SORT;
			tvins.item.mask =  TVIF_TEXT | TVIF_CHILDREN | TVIF_STATE;
			tvins.item.state=0, tvins.item.stateMask=TVIS_EXPANDEDONCE;
			tvins.item.pszText = sn;
		}
		if (had_keys.find(sn) == had_keys.end()) {
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
	mystrhash::iterator i = had_keys.begin();
	while(i != had_keys.end()) {
		char *c = (*i);
		i++;
		delete []c;
	}
	had_keys.clear();
	return count_subkeys;
}

const char* TypeCName(DWORD type) 
{
	switch (type) {
	case REG_SZ:return "REG_SZ";
	case REG_BINARY:return "REG_BINARY";
	case REG_DWORD:return "REG_DWORD";
	case REG_MULTI_SZ:return "REG_MULTI_SZ";
	case REG_DWORD_BIG_ENDIAN:return "REG_DWORD_BIG_ENDIAN";
	case REG_EXPAND_SZ:return "REG_EXPAND_SZ";
	case REG_RESOURCE_LIST:return "REG_RESOURCE_LIST";
	case REG_FULL_RESOURCE_DESCRIPTOR:return "REG_FULL_RESOURCE_DESCRIPTOR";
	case REG_RESOURCE_REQUIREMENTS_LIST:return "REG_RESOURCE_REQUIREMENTS_LIST";
	case REG_LINK:return "REG_LINK";
	case REG_NONE:return "REG_NONE";
	default:return "";
	}
}


BOOL CALLBACK ValTypeDlg (HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam) 
{
	static int edval,cval,lock;
	char s[64];
	int n;
	switch (msg) {
	case WM_INITDIALOG: 
		cval=edval=lParam;
		lock=1;
		sprintf(s,"%i",lParam);
		SetDlgItemText(hwnd,IDC_DECNUM,s);
		sprintf(s,"%X",lParam);
		SetDlgItemText(hwnd,IDC_HEXNUM,s);
		sprintf(s,"0x%08X (%i) \"%s\"",lParam,lParam,TypeCName(lParam));
		SetDlgItemText(hwnd,IDC_TYPEID,s);
		SendDlgItemMessage(hwnd,IDC_DECNUM,EM_SETLIMITTEXT,32,0);
		SendDlgItemMessage(hwnd,IDC_HEXNUM,EM_SETLIMITTEXT,32,0);
		lock=0;
		return 0;
	case WM_CLOSE: EndDialog(hwnd,edval); return 1;
	case WM_COMMAND:
		switch (LOWORD (wParam)) {
		case IDOK: 
			EndDialog(hwnd,cval);
			break;
		case IDCANCEL: EndDialog(hwnd,edval); break; 
			
		case IDC_DECNUM:
			if (HIWORD(wParam)==EN_UPDATE && !lock) {
				GetDlgItemText(hwnd,IDC_DECNUM,s,64);
				sscanf(s,"%i",&cval);
				sprintf(s,"%X",cval);
				lock=1;
				SetDlgItemText(hwnd,IDC_HEXNUM,s);
				sprintf(s,"0x%08X (%i) \"%s\"",cval,cval,TypeCName(cval));
				SetDlgItemText(hwnd,IDC_TYPEID,s);
				lock=0;
			}
			break;
		case IDC_HEXNUM:
			if (HIWORD(wParam)==EN_UPDATE && !lock) {
				GetDlgItemText(hwnd,IDC_HEXNUM,s,64);
				for(n=0,cval=0; isxdigit((unsigned char)s[n]); n++) 
					cval = (cval << 4) | (s[n] <= '9' ? s[n]-'0' : toupper(s[n]) - 'A' + 10);
				sprintf(s,"%i",cval);
				lock=1;
				SetDlgItemText(hwnd,IDC_DECNUM,s);
				sprintf(s,"0x%08X (%i) \"%s\"",cval,cval,TypeCName(cval));
				SetDlgItemText(hwnd,IDC_TYPEID,s);
				lock=0;
			}
			break;
		}
		return 1;
	}
	return 0;
}


void LoadSettings() 
{
	DWORD dw;
	int n;
	//TCHAR s[20];
	HKEY MainKey;
	
	Settings[0]=0x2C, Settings[1]=0, Settings[2]=1; 
	Settings[3]=Settings[4]=Settings[5]=Settings[6]=-1;
	Settings[7]=Settings[8]=10; Settings[9]=600; Settings[10]=400;
	Settings[11]=Settings[12]=150, Settings[13]=400, Settings[14]=1;
	RegCreateKeyEx(HKEY_CURRENT_USER,
		"Software\\Microsoft\\Windows\\CurrentVersion\\Applets\\Regedit",0,"RE",
		REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&MainKey,&dw);
	if (dw==REG_CREATED_NEW_KEY) return;
	dw=64;
	RegQueryValueEx(MainKey,"View",0,NULL,(LPBYTE)&Settings,&dw);
	dw=4;
	if (RegQueryValueEx(MainKey,"FindFlags",0,NULL,(LPBYTE)&n,&dw)!=ERROR_SUCCESS) n=0;
	
	sMatch=n&1, sKeys=(n>>1)&1, sVal=(n>>2)&1, sDat=(n>>3)&1;
	xw=Settings[7], yw=Settings[8], dxw=Settings[9]-xw, dyw=Settings[10]-yw;
	xTree=Settings[11], xName=Settings[12], xData=Settings[13];
	
	RegCloseKey(MainKey);
}


void GetLWHit(LVHITTESTINFO *pinfo) 
{
	DWORD dd=GetMessagePos();
	RECT r1;
	GetWindowRect(ListW,&r1);
	pinfo->pt.x=LOWORD(dd)-r1.left;pinfo->pt.y=HIWORD(dd)-r1.top-GetSystemMetrics(SM_CYEDGE);
	SendMessage(ListW,LVM_HITTEST,0,(LPARAM)pinfo);
}


void GetTWHit(TVHITTESTINFO *pinfo) 
{
	DWORD dd=GetMessagePos();
	RECT r1;
	GetWindowRect(TreeW,&r1);
	pinfo->pt.x=LOWORD(dd)-r1.left;pinfo->pt.y=HIWORD(dd)-r1.top-GetSystemMetrics(SM_CYEDGE);
	SendMessage(TreeW,TVM_HITTEST,0,(LPARAM)pinfo);
}


HMENU TypeMenu(DWORD p,int key,DWORD disable) 
{
	HMENU mn1=CreatePopupMenu();
	if (!key) {
		AppendMenu(mn1,MF_STRING,p+0,"&Key");
		AppendMenu(mn1,MF_SEPARATOR,-1,"");
	}
	AppendMenu(mn1,MF_STRING | disable | MF_CHECKED*(key==1),p+1,"&String Value");
	AppendMenu(mn1,MF_STRING | disable | MF_CHECKED*(key==2),p+2,"&Binary Value");
	AppendMenu(mn1,MF_STRING | disable | MF_CHECKED*(key==3),p+3,"&DWORD Value");
	AppendMenu(mn1,MF_STRING | disable | MF_CHECKED*(key==4),p+4,"&Multistring Value");
	AppendMenu(mn1,MF_STRING | disable | MF_CHECKED*(key==5),p+5,"Big Endian D&WORD Value");
	AppendMenu(mn1,MF_STRING | disable | MF_CHECKED*(key==6),p+6,"&Env. String Value");
	AppendMenu(mn1,MF_STRING | disable | MF_CHECKED*(key==7),p+7,"&Resource List Value");
	AppendMenu(mn1,MF_STRING | disable | MF_CHECKED*(key==8),p+8,"&Full Resource Descriptor Value");
	AppendMenu(mn1,MF_STRING | disable | MF_CHECKED*(key==9),p+9,"Res. Re&quirements List Value");
	AppendMenu(mn1,MF_STRING | disable | MF_CHECKED*(key==10),p+10,"Sym&Link Value");
	AppendMenu(mn1,MF_STRING | disable | MF_CHECKED*(key==11),p+11,"&Value with no type");
	AppendMenu(mn1,MF_STRING | disable | MF_CHECKED*(key==12),p+12,"&Custom value type...");
	return mn1;
}


int GetTypeMnuNo(DWORD type)
 {
	switch (type) {
	case REG_SZ:return 1;
	case REG_BINARY:return 2;
	case REG_DWORD:return 3;
	case REG_MULTI_SZ:return 4;
	case REG_DWORD_BIG_ENDIAN:return 5;
	case REG_EXPAND_SZ:return 6;
	case REG_RESOURCE_LIST:return 7;
	case REG_FULL_RESOURCE_DESCRIPTOR:return 8;
	case REG_RESOURCE_REQUIREMENTS_LIST:return 9;
	case REG_LINK:return 10;
	case REG_NONE:return 11;
	default:return 12;
	}
}


DWORD GetRegValueType(DWORD k,DWORD type) 
{
	switch(k) {
	case 1:return REG_SZ;
	case 2:return REG_BINARY;
	case 3:return REG_DWORD;
	case 4:return REG_MULTI_SZ;
	case 5:return REG_DWORD_BIG_ENDIAN;
	case 6:return REG_EXPAND_SZ;
	case 7:return REG_RESOURCE_LIST;
	case 8:return REG_FULL_RESOURCE_DESCRIPTOR;
	case 9:return REG_RESOURCE_REQUIREMENTS_LIST;
	case 10:return REG_LINK;
	case 11:return REG_NONE;
	default:return DialogBoxParam(hInst,"TYPENO",MainWindow,ValTypeDlg,(LPARAM)type);
	}
}


DWORD GetMinRegValueSize(DWORD v)
 {
	switch (v) {
	case REG_SZ:return 1;//!?
	case REG_BINARY:return 0;
	case REG_DWORD:return 4;
	case REG_MULTI_SZ:return 2;//!?
	case REG_DWORD_BIG_ENDIAN:return 4;
	case REG_EXPAND_SZ:return 1;
	case REG_RESOURCE_LIST:return 4;//!?
	case REG_FULL_RESOURCE_DESCRIPTOR:return 4;//!?
	case REG_RESOURCE_REQUIREMENTS_LIST:return 4;//!?
	case REG_LINK:return 1;//!?
	case REG_NONE:return 0;
	default:return 0;
	}
}


int ValueTypeIcon(DWORD type) 
{
	switch(type) {
	case REG_SZ:return 0;
	case REG_BINARY:case REG_DWORD:return 1;
	case REG_DWORD_BIG_ENDIAN:return 2;
	case REG_EXPAND_SZ:return 3;
	case REG_LINK:return 4;
	case REG_MULTI_SZ:return 5;
	case REG_NONE:default:return 6;
	case REG_FULL_RESOURCE_DESCRIPTOR:
	case REG_RESOURCE_REQUIREMENTS_LIST:
	case REG_RESOURCE_LIST:return 7;
	}
}


void GetLVItemText(HWND ListW, int i, char *&name, DWORD &ns)
{
	if (!ns) {
		ns = 128;
		name = (char*)realloc(name, ns);
	}
	LVITEM lvi;
	lvi.mask = LVIF_TEXT, lvi.iSubItem = 0;
	while(1) {
		lvi.pszText = name, lvi.cchTextMax = ns;
		DWORD rv = SendMessage(ListW, LVM_GETITEMTEXT, (WPARAM)i, (LPARAM)&lvi);
		if (rv < ns - 1) break;
		ns *= 2;
		name = (char*)realloc(name, ns);
	}
}


void GetValueDataString(char *val,char *valb,int m1,DWORD type) 
{
	int k,l; BYTE b; char *valpos;
	switch(type) {
	case REG_SZ:case REG_EXPAND_SZ:
		if (!val[m1-1]) m1--;
		valb[0]='\"';
		for(k=0;k<m1;k++) if (val[k]==0) valb[k+1]=9; else valb[k+1]=val[k];
		valb[k+1]='\"', valb[k+2]=0;
		break;
	case REG_MULTI_SZ:
		if (!val[m1-1]) m1--;
		l=0;
		valb[l++]='\"';
		for(k=0;k<m1;k++) if (val[k]==0) {
			if (k+1<m1) valb[l++]='\"',valb[l++]=9,valb[l++]='\"';
		} else valb[l++]=val[k];
		valb[l++]='\"',valb[l++]=0;
		break;
	case REG_DWORD_BIG_ENDIAN:
		b=val[0],val[0]=val[3],val[3]=b;
		b=val[1],val[1]=val[2],val[2]=b;//dirty
	case REG_DWORD:
		k=*(DWORD*)val;
		if (m1<4) k&=~(-1<<m1*8);
		sprintf((char*)valb,"0x%08X (%i)",k,k);
		break;
	default:
		if (!m1) strcpy((char*)valb,"(No bytes)");
		else {
			for(k=0,valpos=valb;k<m1-1;k++,valpos+=3) 
				sprintf((char*)valpos,"%02X ",(BYTE)val[k]);
			sprintf((char*)valpos,"%02X",(BYTE)val[k]);
		}
		break;
	}
}

//////////////////////////////////////////////////////////////////////////

void ValuesBeginDrag(HWND hwnd, LPARAM lParam) 
{
	NMLISTVIEW &lvnm = *(NMLISTVIEW*)lParam;
	dragitno = lvnm.iItem;
	dragiml = ImageList_Create(1, 1, ILC_COLOR, 0, 1);
	//try to assure that dragged items always look the same way - don't call only
	// ListView_CreateDragImage(ListW, dragitno, &lvnm.ptAction); for the first item
	int n = -1;
	int firstx = -1, firsty = -1;
	POINT pt; RECT rct;
	while((n=ListView_GetNextItem(ListW,n,LVNI_SELECTED)) >= 0) {
		HIMAGELIST il2 = ListView_CreateDragImage(ListW, n, &pt);
		if (!il2) { ErrMsgDlgBox("il2"); continue; }
		if (firsty == -1) firstx = pt.x, firsty = pt.y;
		HIMAGELIST ml = ImageList_Merge(dragiml, 0, il2, 0, 0, pt.y - firsty);
		ImageList_Destroy(dragiml); ImageList_Destroy(il2);
		dragiml = ml;
	}
	
	// DragMove didn't display the result of ImageList_Merge, so let's fix that
	HIMAGELIST l = dragiml;
	int x = -2, y = -2;
	ImageList_GetIconSize(l, &x, &y);
	HDC hdc = GetDC(hwnd); 
	HDC dc1 = CreateCompatibleDC(hdc);
	HBITMAP bm1 = CreateCompatibleBitmap(hdc, x, y);
	if (!SelectObject(dc1, bm1)) ErrMsgDlgBox("SelectObject");
	ImageList_DrawEx(l, 0, dc1, 0, 0, 0, 0, CLR_NONE, CLR_NONE, ILD_IMAGE);
	//BitBlt(hdc, 100, 200, x, y, dc1, 0, 0, SRCCOPY); //for debugging
	DeleteDC(dc1);
	ImageList_RemoveAll(l);
	ImageList_AddMasked(l, bm1, 0);
	//ImageList_DrawEx(l, 1, hdc, 100, 260, 0, 0, CLR_NONE, CLR_NONE, ILD_IMAGE); //for debugging
	ReleaseDC(hwnd, hdc);
	DeleteObject(bm1);
	
	pt = lvnm.ptAction;
	if (!ImageList_BeginDrag(dragiml, 0, pt.x - firstx, pt.y - firsty)) ErrMsgDlgBox("ImageList_BeginDrag (dragiml)");/*error handling*/;
	ClientToScreen(ListW, &pt);
	GetWindowRect(hwnd, &rct);
	if (!ImageList_DragEnter(hwnd, pt.x - rct.left, pt.y - rct.top)) ErrMsgDlgBox("ImageList_DragEnter");
	is_dragging = true;
	SetCapture(hwnd); 
	SetCursor(curs_no); 
	prevdhlti = 0; prev_candrop = false, could_ever_drop = false;
	prevdhltibtm = 0;
	is_key_dragging = false;
	//EnableWindow(ListW, false); SetFocus(LastFocusedW = TreeW);
}

void KeyBeginDrag(HWND hwnd, LPARAM lParam)
{
	NMTREEVIEW &tvnm = *(NMTREEVIEW*)lParam;
	//dragiml = ImageList_Create(16, 16, ILC_COLOR, 0, 1);
	//ImageList_AddIcon(dragiml, regsmallicon);
	dragiml = ImageList_LoadBitmap(hInst, "BMPKEYDRAG", 16, 0, 0x00FFFFFF);
	
	char *s = GetKeyNameByItem(TreeW, currentitem_tv = tvnm.itemNew.hItem);
	if (currentitem) free(currentitem);
	currentitem = s;
	TreeView_SelectItem(TreeW, currentitem_tv);
	
	POINT pt = tvnm.ptDrag; RECT rct;
	TreeView_GetItemRect(TreeW, tvnm.itemNew.hItem, &rct, true);
	if (!ImageList_BeginDrag(dragiml, 0, /*pt.x - rct.left*/25, pt.y - rct.top)) ErrMsgDlgBox("ImageList_BeginDrag (dragiml)");/*error handling*/;
	ClientToScreen(TreeW, &pt);
	GetWindowRect(hwnd, &rct);
	if (!ImageList_DragEnter(hwnd, pt.x - rct.left, pt.y - rct.top)) ErrMsgDlgBox("ImageList_DragEnter");
	is_dragging = true;
	SetCapture(hwnd); 
	SetCursor(curs_no); 
	prevdhlti = 0; prev_candrop = false, could_ever_drop = false;
	prevdhltibtm = 0;
	is_key_dragging = true;
	//EnableWindow(ListW, false); SetFocus(LastFocusedW = TreeW);
}


void ValuesEndDrag(HWND hwnd, bool is_ok)
{
	is_dragging = false;
	ImageList_EndDrag();
	ImageList_DragLeave(hwnd);
	
	ReleaseCapture();
	//EnableWindow(ListW, true); SetFocus(LastFocusedW = ListW);
	ImageList_Destroy(dragiml);
	SetCursor(curs_arr);
	if (prevdhlti) TreeView_SetItemState(TreeW, prevdhlti, 0, TVIS_DROPHILITED/*mask*/);
	is_ok = is_ok && prev_candrop && prevdhlti;
	if (is_ok) {
		fchar s(GetKeyNameByItem(TreeW, prevdhlti));
		if (is_key_dragging) {
			char *l = strrchr(currentitem, '\\'), *m; int sl = strlen(s);
			l = l? l + 1 : currentitem;
			s.c = (char*)realloc(s.c, sl + strlen(l) + 2);
			*(m = s.c + sl) = '\\';
			strcpy(m + 1, l);
			KeyAskMoveOrCopy(hwnd, 4, s);
		} else ValuesAskMoveOrCopy(hwnd, 6, s);
	}
	prevdhlti = 0;
}

void ValuesContinueDrag(HWND hwnd, LPARAM lParam) 
{
	POINT pt = { LOWORD(lParam), HIWORD(lParam) }; RECT rct;
	ClientToScreen(hwnd, &pt);
	GetWindowRect(hwnd, &rct);
	TVHITTESTINFO tvht; memset(&tvht, 0, sizeof(tvht));
	tvht.pt = pt;
	ScreenToClient(TreeW, &tvht.pt);
	HTREEITEM hti = 0;
	if (pt.x > 30000) tvht.flags = TVHT_TOLEFT; // fix wrong sign expansion
	else if (pt.y > 30000) tvht.flags = TVHT_ABOVE; // the same
	else hti = TreeView_HitTest(TreeW, &tvht);
	bool candrop = hti
		&& (tvht.flags == TVHT_ONITEM || tvht.flags == TVHT_ONITEMBUTTON || tvht.flags == TVHT_ONITEMINDENT || tvht.flags == TVHT_ONITEMLABEL);
	if (!candrop) hti = 0;
	if (hti == currentitem_tv) candrop = false;
	if (candrop && !prev_candrop) {
		SetCursor(curs_arr);
		could_ever_drop = true;
	} else if (!candrop && prev_candrop) SetCursor(curs_no);
	prev_candrop = candrop;
	bool needrefresh = false;
	
	if (!candrop && could_ever_drop && (tvht.flags == TVHT_ABOVE || tvht.flags == TVHT_BELOW || tvht.flags == TVHT_TOLEFT || tvht.flags == TVHT_TORIGHT)) {
		bool rf = true;
		ImageList_DragShowNolock(false);
		switch(tvht.flags) {
		case TVHT_ABOVE: rf = SendMessage(TreeW, WM_VSCROLL, SB_LINEUP, 0) != 0; break;
		case TVHT_BELOW: rf = SendMessage(TreeW, WM_VSCROLL, SB_LINEDOWN, 0) != 0; break;
		case TVHT_TOLEFT: rf = SendMessage(TreeW, WM_HSCROLL, SB_LINELEFT, 0) != 0; break;
		case TVHT_TORIGHT: rf = SendMessage(TreeW, WM_HSCROLL, SB_LINERIGHT, 0) != 0; break;
		}
		if (rf) needrefresh = true;
		else ImageList_DragShowNolock(true);
	}
	if (prevdhlti != hti) {
		ImageList_DragShowNolock(false);
		needrefresh = true;
		if (prevdhlti) TreeView_SetItemState(TreeW, prevdhlti, 0, TVIS_DROPHILITED/*mask*/);
		if (hti) TreeView_SetItemState(TreeW, hti, TVIS_DROPHILITED, TVIS_DROPHILITED/*mask*/);
		prevdhlti = hti;
		prevdhltibtm = 0;
	} else if (hti && tvht.flags == TVHT_ONITEMBUTTON) {
		if (!prevdhltibtm) prevdhltibtm = time(0);
		else if (time(0) - prevdhltibtm > 0) {
			//expand item!
			TVITEM item;
			item.hItem = hti, item.mask = TVIF_STATE, item.stateMask = TVIS_EXPANDED | TVIS_EXPANDEDONCE;
			TreeView_GetItem(TreeW, &item);
			if (!(item.state & TVIS_EXPANDED)) {
				ImageList_DragShowNolock(false);
				needrefresh = true;
				item.mask = TVIF_CHILDREN | TVIF_STATE;
				item.cChildren = 1;
				item.state=0, item.stateMask = TVIS_EXPANDEDONCE;
				TreeView_SetItem(TreeW, &item);
				TreeView_Expand(TreeW, hti, TVE_EXPAND);
			}
		}
	}
	if (needrefresh) {
		MSG msg; //ops...
		while(PeekMessage(&msg,NULL,0,0,PM_REMOVE)) {
			if (msg.message == WM_QUIT) { 
				PostQuitMessage(msg.wParam);
				break; 
			}
			TranslateMessage (&msg);
			DispatchMessage(&msg);
		}
		ImageList_DragShowNolock(true);
	}
	
	if (!ImageList_DragMove(pt.x - rct.left, pt.y - rct.top)) 
		ErrMsgDlgBox("ImageList_DragMove");/*error handling*/;
}

void KeyAskMoveOrCopy(HWND hwnd, int mvflag, const char *dst) 
{
	TreeView_EnsureVisible(TreeW, currentitem_tv);
	free(mvcpkeyfrom); free(mvcpkeyto);
	mvcp_move = mvflag;
	mvcpkeyfrom = strdup(currentitem);
	mvcpkeyto = dst? strdup(dst) : 0;
	if (DialogBox(hInst,"MOVEKEY",hwnd,DialogMVCP)) {
		if (mvcp_move & 1) MoveKey(mvcpkeyfrom,mvcpkeyto);
		else CopyKey(mvcpkeyfrom,mvcpkeyto,"copy");
		SendMessage(hwnd,WM_COMMAND,340,0);
	}
}

void ValuesAskMoveOrCopy(HWND hwnd, int mvflag, const char *dst) 
{
	int k = ListView_GetSelectedCount(ListW);
	if (!(currentitem && k)) return;
	TreeView_EnsureVisible(TreeW,currentitem_tv);
	free(mvcpkeyfrom); free(mvcpkeyto);
	mvcp_move = mvflag;
	mvcpkeyfrom = strdup(currentitem);
	mvcpkeyto = dst? strdup(dst) : 0;
	if (DialogBox(hInst,"MOVEKEY",hwnd,DialogMVCP)) {
		if (!stricmp(mvcpkeyfrom, mvcpkeyto)) return;
		DWORD ns,ds; char *s;
		HKEY hk, hk2;
		hk = GetKeyByName(currentitem,KEY_QUERY_VALUE | KEY_SET_VALUE);
		if ((HANDLE)hk==INVALID_HANDLE_VALUE) {
			MessageBox(hwnd,currentitem,"Access denied:",MB_ICONWARNING); 
			return;
		}
		hk2= GetKeyByName(mvcpkeyto,KEY_QUERY_VALUE | KEY_SET_VALUE);
		if ((HANDLE)hk2==INVALID_HANDLE_VALUE) {
			MessageBox(hwnd,mvcpkeyto,"Access denied:",MB_ICONWARNING); 
			RegCloseKey(hk);
			return;
		}
		if (RegQueryInfoKey(hk,NULL,NULL,NULL,NULL,NULL,NULL,NULL,&ns,&ds,NULL,NULL)!=0) {
			RegCloseKey(hk); RegCloseKey(hk2);
			return;
		}
		if (!k) return;//???
		s = (char*)malloc(ns+=32);
		int n = -1;
		while((n=ListView_GetNextItem(ListW,n,LVNI_SELECTED))>=0) {
			*s=0;
			ListView_GetItemText(ListW,n,0,s,ns-1);
			if (RenameKeyValue(hk, hk2, s, s, !(mvcp_move & 1)))
				continue;
			if((mvcp_move & 1) && !ListView_DeleteItem(ListW,n))
				continue;
			n--,k--;
		}
		free(s);
		if (k!=0) {
			MessageBox(hwnd, mvcp_move & 1?
				"Could not move all specified values" : "Could not copy all specified values",
				currentitem,MB_ICONWARNING);
		}
		RegCloseKey(hk); RegCloseKey(hk2);
	}
}

BOOL CALLBACK DialogAbout (HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam) 
{
    switch (msg) {
	case WM_INITDIALOG: SetDlgItemText(hwnd,110,"http://hi.baidu.com/sudami"); return 1;
    case WM_CLOSE: EndDialog(hwnd,0); return 1;
    case WM_COMMAND:
		switch (LOWORD (wParam)) {
		case IDOK: EndDialog(hwnd,1);	break;
		case IDCANCEL: EndDialog(hwnd,0); break; }
		return 1;
	}
	return 0;
}

extern bool allow_delete_important_keys;
bool enable_regreplacekey_menu = false;

BOOL CALLBACK DialogSettings(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam) 
{
	switch (msg) {
	case WM_INITDIALOG: 
		SendDlgItemMessage(hwnd, IDC_C_ALLOWCRIT, BM_SETCHECK, allow_delete_important_keys, 0);
		SendDlgItemMessage(hwnd, IDC_C_ENRRKMENU, BM_SETCHECK, enable_regreplacekey_menu, 0);
		return 1;
	case WM_CLOSE: EndDialog(hwnd,0); return 1;
	case WM_COMMAND:
		switch (LOWORD (wParam)) {
		case IDOK: 
			allow_delete_important_keys = SendDlgItemMessage(hwnd, IDC_C_ALLOWCRIT, BM_GETCHECK, 0, 0) != 0;
			enable_regreplacekey_menu = SendDlgItemMessage(hwnd, IDC_C_ENRRKMENU, BM_GETCHECK, 0, 0) != 0;
			EnableMenuItem(theEditMenu, ID_EDIT_REPLACEKEY, enable_regreplacekey_menu? MF_ENABLED : MF_GRAYED);
			EndDialog(hwnd,1);
			break;
		case IDCANCEL: EndDialog(hwnd,0); break; }
		return 1;
	}
	return 0;
}


void 
FixKeyName (
	IN const char *k0, 
	OUT fchar &key
	)
{
	const char *d, *s = rkeys.LongName2ShortName( k0, &d );
	char *t;
	if (s) {
		t = (char*)malloc(strlen(s) + strlen(d) + 2);
		sprintf(t,"%s\\%s", s, d);
		free(key.c);
		key.c = t;
	}
}


void GetAndFixKeyName(HWND hwnd, int ctrl, fchar &key) 
{
	getDlgItemText(key, hwnd, IDC_E_KEYNAME);
	const char *d, *s = rkeys.LongName2ShortName(key, &d);
	char *t;
	if (s) {
		t = (char*)malloc(strlen(s) + strlen(d) + 2);
		if (*d) sprintf(t, "%s\\%s", s, d);
		else strcpy(t, s);
		free(key.c);
		key.c = t;
		SetDlgItemText(hwnd, IDC_E_KEYNAME, key);
	}
}


BOOL CALLBACK DialogGotoKey(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam) 
{
    switch (msg) {
    case WM_INITDIALOG: {
		fchar &key = *(fchar*)lParam;
		SetDlgEditCtrlHist(hwnd, IDC_E_KEYNAME);
		SetWindowLong(hwnd, DWL_USER, lParam);
		if (key.c) SetDlgItemText(hwnd, IDC_E_KEYNAME, key);
		return 1;
						}
    case WM_CLOSE: EndDialog(hwnd,0); return 1;
    case WM_COMMAND:
		switch (LOWORD (wParam)) {
		case IDOK: {
			fchar &key = *(fchar*)GetWindowLong(hwnd, DWL_USER);
			GetAndFixKeyName(hwnd, IDC_E_KEYNAME, key);
			EndDialog(hwnd,1);
			break;
				   }
		case IDCANCEL: EndDialog(hwnd,0); break; 
		}
		return 1;
	}
	return 0;
}


BOOL CALLBACK DialogAddFavKey(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam) 
{
    switch (msg) {
    case WM_INITDIALOG: {
		fchar &key = *(fchar*)lParam, &title = (&key)[1], &value = (&key)[2], &comment = (&key)[3];
		SetDlgEditCtrlHist(hwnd, IDC_E_KEYNAME);
		SetDlgEditCtrlHist(hwnd, IDC_VNAME);
		SetDlgEditCtrlHist(hwnd, IDC_E_COMMENT);
		SetWindowLong(hwnd, DWL_USER, lParam);
		if (key.c) SetDlgItemText(hwnd, IDC_E_KEYNAME, key);
		if (title.c) SetDlgItemText(hwnd, IDC_E_KEYTITLE, title);
		if (value.c) SetDlgItemText(hwnd, IDC_VNAME, value);
		return 1;
						}
    case WM_CLOSE: EndDialog(hwnd,0); return 1;
    case WM_COMMAND:
		switch (LOWORD (wParam)) {
		case IDOK: {
			fchar &key = *(fchar*)GetWindowLong(hwnd, DWL_USER), &title = (&key)[1], &value = (&key)[2], &comment = (&key)[3];
			GetAndFixKeyName(hwnd, IDC_E_KEYNAME, key);
			getDlgItemText(title, hwnd, IDC_E_KEYTITLE);
			if (SendDlgItemMessage(hwnd, IDC_C_USEVAL, BM_GETCHECK, 0, 0) != 0)
				getDlgItemText(value, hwnd, IDC_VNAME);
			else { free(value.c); value.c = 0; }
			if (SendDlgItemMessage(hwnd, IDC_C_USECOMM, BM_GETCHECK, 0, 0) != 0)
				getDlgItemText(comment, hwnd, IDC_E_COMMENT);
			else { free(comment.c); comment.c = 0; }
			EndDialog(hwnd,1);
			break;
				   }
		case IDCANCEL: EndDialog(hwnd,0); break; 
		case IDC_C_USEVAL:
			EnableWindow(GetDlgItem(hwnd, IDC_VNAME), SendDlgItemMessage(hwnd, IDC_C_USEVAL, BM_GETCHECK, 0, 0) != 0);
			break;
		case IDC_C_USECOMM:
			EnableWindow(GetDlgItem(hwnd, IDC_E_COMMENT), SendDlgItemMessage(hwnd, IDC_C_USECOMM, BM_GETCHECK, 0, 0) != 0);
			break;
		}
		return 1;
	}
	return 0;
}


const char *mvcp_title[4] = { "Copy key", "Move key", "Copy value(s)", "Move value(s)" };

BOOL CALLBACK DialogMVCP (HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam) 
{
    switch (msg) {
	case WM_INITDIALOG: 
		SetWindowText(hwnd, mvcp_title[mvcp_move & 3]);
		if (mvcpkeyfrom) SetDlgItemText(hwnd, IDC_KEYFROM, mvcpkeyfrom);
		if (mvcp_move & 2) SendDlgItemMessage(hwnd, IDC_KEYFROM, EM_SETREADONLY, 1, 0);
		if (mvcpkeyto) SetDlgItemText(hwnd, IDC_KEYTO, mvcpkeyto);
		if (mvcp_move & 4) ShowWindow(GetDlgItem(hwnd, IDC_C_MOVE), SW_SHOW);
		return 1;
    case WM_CLOSE: EndDialog(hwnd,0); return 1;
    case WM_COMMAND:
		switch (LOWORD (wParam)) {
		case IDOK: 
			getDlgItemText(mvcpkeyfrom, hwnd, IDC_KEYFROM);
			getDlgItemText(mvcpkeyto, hwnd, IDC_KEYTO);
			EndDialog(hwnd,1);
			break;
		case IDCANCEL: EndDialog(hwnd,0); RplProgrDlg = 0; break; 
		}
		case IDC_C_MOVE:
			mvcp_move = (mvcp_move & ~1) | (SendDlgItemMessage(hwnd, IDC_C_MOVE, BM_GETCHECK, 0,0) != 0);
			SetWindowText(hwnd, mvcp_title[mvcp_move & 3]);
			break;
			return 1;
	}
	return 0;
}


void getDlgItemText(char *&var, HWND hwnd, int ctrl) 
{
	if (var) free(var), var = 0;
	int len = SendDlgItemMessage(hwnd,ctrl,WM_GETTEXTLENGTH,0,0);
	if (len < 0) return;
	var = (char*)malloc(len + 2);
	GetDlgItemText(hwnd, ctrl, var, len + 1);
}


UINT achar::GetDlgItemText(HWND hwnd, int nIDDlgItem) //single-threaded!
{ 
	DWORD L = SendDlgItemMessage(hwnd, nIDDlgItem, WM_GETTEXTLENGTH,0,0);
	resize(L + 1); //?
	return l = ::GetDlgItemText(hwnd, nIDDlgItem, c, s);
}


LONG achar::QueryValue(HKEY hk, const char *name, DWORD &type) 
{
	l = s;
	long rv = RegQueryValueEx(hk, name, 0, &type, (BYTE*)c, &l);
	if ((rv || !c) && l >= s) {
		resize(); l = s;
		rv = RegQueryValueEx(hk, name, 0, &type, (BYTE*)c, &l);
	}
	return rv;
}

// a kind of ... something approximate
// note that input string is currently supposed to be 0-protected (not necessarily 0-terminated)
int ProcessCEscapes(char *c, int l) 
{
	if (!c) return 0;
	char *e = c + l, *d = c, *cc = c;
	for(; c < e; c++) {
		if (*c != '\\') *d++ = *c;
		else switch(c[1]) {
      case '\\': *d++ = '\\', c++; break;
      case 'r': *d++ = '\r', c++; break;
      case 'n': *d++ = '\n', c++; break;
      case 't': *d++ = '\t', c++; break;
      case 'x': case 'X': 
		  { // hexadecimal; no more than two symbols
			  c++;
			  int cval = 0;
			  if (isxdigit((unsigned char)c[1])) {
				  c++;
				  cval = *c <= '9' ? *c - '0' : toupper(*c) - 'A' + 10;
				  if (isxdigit((unsigned char)c[1])) {
					  c++;
					  cval = (cval << 4) | (*c <= '9' ? *c - '0' : toupper(*c) - 'A' + 10);
				  }
			  }
			  *d++ = cval;
		  }
		  break;
      default:
		  if ('0' <= c[1] && c[1] <= '7') { // octal; no more than three symbols
			  c++; int cval = *c - '0';
			  if ('0' <= c[1] && c[1] <= '7') c++, cval = (cval << 3) | (*c - '0');
			  if ('0' <= c[1] && c[1] <= '7') c++, cval = (cval << 3) | (*c - '0');
			  *d++ = cval;
		  } else {
			  *d++ = *c;
		  }
		}
	}
	*d = 0;
	return d - cc;
}


int MakeCEscapes(const char *c, int l, achar &out) 
{
	if (!c) { out.l = 0; return 0; }
	int sz = l;
	const char *cc = c, *e = c + l;
	for(; cc < e; cc++) if ((unsigned char)*cc < ' ') sz += 3; else if (*cc == '\\') sz++;
	out.resize(sz); out.l = sz;
	char *o = out.c;
	for(cc = c; cc < e; cc++) 
		if ((unsigned char)*cc < ' ') o += sprintf(o, "\\x%02x", *cc);
		else if (*cc == '\\') *o++ = '\\', *o++ = '\\';
		else *o++ = *cc;
		*o = 0;
		return sz;
}


// return true if processing occured
bool MakeCEscapes(achar &r) 
{
	if (!r.size()) return false;
	const char *cc = r.c, *e = r.c + r.l;
	for(; cc < e; cc++) if ((unsigned char)*cc < ' ' || *cc == '\\') break;
	if (cc == e) return false;
	char *c = r.c; r.c = 0;
	MakeCEscapes(c, r.l, r);
	free(c);
	return true;
}


UINT achar::GetDlgItemTextUnCEsc(HWND hwnd, int nIDDlgItem) 
{
	GetDlgItemText(hwnd, nIDDlgItem);
	return l = ProcessCEscapes(c, l);
}


int val_ed_dialog_data::EditValue(HWND hwnd) 
{
	hk=GetKeyByName(keyname,KEY_QUERY_VALUE | KEY_SET_VALUE);
	if ((readonly = (HANDLE)hk==INVALID_HANDLE_VALUE) != 0) {
		hk=GetKeyByName(keyname,KEY_QUERY_VALUE);
	}
	if ((HANDLE)hk==INVALID_HANDLE_VALUE) return 1;
	auto_close_hkey aclhk(hk);
	if (data.QueryValue(hk, name, type)) {
		return 1;
	}
	bool ok = false;
	//is_changed = false;
	switch(type) {
	case REG_SZ: case REG_EXPAND_SZ:
		ok = DialogBoxParam(hInst,"EDSTRING",hwnd,EditString,(LPARAM)this) >=0 && newdata.c;
		break;
	case REG_DWORD_BIG_ENDIAN:
		be_le_swap(data);
	case REG_DWORD:
		if (data.l < 4) data.resize(4);
		newdata.l = (data.l < 4)? 4 : data.l;
		newdata.resize();
		ok = DialogBoxParam(hInst,"EDDWORD",hwnd,EditDWORD,(LPARAM)this) > 0;
		if (type==REG_DWORD_BIG_ENDIAN) be_le_swap(newdata);
		break;
	case REG_MULTI_SZ:
		ok = DialogBoxParam(hInst,"EDMSTRING",hwnd,EditMString,(LPARAM)this) >=0 && newdata.c;
		break;
	default:
		flag1 = type != REG_BINARY;
		ok = DialogBoxParam(hInst,"EDBINARY",hwnd,EditBinary,(LPARAM)this) >=0 && newdata.c;
		break;
	}
	if (!ok) return 1;

	//
	// 关键的一步, 设置键值(value), 改成hive即可
	//
	if ( ReturnType_OK != ModifyValueData_hive( currentitem, name, newdata, type ) )
	{
		RegSetValueEx(hk,name,NULL,type,newdata, newdata.l + (type == REG_SZ || type == REG_EXPAND_SZ));
	}

	return 0;
}


BOOL CALLBACK DialogCR(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam) 
{
    connect_remote_dialog_data *cr;
    switch (msg) {
	case WM_INITDIALOG: 
		SetWindowLong(hwnd, DWL_USER, lParam);
		SendDlgItemMessage(hwnd, IDC_C_HKLM, BM_SETCHECK, 1, 0);
		SendDlgItemMessage(hwnd, IDC_C_HKCR, BM_SETCHECK, 0, 0);
		SendDlgItemMessage(hwnd, IDC_C_HKUS, BM_SETCHECK, 1, 0);
		SendDlgItemMessage(hwnd, IDC_C_HKPD, BM_SETCHECK, 0, 0);
		SendDlgItemMessage(hwnd, IDC_C_HKDD, BM_SETCHECK, 0, 0);
		EnableWindow(GetDlgItem(hwnd, IDC_C_HKCR), 0);
		EnableWindow(GetDlgItem(hwnd, IDOK), 0);
		SetDlgEditCtrlHist(hwnd, IDC_E_COMP);
		return 1;
    case WM_CLOSE: EndDialog(hwnd,0); return 1;
    case WM_COMMAND:
		cr = (connect_remote_dialog_data*)GetWindowLong(hwnd, DWL_USER);
		BROWSEINFO bi; fchar namebuf;  IMalloc *shmalloc;
		switch (LOWORD (wParam)) {
		case IDCANCEL: EndDialog(hwnd,0); break; 
		case IDOK: 
			cr->LM = SendDlgItemMessage(hwnd, IDC_C_HKLM, BM_GETCHECK, 0,0) != 0;
			cr->CR = SendDlgItemMessage(hwnd, IDC_C_HKCR, BM_GETCHECK, 0,0) != 0;
			cr->US = SendDlgItemMessage(hwnd, IDC_C_HKUS, BM_GETCHECK, 0,0) != 0;
			cr->PD = SendDlgItemMessage(hwnd, IDC_C_HKPD, BM_GETCHECK, 0,0) != 0;
			cr->DD = SendDlgItemMessage(hwnd, IDC_C_HKDD, BM_GETCHECK, 0,0) != 0;
			cr->comp.GetDlgItemText(hwnd, IDC_E_COMP);
			EndDialog(hwnd,1); break;
		case IDC_E_COMP:
			if (HIWORD(wParam) == EN_CHANGE) {
				EnableWindow(GetDlgItem(hwnd, IDOK), SendDlgItemMessage(hwnd, IDC_E_COMP, WM_GETTEXTLENGTH,0,0) != 0);
			}
			break;
		case IDC_BROWSECOMP:
			if (!co_initialized) CoInitialize(0), co_initialized = true;
			bi.hwndOwner = hwnd;
			bi.pidlRoot = 0; bi.pszDisplayName = namebuf.c = (char*)malloc(MAX_PATH);
			bi.lpszTitle = "Select computer";
#ifndef BIF_NEWDIALOGSTYLE
#define BIF_NEWDIALOGSTYLE     0x0040
#endif
			bi.ulFlags  = BIF_BROWSEFORCOMPUTER /*|BIF_NEWDIALOGSTYLE*/;
			bi.lpfn = 0, bi.lParam = 0;
			SHGetSpecialFolderLocation(hwnd, CSIDL_NETWORK, (ITEMIDLIST **)&bi.pidlRoot);
			if (SHBrowseForFolder(&bi)) {
				SetDlgItemText(hwnd, IDC_E_COMP, namebuf);
			}
			if (bi.pidlRoot) {
				SHGetMalloc(&(shmalloc = 0));
				if (shmalloc) {
					shmalloc->Free((void*)bi.pidlRoot);
					shmalloc->Release();
				}
			}
			break;
		}
		return 1;
	}
	return 0;
}


BOOL CALLBACK DialogDcR(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam) 
{
    disconnect_remote_dialog_data *cr;
    int n;
    switch (msg) {
	case WM_INITDIALOG: 
		SetWindowLong(hwnd, DWL_USER, lParam);
		cr = (disconnect_remote_dialog_data*)lParam;
		for(n = 0; n < cr->numsel; n++) {
			SendDlgItemMessage(hwnd, IDC_L_NAMES, LB_ADDSTRING, 0, (LONG)cr->keys[n]);
		}
		return 1;
    case WM_CLOSE: EndDialog(hwnd,0); return 1;
    case WM_COMMAND:
		cr = (disconnect_remote_dialog_data*)GetWindowLong(hwnd, DWL_USER);
		switch (LOWORD (wParam)) {
		case IDCANCEL: EndDialog(hwnd,0); break; 
		case IDOK: 
			for(n = 0; n < cr->numsel; n++) {
				if (!SendDlgItemMessage(hwnd, IDC_L_NAMES, LB_GETSEL, n, 0)) cr->keys[n] = 0;
			}
			EndDialog(hwnd,1); break;
		case IDC_B_ALL:
			SendDlgItemMessage(hwnd, IDC_L_NAMES, LB_SELITEMRANGEEX, 0, cr->numsel - 1);
			break;
		}
		return 1;
	}
	return 0;
}


void CheckDisconnRemoteMenuState() 
{
	bool en = false;
	for(int n = 0; n < rkeys.v.size(); n++) if (rkeys.v[n].ki.flags & IS_REMOTE_KEY) en = true;
	EnableMenuItem(theFileMenu, IDM_REGISTRY_DISCONNECT, en? MF_ENABLED : MF_GRAYED);
}

struct regasyncconn {
	const char *comp;
	HKEY node, *out;
	int retcode;
};


DWORD WINAPI regconnectthread(LPVOID p) 
{
	regasyncconn *s = (regasyncconn*)p;
	s->retcode = RegConnectRegistry(s->comp, s->node, s->out);
	return s->retcode;
}


int ConnectRegistry(achar &comp, HKEY node, const char *node_name5, TVINSERTSTRUCT &tvins) 
{
	if (rr_connecting) return 1;
	HKEY hk;
	char *kn = (char*)malloc(comp.l + 6);
	strcpy(kn, comp.c); strcpy(kn + comp.l, node_name5);
	regasyncconn rasc = { comp.c, node, &hk, -1 };
	{
		fchar s(malloc(comp.l + 40));
		sprintf(s, "Connecting %s ...", kn);
		SendMessage(SbarW, SB_SETTEXT, 0, (LPARAM)s.c);
	}
	DWORD tid;
	HANDLE h = CreateThread(0, 32768, regconnectthread, &rasc, 0, &tid);
	Sleep(50);
	bool hasquit = false;
	WPARAM quitParam = 0;
	if (!h) {
		rasc.retcode = RegConnectRegistry(comp.c, node, &hk);
	} else {
		rr_connecting = true;
		while (1) {
			DWORD m = MsgWaitForMultipleObjects(1, &h, false, 500, QS_ALLEVENTS);
			MSG msg;
			if (m == WAIT_OBJECT_0) break;
			while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) { 
				if (msg.message == WM_QUIT) {
					hasquit = true;
					quitParam = msg.wParam;
					MainWindow = 0;
					continue;
				}
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			} 
		}
		CloseHandle(h);
		rr_connecting = false;
	}
	if (!rasc.retcode) {
		tvins.item.pszText = kn;
		HTREEITEM item = TreeView_InsertItem(TreeW, &tvins);
		if (rkeys.add(kn, hk, CAN_LOAD_SUBKEY | IS_REMOTE_KEY, item)) {
			TreeView_DeleteItem(TreeW, item);
			free(kn);
		}
		SendMessage(SbarW, SB_SETTEXT, 0, (LPARAM)"ok");
	} else {
		ErrMsgDlgBox(kn, rasc.retcode);
		free(kn);
		SendMessage(SbarW, SB_SETTEXT, 0, (LPARAM)"failed");
	}
	if (hasquit) PostQuitMessage(quitParam);
	return rasc.retcode;
}


void ErrMsgDlgBox(LPCTSTR sss, DWORD le) 
{
	TCHAR* lpMsgBuf;
	DWORD n=500;
	DWORD LE=le? le : GetLastError();
	
	FormatMessage( 
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		LE,
		0 /*MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT)*/, // Default language
		(LPTSTR)&lpMsgBuf,
		0,
		NULL 
		);
	MessageBox( NULL, lpMsgBuf, sss, MB_OK|MB_ICONINFORMATION );
	LocalFree( lpMsgBuf );
}


void ErrMsgDlgBox2(LPCTSTR sss, LPCTSTR sss1, DWORD le) 
{
	TCHAR* lpMsgBuf;
	DWORD n=500;
	DWORD LE=le? le : GetLastError();
	
	FormatMessage( 
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		LE,
		0 /*MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT)*/, // Default language
		(LPTSTR)&lpMsgBuf,
		0,
		NULL 
		);
	fchar m1(malloc(strlen(lpMsgBuf) + strlen(sss1) + 10));
	sprintf(m1, "%s\nfailed:\n%s", sss1, lpMsgBuf);
	MessageBox( NULL, m1, sss, MB_OK|MB_ICONINFORMATION );
	LocalFree( lpMsgBuf );
}


void CommDlgErrMsgDlgBox(LPCTSTR sss, DWORD le) 
{
	const char *c;
	switch(le) {
	case CDERR_DIALOGFAILURE: c = "CDERR_DIALOGFAILURE"; break;
	case CDERR_GENERALCODES: c = "CDERR_GENERALCODES"; break;
	case CDERR_STRUCTSIZE: c = "CDERR_STRUCTSIZE"; break;
	case CDERR_INITIALIZATION: c = "CDERR_INITIALIZATION"; break;
	case CDERR_NOTEMPLATE: c = "CDERR_NOTEMPLATE"; break;
	case CDERR_NOHINSTANCE: c = "CDERR_NOHINSTANCE"; break;
	case CDERR_LOADSTRFAILURE: c = "CDERR_LOADSTRFAILURE"; break;
	case CDERR_FINDRESFAILURE: c = "CDERR_FINDRESFAILURE"; break;
	case CDERR_LOADRESFAILURE: c = "CDERR_LOADRESFAILURE"; break;
	case CDERR_LOCKRESFAILURE: c = "CDERR_LOCKRESFAILURE"; break;
	case CDERR_MEMALLOCFAILURE: c = "CDERR_MEMALLOCFAILURE"; break;
	case CDERR_MEMLOCKFAILURE: c = "CDERR_MEMLOCKFAILURE"; break;
	case CDERR_NOHOOK: c = "CDERR_NOHOOK"; break;
	case CDERR_REGISTERMSGFAIL: c = "CDERR_REGISTERMSGFAIL"; break;
	case PDERR_PRINTERCODES: c = "PDERR_PRINTERCODES"; break;
	case PDERR_SETUPFAILURE: c = "PDERR_SETUPFAILURE"; break;
	case PDERR_PARSEFAILURE: c = "PDERR_PARSEFAILURE"; break;
	case PDERR_RETDEFFAILURE: c = "PDERR_RETDEFFAILURE"; break;
	case PDERR_LOADDRVFAILURE: c = "PDERR_LOADDRVFAILURE"; break;
	case PDERR_GETDEVMODEFAIL: c = "PDERR_GETDEVMODEFAIL"; break;
	case PDERR_INITFAILURE: c = "PDERR_INITFAILURE"; break;
	case PDERR_NODEVICES: c = "PDERR_NODEVICES"; break;
	case PDERR_NODEFAULTPRN: c = "PDERR_NODEFAULTPRN"; break;
	case PDERR_DNDMMISMATCH: c = "PDERR_DNDMMISMATCH"; break;
	case PDERR_CREATEICFAILURE: c = "PDERR_CREATEICFAILURE"; break;
	case PDERR_PRINTERNOTFOUND: c = "PDERR_PRINTERNOTFOUND"; break;
	case PDERR_DEFAULTDIFFERENT: c = "PDERR_DEFAULTDIFFERENT"; break;
	case CFERR_CHOOSEFONTCODES: c = "CFERR_CHOOSEFONTCODES"; break;
	case CFERR_NOFONTS: c = "CFERR_NOFONTS"; break;
	case CFERR_MAXLESSTHANMIN: c = "CFERR_MAXLESSTHANMIN"; break;
	case FNERR_FILENAMECODES: c = "FNERR_FILENAMECODES"; break;
	case FNERR_SUBCLASSFAILURE: c = "FNERR_SUBCLASSFAILURE"; break;
	case FNERR_INVALIDFILENAME: c = "FNERR_INVALIDFILENAME"; break;
	case FNERR_BUFFERTOOSMALL: c = "FNERR_BUFFERTOOSMALL"; break;
	case FRERR_FINDREPLACECODES: c = "FRERR_FINDREPLACECODES"; break;
	case FRERR_BUFFERLENGTHZERO: c = "FRERR_BUFFERLENGTHZERO"; break;
	case CCERR_CHOOSECOLORCODES: c = "CCERR_CHOOSECOLORCODES"; break;
	default: c = "Unknown common dialogs error";
	}
	MessageBox( NULL, c, sss, MB_OK|MB_ICONINFORMATION );
}


int EnablePrivilege_NT(LPCTSTR where, LPCTSTR name) 
{
	LUID luidSD;
	TOKEN_PRIVILEGES tp;
    HANDLE hCurProc, CurProcToken;
	hCurProc = GetCurrentProcess();
	if (!OpenProcessToken(hCurProc, TOKEN_ADJUST_PRIVILEGES, &CurProcToken)) return 1;
    auto_close_handle A(CurProcToken);
    if (!LookupPrivilegeValue(where, name, &luidSD)) return 2;
	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = luidSD;
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    if (!AdjustTokenPrivileges(CurProcToken, false, &tp, 0, NULL, NULL)) return 3;
    return 0;
}


#if 0
class MySecur : public ISecurityInformation {
public:
	HRESULT GetObjectInformation(PSI_OBJECT_INFO pObjectInfo);
	HRESULT GetSecurity(SECURITY_INFORMATION RequestedInformation, PSECURITY_DESCRIPTOR* ppSecurityDescriptor, BOOL fDefault);
	HRESULT SetSecurity(SECURITY_INFORMATION SecurityInformation, PSECURITY_DESCRIPTOR pSecurityDescriptor);
	HRESULT GetAccessRights(const GUID* pguidObjectType, DWORD dwFlags, PSI_ACCESS* ppAccess, ULONG* pcAccesses, ULONG* piDefaultAccess);
	HRESULT MapGeneric(const GUID* pguidObjectType, UCHAR* pAceFlags, ACCESS_MASK* pMask);
	HRESULT GetInheritTypes(PSI_INHERIT_TYPE* ppInheritTypes, ULONG* pcInheritTypes);
	HRESULT PropertySheetPageCallback(HWND hwnd, UINT uMsg, SI_PAGE_TYPE uPage);
};
int EditKeySecurity(const char *name) {
	//EnablePrivilege_NT(0/*comp name!*/, SE_SECURITY_NAME);
	HKEY hk = GetKeyByName(name, KEY_EXECUTE | STANDARD_RIGHTS_READ /*| ACCESS_SYSTEM_SECURITY*/ /*| WRITE_DAC | WRITE_OWNER*/);
	//DisablePrivilege_NT(0/*comp name!*/, SE_SECURITY_NAME);
	if (hk == INVALID_HANDLE_VALUE) return 1;
	// To read the SACL from the security descriptor, the calling process must have been granted ACCESS_SYSTEM_SECURITY access when the key was opened. The proper way to get this access is to enable the SE_SECURITY_NAME privilege in the caller's current token, open the handle for ACCESS_SYSTEM_SECURITY access, and then disable the privilege.
	//RegGetKeySecurity(hk,,, &sz);
	//RegSetKeySecurity(hk,,);
	//MySecur mysecur;
	//EditSecurity(MainWindow, &mysecur);
	return 0;
}
#endif


void AddKeyToFavorites(fchar &title, fchar &key, fchar &value, fchar &comment) 
{
	HKEY hk;
	DWORD LE = RegCreateKeyEx(HKEY_CURRENT_USER, REFAVPATH, 0, 0, 0, KEY_SET_VALUE, 0, &hk, 0);
	if (LE) {
		ErrMsgDlgBox2("Could not save favorite!", "RegOpenKeyEx for key\n"REFAVPATH"", LE);
		return;
	}
	const char *d, *s = rkeys.ShortName2LongName(key, &d);
	fchar t;
	int vaddl = (value.c? strlen(value) + 3 : 0) + (comment.c? strlen(comment) + 4 : 0);
	if (s && d - key.c == 5) { // i.e. local
		t.c = (char*)malloc(MYCOMPLEN + strlen(s) + strlen(d) + 2 + vaddl);
		sprintf(t, MYCOMP"%s\\%s", s, d);
	} else { // remote?
		if (vaddl) t.c = (char*)malloc(strlen(key) + vaddl);
	}
	if (vaddl) {
		char *e = t.c + strlen(t.c) + 1;
		if (value.c) { strcpy(e, value.c); e += strlen(value) + 1; }
		else {
			if (comment.c) *e++ = 1;
			*e++ = 0;
		}
		if (comment.c) { strcpy(e, comment.c); e += strlen(comment) + 1; }
		else *e++ = 0;
		*e++ = 0; //2nd sero
		vaddl = e - t.c;
	}
	
	DWORD type = vaddl? REG_MULTI_SZ : REG_SZ;
	char *key2 = t.c? t : key;
	LE = RegSetValueEx(hk, title, NULL, type, (BYTE*)key2, vaddl? vaddl : strlen(key2) + 1);
	if (LE) {
		ErrMsgDlgBox2("Could not save favorite!", "RegSetValueEx for key\n"REFAVPATH"", LE);
		RegCloseKey(hk);
		return;
	}
	RegCloseKey(hk);
}


void 
FavKeyName2ShortName (
	IN const char *k0,
	OUT fchar &key
	) 
{
	PCHAR ptr ;
	if ( 0 == strnicmp( k0, MYCOMP, MYCOMPLEN ) ) 
	{
		FixKeyName( k0 + MYCOMPLEN, key );
		return ;
	}
	
	if ( 'H' != *k0 ) // 形如 "我的电脑\HKEY_LOCAL_MACHINE\..."
	{
		if ( ptr = strchr( k0, '\\' ) )
		{
			ptr++ ;
			char *t = strdup(ptr);
			free( key.c );
			key.c = t;
			return ;
		}
	}

	char *t = strdup(k0);
	free( key.c );
	key.c = t;
	return ;
}

void AddFavoritesToMenu(HMENU menu) 
{
	HKEY hk;
	DWORD LE = RegOpenKeyEx(HKEY_CURRENT_USER, REFAVPATH, 0, KEY_EXECUTE | KEY_QUERY_VALUE, &hk);
	if (LE) return;
	value_iterator i(hk);
	if (i.err()) {
		RegCloseKey(hk);
		return;
	}

	// 因为要增加收藏夹,故先清空之
	while(RemoveMenu(menu, 4, MF_BYPOSITION)); //clear the menu
	DWORD p = 41000; // menu item id's

	for(vector<favitem_t>::iterator j = favItems.begin(); j != favItems.end(); j++) 
	{
		free(j->name);  free(j->key);
		free(j->value); free(j->comment);
	}
	favItems.clear();

	// 开始添加...
	for (; !i.end(); i++) // i++, 即枚举 ...\\Favorites键下的所有值的内容
	{
		if (i.is_ok && (i.type == REG_SZ || i.type == REG_MULTI_SZ)) 
		{
			AppendMenu( menu, MF_STRING, p++, i.name );

			favitem_t fit = { strdup(i.name), strdup(i.data), 0, 0 };
			if (i.type == REG_MULTI_SZ) 
			{
				if (*(fit.value = strchr(i.data, 0) + 1) && fit.value < i.data.c + i.data.l) {
					fit.comment = strchr(fit.value, 0) + 1;
					if (*fit.value == 1) fit.value = 0;
					if (fit.comment >= i.data.c + i.data.l || !*fit.comment) fit.comment = 0;
				} else {
					fit.value = 0;
				}
			}

			if (fit.value) fit.value = strdup(fit.value);
			if (fit.comment) fit.comment = strdup(fit.comment);
			favItems.push_back(fit);
			if (p >= 41100) break; //only 100 items allowed
		}
	}

	RegCloseKey(hk);
}


const int MAX_KEY_TITLE_LEN = 24;
void SuggestTitleForKey(const char *key, fchar &title) 
{
	int l = strlen(key);
	if (l <= MAX_KEY_TITLE_LEN) {
		title.c = strdup(key);
		return;
	}
	title.c = (char*)malloc(MAX_KEY_TITLE_LEN + 1);
	memcpy(title.c, key, 4);
	memcpy(title.c + 4, "...", 3);
	strcpy(title.c + 7, key + (l - MAX_KEY_TITLE_LEN + 7));
}


//////////////////////////////////////////////////////////////////////////