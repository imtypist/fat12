#pragma once
#include <windows.h>

#define MY_FILE_BEGIN           0
#define MY_FILE_CURRENT         1
#define MY_FILE_END             2

#ifdef __cplusplus
extern "C" {
#endif

/*
BOOL StartupDisk(LPCTSTR lpszFile);
用途：启动硬盘。该API必须先于所有其他API调用，传入已存在路径下的文件名。
	如果该文件名已存在，则将其作为文件系统打开；
	如果不存在，则创建该文件。
	该文件用于存储你实现的FAT12文件系统，必须符合规定的FAT12文件格式。
	测评时我们会对该文件格式进行检查。
传入参数：文件路径，注意“\”要用"\\"转义。
返回值：成功返回TRUE，失败返回FALSE；
注意：传入参数中，文件路径必须存在，文件名可以存在（表示打开一个已存在的文件系统），也可以不存在（表示新建一个空的文件系统）
*/
BOOL StartupDisk(LPCTSTR lpszFile);

int StartupDiskA(LPCSTR lpszFile);

int StartupDiskW(LPCWSTR lpszFile);

/*
void ShutdownDisk();
用途：关闭硬盘。该API在完成对文件系统的操作后调用
*/
void ShutdownDisk();

/*
ReadFromDisk(LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead)
用途：从文件系统中读取指定长度的数据。
传入参数：
	lpBuffer：接受数据的缓冲区，由你创建好后传入
	nNumberOfBytesToRead：要读取的数据长度，小于等于你创建的缓冲区大小
	lpNumberOfBytesRead：指针，指向一个DWORD类型变量，表示实际读到的数据【可选，不需要的话设为NULL就好】
返回值：零表示失败，非零表示成功
*/
int ReadFromDisk(LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead);

/*
int WriteToDisk(LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten)
用途：向虚拟磁盘中写入指定长度的数据。
传入参数：
lpBuffer：待写入数据所在的缓冲区
nNumberOfBytesToWrite：待写入数据的长度
lpNumberOfBytesWritten：指针，指向一个DWORD类型变量，表示实际写入的数据【可选，不需要的话设为NULL就好】
返回值：
非零表示成功，零表示失败。
*/
int WriteToDisk(LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten);

/*
int SetHeaderOffset(LONG lDistanceToMove, PLONG lpDistanceToMoveHigh, DWORD dwMoveMethod);
用途：移动磁头，读写不同位置
lDistanceToMove:DWORD类型，用于指定偏移位置。如果下一个参数为NULL，则该参数单独指定32位有符号数的偏移范围；如果下一个参数不为NULL，则该参数和其一起用于指定64位偏移中的低32位。
lpDistanceToMoveHigh：指向一个DWORD类型，如果为NULL，则上一个参数单独指定32位偏移范围，如果该参数不为NULL，则与前一个参数共同指定一个64位的偏移。
dwMoveMethod：偏移的起始位置，有如下三种类型可选：
FILE_BEGIN：从头部开始计算偏移
FILE_CURRENT：从当前磁头位置开始计算偏移
FILE_END：从末尾开始计算偏移
*/
int SetHeaderOffset(LONG lDistanceToMove, PLONG lpDistanceToMoveHigh, DWORD dwMoveMethod);

#ifdef _UNICODE
#define StartupDisk  StartupDiskW
#else
#define StartupDisk  StartupDiskA
#endif 

#ifdef __cplusplus
}
#endif

