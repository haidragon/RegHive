   #include "regedit.h"
#include "resource.h"

rk_init_s rkeys;

HKEY 
rk_init_s::KeyByName (
	IN const char *name, 
	OUT const char **out
	) 
{
	char *c = strchr(name, '\\');
	if (c) *c = 0;
	n2kmap::iterator i = n2k.find((char*)name);
	if (c) *c = '\\';
	out && (*out = c? c + 1: strchr(name, 0));
	if (i == n2k.end()) return (HKEY)INVALID_HANDLE_VALUE;

	return i->second.hkey;
}

HKRootName2Handle_item_type rkeys_basic[7] = {
	{"HKEY_CLASSES_ROOT", {HKEY_CLASSES_ROOT,    "HKEY_CLASSES_ROOT",    4, 0}},
	{"HKEY_CURRENT_USER", {HKEY_CURRENT_USER,    "HKEY_CURRENT_USER",    4, 0}},
	{"HKEY_LOCAL_MACHINE", {HKEY_LOCAL_MACHINE,   "HKEY_LOCAL_MACHINE",   5, 0}},
	{"HKEY_USERS", {HKEY_USERS,           "HKEY_USERS",           5, 0}},
	{"HKEY_CURRENT_CONFIG", {HKEY_CURRENT_CONFIG,  "HKEY_CURRENT_CONFIG",  4, 0}},
	{"HKEY_DYN_DATA", {HKEY_DYN_DATA,        "HKEY_DYN_DATA",        4, 0}},
	{"HKEY_PERFORMANCE_DATA", {HKEY_PERFORMANCE_DATA,"HKEY_PERFORMANCE_DATA",4, 0}}
};

int rk_init_s::add(const char *name, HKEY hkey, int flags, HTREEITEM item) {
	HKRootName2Handle_item_type r = { name, {hkey, 0, flags, item}};
	if (!n2k.insert(n2kmap::value_type(name, r.ki)).second) return 1;
	k2n.insert(k2nmap::value_type(hkey, name));
	v.push_back(r);
	return 0;
}
rk_init_s::rk_init_s() {
	for(int n = 0; n < 7; n++) {
		n2k.insert(n2kmap::value_type(rkeys_basic[n].name, rkeys_basic[n].ki));
		n2k.insert(n2kmap::value_type(rkeys_basic[n].ki.full_name, rkeys_basic[n].ki));
		k2n.insert(k2nmap::value_type(rkeys_basic[n].ki.hkey, rkeys_basic[n].name));
		lk2sk.insert(namap::value_type(rkeys_basic[n].ki.full_name, rkeys_basic[n].name));
		v.push_back(rkeys_basic[n]);
	}
}
int rk_init_s::remove(const char *name, HKRootName2Handle_item_type &out) {
	n2kmap::iterator i = n2k.find(name);
	if (i == n2k.end()) return 1;
	if (i->second.flags & CANT_REMOVE_KEY) return 2;
	const char *c = i->first;
	for(size_t n = 0, k = 0; n < v.size(); n++) {
		if (n != k) v[k] = v[n];
		if (v[n].name == c) out = v[n];
		else k++;
	}
	if (n == k) return 3; // something very strange...
	v.pop_back();
	n2k.erase(i);
	k2n.erase(out.ki.hkey);
	return 0;
}

const char *rk_init_s::ShortName2LongName(const char *name, const char **out) {
	char *c = strchr(name, '\\');
	if (c) *c = 0;
	n2kmap::iterator i = n2k.find((char*)name);
	if (c) *c = '\\';
	out && (*out = c? c + 1: strchr(name, 0));
	if (i == n2k.end()) return 0;
	return i->second.full_name;
}

const char *
rk_init_s::LongName2ShortName (
	IN const char *name, 
	OUT const char **out
	) 
{
	char *c = strchr(name, '\\') ;
	if (c) *c = 0 ;

	namap::iterator i = lk2sk.find( (char*)name );
	if (c) *c = '\\';
	out && (*out = c? c + 1: strchr(name, 0));
	if (i == lk2sk.end()) return 0;
	return i->second;
}

//Caller must free handle:
LONG lastRegErr = 0;

HKEY 
GetKeyByName (
	IN const char *c,
	IN REGSAM samDesired
	) 
{
	const char *c1;
	HKEY at = rkeys.KeyByName(c, &c1), rk;

	if (*c1 && at != (HKEY)INVALID_HANDLE_VALUE) 
	{
		if (RegOpenKeyEx(at,c1,0,samDesired,&rk)==ERROR_SUCCESS) return rk;
		else {
			return (HKEY)INVALID_HANDLE_VALUE;
		}
	}

	if (c1 - c == 4) return at;
	LONG err = RegOpenKeyEx(at,"",0,samDesired,&rk);
	if (err == ERROR_SUCCESS) return rk; //Prevent "at" destruction by RegCloseKey
	lastRegErr = err;
	return (HKEY)INVALID_HANDLE_VALUE;
}
//Caller must free handle:
HKEY CreateKeyByName(const char *c,const char *cls,REGSAM samDesired) {
	const char *c1;
	HKEY at = rkeys.KeyByName(c, &c1), rk; DWORD d;
	if (*c1 && at != (HKEY)INVALID_HANDLE_VALUE) {
		if (RegCreateKeyEx(at,c1,0,(char*)cls,REG_OPTION_NON_VOLATILE,
			samDesired,NULL,&rk,&d)==ERROR_SUCCESS) return rk;
		else {
			return (HKEY)INVALID_HANDLE_VALUE;
		}
	}
	if (c1 - c == 4) return at;
	if (RegOpenKeyEx(at,"",0,samDesired,&rk)==ERROR_SUCCESS) return rk; //Prevent "at" destruction by RegCloseKey
	return (HKEY)INVALID_HANDLE_VALUE;
}
int DeleteKeyByName(char *c) {
	const char *c1;
	HKEY at = rkeys.KeyByName(c, &c1);
	if (*c1 && at != (HKEY)INVALID_HANDLE_VALUE) {
		return RegDeleteKey(at, c1);
	}
	return !ERROR_SUCCESS;
}
//Caller must free handle:
HKEY GetKeyByItem(HWND tv,HTREEITEM item,REGSAM samDesired) {
	char *c=GetKeyNameByItem(tv,item);
	HKEY k=GetKeyByName(c,samDesired);
	free(c);
	return k;
}

//////////////////////////////////////////////////////////////////////////

#define ARRAYSIZEOF(x)	sizeof (x) / sizeof (x[0])


//
// 要保护的敏感游戏程序
//
static const PCHAR g_cannt_Rename_RegKey[] = {
	"HKEY_CLASSES_ROOT",
	"HKEY_CURRENT_USER",
	"HKEY_LOCAL_MACHINE",
	"HKEY_USERS",
	"HKEY_CURRENT_CONFIG",
	"HKEY_LOCAL_MACHINE\\HARDWARE",
	"HKEY_LOCAL_MACHINE\\SAM",
	"HKEY_LOCAL_MACHINE\\SECURITY",
	"HKEY_LOCAL_MACHINE\\SOFTWARE",
	"HKEY_LOCAL_MACHINE\\SYSTEM",
	"HKEY_LOCAL_MACHINE\\SYSTEM\\ControlSet001",
	"HKEY_LOCAL_MACHINE\\SYSTEM\\ControlSet002"
} ;


BOOL 
CanKeyBeRenamed (
	IN HWND tv,
	IN HTREEITEM item
	) 
{
	ULONG index		= 0		;
	BOOL  bAllowed	= TRUE  ;

	char *c=GetKeyNameByItem(tv,item);

	for ( index = 0; index < ARRAYSIZEOF(g_cannt_Rename_RegKey); index++ )
	{
		if( (0 == stricmp( c, g_cannt_Rename_RegKey[index])) ) {
			bAllowed = FALSE ; break ;	
		}
	}

	free( c );
	return bAllowed ;
}


BOOL CloseKey_NHC(HKEY hk) {
	if (hk!=HKEY_CLASSES_ROOT && hk!=HKEY_CURRENT_USER &&
		hk!=HKEY_LOCAL_MACHINE && hk!=HKEY_USERS &&
		hk!=HKEY_CURRENT_CONFIG && hk!=HKEY_DYN_DATA && hk!=HKEY_PERFORMANCE_DATA &&
		(HANDLE)hk!=INVALID_HANDLE_VALUE)
		return RegCloseKey(hk);
	else return 0;
}

int 
RenameKeyValue (
	HKEY hk, 
	HKEY hk2, 
	const char *newname, 
	const char *oldname, 
	bool copy
	) 
{
	DWORD vkdl,type;
	if (RegQueryInfoKey(hk,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
		NULL,&vkdl,NULL,NULL)!=ERROR_SUCCESS) return 1;
	if (RegQueryValueEx(hk2,newname,NULL,NULL,NULL,NULL)==ERROR_SUCCESS) {
		return 1;
	}
	char *vd=(char*)malloc(++vkdl+1);
	if (RegQueryValueEx(hk,oldname,NULL,&type,(BYTE*)vd,&vkdl)!=ERROR_SUCCESS ||
		RegSetValueEx(hk2,newname,NULL,type,(BYTE*)vd,vkdl)!=ERROR_SUCCESS) {
		free(vd);
		return 1;
	}
	if (!copy) RegDeleteValue(hk,oldname);
	free(vd);
	return 0;
}

// 0=failure:
BOOL IsSubKey(const char *what,const char *of)
{
	int w=strlen(of);
	if (of[w-1]=='\\') w--;
	return !strncmp(of,what,w) && (!what[w] || what[w]=='\\');
}

// Caller must free memory:
char *
GetKeyNameByItem (
	HWND tv,
	HTREEITEM item
	) 
{
	char buf[4096], *rv, *rvpos ;
	int posbuf=0;
	TVITEM tvi;
	HTREEITEM hti = item ;

	if (hti==NULL) // 检验参数合法性
	{
		rv = (char*)malloc(10);
		strcpy( rv, "error!(0)" );
		return rv;
	}

	do 
	{	// 得到该键的全路径
		tvi.mask = TVIF_HANDLE | TVIF_TEXT ;
		tvi.hItem = hti ;
		tvi.pszText = buf + posbuf ;
		tvi.cchTextMax = 4095 - posbuf ;
		if( !TreeView_GetItem( tv, &tvi ) )
		{
			ErrMsgDlgBox( "GetKeyNameByItem" );
			break ;
		}

		hti = TreeView_GetParent( tv, hti );
		posbuf += strlen( tvi.pszText ) + 1 ;

	} while( hti !=NULL && posbuf < 4096 );

	if (posbuf >= 4096 )
	{
		rv=(char*)malloc(10);
		sprintf(rv,"error!(1)");
		return rv;
	}

	rvpos = rv = (char*)malloc( posbuf );
	if (!rv) return "MEM_ERR";
	for( posbuf--; posbuf >= 0 ; ) 
	{
		for(posbuf--; posbuf>=0 && buf[posbuf]; posbuf--);
		strcpy(rvpos,buf+posbuf+1);
		rvpos+=strlen(buf+posbuf+1);
		*(rvpos++)='\\';
	}

	if (rvpos>rv) *(--rvpos)=0;
	return rv;
}


// 展开树形控件到指定的子键处
HTREEITEM 
ShowItemByKeyName (
	IN HWND tv, 
	IN const char *key
	) 
{
	HKEY hk = GetKeyByName( key, KEY_EXECUTE );
	if (hk == (HKEY)INVALID_HANDLE_VALUE) return 0;
	CloseKey_NHC(hk);

	const char *c = key;
	HTREEITEM hfc = 0, parent = 0;
	TVITEM tvi;
	TVINSERTSTRUCT tvins;
	char buf[4096];
	char sztmp[4096] = "" ;

	if ( 0 == strncmp( c, "HKLM", 4 ) )
	{
		// 转换成 HKEY_LOCAL_MACHINE
		char* ptrxx ;
		if ( ptrxx = strchr( c, '\\' ) ) {
			sprintf( sztmp, "HKEY_LOCAL_MACHINE%s", ptrxx );
		} else {
			sprintf( sztmp, "%s", "HKEY_LOCAL_MACHINE" );
		}

		c = sztmp ;
	}

	if ( 0 == strncmp( c, "HKCU", 4 ) )
	{
		// 转换成 HKEY_CURRENT_USER
		char* ptrxx ;
		if ( ptrxx = strchr( c, '\\' ) ) {
			sprintf( sztmp, "HKEY_CURRENT_USER%s", ptrxx );
		} else {
			sprintf( sztmp, "%s", "HKEY_CURRENT_USER" );
		}
		
		c = sztmp ;
	}

	while(c && *c) {
		hfc = TreeView_GetChild(tv, parent);
		char *d = strchr(c, '\\');
		if (d) *d = 0;
		bool already_expanded;
		for(; hfc; hfc = TreeView_GetNextSibling(tv, hfc)) {
			tvi.mask=TVIF_HANDLE | TVIF_TEXT | TVIF_STATE;
			tvi.hItem=hfc;
			tvi.stateMask = TVIS_EXPANDED;
			tvi.pszText = buf, tvi.cchTextMax = 4095;
			if (!TreeView_GetItem(tv, &tvi)) {ErrMsgDlgBox("ShowItemByKeyName"); return 0;}
			if (!stricmp(buf, c)) break;
		}
		if (hfc) already_expanded = (tvi.state & TVIS_EXPANDED) != 0;
		else {
			// as far as the registry key exists...
			tvins.hParent = parent, tvins.hInsertAfter = TVI_SORT;
			tvins.item.mask =  TVIF_TEXT | TVIF_CHILDREN | TVIF_STATE;
			tvins.item.state=0, tvins.item.stateMask=TVIS_EXPANDEDONCE;
			tvins.item.pszText = (char*)c;
			tvins.item.cChildren = d != 0;
			hfc = TreeView_InsertItem(tv, &tvins);
			already_expanded = false;
		} 
		if (hfc && !already_expanded && d) {
			TreeView_Expand(tv, hfc, TVE_EXPAND);
		}
		if (d) *d = '\\', c = d + 1;
		else c = 0;
		parent = hfc;
	}
	if (hfc) {
		TreeView_EnsureVisible(tv, hfc);
		TreeView_SelectItem(tv, hfc);
	}
	return hfc;
}


int SelectItemByValueName(HWND lv, const char *name) {
    LVFINDINFO lvfi; memset(&lvfi, 0, sizeof lvfi);
    lvfi.flags = LVFI_STRING, lvfi.psz = name;
    int n = ListView_FindItem(lv, -1, &lvfi);
    if (n < 0) return n;
    ListView_SetItemState(lv, n, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
    return 0;
}

struct key_copy_cookie {
	const char *dstctrl, *i_kn;
	bool skip_all_err, ignore_err;
	int i_ec;
	key_copy_cookie(const char *dstctrl_) : dstctrl(dstctrl_), i_kn(0), skip_all_err(0), ignore_err(0), i_ec(0) {}
};

BOOL CALLBACK DialogKCE(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam) {
	key_copy_cookie *kc = (key_copy_cookie*)GetWindowLong(hwnd, DWL_USER);
	switch (msg) {
	case WM_INITDIALOG: 
		SetWindowLong(hwnd, DWL_USER, lParam);
		kc = (key_copy_cookie*)lParam;
		SetDlgItemText(hwnd, IDC_KEYNAME, kc->i_kn);
		if (kc->i_ec != 1) kc->ignore_err = false, EnableWindow(GetDlgItem(hwnd, IDIGNORE), false);
		SendDlgItemMessage(hwnd, IDIGNORE, BM_SETCHECK, kc->ignore_err, 0);
		return 1;
	case WM_CLOSE: EndDialog(hwnd, IDCANCEL); return 1;
	case WM_COMMAND:
		switch (LOWORD (wParam)) {
		case IDOK: EndDialog(hwnd, IDOK); break;
		case IDYES: kc->skip_all_err = true; EndDialog(hwnd, IDOK); break;
		case IDCANCEL: EndDialog(hwnd, IDCANCEL); break; 
		case IDIGNORE: if (kc->i_ec == 1) kc->ignore_err = SendDlgItemMessage(hwnd, IDIGNORE, BM_GETCHECK, 0, 0) != 0; break;
		}
		return 1;
	}
	return 0;
}

static int CopyKeyLight(HKEY src, HKEY dst, key_copy_cookie *cookie) {
	int n=0,k,ec=1;
	HKEY src1,dst1;
	DWORD cknl,ccnl, tnl,tcl, tmp;
	BYTE *cknb,*ccnb;
	FILETIME lwt;
	if (RegQueryInfoKey(src,NULL,NULL,NULL,NULL,&cknl,&ccnl,NULL,0,0,NULL,NULL)!=ERROR_SUCCESS) {
		//todo: error msg
		return 0;
	}
	cknb=(BYTE*)malloc(++cknl); ccnb=(BYTE*)malloc(++ccnl);
	//Copy values:
	value_iterator i(src);
	for(; !i.end(); i++) if (i.is_ok) {
		if (RegSetValueEx(dst, i.name, 0,i.type, i.data,i.data.l)!=ERROR_SUCCESS) ec|=2;
	}
	if (i.err()) ec|=2;
	//Copy subkeys
	k = 0;
	const char *const dstctrl = cookie->dstctrl;
	if (dstctrl) for(; dstctrl[k] && dstctrl[k] != '\\'; k++);
	n=0; tnl=cknl, tcl=ccnl;
	while(RegEnumKeyEx(src,n++,(char*)cknb,&tnl,NULL,(char*)ccnb,&tcl,&lwt)==ERROR_SUCCESS) {
		if (ec & 4) break;
		const char *c;
		if(k && !strncmp(dstctrl, (char*)cknb, k) && !cknb[k]) {
			c = dstctrl + k;
			if (*c == '\\') c++;
			if (*c == 0) { tnl = cknl, tcl = ccnl; continue; } //reached dst root!
		} else c = NULL;
		src1 = (HKEY)INVALID_HANDLE_VALUE;
		int srcerr = RegOpenKeyEx(src, (LPCSTR)cknb, 0, KEY_READ, &src1);
		if (!srcerr &&
			!RegCreateKeyEx(dst,(LPCSTR)cknb,0,(char*)ccnb,REG_OPTION_NON_VOLATILE,
			KEY_WRITE,NULL,&dst1,&tmp)) {
			cookie->dstctrl = c;
			ec |= CopyKeyLight(src1, dst1, cookie);
			CloseKey_NHC(dst1);
		} else {
			if (srcerr) {
				cookie->i_kn = (char*)cknb, cookie->i_ec = ec;
				if (!cookie->skip_all_err && DialogBoxParam(hInst, "ASKSRCCOPYFAIL", MainWindow, DialogKCE, (LPARAM)cookie) == IDCANCEL)
					ec |= 4;
				if (!cookie->ignore_err) ec |= 2;
			} else {
				ec |= 2;
			}
		}
		CloseKey_NHC(src1);
		tnl = cknl, tcl = ccnl;
	}
	cookie->dstctrl = dstctrl; //restore
	free(cknb); free(ccnb);
	return ec;
}

extern bool replace_dstkeyexists_all;
extern HWND RplProgrDlg;
BOOL CALLBACK DialogAR (HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);
bool flag_merge_chosen = false;
// CopyKey:
// retval: 1 = ok, 0 and others = failure;
// act="copy"/"move"
int CopyKey(const char *src,const char *dst,const char *act) {
	HKEY sk,dk;
	int n; DWORD k;
	char *cls;
	for(n=0;dst[n];n++) if (dst[n]<32) {
		MessageBox(MainWindow,"Destination name contains illegal characters","Error!",0);
		return 0;
	}
	if (IsSubKey(src,dst) && IsSubKey(dst,src)) {//;)
		char *s=(char*)malloc(strlen(src)+strlen(dst)+40);
		sprintf(s,"Can't %s \n%s\n onto itself!",act,src);
		MessageBox(MainWindow,s,"Error!",0);
		return 0;
	}
	dk=GetKeyByName((char*)dst,KEY_WRITE);
	if ((HANDLE)dk!=INVALID_HANDLE_VALUE) {
		CloseKey_NHC(dk);
		if (!replace_dstkeyexists_all || !RplProgrDlg) {
			confirm_replace_dialog_data crd = { src, 0, dst, 0, 0, 0, 0 };
			int l = DialogBoxParam(hInst, RplProgrDlg? "ASKDSTKEYEXISTS" : "ASKDSTKEYEXISTS2",MainWindow,DialogAR, (long)&crd);
			if (l == 0 || l == 2) {
				if (l == 0 && RplProgrDlg) SendMessage(RplProgrDlg, WM_CLOSE, 0, 0);
				return 0;
			} else {
				if (l == 3 && RplProgrDlg) replace_dstkeyexists_all = true;
				if (!RplProgrDlg) flag_merge_chosen = true;
			}
		}
		//char *s=(char*)malloc(strlen(src)+strlen(dst)+60);
		//sprintf(s,"Can't %s %s onto %s\n because destination already exists!",act,src,dst);
		//MessageBox(MainWindow,s,"Error!",0);
		//return 0;
	}
	sk=GetKeyByName((char*)src,KEY_READ);
	if ((HANDLE)sk==INVALID_HANDLE_VALUE) {
		char *s=(char*)malloc(strlen(src)+30);
		sprintf(s,"Can't open key %s",src);
		MessageBox(MainWindow,s,"Error!",0);
		return 0;
	}
	k=0;
	RegQueryInfoKey(sk,NULL,&k,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL);
	cls=(char*)malloc(++k);
	*cls=0;
	RegQueryInfoKey(sk,cls,&k,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL);
	dk=CreateKeyByName((char*)dst,cls,KEY_WRITE);
	if ((HANDLE)dk==INVALID_HANDLE_VALUE) {
		char *s=(char*)malloc(strlen(src)+30);
		sprintf(s,"Can't create key %s",dst);
		MessageBox(MainWindow,s,"Error!",0);
		CloseKey_NHC(sk); free(cls);
		return 0;
	}
	free(cls);
	if (IsSubKey(dst,src)) {
		cls=(char*)dst+strlen(src);
		if (*cls=='\\') cls++;
	} else cls="";
	key_copy_cookie cookie(cls);
	n = CopyKeyLight(sk, dk, &cookie);
	/*if (n==0 || n==2) {
	//Fatal error
	} else if (n==3) {
	//Non-fatal error
}*/
	CloseKey_NHC(sk);
	CloseKey_NHC(dk);
	return n;
}

int DeleteAllSubkeys(HKEY src) {
	int ec=1;
	HKEY src1;
	DWORD cknl,ccnl, tnl,tcl;
	BYTE *cknb,*ccnb;
	FILETIME lwt;
	if (RegQueryInfoKey(src,NULL,NULL,NULL,NULL,&cknl,&ccnl,NULL,NULL,NULL,NULL,NULL)!=ERROR_SUCCESS) {
		//todo: error msg
		return 0;
	}
	cknb=(BYTE*)malloc(++cknl); ccnb=(BYTE*)malloc(++ccnl);
	tnl=cknl, tcl=ccnl;
	int skid = 0;
	while(RegEnumKeyEx(src,skid,(char*)cknb,&tnl,NULL,(char*)ccnb,&tcl,&lwt)==ERROR_SUCCESS) {
		if (RegOpenKeyEx(src,(LPCSTR)cknb,0,
			KEY_CREATE_SUB_KEY | KEY_READ,&src1)==ERROR_SUCCESS) {
			ec|=DeleteAllSubkeys(src1);
			CloseKey_NHC(src1);
			if (RegDeleteKey(src,(LPCSTR)cknb)) skid++;
		} else { 
			if (ec & 2) skid++;
			ec|=2;
		}
		tnl=cknl, tcl=ccnl;
	}
	free(cknb); free(ccnb);
	return ec;
}

static int DeleteKeyLight(HKEY src,const char *dstctrl) {
	int k=0,ec=1;
	HKEY src1;
	DWORD cknl,ccnl, tnl,tcl, skinx = 0;
	BYTE *cknb,*ccnb;
	FILETIME lwt;
	if (RegQueryInfoKey(src,NULL,NULL,NULL,NULL,&cknl,&ccnl,NULL,NULL,NULL,NULL,NULL)!=ERROR_SUCCESS) {
		//todo: error msg
		return 0;
	}
	cknb=(BYTE*)malloc(++cknl); ccnb=(BYTE*)malloc(++ccnl);
	if (dstctrl) for(;dstctrl[k] && dstctrl[k]!='\\';k++);
	tnl=cknl, tcl=ccnl;
	while(RegEnumKeyEx(src,skinx,(char*)cknb,&tnl,NULL,(char*)ccnb,&tcl,&lwt)==ERROR_SUCCESS) {
		const char *c;
		if(k && !strncmp(dstctrl,(LPCSTR)cknb,k) && strlen((LPCSTR)cknb)==(DWORD)k) {
			c=dstctrl+k;
			if (*c=='\\') c++;
			if (*c==0) {tnl=cknl, tcl=ccnl; skinx++; continue;} //reached dst root!
		} else c=NULL;
		//m.b. skinx=0
		if (RegOpenKeyEx(src,(LPCSTR)cknb,0,
			KEY_CREATE_SUB_KEY | KEY_READ,&src1)==ERROR_SUCCESS) {
			ec|=DeleteKeyLight(src1,c);
			CloseKey_NHC(src1);
			RegDeleteKey(src,(LPCSTR)cknb);
		} else ec|=2;
		tnl=cknl, tcl=ccnl;
	}
	free(cknb); free(ccnb);
	return ec;
}
BOOL DeleteKeyValues(HKEY src) {
	int ec=1,dbg;
	DWORD vknl, tnl;
	BYTE *vknb;
	if (RegQueryInfoKey(src,NULL,NULL,NULL,NULL,NULL,NULL,NULL,&vknl,NULL,NULL,NULL)!=ERROR_SUCCESS) {
		//todo: error msg
		return 0;
	}
	vknb=(BYTE*)malloc(++vknl);
	//Copy values:
	tnl=vknl;
	while((dbg=RegEnumValue(src,0,(char*)vknb,&tnl,NULL,NULL,NULL,NULL))==ERROR_SUCCESS) {
		if (RegDeleteValue(src,(LPCSTR)vknb)!=ERROR_SUCCESS) ec|=2;
		tnl=vknl;
	}
	free(vknb);
	return ec;
}

const PCHAR g_important_keys_list[] = {
	"HKEY_LOCAL_MACHINE\\Software", 
	"HKEY_LOCAL_MACHINE\\Software\\Microsoft", 
	"HKEY_LOCAL_MACHINE\\Software\\Microsoft\\Windows",
	"HKEY_LOCAL_MACHINE\\Software\\Microsoft\\Windows NT",
	"HKEY_LOCAL_MACHINE\\Software\\Microsoft\\Windows\\CurrentVersion", 
	"HKEY_LOCAL_MACHINE\\Software\\Microsoft\\Windows NT\\CurrentVersion",
	"HKEY_LOCAL_MACHINE\\HARDWARE", 
	"HKEY_LOCAL_MACHINE\\SAM", 
	"HKEY_LOCAL_MACHINE\\SYSTEM", 
	"HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet",
	"HKEY_LOCAL_MACHINE\\SYSTEM\\MountedDevices",
	"HKEY_CURRENT_USER\\Software", 
	"HKEY_CURRENT_USER\\Software\\Microsoft", 
	"HKEY_CURRENT_USER\\Software\\Microsoft\\Windows",
	"HKEY_CURRENT_USER\\Software\\Microsoft\\Windows NT",
	"HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion", 
	"HKEY_CURRENT_USER\\Software\\Microsoft\\Windows NT\\CurrentVersion",
	"HKEY_CLASSES_ROOT\\CLSID", 
	"HKEY_LOCAL_MACHINE\\Software\\Classes", 
	"HKEY_LOCAL_MACHINE\\Software\\Classes\\CLSID"
};

bool allow_delete_important_keys = false;

//slow!!!
bool 
CanDeleteThisKey (
	IN const char *name, 
	IN bool qConfig
	) 
{
	ULONG index = 0 ;

	const char *c = strrchr(name, '\\');
	const char *name2 = strchr(name, ':'), *nmend = strchr(name, '\\');
	nmend = nmend? nmend : name + strlen(name);
	if (!name2 || name2 > nmend) name2 = name;
	if (c && !c[1]) {
		for(; c > name2 && c[-1] == '\\'; c--);
	} else
		c = nmend + strlen(nmend);
	bool baddel = c == nmend;

	if (!baddel) 
	{
		for ( index = 0; index < ARRAYSIZEOF(g_important_keys_list); index++ )
		{
			if( 0 == strnicmp( g_important_keys_list[index], name2, c - name2 ) ) 
			{
				baddel = true;
				break;	
			}
		}
	}

	if (!baddel) return true;
	const char *wstr;
	if (qConfig && !allow_delete_important_keys) {
		MessageBox(MainWindow, "You can't delete this key.\nIf you are really sure, turn on \"allow critically important keys deletion\"", name, MB_ICONSTOP);
		return false;
	} else if (qConfig) {
		wstr = "You are about to destroy your Windows installation.\n\nAre you sure you want to continue?";
	} else {
		wstr = "You should not delete this key.\nOnce this key is deleted, your Windows installation may be destroyed.\n\nAre you sure you want to continue?";
	}

	return MessageBox(MainWindow, wstr, name, MB_OKCANCEL | MB_ICONWARNING) == IDOK;
}


// Be sure that neither src nor dst end with backslash!
int MoveKey(const char *src, const char *dst) {
	char *kc;
	if (!CanDeleteThisKey(src, false)) return 0;
	if (!(kc = strrchr(src, '\\'))) return 0;
	int rv = 0, n;
	rv = CopyKey(src, dst, "move"); 
	if (rv != 1) {
		MessageBox(MainWindow, "Errors happened while copying the key.\nSource key will not be deleted.\n", "Move failed", MB_OK);
		return rv;
	}
	//Removing src
	char *cc = (char*)malloc(strlen(src) + 1);
	for(n = 0; src[n] && src + n != kc; n++) cc[n] = src[n];
	cc[n] = 0;
	auto_close_hkey 
		mk(GetKeyByName(cc,KEY_CREATE_SUB_KEY | KEY_READ)),
		sk(GetKeyByName((char*)src,KEY_CREATE_SUB_KEY | KEY_READ | KEY_SET_VALUE));
	free(cc);
	if ((HANDLE)sk.hk == INVALID_HANDLE_VALUE || (HANDLE)mk.hk == INVALID_HANDLE_VALUE) {
		fchar s(malloc(strlen(src) + 30));
		sprintf(s, "Can't delete key %s", src);
		MessageBox(MainWindow, s, "Error!", 0);
		return 0;
	}
	if (!CanDeleteThisKey(src, true)) return 0;
	if (IsSubKey(dst, src)) {
		char *c;
		if (*(c = (char*)dst + strlen(src)) == '\\') c++;
		DeleteKeyLight(sk.hk, c);
		DeleteKeyValues(sk.hk);
	} else {
		//Straight removing
		DeleteKeyLight(sk.hk, NULL);
		RegDeleteKey(mk.hk, kc+1);
	}
	return rv;
}
