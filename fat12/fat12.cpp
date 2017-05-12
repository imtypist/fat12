#include "fat12.h"
#include <iostream>
#pragma comment(lib, "DiskLib.lib")
using namespace std;

const char* fs = "d:\\floppy.img";
struct BPB bpb;
struct BPB* bpb_ptr = &bpb;

DWORD MyCreateFile(char *pszFolderPath, char *pszFileName) {
	
	return 0;
}

BOOL init() {
	if (StartupDisk((LPCTSTR)fs)) {
		cout << "start up disk..." << endl;
		//载入BPB,偏移11个字节读取
		SetHeaderOffset(11, NULL, FILE_BEGIN);
		if (ReadFromDisk(bpb_ptr, 25, NULL) != 0) {
			//初始化各个全局变量  
			BytsPerSec = bpb_ptr->BPB_BytsPerSec;
			SecPerClus = bpb_ptr->BPB_SecPerClus;
			RsvdSecCnt = bpb_ptr->BPB_RsvdSecCnt;
			NumFATs = bpb_ptr->BPB_NumFATs;
			RootEntCnt = bpb_ptr->BPB_RootEntCnt;
			if (bpb_ptr->BPB_FATSz16 != 0) {
				FATSz = bpb_ptr->BPB_FATSz16;
			}
			else {
				FATSz = bpb_ptr->BPB_TotSec32;
			}
			return TRUE;
		}
		else {
			cout << "read BPB fail..." << endl;
		}
	}
	else {
		cout << "cannot start up disk..." << endl;
	}
	return FALSE;
}