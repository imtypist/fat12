#pragma once
#include <windows.h>

#define MY_FILE_BEGIN           0
#define MY_FILE_CURRENT         1
#define MY_FILE_END             2

#ifdef __cplusplus
extern "C" {
#endif

	DWORD MyCreateFile(char *pszFolderPath, char *pszFileName);

	DWORD MyOpenFile(char *pszFolderPath, char *pszFileName);

	void MyCloseFile(DWORD dwHandle);

	BOOL MyDeleteFile(char *pszFolderPath, char *pszFileName);

	DWORD MyWriteFile(DWORD dwHandle, LPVOID pBuffer, DWORD dwBytesToWrite);

	DWORD MyReadFile(DWORD dwHandle, LPVOID pBuffer, DWORD dwBytesToRead);

	BOOL MyCreateDirectory(char *pszFolderPath, char *pszFolderName);

	BOOL MyDeleteDirectory(char *pszFolderPath, char *pszFolderName);

	BOOL MySetFilePointer(DWORD dwFileHandle, int nOffset, DWORD dwMoveMethod);

#ifdef __cplusplus
}
#endif