// CreateGuid.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include <iostream>

#include <string>
#include <stdio.h>
#include <iostream>
using namespace std;

#pragma comment(lib, "ole32")

//typedef struct _GUID {
//	DWORD Data1;
//	WORD Data2;
//	WORD Data3;
//	BYTE Data4[8];
//} GUID;

//举一个例子：
//假设一个GUID的格式是这样的 6B29FC40 - CA47 - 1067 - B31D - 00DD010662DA
//其中Data1 是32位，可以看做8个四位十六进制数，对应于上面的6B29FC40
//其中Data2 是16位，可以看做4个四位十六进制数，对应于上面的CA47
//其中Data3 是16位，可以看做4个四位十六进制数，对应于上面的1067
//其中Data4 比较特殊，是8个字节也就可以看做16个四位十六进制数
//取其Data4[0], Data4[1]来组成4个四位十六进制数，对应于上面的B31D
//取其Data4[2], Data4[3]来组成4个四位十六进制数，对应于上面的00DD
//取其Data4[4], Data4[5]来组成4个四位十六进制数，对应于上面的0106
//取其Data4[6], Data4[7]来组成4个四位十六进制数，对应于上面的62DA
//*注意：四位十六进制数对应一个GUID字符。

#ifdef WIN32
#include <objbase.h>
#else
#include <uuid/uuid.h>
#endif

GUID CreateGuid()
{
	GUID guid;
#ifdef WIN32
	CoCreateGuid(&guid);
#else
	uuid_generate(reinterpret_cast<unsigned char *>(&guid));
#endif
	return guid;
}

std::string GuidToString(const GUID &guid)
{
	char buf[64] = { 0 };
#ifdef __GNUC__
	snprintf(
#else // MSVC
	_snprintf_s(
#endif
		buf,
		sizeof(buf),
		"{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
		guid.Data1, guid.Data2, guid.Data3,
		guid.Data4[0], guid.Data4[1],
		guid.Data4[2], guid.Data4[3],
		guid.Data4[4], guid.Data4[5],
		guid.Data4[6], guid.Data4[7]);
	return std::string(buf);
}

int main()
{
	//CreateGuid();
	cout << GuidToString(CreateGuid()) << endl;
    std::cout << "Hello World!\n"; 
}
