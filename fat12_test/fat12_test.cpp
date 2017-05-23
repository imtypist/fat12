#include <iostream>
#include <windows.h>
using namespace std;

#define MY_FILE_BEGIN           0
#define MY_FILE_CURRENT         1
#define MY_FILE_END             2

typedef DWORD (*MyCreateFile)(char *pszFolderPath, char *pszFileName);
typedef DWORD (*MyWriteFile)(DWORD dwHandle, LPVOID pBuffer, DWORD dwBytesToWrite);
typedef BOOL (*MyDeleteFile)(char *pszFolderPath, char *pszFileName);
typedef DWORD(*MyOpenFile)(char *pszFolderPath, char *pszFileName);
typedef void (*MyCloseFile)(DWORD dwHandle);
typedef DWORD (*MyReadFile)(DWORD dwHandle, LPVOID pBuffer, DWORD dwBytesToRead);
typedef BOOL (*MyCreateDirectory)(char *pszFolderPath, char *pszFolderName);
typedef BOOL (*MyDeleteDirectory)(char *pszFolderPath, char *pszFolderName);
typedef BOOL (*MySetFilePointer)(DWORD dwFileHandle, int nOffset, DWORD dwMoveMethod);

int main() {
	DWORD res;
	HMODULE h = LoadLibraryA("..\\Debug\\fat12.dll");
	if (NULL == h)
	{
		cout << "can not load library..." << endl;
		return 1;
	}
	// 创建文件
	/*
	MyCreateFile MCF = (MyCreateFile)GetProcAddress(h, "MyCreateFile");
	res = MCF("c:\\","test1.txt");
	cout << "MyCreateFile => return " << res << endl;
	//*/

	// 删除文件
	/*
	MyDeleteFile MDF = (MyDeleteFile)GetProcAddress(h, "MyDeleteFile");
	res = MDF("c:\\", "test1.txt");
	cout << "MyDeleteFile => return " << res << endl;
	//*/

	// 打开文件
	//*
	MyOpenFile MOF = (MyOpenFile)GetProcAddress(h, "MyOpenFile");
	res = MOF("c:\\", "test1.txt");
	cout << "MyOpenFile => return " << res << endl;
	//*/

	// 关闭文件
	//*
	MyCloseFile MCF = (MyCloseFile)GetProcAddress(h, "MyCloseFile");
	MCF(res);
	cout << "MyCloseFile => void"<< endl;
	//*/
	
	// 移动文件指针
	//*
	MySetFilePointer MFP = (MySetFilePointer)GetProcAddress(h, "MySetFilePointer");
	res = MFP(res, -64, MY_FILE_END);
	cout << "MySetFilePointer => return " << res << endl;
	//*/

	// 写文件
	/*
	MyWriteFile MWF = (MyWriteFile)GetProcAddress(h, "MyWriteFile");
	char pBuffer[768] = { 0 };
	for (int i = 0; i < 768; i++) {
		pBuffer[i] = '6';
	}
	res = MWF(res, &pBuffer, 768);
	cout << "MyWriteFile => return " << res << endl;
	//*/

	// 读文件
	/*
	MyReadFile MRF = (MyReadFile)GetProcAddress(h, "MyReadFile");
	char rBuffer[1024] = { 0 };
	res = MRF(res, &rBuffer, 768);
	cout << "MyReadFile => return " << res << endl;
	cout << "rBuffer => " << rBuffer << endl;
	cout << "rBuffer length => " << strlen(rBuffer) << endl;
	//*/

	// 创建目录
	/*
	MyCreateDirectory MCD = (MyCreateDirectory)GetProcAddress(h, "MyCreateDirectory");
	res = MCD("c:\\", "huang");
	cout << "MyCreateDirectory => return " << res << endl;
	//*/

	// 删除目录
	/*
	MyDeleteDirectory MCD = (MyDeleteDirectory)GetProcAddress(h, "MyDeleteDirectory");
	res = MCD("c:\\", "huang");
	cout << "MyDeleteDirectory => return " << res << endl;
	//*/
	FreeLibrary(h);
	return 0;
}