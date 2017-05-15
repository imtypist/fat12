#include "fat12.h"
#pragma comment(lib, "DiskLib.lib")
using namespace std;

const char* fs = "d:\\floppy.img";

BPB bpb;
BPB* bpb_ptr = &bpb;

RootEntry rootEntry;
RootEntry* rootEntry_ptr = &rootEntry;

DWORD MyCreateFile(char *pszFolderPath, char *pszFileName) {
	DWORD FileHandle = 0;
	u16 FstClus;
	if (initBPB()) {
		if (FstClus = isPathExist(pszFolderPath)) {
			if (isFileExist(pszFileName, FstClus)) {
				cout << pszFolderPath << '\\' << pszFileName << " has existed!" << endl;
			}
		}
	}
	ShutdownDisk();
	return FileHandle;
}

BOOL initBPB() {
	if (StartupDisk(fs)) {
		cout << "start up disk..." << endl;
		//����BPB,ƫ��11���ֽڶ�ȡ
		SetHeaderOffset(11, NULL, FILE_BEGIN);
		if (ReadFromDisk(bpb_ptr, 25, NULL) != 0) {
			//��ʼ������ȫ�ֱ���  
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
			cout << "ÿ�����ֽ�����" << BytsPerSec << endl; // 512
			cout << "ÿ����������" << SecPerClus << endl; // 1
			cout << "Boot��¼ռ�õ���������" << RsvdSecCnt << endl; // 1
			cout << "FAT�������" << NumFATs << endl; // 2
			cout << "��Ŀ¼����ļ�����" << RootEntCnt << endl; // 224
			cout << "����������" << TotSec << endl; // 2880
			cout << "ÿFAT��������" << FATSz << endl; // 9
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

BOOL isFileExist(char *pszFileName, u16 FstClus) {
	char filename[13];
	int dataBase;
	BOOL isExist = FALSE;
	// ������ǰĿ¼������Ŀ
	do {
		if (FstClus == 0) {
			// ��Ŀ¼��ƫ��
			dataBase = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec;
		}
		else {
			// �������ļ���ַƫ��
			dataBase = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec + RootEntCnt * 32 + (FstClus - 2) * BytsPerSec;
		}
		for (int i = 0; i < RootEntCnt; i++) {
			SetHeaderOffset(dataBase, NULL, FILE_BEGIN);
			if (ReadFromDisk(rootEntry_ptr, 32, NULL) != 0) {
				// Ŀ¼0x10���ļ�0x20�����0x28
				int len_of_filename = 0;
				if (rootEntry_ptr->DIR_Attr == 0x20) {
					for (int j = 0; j < 11; j++) {
						if (rootEntry_ptr->DIR_Name[j] != ' ') {
							filename[len_of_filename++] = rootEntry_ptr->DIR_Name[j];
						}
						else {
							filename[len_of_filename++] = '.';
							while (rootEntry_ptr->DIR_Name[j] == ' ') j++;
							j--;
						}
					}
					filename[len_of_filename] = '\0';
					if (strcmp(filename, pszFileName) == 0) {
						isExist = TRUE;
						break;
					}
				}
			}
			dataBase += 32;
		}
		if (isExist) break;
	} while ((FstClus = getFATValue(FstClus)) == 0xFFF || FstClus == 0);
	return isExist;
}

u16 isDirectoryExist(char *FolderName, u16 FstClus) {
	char directory[12];
	int dataBase;
	u16 isExist = 0;
	// ������ǰĿ¼������Ŀ
	do {
		if (FstClus == 0) {
			// ��Ŀ¼��ƫ��
			dataBase = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec;
		}
		else {
			// �������ļ���ַƫ��
			dataBase = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec + RootEntCnt * 32 + (FstClus - 2) * BytsPerSec;
		}
		for (int i = 0; i < RootEntCnt; i++) {
			SetHeaderOffset(dataBase, NULL, FILE_BEGIN);
			if (ReadFromDisk(rootEntry_ptr, 32, NULL) != 0) {
				// Ŀ¼0x10���ļ�0x20�����0x28
				if (rootEntry_ptr->DIR_Attr == 0x10) {
					for (int j = 0; j < 11; j++) {
						if (rootEntry_ptr->DIR_Name[j] != ' ') {
							directory[j] = rootEntry_ptr->DIR_Name[j];
							if (j == 10) {
								directory[11] = '\0';
								break;
							}
						}
						else {
							directory[j] = '\0';
							break;
						}
					}
					if (strcmp(directory, FolderName) == 0) {
						isExist = rootEntry_ptr->DIR_FstClus;
						break;
					}
				}
			}
			dataBase += 32;
		}
		if (isExist) break;
	} while ((FstClus = getFATValue(FstClus)) == 0xFFF || FstClus == 0);
	return isExist;
}

u16 isPathExist(char *pszFolderPath) {
	char directory[12]; // ���Ŀ¼��
	u16 FstClus = 0;
	/* ��3��ʼ�������̷�C:\\ */
	int i = 3, len = 0;
	while (pszFolderPath[i] != '\0') {
		if (pszFolderPath[i] == '\\') {
			directory[len] = '\0';
			//cout << directory << endl;
			if (FstClus = isDirectoryExist(directory, FstClus)) {
				len = 0;
			}
			else {
				len = 0;
				break;
			}
			i++;
		}
		else {
			directory[len++] = pszFolderPath[i++];
		}
	}
	if (len > 0) {
		directory[len] = '\0';
		//cout << directory << endl;
		FstClus = isDirectoryExist(directory, FstClus);
	}
	return FstClus;
}

u16 getFATValue(u16 FstClus) {
	//FAT1��ƫ���ֽ�  
	int fatBase = RsvdSecCnt * BytsPerSec;
	//FAT���ƫ���ֽ�  
	int fatPos = fatBase + FstClus * 3 / 2;
	//��żFAT���ʽ��ͬ��������д�����0��FAT�ʼ  
	int type;
	if (FstClus % 2 == 0) {
		type = 0;
	}
	else {
		type = 1;
	}
	//�ȶ���FAT�����ڵ������ֽ�  
	u16 bytes;
	u16* bytes_ptr = &bytes;
	SetHeaderOffset(fatPos, NULL, FILE_BEGIN);
	if (ReadFromDisk(bytes_ptr, 2, NULL) != 0) {
		//u16Ϊshort����ϴ洢��Сβ˳���FAT��ṹ���Եõ�  
		//typeΪ0�Ļ���ȡbyte2�ĵ�4λ��byte1���ɵ�ֵ��typeΪ1�Ļ���ȡbyte2��byte1�ĸ�4λ���ɵ�ֵ  
		if (type == 0) {
			return bytes << 4;
		}
		else {
			return bytes >> 4;
		}
	}
}