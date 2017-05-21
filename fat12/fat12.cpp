#include "fat12.h"
#pragma comment(lib, "DiskLib.lib")
using namespace std;

const char* fs = "d:\\floppy.img";

FileHandle* dwHandles[MAX_NUM] = { NULL };

BPB bpb;
BPB* bpb_ptr = &bpb;

RootEntry rootEntry;
RootEntry* rootEntry_ptr = &rootEntry;

DWORD MyCreateFile(char *pszFolderPath, char *pszFileName) {
	DWORD FileHandle = 0;
	u16 FstClus;
	u32 FileSize = 0; // 初始值为0
	RootEntry FileInfo;
	RootEntry* FileInfo_ptr = &FileInfo;
	if (initBPB()) {
		// 路径存在或者为根目录
		if ((FstClus = isPathExist(pszFolderPath)) || strlen(pszFolderPath) == 3) {
			if (isFileExist(pszFileName, FstClus)) {
				cout << pszFolderPath << '\\' << pszFileName << " has existed!" << endl;
			}
			else {
				initFileInfo(FileInfo_ptr, pszFileName, 0x20, FileSize);
				if (writeEmptyClus(FstClus, FileSize, FileInfo_ptr) == TRUE) {
					// 创建句柄
					FileHandle = createHandle(FileInfo_ptr);
				}
			}
		}
	}
	ShutdownDisk();
	return FileHandle;
}

DWORD MyOpenFile(char *pszFolderPath, char *pszFileName) {
	DWORD FileHandle = 0;
	u16 FstClus;
	BOOL isExist = FALSE;
	char filename[13];
	RootEntry FileInfo;
	RootEntry* FileInfo_ptr = &FileInfo;
	if (initBPB()) {
		if (FstClus = isPathExist(pszFolderPath) || strlen(pszFolderPath) == 3) {
			if (isFileExist(pszFileName, FstClus)) {
				int dataBase;
				do {
					if (FstClus == 0) {
						// 根目录区偏移
						dataBase = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec;
					}
					else {
						// 数据区文件首址偏移
						dataBase = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec + RootEntCnt * 32 + (FstClus - 2) * BytsPerSec;
					}
					for (int i = 0; i < RootEntCnt; i++) {
						SetHeaderOffset(dataBase, NULL, FILE_BEGIN);
						if (ReadFromDisk(FileInfo_ptr, 32, NULL) != 0) {
							// 目录0x10，文件0x20，卷标0x28
							int len_of_filename = 0;
							if (FileInfo_ptr->DIR_Attr == 0x20) {
								for (int j = 0; j < 11; j++) {
									if (FileInfo_ptr->DIR_Name[j] != ' ') {
										filename[len_of_filename++] = FileInfo_ptr->DIR_Name[j];
									}
									else {
										filename[len_of_filename++] = '.';
										while (FileInfo_ptr->DIR_Name[j] == ' ') j++;
										j--;
									}
								}
								filename[len_of_filename] = '\0';
								// 忽略大小写比较
								if (_stricmp(filename, pszFileName) == 0) {
									isExist = TRUE;
									break;
								}
							}
						}
						dataBase += 32;
					}
					if (isExist) break;
				} while ((FstClus = getFATValue(FstClus)) == 0xFFF || FstClus == 0);
				FileHandle = createHandle(FileInfo_ptr);
			}
		}
	}
	ShutdownDisk();
	return FileHandle;
}

void MyCloseFile(DWORD dwHandle) {
	free(dwHandles[dwHandle]);
	dwHandles[dwHandle] = NULL;
}

BOOL MyDeleteFile(char *pszFolderPath, char *pszFileName) {
	BOOL result = FALSE;
	u16 FstClus;
	char filename[13];
	RootEntry FileInfo;
	RootEntry* FileInfo_ptr = &FileInfo;
	if (initBPB()) {
		if (FstClus = isPathExist(pszFolderPath) || strlen(pszFolderPath) == 3) {
			if (isFileExist(pszFileName, FstClus)) {
				int dataBase;
				do {
					if (FstClus == 0) {
						// 根目录区偏移
						dataBase = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec;
					}
					else {
						// 数据区文件首址偏移
						dataBase = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec + RootEntCnt * 32 + (FstClus - 2) * BytsPerSec;
					}
					for (int i = 0; i < RootEntCnt; i++) {
						SetHeaderOffset(dataBase, NULL, FILE_BEGIN);
						if (ReadFromDisk(FileInfo_ptr, 32, NULL) != 0) {
							// 目录0x10，文件0x20，卷标0x28
							int len_of_filename = 0;
							if (FileInfo_ptr->DIR_Attr == 0x20) {
								for (int j = 0; j < 11; j++) {
									if (FileInfo_ptr->DIR_Name[j] != ' ') {
										filename[len_of_filename++] = FileInfo_ptr->DIR_Name[j];
									}
									else {
										filename[len_of_filename++] = '.';
										while (FileInfo_ptr->DIR_Name[j] == ' ') j++;
										j--;
									}
								}
								filename[len_of_filename] = '\0';
								// 忽略大小写比较
								if (_stricmp(filename, pszFileName) == 0) {
									// 上面读取了32字节，复位一下，第一字节写入0xe5
									SetHeaderOffset(dataBase, NULL, FILE_BEGIN);
									u8 del = 0xE5;
									if (WriteToDisk(&del, 1, NULL) != 0) {
										// 回收簇
										u16 fileClus = FileInfo_ptr->DIR_FstClus; // 首簇
										u16 bytes;
										u16* bytes_ptr = &bytes;
										// 下一簇为末尾簇退出循环
										while (fileClus != 0xFFF) {
											int clusBase = RsvdSecCnt * BytsPerSec + fileClus * 3 / 2;
											SetHeaderOffset(clusBase, NULL, FILE_BEGIN);
											u16 tempClus = getFATValue(fileClus); // 暂存下一簇，当前簇内容刷新成0
											if (ReadFromDisk(bytes_ptr, 2, NULL) != 0) {
												if (fileClus % 2 == 0) {
													bytes = bytes >> 12;
													bytes = bytes << 12; // 低12位置0
												}
												else {
													bytes = bytes << 12;
													bytes = bytes >> 12; // 高12位置0
												}
												SetHeaderOffset(clusBase, NULL, FILE_BEGIN);
												WriteToDisk(bytes_ptr, 2, NULL); // 写回，回收该簇
											}
											fileClus = tempClus; // 更新偏移量
										}
										result = TRUE;
										break;
									}
								}
							}
						}
						dataBase += 32;
					}
					if (result) break;
				} while ((FstClus = getFATValue(FstClus)) == 0xFFF || FstClus == 0);
			}
		}
	}
	ShutdownDisk();
	return result;
}

DWORD MyWriteFile(DWORD dwHandle, LPVOID pBuffer, DWORD dwBytesToWrite) {
	DWORD result = 0;
	FileHandle* hd = dwHandles[dwHandle];
	if (hd == NULL) return -1;
	u16 FstClus = hd->fileInfo.DIR_FstClus;
	LONG offset = hd->offset; // 文件指针当前偏移
	int curClusNum = offset / BytsPerSec; // 当前指针在第几个扇区
	int curClusOffset = offset % BytsPerSec; // 当前在扇区内偏移
	while (curClusNum) {
		FstClus = getFATValue(FstClus);
		curClusNum--;
	}// 获取当前指针所指扇区
	int dataBase = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec + RootEntCnt * 32 + (FstClus - 2) * BytsPerSec;
	int dataOffset = dataBase + curClusOffset; // 拿到文件指针所指位置
	int lenOfBuffer = dwBytesToWrite; // 缓冲区待写入长度
	char* cBuffer = (char*)malloc(sizeof(u8)*lenOfBuffer);
	memcpy(cBuffer, pBuffer, lenOfBuffer); // 复制过来
	SetHeaderOffset(dataOffset, NULL, FILE_BEGIN);
	if (BytsPerSec - offset >= lenOfBuffer) {
		if (WriteToDisk(pBuffer, lenOfBuffer, &result) == 0) {
			return -1;
		}
	}
	else {
		DWORD temp;
		u16 tempClus;
		u16 bytes; // 每次读取的簇号
		u16* bytes_ptr = &bytes;
		int fatBase = RsvdSecCnt * BytsPerSec;
		if (WriteToDisk(pBuffer, BytsPerSec - offset, &temp) == 0) {
			return -1;
		}
		result += temp; // 记录写入长度
		int leftLen = lenOfBuffer - (BytsPerSec - offset); // 剩余长度
		int hasWritten = BytsPerSec - offset;
		do {
			tempClus = getFATValue(FstClus); // 尝试拿下一个FAT
			if (tempClus == 0xFFF) {
				tempClus = setFATValue(1);
				SetHeaderOffset((fatBase + FstClus * 3 / 2), NULL, FILE_BEGIN);
				if (ReadFromDisk(bytes_ptr, 2, NULL) != 0) {
					if (FstClus % 2 == 0) {
						bytes = bytes >> 12;
						bytes = bytes << 12; // 保留高四位，低12位为0
						bytes = bytes | tempClus;
					}
					else {
						bytes = bytes << 12;
						bytes = bytes >> 12; // 保留低四位，高12位为0
						tempClus = tempClus << 4;
						bytes = bytes | tempClus;
					}
					SetHeaderOffset((fatBase + FstClus * 3 / 2), NULL, FILE_BEGIN);
					if (WriteToDisk(bytes_ptr, 2, NULL) == 0) {
						return -1;
					}
				}
			}
			FstClus = getFATValue(FstClus); // 真正拿到下一个FAT
			dataBase = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec + RootEntCnt * 32 + (FstClus - 2) * BytsPerSec; // 刷新扇区偏移
			SetHeaderOffset(dataOffset, NULL, FILE_BEGIN);
			if (leftLen > BytsPerSec) {
				if (WriteToDisk(&cBuffer[hasWritten - 1], BytsPerSec, &temp) == 0) {
					return -1;
				}
				hasWritten += BytsPerSec;
			}
			else {
				if (WriteToDisk(&cBuffer[hasWritten - 1], leftLen, &temp) == 0) {
					return -1;
				}
				hasWritten += leftLen;
			}
			leftLen -= BytsPerSec;
			result += temp;
		} while (leftLen <= 0);
	}
	return result;
}

DWORD MyReadFile(DWORD dwHandle, LPVOID pBuffer, DWORD dwBytesToRead) {
	DWORD result = 0;
	FileHandle* hd = dwHandles[dwHandle];
	if (hd == NULL) return -1;
	u16 FstClus = hd->fileInfo.DIR_FstClus;
	LONG offset = hd->offset; // 文件指针当前偏移
	int curClusNum = offset / BytsPerSec; // 当前指针在第几个扇区
	int curClusOffset = offset % BytsPerSec; // 当前在扇区内偏移
	while (curClusNum) {
		FstClus = getFATValue(FstClus);
		curClusNum--;
	}// 获取当前指针所指扇区
	int dataBase = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec + RootEntCnt * 32 + (FstClus - 2) * BytsPerSec;
	int dataOffset = dataBase + curClusOffset; // 拿到文件指针所指位置
	int lenOfBuffer = dwBytesToRead; // 缓冲区待读入长度
	char* cBuffer = (char*)malloc(sizeof(u8)*lenOfBuffer); // 创建一个缓冲区
	SetHeaderOffset(dataOffset, NULL, FILE_BEGIN);
	// 读取
	if (BytsPerSec - offset >= lenOfBuffer) {
		if (ReadFromDisk(cBuffer, lenOfBuffer, &result) == 0) {
			return -1;
		}
	}
	else {
		DWORD temp;
		if (ReadFromDisk(cBuffer, BytsPerSec - offset, &temp) == 0) {
			return -1;
		}
		result += temp; // 记录读取到的长度
		int leftLen = lenOfBuffer - (BytsPerSec - offset); // 剩余长度
		int hasRead = BytsPerSec - offset;
		do {
			FstClus = getFATValue(FstClus); // 拿到下一个FAT
			if (FstClus == 0xFFF) {
				break;
			}
			dataBase = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec + RootEntCnt * 32 + (FstClus - 2) * BytsPerSec; // 刷新扇区偏移
			SetHeaderOffset(dataOffset, NULL, FILE_BEGIN);
			if (leftLen > BytsPerSec) {
				if (ReadFromDisk(&cBuffer[hasRead - 1], BytsPerSec, &temp) == 0) {
					return -1;
				}
				hasRead += BytsPerSec;
			}
			else {
				if (WriteToDisk(&cBuffer[hasRead - 1], leftLen, &temp) == 0) {
					return -1;
				}
				hasRead += leftLen;
			}
			leftLen -= BytsPerSec; // 直接减掉一个扇区，只要是<=0就退出循环
			result += temp;
		} while (leftLen <= 0);
	}
	memcpy(pBuffer, cBuffer, lenOfBuffer); // 写入缓冲区
	return result;
}

BOOL MyCreateDirectory(char *pszFolderPath, char *pszFolderName) {
	u16 FstClus;
	BOOL result = FALSE;
	int dataBase;
	if (initBPB()) {
		// 路径存在或者为根目录
		if ((FstClus = isPathExist(pszFolderPath)) || strlen(pszFolderPath) == 3) {
			if (isDirectoryExist(pszFolderName, FstClus)) {
				cout << pszFolderPath << '\\' << pszFolderName << " has existed!" << endl;
			}
			else {
				do {
					if (FstClus == 0) {
						// 根目录区偏移
						dataBase = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec;
					}
					else {
						// 数据区文件首址偏移
						dataBase = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec + RootEntCnt * 32 + (FstClus - 2) * BytsPerSec;
					}
					for (int i = 0; i < RootEntCnt; i++) {
						SetHeaderOffset(dataBase, NULL, FILE_BEGIN);
						if (ReadFromDisk(rootEntry_ptr, 32, NULL) != 0) {
							// 目录项可用
							if (rootEntry_ptr->DIR_Attr == 0x00 || rootEntry_ptr->DIR_Attr == 0xE5) {
								time_t ts = getTS();
								rootEntry_ptr->DIR_FstClus = setFATValue(1); // 为目录分配一个簇
								strcpy(rootEntry_ptr->DIR_Name, pszFolderName);
								rootEntry_ptr->DIR_Attr = 0x10; // 目录
								rootEntry_ptr->DIR_FileSize = BytsPerSec; // 1扇区
								rootEntry_ptr->DIR_WrtDate = getDOSDate(ts);
								rootEntry_ptr->DIR_WrtTime = getDOSTime(ts);
								SetHeaderOffset(dataBase, NULL, FILE_BEGIN); // 磁头复位
								if (WriteToDisk(rootEntry_ptr, 32, NULL) != 0) {
									result = TRUE;
									break;
								}
							}
						}
						dataBase += 32;
					}
					if (result) break;
				} while ((FstClus = getFATValue(FstClus)) == 0xFFF || FstClus == 0);
			}
		}
	}
	ShutdownDisk();
	return result;
}

BOOL MyDeleteDirectory(char *pszFolderPath, char *pszFolderName) {
	u16 FstClus;
	BOOL result = FALSE;
	char directory[12];
	int dataBase;
	if (initBPB()) {
		// 路径存在或者为根目录
		if ((FstClus = isPathExist(pszFolderPath)) || strlen(pszFolderPath) == 3) {
			// 待删除目录存在
			if (isDirectoryExist(pszFolderName, FstClus)) {
				do {
					if (FstClus == 0) {
						// 根目录区偏移
						dataBase = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec;
					}
					else {
						// 数据区文件首址偏移
						dataBase = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec + RootEntCnt * 32 + (FstClus - 2) * BytsPerSec;
					}
					for (int i = 0; i < RootEntCnt; i++) {
						SetHeaderOffset(dataBase, NULL, FILE_BEGIN);
						if (ReadFromDisk(rootEntry_ptr, 32, NULL) != 0) {
							// 目录0x10，文件0x20，卷标0x28
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
								// 忽略大小写比较
								if (_stricmp(directory, pszFolderName) == 0) {
									SetHeaderOffset(dataBase, NULL, FILE_BEGIN);
									u8 del = 0xE5;
									if (WriteToDisk(&del, 1, NULL) != 0) {
										// 回收簇
										u16 fileClus = rootEntry_ptr->DIR_FstClus; // 首簇
										u16 bytes;
										u16* bytes_ptr = &bytes;
										// 下一簇为末尾簇退出循环
										while (fileClus != 0xFFF) {
											int clusBase = RsvdSecCnt * BytsPerSec + fileClus * 3 / 2;
											SetHeaderOffset(clusBase, NULL, FILE_BEGIN);
											u16 tempClus = getFATValue(fileClus); // 暂存下一簇，当前簇内容刷新成0
											if (ReadFromDisk(bytes_ptr, 2, NULL) != 0) {
												if (fileClus % 2 == 0) {
													bytes = bytes >> 12;
													bytes = bytes << 12; // 低12位置0
												}
												else {
													bytes = bytes << 12;
													bytes = bytes >> 12; // 高12位置0
												}
												SetHeaderOffset(clusBase, NULL, FILE_BEGIN);
												WriteToDisk(bytes_ptr, 2, NULL); // 写回，回收该簇
											}
											fileClus = tempClus; // 更新偏移量
										}
										result = TRUE;
										break;
									}
								}
							}
						}
						dataBase += 32;
					}
					if (result) break;
				} while ((FstClus = getFATValue(FstClus)) == 0xFFF || FstClus == 0);
			}
		}
	}
	ShutdownDisk();
	return result;
}

BOOL MySetFilePointer(DWORD dwFileHandle, int nOffset, DWORD dwMoveMethod) {
	FileHandle* hd = dwHandles[dwFileHandle];
	if (hd == NULL) return FALSE; // 句柄不存在
	LONG curOffset = nOffset + hd->offset; // current模式下偏移后的位置
	u16 currentClus = hd->fileInfo.DIR_FstClus; // 首簇
	int fileSize = hd->fileInfo.DIR_FileSize; // 文件大小
	int fileBase = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec + RootEntCnt * 32 + (currentClus - 2) * BytsPerSec;
	switch (dwMoveMethod) {
	case MY_FILE_BEGIN:
		if (nOffset < 0) {
			hd->offset = 0; // 小于0，置为0
		}
		else if (nOffset > fileSize) {
			hd->offset = fileSize;
		}
		else {
			hd->offset = nOffset;
		}
		break;
	case MY_FILE_CURRENT:
		if (curOffset < 0) {
			hd->offset = 0;
		}
		else if (curOffset > fileSize) {
			hd->offset = fileSize;
		}
		else {
			hd->offset = curOffset;
		}
		break;
	case MY_FILE_END:
		if (nOffset > 0) {
			hd->offset = fileSize;
		}
		else if (nOffset < -fileSize) {
			hd->offset = 0;
		}
		else {
			hd->offset = fileSize - nOffset;
		}
		break;
	}
	return TRUE;
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

BOOL isFileExist(char *pszFileName, u16 FstClus) {
	char filename[13];
	int dataBase;
	BOOL isExist = FALSE;
	// 遍历当前目录所有项目
	do {
		if (FstClus == 0) {
			// 根目录区偏移
			dataBase = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec;
		}
		else {
			// 数据区文件首址偏移
			dataBase = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec + RootEntCnt * 32 + (FstClus - 2) * BytsPerSec;
		}
		for (int i = 0; i < RootEntCnt; i++) {
			SetHeaderOffset(dataBase, NULL, FILE_BEGIN);
			if (ReadFromDisk(rootEntry_ptr, 32, NULL) != 0) {
				// 目录0x10，文件0x20，卷标0x28
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
					// 忽略大小写比较
					if (_stricmp(filename, pszFileName) == 0) {
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
	// 遍历当前目录所有项目
	do {
		if (FstClus == 0) {
			// 根目录区偏移
			dataBase = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec;
		}
		else {
			// 数据区文件首址偏移
			dataBase = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec + RootEntCnt * 32 + (FstClus - 2) * BytsPerSec;
		}
		for (int i = 0; i < RootEntCnt; i++) {
			SetHeaderOffset(dataBase, NULL, FILE_BEGIN);
			if (ReadFromDisk(rootEntry_ptr, 32, NULL) != 0) {
				// 目录0x10，文件0x20，卷标0x28
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
					// 忽略大小写比较
					if (_stricmp(directory, FolderName) == 0) {
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
	char directory[12]; // 存放目录名
	u16 FstClus = 0;
	/* 从3开始，跳过盘符C:\\ */
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
	//FAT1的偏移字节  
	int fatBase = RsvdSecCnt * BytsPerSec;
	//FAT项的偏移字节  
	int fatPos = fatBase + FstClus * 3 / 2;
	//奇偶FAT项处理方式不同，分类进行处理，从0号FAT项开始  
	int type;
	if (FstClus % 2 == 0) {
		type = 0;
	}
	else {
		type = 1;
	}
	//先读出FAT项所在的两个字节  
	u16 bytes;
	u16* bytes_ptr = &bytes;
	SetHeaderOffset(fatPos, NULL, FILE_BEGIN);
	if (ReadFromDisk(bytes_ptr, 2, NULL) != 0) {
		//u16为short，结合存储的小尾顺序和FAT项结构可以得到  
		//type为0的话，取byte2的低4位和byte1构成的值，type为1的话，取byte2和byte1的高4位构成的值  
		if (type == 0) {
			// 注意移回来，要不然扩大了
			bytes = bytes << 4;
			bytes = bytes >> 4;
			return bytes;
		}
		else {
			bytes = bytes >> 4;
			return bytes;
		}
	}
}

void initFileInfo(RootEntry* FileInfo_ptr, char* FileName, u8 FileAttr, u32 FileSize) {
	time_t ts = getTS();
	FileInfo_ptr->DIR_Attr = FileAttr;
	FileInfo_ptr->DIR_WrtDate = getDOSDate(ts);
	FileInfo_ptr->DIR_WrtTime = getDOSTime(ts);
	if (FileAttr == 0x10) {
		FileInfo_ptr->DIR_FileSize = BytsPerSec;
		strcpy(FileInfo_ptr->DIR_Name, FileName);
	}
	else {
		FileInfo_ptr->DIR_FileSize = FileSize;
		int i = 0;
		while (FileName[i] != '\0') {
			if (FileName[i] == '.') {
				i++;
				break;
			}
			else {
				FileInfo_ptr->DIR_Name[i] = FileName[i];
				i++;
			}

		}
		memcpy(&FileInfo_ptr->DIR_Name[8], &FileName[i], 3);
	}
	int clusNum = FileSize / BytsPerSec + 1;
	FileInfo_ptr->DIR_FstClus = setFATValue(clusNum);
}

BOOL writeEmptyClus(u16 FstClus, u32 FileSize, RootEntry* FileInfo) {
	int dataBase;
	BOOL success = FALSE;
	do {
		if (FstClus == 0) {
			// 根目录区偏移
			dataBase = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec;
		}
		else {
			// 数据区文件首址偏移
			dataBase = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec + RootEntCnt * 32 + (FstClus - 2) * BytsPerSec;
		}
		for (int i = 0; i < RootEntCnt; i++) {
			SetHeaderOffset(dataBase, NULL, FILE_BEGIN);
			if (ReadFromDisk(rootEntry_ptr, 32, NULL) != 0) {
				// 说明该目录项可用
				if (rootEntry_ptr->DIR_Attr == 0x00 || rootEntry_ptr->DIR_Attr == 0xE5) {
					if (WriteToDisk(FileInfo, 32, NULL) != 0) {
						success = TRUE;
						break;
					}
				}
			}
			dataBase += 32;
		}
		if (success) break;
	} while ((FstClus = getFATValue(FstClus)) == 0xFFF || FstClus == 0);
	return success;
}

u16 setFATValue(int clusNum) {
	int fatBase = RsvdSecCnt * BytsPerSec;
	int fatPos = fatBase + 3; // 从2号簇开始查找，放空3字节
	//先读出FAT项所在的两个字节
	u16 clus = 2;
	int i = 0;
	u16 bytes; // 每次读取的簇号
	u16* bytes_ptr = &bytes;
	u16 FstClus;
	u16 preClus;
	do {
		SetHeaderOffset(fatPos, NULL, FILE_BEGIN);
		if (ReadFromDisk(bytes_ptr, 2, NULL) != 0) {
			// 簇号为偶数 
			if (clus % 2 == 0) {
				bytes = bytes << 4;
				bytes = bytes >> 4; // 这边不移回来也可以，反正都是0
			}
			else {
				bytes = bytes >> 4;
			}
			if (bytes == 0x000) {
				if (i > 0) {
					SetHeaderOffset((fatBase + preClus * 3 / 2), NULL, FILE_BEGIN);
					if (ReadFromDisk(bytes_ptr, 2, NULL) != 0) {
						if (preClus % 2 == 0) {
							bytes = bytes >> 12;
							bytes = bytes << 12; // 保留高四位，低12位为0
							bytes = bytes | clus; // 与当前clus按位或
						}
						else {
							bytes = bytes << 12;
							bytes = bytes >> 12; // 保留低四位，高12位为0
							bytes = bytes | (clus << 4);
						}
						SetHeaderOffset((fatBase + preClus * 3 / 2), NULL, FILE_BEGIN);
						WriteToDisk(bytes_ptr, 2, NULL);
					}
				}
				else {
					FstClus = clus; // 保存首簇
				}
				preClus = clus;
				if (clusNum == ++i) break; // 到尾簇退出循环
			}
		}
		fatPos++; // 往后偏一个字节
		clus++; // 簇号加一
	} while (TRUE);
	// 尾簇补0xfff
	SetHeaderOffset((fatBase + preClus * 3 / 2), NULL, FILE_BEGIN);
	if (ReadFromDisk(bytes_ptr, 2, NULL) != 0) {
		if (preClus % 2 == 0) {
			bytes = bytes >> 12;
			bytes = bytes << 12; // 保留高四位，低12位为0
			bytes = bytes | 0x0FFF;
		}
		else {
			bytes = bytes << 12;
			bytes = bytes >> 12; // 保留低四位，高12位为0
			bytes = bytes | 0xFFF0;
		}
		SetHeaderOffset((fatBase + preClus * 3 / 2), NULL, FILE_BEGIN);
		WriteToDisk(bytes_ptr, 2, NULL);
	}
	return FstClus;
}

DWORD createHandle(RootEntry* FileInfo) {
	int i;
	FileHandle* hd = (FileHandle*)malloc(sizeof(FileHandle)); // 统一在这里malloc
	for (i = 1; i < MAX_NUM; i++) {
		if (dwHandles[i] == NULL) {
			memcpy(&hd->fileInfo, FileInfo, 32);
			hd->offset = 0; // 偏移量初始化为0
			dwHandles[i] = hd;
			break;
		}
	}
	return i;
}