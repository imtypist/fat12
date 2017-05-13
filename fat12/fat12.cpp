#include "fat12.h"
#include <iostream>
#pragma comment(lib, "DiskLib.lib")
using namespace std;

const char* fs = "d:\\floppy.img";

BPB bpb;
BPB* bpb_ptr = &bpb;

RootEntry rootEntry;
RootEntry* rootEntry_ptr = &rootEntry;

DWORD MyCreateFile(char *pszFolderPath, char *pszFileName) {
	if (initBPB()) {
		isFileExist(pszFolderPath, pszFileName);
	}
	ShutdownDisk();
	return 0;
}

BOOL initBPB() {
	if (StartupDisk(fs)) {
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
			if (bpb_ptr->BPB_TotSec16 != 0) {
				TotSec = bpb_ptr->BPB_TotSec16;
			}
			else {
				TotSec = bpb_ptr->BPB_TotSec32;
			}
			FATSz = bpb_ptr->BPB_FATSz16;
			/*
			cout << "每扇区字节数：" << BytsPerSec << endl; // 512
			cout << "每簇扇区数：" << SecPerClus << endl; // 1
			cout << "Boot记录占用的扇区数：" << RsvdSecCnt << endl; // 1
			cout << "FAT表个数：" << NumFATs << endl; // 2
			cout << "根目录最大文件数：" << RootEntCnt << endl; // 224
			cout << "扇区总数：" << TotSec << endl; // 2880
			cout << "每FAT扇区数：" << FATSz << endl; // 9
			*/
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

BOOL isFileExist(char *pszFolderPath, char *pszFileName) {
	int rootBase = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec;
	int i;
	char filename[13];
	for (i = 0; i < RootEntCnt; i++) {
		SetHeaderOffset(rootBase, NULL, FILE_BEGIN);
		if (ReadFromDisk(rootEntry_ptr, 32, NULL) != 0) {
			if (rootEntry_ptr->DIR_Name[0] == '\0' || ((rootEntry_ptr->DIR_Name[0]^0xe5) == 0))
				continue;
			// 目录0x10，文件0x20，卷标0x28
			int j;
			if ((rootEntry_ptr->DIR_Attr^0x10) == 0) {
				for (j = 0; j < 13; j++) {
					if (rootEntry_ptr->DIR_Name[j] != ' ') {
						filename[j] = rootEntry_ptr->DIR_Name[j];
					}
					else {
						filename[j] = '\0';
						break;
					}
				}
				cout << "目录:" << filename << endl;
			}
			else if((rootEntry_ptr->DIR_Attr^0x20) == 0) {
				int len = 0;
				for (j = 0; j < 13; j++) {
					if (rootEntry_ptr->DIR_Name[j] != ' ') {
						filename[len] = rootEntry_ptr->DIR_Name[j];
						len++;
					}
					else {
						filename[len] = '.';
						while (rootEntry_ptr->DIR_Name[j] == ' ') j++;
						j--;
					}
				}
				filename[++len] = '\0';
				cout << "文件:" << filename << endl;
			}
			else if ((rootEntry_ptr->DIR_Attr^0x28) == 0) {
				for (j = 0; j < 13; j++) {
					if (rootEntry_ptr->DIR_Name[j] != ' ') {
						filename[j] = rootEntry_ptr->DIR_Name[j];
					}
					else {
						filename[j] = '\0';
						break;
					}
				}
				cout << "卷标:" << filename << endl;
			}
		}
		rootBase += 32;
	}
	return TRUE;
}