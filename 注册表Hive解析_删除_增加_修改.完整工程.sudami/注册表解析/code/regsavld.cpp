#include "regedit.h"
#include "resource.h"

#include "regsavld.h"

BOOL CALLBACK DialogLDH(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam) 
{
    load_hive_dialog_data *dd;
	char *c ;

    switch (msg) 
	{
	case WM_INITDIALOG: 
		SetWindowLong(hwnd, DWL_USER, lParam);
		dd = (load_hive_dialog_data*)lParam;
		SetDlgItemText(hwnd, IDC_E_KEYNAME, dd->root_key_name);
		SetEditCtrlHistAndText(hwnd, IDC_E_FILENAME, dd->fname.c? dd->fname.c : "");
		SetDlgEditCtrlHist(hwnd, IDC_E_SUBKEYNAME);
		
		// 加载hive前需提升权限 
		c = strchr(dd->root_key_name, ':'); if (c) *c = 0; 
		EnablePrivilege_NT( c? dd->root_key_name : 0, SE_RESTORE_NAME ) ;
		return 1;

    case WM_CLOSE: EndDialog(hwnd,0); return 1;
    case WM_COMMAND:
		dd = (load_hive_dialog_data*)GetWindowLong(hwnd, DWL_USER);
		switch (LOWORD (wParam)) {
		case IDCANCEL: EndDialog(hwnd,0); break; 
		case IDOK: 
			dd->fname.GetDlgItemText(hwnd, IDC_E_FILENAME);
			dd->subkey_name.GetDlgItemText(hwnd, IDC_E_SUBKEYNAME);
			if (!dd->subkey_name.size()) {
				char *c = strrchr(dd->fname, '\\'); if (!c) c = dd->fname; else c++;
				SetDlgItemText(hwnd, IDC_E_SUBKEYNAME, c);
				return 1;
			}
			EndDialog(hwnd,1); break;
		case IDC_B_CHOOSE:
			{
				achar tname;
				tname.GetDlgItemText(hwnd, IDC_E_FILENAME);
				if (!DisplayOFNdlg(tname, "Choose registry file", 0)) {
					SetDlgItemText(hwnd, IDC_E_FILENAME, tname.c? tname.c : "");
				}
			}
			break;
		case IDC_B_GETPRIV: 
			{
				c = strchr(dd->root_key_name, ':');
				if (c) *c = 0;
				if (!EnablePrivilege_NT(c? dd->root_key_name : 0, SE_RESTORE_NAME)) {
					if (!c) has_rest_priv = true;
					char msgbuf[100] = "Enabled \""SE_RESTORE_NAME"\"";
					if (c) strcat(msgbuf, " on "), strcat(msgbuf, dd->root_key_name);
					SetDlgItemText(hwnd, IDC_SEWARN, msgbuf);
				}
				if (c) *c = ':';
			}
			break;
		}
		return 1;
	}
	return 0;
}


BOOL CALLBACK DialogSVK(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam) 
{
	save_key_dialog_data *dd;
	char *c, *cc ;

	switch (msg)
	{
	case WM_INITDIALOG: 
		SetWindowLong(hwnd, DWL_USER, lParam);
		dd = (save_key_dialog_data*)lParam;
		SetEditCtrlHistAndText(hwnd, IDC_E_KEYNAME, dd->key_name);
		SetEditCtrlHistAndText(hwnd, IDC_E_FILENAME, dd->fname.c? dd->fname.c : "");

		c = strchr(dd->key_name, ':'); 
		cc = strchr(dd->key_name, '\\');
		if (c > cc) c = 0;
		if (c) *c = 0;
		EnablePrivilege_NT(c? dd->key_name : (char*)0, SE_BACKUP_NAME);
		return 1;

    case WM_CLOSE: EndDialog(hwnd,0); return 1;
    case WM_COMMAND:
		dd = (save_key_dialog_data*)GetWindowLong(hwnd, DWL_USER);
		switch (LOWORD (wParam)) {
		case IDCANCEL: EndDialog(hwnd,0); break; 
		case IDOK: 
			dd->fname.GetDlgItemText(hwnd, IDC_E_FILENAME);
			dd->key_name.GetDlgItemText(hwnd, IDC_E_KEYNAME);
			if (!dd->key_name.size()) {
				c = strrchr(dd->key_name, '\\'); if (!c) c = dd->key_name; else c++;
				SetDlgItemText(hwnd, IDC_E_FILENAME, c);
				return 1;
			}
			EndDialog(hwnd,1); break;
		case IDC_B_CHOOSE:
			{
				achar tname;
				tname.GetDlgItemText(hwnd, IDC_E_FILENAME);
				if (!DisplayOFNdlg(tname, "Choose save file", 0, true, true)) {
					SetDlgItemText(hwnd, IDC_E_FILENAME, tname.c? tname.c : "");
				}
			}
			break;
		case IDC_B_GETPRIV: 
			break;
		}
		return 1;
	}
	return 0;
}

BOOL CALLBACK DialogLDK(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam) {
	load_key_dialog_data *dd;
	char *c, *cc;

	switch (msg) 
	{
	case WM_INITDIALOG: 
		SetWindowLong(hwnd, DWL_USER, lParam);
		dd = (load_key_dialog_data*)lParam;
		SetEditCtrlHistAndText(hwnd, IDC_E_KEYNAME, dd->key_name);
		SetEditCtrlHistAndText(hwnd, IDC_E_FILENAME, dd->fname.c? dd->fname.c : "");
		SendDlgItemMessage(hwnd, IDC_CHK_FORCE,   BM_SETCHECK, dd->force, 0);
		SendDlgItemMessage(hwnd, IDC_CHK_NOLAZY , BM_SETCHECK, dd->nolazy, 0);
		SendDlgItemMessage(hwnd, IDC_CHK_REFRESH, BM_SETCHECK, dd->refresh, 0);
		SendDlgItemMessage(hwnd, IDC_CHK_HIVEVOL, BM_SETCHECK, dd->volatil, 0);
	
		c = strchr(dd->key_name, ':'); 
		cc = strchr(dd->key_name, '\\');
		if (c > cc) c = 0;
		if (c) *c = 0;
		EnablePrivilege_NT( c? dd->key_name : (char*)0, SE_RESTORE_NAME );
		return 1;

    case WM_CLOSE: EndDialog(hwnd,0); return 1;
    case WM_COMMAND:
		dd = (load_key_dialog_data*)GetWindowLong(hwnd, DWL_USER);
		switch (LOWORD (wParam)) 
		{
		case IDCANCEL: EndDialog(hwnd,0); break; 
		case IDOK: 
			dd->fname.GetDlgItemText(hwnd, IDC_E_FILENAME);
			dd->key_name.GetDlgItemText(hwnd, IDC_E_KEYNAME);
			dd->force  =  SendDlgItemMessage(hwnd, IDC_CHK_FORCE, BM_GETCHECK, 0, 0) != 0;
			dd->nolazy  = SendDlgItemMessage(hwnd, IDC_CHK_NOLAZY, BM_GETCHECK, 0, 0) != 0;
			dd->refresh = SendDlgItemMessage(hwnd, IDC_CHK_REFRESH, BM_GETCHECK, 0, 0) != 0;
			dd->volatil = SendDlgItemMessage(hwnd, IDC_CHK_HIVEVOL, BM_GETCHECK, 0, 0) != 0;
			if (!dd->key_name.size()) {
				char *c = strrchr(dd->key_name, '\\'); if (!c) c = dd->key_name; else c++;
				SetDlgItemText(hwnd, IDC_E_FILENAME, c);
				return 1;
			}
			EndDialog(hwnd,1); break;

		case IDC_B_CHOOSE:
			{
				achar tname;
				tname.GetDlgItemText(hwnd, IDC_E_FILENAME);
				if (!DisplayOFNdlg(tname, "Load key from file", 0, false, false)) {
					SetDlgItemText(hwnd, IDC_E_FILENAME, tname.c? tname.c : "");
				}
			}
			break;
		case IDC_B_GETPRIV: 
			break;
		}
		return 1;
	}
	return 0;
}


BOOL CALLBACK DialogRplK(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam) {
	replace_key_dialog_data *dd;
	char *c, *cc;
	switch (msg) 
	{
	case WM_INITDIALOG: 
		SetWindowLong(hwnd, DWL_USER, lParam);
		dd = (replace_key_dialog_data*)lParam;
		SetEditCtrlHistAndText(hwnd, IDC_E_KEYNAME, dd->key_name);
		SetEditCtrlHistAndText(hwnd, IDC_E_FILENAME, dd->fname_new.c? dd->fname_new.c : "");
		SetEditCtrlHistAndText(hwnd, IDC_E_FILENAME2, dd->fname_old.c? dd->fname_old.c : "");
		SetDlgItemText(hwnd, IDC_SEWARN, has_rest_priv? "You have \""SE_RESTORE_NAME"\"" : "You don't have \""SE_RESTORE_NAME"\"");
	
		c = strchr(dd->key_name, ':');
		cc = strchr(dd->key_name, '\\');
		if (c > cc) c = 0;
		if (c) *c = 0;
		EnablePrivilege_NT( c? dd->key_name : (char*)0, SE_RESTORE_NAME );
		return 1;

    case WM_CLOSE: EndDialog(hwnd,0); return 1;
    case WM_COMMAND:
		dd = (replace_key_dialog_data*)GetWindowLong(hwnd, DWL_USER);
		switch (LOWORD (wParam)) {
		case IDCANCEL: EndDialog(hwnd,0); break; 
		case IDOK: 
			dd->fname_new.GetDlgItemText(hwnd, IDC_E_FILENAME);
			dd->fname_old.GetDlgItemText(hwnd, IDC_E_FILENAME2);
			dd->key_name.GetDlgItemText(hwnd, IDC_E_KEYNAME);
			if (!dd->key_name.size()) {
				char *c = strrchr(dd->key_name, '\\'); if (!c) c = dd->key_name; else c++;
				SetDlgItemText(hwnd, IDC_E_FILENAME, c);
				return 1;
			}
			EndDialog(hwnd,1); break;
		case IDC_B_CHOOSE:
			{
				achar tname;
				tname.GetDlgItemText(hwnd, IDC_E_FILENAME);
				if (!DisplayOFNdlg(tname, "Choose file with new key data", 0, true, false)) {
					SetDlgItemText(hwnd, IDC_E_FILENAME, tname.c? tname.c : "");
				}
			}
			break;
		case IDC_B_CHOOSE2:
			{
				achar tname;
				tname.GetDlgItemText(hwnd, IDC_E_FILENAME2);
				if (!DisplayOFNdlg(tname, "Choose file to save old key data to", 0, true, true)) {
					SetDlgItemText(hwnd, IDC_E_FILENAME2, tname.c? tname.c : "");
				}
			}
			break;
		case IDC_B_GETPRIV: 
			break;
		}
		return 1;
	}
	return 0;
}

int DisplayOFNdlg(achar &name, const char *title, const char *filter, bool no_RO, bool for_save) {
	if (name.s < 4096) name.resize(4096);
	OPENFILENAME ofn;
#ifdef OPENFILENAME_SIZE_VERSION_400
	ofn.lStructSize=OPENFILENAME_SIZE_VERSION_400; //????
#else
	ofn.lStructSize=sizeof(OPENFILENAME);
#endif
	ofn.hwndOwner=MainWindow;
	ofn.hInstance=NULL;
	ofn.lpstrFilter = filter;//"Executable files\0*.exe;*.bat;*.com\0All files\0*.*\0";
	ofn.lpstrCustomFilter=NULL;
	ofn.nMaxCustFilter=0;
	ofn.nFilterIndex=1;
	ofn.lpstrFile = name.c;
	ofn.nMaxFile = name.s;
	ofn.lpstrFileTitle=NULL; 
	ofn.nMaxFileTitle=0;
	ofn.lpstrInitialDir=NULL;
	ofn.lpstrTitle = title; //"Choose a file to open";
#ifndef OFN_FORCESHOWHIDDEN
#define OFN_FORCESHOWHIDDEN 0x10000000
#endif
	ofn.Flags=OFN_FORCESHOWHIDDEN;
	if (no_RO)    ofn.Flags |= OFN_HIDEREADONLY;
	if (!for_save) ofn.Flags |= OFN_FILEMUSTEXIST;
	ofn.nFileOffset=0;
	ofn.nFileExtension=0;
	ofn.lpstrDefExt=0; //"reg"
	ofn.lCustData=0;
	ofn.lpfnHook=NULL;
	ofn.lpTemplateName=NULL;
	if ((for_save? GetSaveFileName : GetOpenFileName)(&ofn)) {
		name.checklen();
		return 0;
	}
	DWORD err = CommDlgExtendedError();
	if (err) CommDlgErrMsgDlgBox("DisplayOFNdlg", err);
	return 1;
}

inline char *chomp(char *c) {
	if (!*c) return c;
	char *d = strchr(c, 0);
	if (d[-1] == '\n') {
		*--d = 0;
		if (d == c) return d;
	}
	if (d[-1] == '\r') *--d = 0;
	return d;
}

char cslashed(char c) {
	switch(c) {
		//case '\\': return '\\';
		//case '"': return '"';
		//case '\'': return '\'';
	case '0': return '\0';
	case 'n': return '\n';
	case 'r': return '\r';
	case 't': return '\t';
	default: return c;
	}
}

char *gethex(char *c, DWORD &d) {
	for(d = 0; isxdigit((unsigned char)*c); c++) 
		d = (d<<4)| ((*c<='9')? *c-'0' : toupper(*c)-'A'+10);
	return c;
}

int LoadDump8bit(const char *fname) {
	FILE *f = fopen(fname, "r");
	FILE *dbgf = 0;//fopen("F:\\mmmm\\Misc\\regedt33\\dbgf.txt", "w");
	if (!f) {
		MessageBox(MainWindow, fname, "Could not read file", MB_ICONERROR);
		return 1;
	}
	char buf[2048], buf_vn[2048];
	vector<unsigned char> buf_vd;
	bool chkid = true;
	bool bin_line_cont = false; DWORD type = 0;
	bool name_set = false;
	auto_close_hkey hk((HKEY)INVALID_HANDLE_VALUE);
	while(fgets(buf, 2048, f)) {
		char *e = chomp(buf);
		if (chkid) {
			if (strcmp(buf, "REGEDIT4")) {
				MessageBox(MainWindow, fname, "Incorrect file signature", MB_ICONERROR);
				return 2;
			}
			chkid = false;
			continue;
		}
		if (!*buf) {
			if (dbgf) fprintf(dbgf, "CloseKey_NHC\n");
			CloseKey_NHC(hk.hk); hk.hk = (HKEY)INVALID_HANDLE_VALUE;
			continue;
		}
		if (*buf == '[') {
			if (e[-1] != ']') {
				MessageBox(MainWindow, fname, "Incorrect file format", MB_ICONERROR);
				return 3;
			}
			*--e = 0;
			char *keyname = buf + 1;
			//TranslateKeyName(keyname, );
			CloseKey_NHC(hk.hk); if (dbgf) fprintf(dbgf, "CloseKey_NHC\n");
			hk.hk = CreateKeyByName(keyname, 0, KEY_WRITE); if (dbgf) fprintf(dbgf, "CreateKeyByName(%s)\n", keyname);
			continue;
		}
		if (*buf == '"' || *buf == '@') {
			char *c = buf, *v = buf_vn;
			if (*c != '@') for(c++; *c && *c != '"'; c++) {
				if (*c == '\\') *v++ = cslashed(*++c);
				else *v++ = *c;
			}
			if (!*c || c[1] != '=') continue;
			name_set = true;
			*v = 0;
			c += 2;
			buf_vd.clear();
			DWORD D = 0;
			bin_line_cont = false; type = 0;
			switch(*c) {
			case '"':
				for(c++; *c && *c != '"'; c++) {
					if (*c == '\\') buf_vd.push_back(cslashed(*++c));
					else buf_vd.push_back(*c);
				}
				buf_vd.push_back(0);
				type = 1;
				break;
			case 'h':
				c += 3; type = 3;
				if (*c == '(') {
					c++;
					c = gethex(c, type);
					if (*c != ')') break;
					c++;
				}
				do {
					c++;
					c = gethex(c, D);
					buf_vd.push_back(D);
				} while(*c == ',' && isxdigit(c[1]));
				if (*c == ',' && c[1] == '\\') bin_line_cont = true;
				break;
			case 'd':
				c += 6; buf_vd.resize(4);
				c = gethex(c, D);
				*(DWORD*)buf_vd.begin() = D;
				type = 4;
				break;
			}
			if (bin_line_cont) continue;
		}
		if (*buf == ' ' && bin_line_cont) {
			char *c = buf;
			while(*c == ' ' || *c == '\t') c++;
			bin_line_cont = false;
			if (!isxdigit((unsigned char)*c)) continue;
			do {
				c++; DWORD D;
				c = gethex(c, D);
				buf_vd.push_back(D);
			} while(*c == ',' && isxdigit(c[1]));
			if (*c == ',' && c[1] == '\\') { bin_line_cont = true; continue; }
		}
		if (name_set) {
			if (dbgf) fprintf(dbgf, "RegSetValueEx(%s, ..., sz = %u)\n", buf_vn, buf_vd.size());
			RegSetValueEx(hk.hk, buf_vn, NULL, type, (BYTE*)buf_vd.begin(), buf_vd.size());
			name_set = false;
		}
	}
	if (ferror(f)) {
		MessageBox(MainWindow, fname, "File read error", MB_ICONERROR);
	}
	fclose(f);
	if (dbgf) fclose(dbgf);
	return 0;
}
