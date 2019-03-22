#ifndef __AUTOMGM_H__
#define __AUTOMGM_H__
#pragma once

/// Simple useful classes for dealing with allocated character strings
struct achar 
{
	char *c;
	DWORD s, l;

	operator char*() { return c;} // 隐式转换成 char* 类型
	operator unsigned char*() { return (unsigned char*)c;} // 隐式转换成 unsigned char* 类型
	operator const char*() const { return c;} // 隐式转换成 const char* 类型
	operator const unsigned char*() const // 隐式转换成 const unsigned char* 类型
	{ 
		return (unsigned char*)c ;
	} 
	
	achar() { c = 0; s = 0; l = 0; } // 构造函数
	
	achar( DWORD n ) // 构造函数. 分配一块 n大小的内存
	{
		c = (char*) malloc( s = ++n );
		l = 0; 
	}
	
	achar( const achar &a ) // 构造函数. 复制份结构体
	{ 
		s = a.s, l = a.l ;
		c = a.c ? (char*) malloc(s) : 0 ; 
		c && memcpy( c, a.c, l + 1 ) ;
	}
	
	void operator =(const achar &a) // 重载相等判断运算. 将对象与一个字符串指针进行赋值
	{ 
		s = a.s, l = a.l; 
		c = a.c ? (char*) realloc(c, s) : (free(c),0) ; 
		c && memcpy(c, a.c, l + 1) ; 
	}
	
	achar( const char *ss ) // 赋值字符串到结构体中,需要resize一下
	{ 
		c = 0; 
		if (ss) {
			strcpy( resize( l = strlen(ss) ), ss );
		} else {
			(s = l = 0); 
		}
	}
	
	~achar() { free(c); } // 析构函数
	
	char *resize(DWORD n) 
	{ 
		c = (char*) realloc( c, s = ++n );
		return c; 
	}

	char *resize()
	{ 
		return l >= s? resize(l) : c;
	}

	void checklen() { l = c? strlen(c) : 0; } // 得到字符串的长度
	DWORD asize() const { return s; }
	DWORD size() const { return l; }

	// 将字符串转换为小写
	void strlwr() 
	{ 
		for( char *s = c, *e = c + l; s < e; s++ ) {
			*s = tolower(*s) ;
		}
	}

	UINT GetDlgItemText( HWND hwnd, int nIDDlgItem );
	UINT GetDlgItemTextUnCEsc( HWND hwnd, int nIDDlgItem );
	LONG QueryValue( HKEY hk, const char *name, DWORD &type );
};



struct fchar 
{
	char *c;
	fchar() : c(0) {} // 构造函数. 初始化
	explicit fchar(void *v) : c((char*)v) {} // 构造函数. 赋值
	fchar(fchar &f) : c(f.c) { f.c = 0; } // 构造函数. 结构体赋值

	operator char*&() { return c;}
	operator unsigned char*&() { return (unsigned char*&)c;}
	char &operator *() { return *c; }
	operator const char*() const { return c;}
	operator const unsigned char*() const { return (unsigned char*)c;}
	//    operator bool() const { return c != 0; }
	~fchar() { free(c); }
private:
	//  fchar(const fchar &f);
};


struct auto_close_handle
 {
	HANDLE h;
	auto_close_handle(HANDLE H) : h(H) {}
	~auto_close_handle() { CloseHandle(h); }
};

struct auto_close_hkey {
	HKEY hk;
	auto_close_hkey(HKEY HK) : hk(HK) {}
	~auto_close_hkey() { if (hk != (HKEY)INVALID_HANDLE_VALUE) RegCloseKey(hk); }
};

#endif //__AUTOMGM_H__
