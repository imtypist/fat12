#pragma once
#include <iostream>
#include "DiskLib.h"
#include "disk.h"
#include "dostime.h"

#ifdef DLL_API  
#else  
#define DLL_API __declspec(dllexport)  
#endif

#ifdef __cplusplus
extern "C" {
#endif

DLL_API DWORD MyCreateFile(char *pszFolderPath, char *pszFileName);
/*
要求：在指定目录下创建指定文件，如果该文件不存在的话。如果目录不存在或文件已存在，则函数失败返回0；
如果成功则返回一个表示该文件的标识（类似于Window的句柄，内部数据结构及映射方法你来定）
pszFolderPath：目录路径，如"C:\\Test\\Test01"等等
pszFileName：文件名，如"Test.txt"等等
*/

DLL_API DWORD MyOpenFile(char *pszFolderPath, char *pszFileName);
/*
要求：打开指定目录下的指定文件，如果目录不存在或者文件不存在，则返回0表示失败；
如果成功则返回一个表示该文件的标识（类似于Window的句柄，内部数据结构及映射方法你来定）
pszFolderPath：目录路径，如"C:\\Test\\Test01"等等
pszFileName：文件名，如"Test.txt"等等
*/

DLL_API void MyCloseFile(DWORD dwHandle);
/*
要求：关闭该文件；
传入参数：
dwHandle：传入标识该文件的句柄，就是MyCreateFile返回的那个
*/

DLL_API BOOL MyDeleteFile(char *pszFolderPath, char *pszFileName);
/*
要求：删除指定目录下的指定文件，如果目录不存在或者文件不存在，则返回0表示失败，否则返回TRUE表示成功；
传入参数：
pszFolderPath：目录路径，如"C:\\Test\\Test01"等等
pszFileName：文件名，如"Test.txt"等等
*/

DLL_API DWORD MyWriteFile(DWORD dwHandle, LPVOID pBuffer, DWORD dwBytesToWrite);
/*
要求：将pBuffer中dwBytesToWrite长度的数据写入指定文件的文件指针位置。
传入参数：
dwHandle：MyOpenFile返回的值，在这个函数中又原封不动的传给你，其内部数据结构由你来定。
pBuffer：指向待写入数据的缓冲区
dwBytesToWrite：待写入数据的长度
返回值：成功写入的长度，-1表示失败。
*/

DLL_API DWORD MyReadFile(DWORD dwHandle, LPVOID pBuffer, DWORD dwBytesToRead);
/*
要求：读取指定文件中、指定长度的数据到传入的缓冲区。
传入参数：
dwHandle：MyOpenFile返回的值，在这个函数中又原封不动的传给你，其内部数据结构由你来定。
pBuffer：指向接收数据的缓冲区
dwBytesToRead：待读取数据的长度
返回值：成功读取的长度，-1表示失败。
*/

DLL_API BOOL MyCreateDirectory(char *pszFolderPath, char *pszFolderName);
/*
要求：在指定路径下，创建指定名称的文件夹。如果目录不存在或待创建的文件夹已存在，则返回FALSE。
创建成功返回TRUE；
传入参数：
pszFolderPath：目录路径，如"C:\\Test\\Test01"等等
pszFolderName：文件夹名称，如"MyFolder"等等
返回值：如果目录不存在或待创建的文件夹已存在，则返回FALSE。创建成功返回TRUE；
*/

DLL_API BOOL MyDeleteDirectory(char *pszFolderPath, char *pszFolderName);
/*
要求：在指定路径下，删除指定名称的文件夹。如果目录不存在或待删除的文件夹不存在，则返回FALSE。
删除成功返回TRUE；
传入参数：
pszFolderPath：目录路径，如"C:\\Test\\Test01"等等
pszFolderName：文件夹名称，如"MyFolder"等等
返回值：如果目录不存在或待删除的文件夹不存在，则返回FALSE。删除成功返回TRUE；
*/

DLL_API BOOL MySetFilePointer(DWORD dwFileHandle, int nOffset, DWORD dwMoveMethod);
/*
用途：移动指定文件的文件头，读写不同位置。如果文件句柄不存在，返回FALSE，否则返回TRUE
dwHandle：MyOpenFile返回的值，在这个函数中又原封不动的传给你，其内部数据结构由你来定。
nOffset：32位偏移量，可正可负可为零。
dwMoveMethod：偏移的起始位置，有如下三种类型可选：
MY_FILE_BEGIN：从头部开始计算偏移
MY_FILE_CURRENT：从当前磁头位置开始计算偏移
MY_FILE_END：从末尾开始计算偏移
*/

#ifdef __cplusplus
}
#endif