#ifndef _REGEDIT33_H_
#define _REGEDIT33_H_
#pragma once

//#define _WIN32_IE 0x0200
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <ctype.h>
#include <windows.h>
#include <commctrl.h>

// Note: to build the program, I use a slightly patched SGI STL 3.3
// Here it is: http://melkov.narod.ru/misc/stl-fix/
#include <hash_set.h>
#include <hash_map.h>

#include "automgm.h"

/// Iterator for values of a registry key
struct value_iterator
 {
	bool is_end, is_err, is_ok;
	DWORD num_val, cur_val;
	achar name, data; 
	DWORD type;
	HKEY hk;

	// 构造函数
	value_iterator(HKEY hk_) : is_end(1), is_err(0), is_ok(0), num_val(0), cur_val(0), hk(hk_)
	{
		DWORD l_name, l_data;
		if ((HANDLE)hk == INVALID_HANDLE_VALUE
			|| RegQueryInfoKey(hk,0,0,0,0,0,0,&num_val,&l_name,&l_data,NULL,NULL)) {
			is_err = true;
			return;
		}

		name.resize(l_name); 
		data.resize(l_data); //??
		if (num_val > 0) get(), is_end = false;
	}

	void get() // 取得子键的名字 & 内容
	{
		name.l = name.s, data.l = data.s;
		is_ok = !RegEnumValue(hk, cur_val, name.c,&name.l, NULL,&type, data,&data.l);
/*++
LONG WINAPI RegEnumValue(
__in         HKEY hKey,
__in         DWORD dwIndex,
__out        LPTSTR lpValueName,
__inout      LPDWORD lpcchValueName,
__reserved   LPDWORD lpReserved,
__out_opt    LPDWORD lpType,
__out_opt    LPBYTE lpData,
__inout_opt  LPDWORD lpcbData
);
--*/
	}

	void operator ++(int) // 重载自加操作符. 递增遍历子键自身的值
	{
		if (++cur_val < num_val) {
			get();
		} else {
			is_end = true ;
		}
	}

	bool end() { return is_end; }
	bool err() { return is_err; }

private:
	void operator =(const value_iterator&) ; // 

};


extern bool has_rest_priv, has_back_priv;
extern HINSTANCE hInst;
extern HWND MainWindow, SbarW, hwndToolTip;

//////////////////////////////////////////////////////////////////////////

typedef struct _phive_union {
	struct hive *pHive_HKLM_SAM;
	struct hive *pHive_HKLM_SECURITY;
	struct hive *pHive_HKLM_SOFTWARE;
	struct hive *pHive_HKLM_SYSTEM;
	struct hive *pHive_HKCU;
	
	struct hive *pHive_OTHER_USER[10];
	
} PHIVE_UNION ;


struct connect_remote_dialog_data {
	bool LM, CR, US, PD, DD;
	achar comp;
};
struct disconnect_remote_dialog_data {
	int numsel;
	const char **keys;
	disconnect_remote_dialog_data() : numsel(0), keys(0) {}
	~disconnect_remote_dialog_data() { free(keys); }
};



//////////////////////////////////////////////////////////////////////////

LRESULT CALLBACK WindowProc (HWND,UINT,WPARAM,LPARAM);
LRESULT CALLBACK MyHexEditProc (HWND,UINT,WPARAM,LPARAM);
BOOL CALLBACK EditString (HWND,UINT,WPARAM,LPARAM);
BOOL CALLBACK EditMString (HWND,UINT,WPARAM,LPARAM);
BOOL CALLBACK EditBinary (HWND,UINT,WPARAM,LPARAM);
BOOL CALLBACK EditDWORD (HWND,UINT,WPARAM,LPARAM);
BOOL CALLBACK DialogAbout (HWND,UINT,WPARAM,LPARAM);
BOOL CALLBACK DialogSettings(HWND,UINT,WPARAM,LPARAM);
BOOL CALLBACK DialogMVCP (HWND,UINT,WPARAM,LPARAM);
BOOL CALLBACK DialogCR(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);
BOOL CALLBACK DialogDcR(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);
BOOL CALLBACK DialogGotoKey(HWND,UINT,WPARAM,LPARAM);
BOOL CALLBACK DialogAddFavKey(HWND,UINT,WPARAM,LPARAM);

void AddFavoritesToMenu(HMENU menu);
void CheckDisconnRemoteMenuState();
void DoSearchAndReplace(HWND, bool, bool);
void DoSearchAndReplaceNext(HWND hwnd);
void AddKeyToFavorites(fchar &title, fchar &key, fchar &value, fchar &comment);
void SuggestTitleForKey(const char *key, fchar &title);
void FavKeyName2ShortName(const char *k0, fchar &key);
void ValuesAskMoveOrCopy(HWND hwnd, int mvflag, const char *dst);
void KeyAskMoveOrCopy(HWND hwnd, int mvflag, const char *dst);
void ValuesEndDrag(HWND hwnd, bool is_ok);

void GetLWHit(LVHITTESTINFO*);
void GetTWHit(TVHITTESTINFO*);
HMENU TypeMenu(DWORD,int,DWORD);
int GetTypeMnuNo(DWORD);
DWORD GetMinRegValueSize(DWORD);
DWORD GetRegValueType(DWORD,DWORD);
int ValueTypeIcon(DWORD);
void GetLVItemText(HWND ListW, int i, char *&name, DWORD &ns);

int RefreshSubtree_normal(HTREEITEM hfc, HKEY hk, const char *kname );
int ConnectRegistry(achar &comp, HKEY node, const char *node_name5, TVINSERTSTRUCT &tvins);

int DisplayOFNdlg(achar &name, const char *title, const char *filter, bool no_RO = true, bool for_save = false);
int SetDlgEditCtrlHist(HWND hwnd, int item);
int SetEditCtrlHistAndText(HWND hwnd, int item, const char *c);
void LoadSettings();
void ErrMsgDlgBox(LPCTSTR, DWORD le = 0);
const char* TypeCName(DWORD type);
void CommDlgErrMsgDlgBox(LPCTSTR, DWORD);
int EnablePrivilege_NT(LPCTSTR where, LPCTSTR name);
void getDlgItemText(char *&var, HWND hwnd, int ctrl);
void he_DefaultWindowSize(int &x, int &y);

//////////////////////////////////////////////////////////////////////////


//----- keytools.cpp -----
/// Caller must free handle: ///
HKEY GetKeyByName(const char*,REGSAM);
/// Caller must free handle: ///
HKEY CreateKeyByName(const char *c,const char *cls,REGSAM samDesired);
BOOL CloseKey_NHC(HKEY);

extern LONG lastRegErr;
/// Caller must free memory: ///
char *GetKeyNameByItem(HWND,HTREEITEM);
/// Caller must free handle: ///
HKEY GetKeyByItem(HWND,HTREEITEM,REGSAM);
BOOL CanKeyBeRenamed(HWND,HTREEITEM);
HTREEITEM ShowItemByKeyName(HWND tv, const char *key);
int SelectItemByValueName(HWND lv, const char *name);

int RenameKeyValue(HKEY hk, HKEY hk2, const char *newname, const char *oldname, bool copy = false);
BOOL IsSubKey(const char*,const char*);
int CopyKey(const char*,const char*,const char*);
int MoveKey(const char*,const char*);
int DeleteAllSubkeys(HKEY);
int DeleteKeyByName(char*);
bool CanDeleteThisKey(const char *name, bool qConfig);

void GetValueDataString(char*,char*,int,DWORD);

struct str_equal_to // 区分大小写
{
	bool operator()(const char *x, const char *y) const { return !strcmp(x,y); }
};

struct str_iequal_to // 不区分大小写
{
	bool operator()(const char *x, const char *y) const { return !stricmp(x,y); } 
};


// 一个简单的算法,将字符串转换成一个唯一的hash值
struct hash_stri { 

	size_t operator()(const char* s) const 
	{
		unsigned long h = 0;
		for ( ; *s; ++s) {
			h = 5 * h + toupper((unsigned char)*s); // 将字符s转换为大写英文字母

		}
		return size_t(h);
	}
};


struct hash<HKEY> // 调用STL提供的模板来生成相应的hash值
{ 
	size_t operator()(HKEY h) const 
	{ 
		return (size_t)h; 
	} 
}; 


struct confirm_replace_dialog_data 
{
    const char *keyname, *oldvalue, *newvalue, *olddata;
    int olddatalen;
    const char *newdata;
    int newdatalen;
};


struct val_ed_dialog_data
 {
	const char *keyname;
	fchar name;
	achar data, newdata;
	DWORD type;
	HKEY hk;
	bool readonly/*, is_changed*/, flag1;
	int applyEn;
	int EditValue( HWND hwnd );
};

inline void be_le_swap(char *c) 
{
	char tmp; 
	tmp=c[0], c[0]=c[3]; c[3]=tmp; // 0 3 交换
	tmp=c[1], c[1]=c[2]; c[2]=tmp; // 1 2 交换
}

enum RKI_FLAGS 
{
	CAN_LOAD_SUBKEY = 1,
	IS_REMOTE_KEY   = 2,
	CANT_REMOVE_KEY = 4
};

struct RootHandleInfo_type {
	HKEY hkey;
	const char *full_name;
	int flags;
	HTREEITEM item;
};

struct HKRootName2Handle_item_type {
	const char *name;
	RootHandleInfo_type ki;
};

extern HKRootName2Handle_item_type rkeys_basic[7];

typedef hash_map<const char*, RootHandleInfo_type, hash_stri, str_iequal_to> n2kmap;
typedef hash_map<HKEY, const char*> k2nmap;
typedef hash_map<const char*, const char *, hash_stri, str_iequal_to> namap;

extern struct rk_init_s {
	n2kmap n2k;
	k2nmap k2n;
	namap  lk2sk;
	vector<HKRootName2Handle_item_type> v;
	rk_init_s();
	size_t size() const { return v.size(); }
	HKEY KeyByName(const char *name, const char **out = 0);
	const char *ShortName2LongName(const char *name, const char **out);
	const char *LongName2ShortName(const char *name, const char **out);
	int add(const char *name, HKEY hkey, int flags, HTREEITEM item);
	int remove(const char *name, HKRootName2Handle_item_type &out);
} rkeys;

//--end keytools.cpp -----

//////////////////////////////////////////////////////////////////////////

#endif //_REGEDIT33_H_
