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

	// 打开文件
	//*
	MyOpenFile MOF = (MyOpenFile)GetProcAddress(h, "MyOpenFile");
	res = MOF("c:\\", "test1.txt");
	cout << "MyOpenFile => return " << res << endl;
	//*/
	// 移动文件指针
	//*
	MySetFilePointer MFP = (MySetFilePointer)GetProcAddress(h, "MySetFilePointer");
	MFP(res, 0, MY_FILE_END);
	//*/

	// 写文件
	//*
	MyWriteFile MWF = (MyWriteFile)GetProcAddress(h, "MyWriteFile");
	char pBuffer[512] = { 0 };
	for (int i = 0; i < 512; i++) {
		pBuffer[i] = 'c';
	}
	res = MWF(res, &pBuffer, 512);
	cout << "MyWriteFile => return " << res << endl;
	//*/

	// 读文件
	/*
	MyReadFile MRF = (MyReadFile)GetProcAddress(h, "MyReadFile");
	char rBuffer[32] = { 0 };
	res = MRF(res, &rBuffer, 30);
	cout << "MyReadFile => return " << res << endl;
	cout << "rBuffer => " << rBuffer << endl;
	/*/
	FreeLibrary(h);
	return 0;
}