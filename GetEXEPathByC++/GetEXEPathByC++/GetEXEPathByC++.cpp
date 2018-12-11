// GetEXEPathByC++.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include <iostream>
#include <windows.h>

using namespace std;

int main()
{
	//char path_buffer[_MAX_PATH];
	//char drive[_MAX_DRIVE];
	//char dir[_MAX_DIR];
	//char fname[_MAX_FNAME];
	//char ext[_MAX_EXT];
	//errno_t err;

	//err = _makepath_s(path_buffer, _MAX_PATH, "c", "\\sample\\crt\\",
	//	"crt_makepath_s", "c");

	//if (err != 0)
	//{
	//	printf("Error creating path. Error code %d.\n", err);
	//	exit(1);
	//}
	//printf("Path created with _makepath_s: %s\n\n", path_buffer);
	//err = _splitpath_s(path_buffer, drive, _MAX_DRIVE, dir, _MAX_DIR, fname,
	//	_MAX_FNAME, ext, _MAX_EXT);

	//if (err != 0)
	//{
	//	printf("Error splitting the path. Error code %d.\n", err);
	//	exit(1);
	//}
	//printf("Path extracted with _splitpath_s:\n");
	//printf("  Drive: %s\n", drive);
	//printf("  Dir: %s\n", dir);
	//printf("  Filename: %s\n", fname);
	//printf("  Ext: %s\n", ext);

	//获取应用程序目录
	CHAR szapipath[_MAX_PATH];//（D:\Documents\Downloads\TEST.exe）
	memset(szapipath, 0, _MAX_PATH);
	GetModuleFileName(NULL, szapipath, _MAX_PATH);

	//获取应用程序名称
	char szPath[_MAX_PATH] = "";//（TEST.exe）
	char *pbuf = NULL;
	char* szLine = strtok_s(szapipath, "\\", &pbuf);

	while (NULL != szLine)
	{
		string str = szLine;
		if (str.find(".exe") == str.npos)
		{
			strcat_s(szPath, szLine);
			strcat_s(szPath, "//");
		}

		szLine = strtok_s(NULL, "\\", &pbuf);

	}


	cout << szPath << endl;//(TEST)



	system("pause");
}
