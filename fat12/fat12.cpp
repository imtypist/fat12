#include "fat12.h"
#pragma comment(lib, "DiskLib.lib")
using namespace std;

const char* fs = "d:\\floppy.img";

FileHandle* dwHandles[MAX_NUM] = { NULL };

u8* setzero = (u8*)calloc(512, sizeof(u8)); // 用于创建目录时清0

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
	memset(FileInfo_ptr, 0, sizeof(RootEntry));
	if (initBPB()) {
		// 路径存在或者为根目录
		if ((FstClus = isPathExist(pszFolderPath)) || strlen(pszFolderPath) == 3) {
			if (isFileExist(pszFileName, FstClus)) {
				cout << pszFolderPath << '\\' << pszFileName << " has existed!" << endl;
			}
			else {
				initFileInfo(FileInfo_ptr, pszFileName, 0x20, FileSize);
				if (writeEmptyClus(FstClus, FileInfo_ptr) == TRUE) {
					// 创建句柄
					FileHandle = createHandle(FileInfo_ptr, FstClus);
				}
			}
		}
	}
	ShutdownDisk();
	return FileHandle;
}

DWORD MyOpenFile(char *pszFolderPath, char *pszFileName) {
	DWORD FileHandle = 0;
	u16 FstClus = 0;
	BOOL isExist = FALSE;
	char filename[13];
	RootEntry FileInfo;
	RootEntry* FileInfo_ptr = &FileInfo;
	if (initBPB()) {
		// fix bug:优先级，赋值记得加括号
		if ((FstClus = isPathExist(pszFolderPath)) || strlen(pszFolderPath) == 3) {
			u16 parentClus = FstClus;
			if (isFileExist(pszFileName, FstClus)) {
				int dataBase;
				do {
					int loop;
					if (FstClus == 0) {
						// 根目录区偏移
						dataBase = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec;
						loop = RootEntCnt;
					}
					else {
						// 数据区文件首址偏移
						dataBase = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec + RootEntCnt * 32 + (FstClus - 2) * BytsPerSec;
						loop = BytsPerSec / 32;
					}
					for (int i = 0; i < loop; i++) {
						SetHeaderOffset(dataBase, NULL, FILE_BEGIN);
						if (ReadFromDisk(FileInfo_ptr, 32, NULL) != 0) {
							// 目录0x10，文件0x20，卷标0x28
							if (FileInfo_ptr->DIR_Name[0] != 0xE5 && FileInfo_ptr->DIR_Name[0] != 0 && FileInfo_ptr->DIR_Name[0] != 0x2E) {
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
						}
						dataBase += 32;
					}
					if (isExist) { 
						FileHandle = createHandle(FileInfo_ptr, parentClus);
						break; 
					}
				} while ((FstClus = getFATValue(FstClus)) != 0xFFF && FstClus != 0);
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
		if ((FstClus = isPathExist(pszFolderPath)) || strlen(pszFolderPath) == 3) {
			if (isFileExist(pszFileName, FstClus)) {
				int dataBase;
				do {
					int loop;
					if (FstClus == 0) {
						// 根目录区偏移
						dataBase = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec;
						loop = RootEntCnt;
					}
					else {
						// 数据区文件首址偏移
						dataBase = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec + RootEntCnt * 32 + (FstClus - 2) * BytsPerSec;
						loop = BytsPerSec / 32;
					}
					for (int i = 0; i < loop; i++) {
						SetHeaderOffset(dataBase, NULL, FILE_BEGIN);
						if (ReadFromDisk(FileInfo_ptr, 32, NULL) != 0) {
							// 目录0x10，文件0x20，卷标0x28
							if (FileInfo_ptr->DIR_Name[0] != 0xE5 && FileInfo_ptr->DIR_Name[0] != 0 && FileInfo_ptr->DIR_Name[0] != 0x2E) {
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
												u16 tempClus = getFATValue(fileClus); // 暂存下一簇，当前簇内容刷新成0
												SetHeaderOffset(clusBase, NULL, FILE_BEGIN);
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
						}
						dataBase += 32;
					}
					if (result) break;
				} while ((FstClus = getFATValue(FstClus)) != 0xFFF && FstClus != 0);
			}
		}
	}
	ShutdownDisk();
	return result;
}

DWORD MyWriteFile(DWORD dwHandle, LPVOID pBuffer, DWORD dwBytesToWrite) {
	DWORD result = 0;
	FileHandle* hd = dwHandles[dwHandle];
	if (hd == NULL || initBPB() == FALSE) return -1;
	u16 FstClus = hd->fileInfo.DIR_FstClus;
	LONG offset = hd->offset; // 文件指针当前偏移
	int curClusNum = offset / BytsPerSec; // 当前指针在第几个扇区
	int curClusOffset = offset % BytsPerSec; // 当前在扇区内偏移
	while (curClusNum) {
		if (getFATValue(FstClus) == 0xFFF) {
			break;
		}
		FstClus = getFATValue(FstClus);
		curClusNum--;
	}// 获取当前指针所指扇区
	int dataBase = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec + RootEntCnt * 32 + (FstClus - 2) * BytsPerSec;
	int dataOffset = dataBase + curClusOffset; // 拿到文件指针所指位置
	int lenOfBuffer = dwBytesToWrite; // 缓冲区待写入长度
	char* cBuffer = (char*)malloc(sizeof(u8)*lenOfBuffer);
	memcpy(cBuffer, pBuffer, lenOfBuffer); // 复制过来
	SetHeaderOffset(dataOffset, NULL, FILE_BEGIN);
	if ((BytsPerSec - curClusOffset >= lenOfBuffer) && curClusNum == 0) {
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
		int leftLen = lenOfBuffer;
		int hasWritten = 0;
		if (curClusNum == 0) {
			if (WriteToDisk(pBuffer, BytsPerSec - curClusOffset, &temp) == 0) {
				return -1;
			}
			result += temp; // 记录写入长度
			leftLen = lenOfBuffer - (BytsPerSec - curClusOffset); // 剩余长度
			hasWritten = BytsPerSec - curClusOffset;
		}
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
						bytes = bytes | (tempClus << 4);
					}
					SetHeaderOffset((fatBase + FstClus * 3 / 2), NULL, FILE_BEGIN);
					if (WriteToDisk(bytes_ptr, 2, NULL) == 0) {
						return -1;
					}
				}
			}
			FstClus = tempClus; // 真正拿到下一个FAT
			dataBase = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec + RootEntCnt * 32 + (FstClus - 2) * BytsPerSec; // 刷新扇区偏移
			SetHeaderOffset(dataBase, NULL, FILE_BEGIN); // 一定是从扇区头开始写
			if (leftLen > BytsPerSec) {
				if (WriteToDisk(&cBuffer[hasWritten], BytsPerSec, &temp) == 0) {
					return -1;
				}
				hasWritten += BytsPerSec;
			}
			else {
				if (WriteToDisk(&cBuffer[hasWritten], leftLen, &temp) == 0) {
					return -1;
				}
				hasWritten += leftLen;
			}
			leftLen -= BytsPerSec;
			result += temp;
		} while (leftLen > 0);
	}
	// 刷新文件大小
	if ((offset + result) > hd->fileInfo.DIR_FileSize) {
		int dBase;
		BOOL isExist = FALSE;
		hd->fileInfo.DIR_FileSize += (offset + result) - hd->fileInfo.DIR_FileSize;
		// 遍历当前目录所有项目
		u16 parentClus = hd->parentClus;
		do {
			int loop;
			if (parentClus == 0) {
				dBase = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec;
				loop = RootEntCnt;
			}
			else {
				dBase = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec + RootEntCnt * 32 + (parentClus - 2) * BytsPerSec;
				loop = BytsPerSec / 32;
			}
			for (int i = 0; i < loop; i++) {
				SetHeaderOffset(dBase, NULL, FILE_BEGIN);
				if (ReadFromDisk(rootEntry_ptr, 32, NULL) != 0) {
					if (rootEntry_ptr->DIR_Attr == 0x20) {
						if (_stricmp(rootEntry_ptr->DIR_Name, hd->fileInfo.DIR_Name) == 0) {
							SetHeaderOffset(dBase, NULL, FILE_BEGIN);
							WriteToDisk(&hd->fileInfo, 32, NULL);
							isExist = TRUE;
							break;
						}
					}
				}
				dBase += 32;
			}
			if (isExist) break;
		} while ((parentClus = getFATValue(parentClus)) != 0xFFF && parentClus != 0);
	}
	ShutdownDisk();
	MySetFilePointer(dwHandle, result, MY_FILE_CURRENT); //偏移量刷新
	return result;
}

DWORD MyReadFile(DWORD dwHandle, LPVOID pBuffer, DWORD dwBytesToRead) {
	DWORD result = 0;
	FileHandle* hd = dwHandles[dwHandle];
	if (hd == NULL || initBPB() == FALSE) return -1;
	u16 FstClus = hd->fileInfo.DIR_FstClus;
	LONG offset = hd->offset; // 文件指针当前偏移
	int curClusNum = offset / BytsPerSec; // 当前指针在第几个扇区
	int curClusOffset = offset % BytsPerSec; // 当前在扇区内偏移
	while (curClusNum) {
		if (getFATValue(FstClus) == 0xFFF) {
			break;
		}
		FstClus = getFATValue(FstClus);
		curClusNum--;
	}// 获取当前指针所指扇区
	if (curClusNum > 0 || offset > hd->fileInfo.DIR_FileSize) return -1; // 超出文件偏移范围了
	int dataBase = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec + RootEntCnt * 32 + (FstClus - 2) * BytsPerSec;
	int dataOffset = dataBase + curClusOffset; // 拿到文件指针所指位置
	int lenOfBuffer = dwBytesToRead; // 缓冲区待读入长度
	if (hd->fileInfo.DIR_FileSize - offset < lenOfBuffer) {
		lenOfBuffer = hd->fileInfo.DIR_FileSize - offset;
	}
	char* cBuffer = (char*)malloc(sizeof(u8)*lenOfBuffer); // 创建一个缓冲区
	memset(cBuffer, 0, lenOfBuffer);
	SetHeaderOffset(dataOffset, NULL, FILE_BEGIN);
	// 读取
	if (BytsPerSec - curClusOffset >= lenOfBuffer) {
		if (ReadFromDisk(cBuffer, lenOfBuffer, &result) == 0) {
			return -1;
		}
	}
	else {
		DWORD temp;
		if (ReadFromDisk(cBuffer, BytsPerSec - curClusOffset, &temp) == 0) {
			return -1;
		}
		result += temp; // 记录读取到的长度
		int leftLen = lenOfBuffer - (BytsPerSec - curClusOffset); // 剩余长度
		int hasRead = BytsPerSec - curClusOffset;
		do {
			FstClus = getFATValue(FstClus); // 拿到下一个FAT
			if (FstClus == 0xFFF) {
				break;
			}
			dataBase = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec + RootEntCnt * 32 + (FstClus - 2) * BytsPerSec; // 刷新扇区偏移
			SetHeaderOffset(dataBase, NULL, FILE_BEGIN);
			if (leftLen > BytsPerSec) {
				if (ReadFromDisk(&cBuffer[hasRead], BytsPerSec, &temp) == 0) {
					return -1;
				}
				hasRead += BytsPerSec;
			}
			else {
				if (ReadFromDisk(&cBuffer[hasRead], leftLen, &temp) == 0) {
					return -1;
				}
				hasRead += leftLen;
			}
			leftLen -= BytsPerSec; // 直接减掉一个扇区，只要是<=0就退出循环
			result += temp;
		} while (leftLen > 0);
	}
	memcpy(pBuffer, cBuffer, lenOfBuffer); // 写入缓冲区
	ShutdownDisk();
	MySetFilePointer(dwHandle, result, MY_FILE_CURRENT); //偏移量刷新
	return result;
}

BOOL MyCreateDirectory(char *pszFolderPath, char *pszFolderName) {
	u16 FstClus;
	u16 originClus;
	BOOL result = FALSE;
	int dataBase;
	if (initBPB()) {
		// 路径存在或者为根目录
		if ((FstClus = isPathExist(pszFolderPath)) || strlen(pszFolderPath) == 3) {
			originClus = FstClus;
			if (isDirectoryExist(pszFolderName, FstClus)) {
				cout << pszFolderPath << '\\' << pszFolderName << " has existed!" << endl;
			}
			else {
				do {
					int loop;
					if (FstClus == 0) {
						// 根目录区偏移
						dataBase = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec;
						loop = RootEntCnt;
					}
					else {
						// 数据区文件首址偏移
						dataBase = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec + RootEntCnt * 32 + (FstClus - 2) * BytsPerSec;
						loop = BytsPerSec / 32;
					}
					for (int i = 0; i < loop; i++) {
						SetHeaderOffset(dataBase, NULL, FILE_BEGIN);
						if (ReadFromDisk(rootEntry_ptr, 32, NULL) != 0) {
							// 目录项可用
							if (rootEntry_ptr->DIR_Name[0] == 0x00 || rootEntry_ptr->DIR_Name[0] == 0xE5) {
								initFileInfo(rootEntry_ptr, pszFolderName, 0x10, 0); // 文件夹大小为0
								SetHeaderOffset(dataBase, NULL, FILE_BEGIN); // 磁头复位
								if (WriteToDisk(rootEntry_ptr, 32, NULL) != 0) {
									// 创建 . 和 ..目录
									int dBase = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec + RootEntCnt * 32 + (rootEntry_ptr->DIR_FstClus - 2) * BytsPerSec;
									SetHeaderOffset(dBase, NULL, FILE_BEGIN);
									WriteToDisk(setzero, BytsPerSec, NULL); // 目录创建初始清0
									// .
									SetHeaderOffset(dBase, NULL, FILE_BEGIN);
									rootEntry_ptr->DIR_FileSize = 0;
									rootEntry_ptr->DIR_Name[0] = 0x2E;
									for (int i = 1; i < 11; i++) {
										rootEntry_ptr->DIR_Name[i] = 0x20;
									}
									WriteToDisk(rootEntry_ptr, 32, NULL);
									// ..
									SetHeaderOffset(dBase + 32, NULL, FILE_BEGIN);
									rootEntry_ptr->DIR_Name[1] = 0x2E;
									rootEntry_ptr->DIR_FstClus = originClus;
									WriteToDisk(rootEntry_ptr, 32, NULL);
									result = TRUE;
									break;
								}
							}
						}
						dataBase += 32;
					}
					if (result) break;
				} while ((FstClus = getFATValue(FstClus)) != 0xFFF && FstClus != 0);
			}
		}
	}
	ShutdownDisk();
	return result;
}

BOOL MyDeleteDirectory(char *pszFolderPath, char *pszFolderName) {
	u16 FstClus;
	BOOL result = FALSE;
	if (initBPB()) {
		// 路径存在或者为根目录
		if ((FstClus = isPathExist(pszFolderPath)) || strlen(pszFolderPath) == 3) {
			// 待删除目录存在
			if (isDirectoryExist(pszFolderName, FstClus)) {
				int dataBase;
				int loop;
				char directory[12];
				u8 del = 0xE5;
				RootEntry fd;
				RootEntry* fd_ptr = &fd;
				do {
					if (FstClus == 0) {
						// 根目录区偏移
						dataBase = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec;
						loop = RootEntCnt;
					}
					else {
						// 数据区文件首址偏移
						dataBase = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec + RootEntCnt * 32 + (FstClus - 2) * BytsPerSec;
						loop = BytsPerSec / 32;
					}
					for (int i = 0; i < loop; i++) {
						SetHeaderOffset(dataBase, NULL, FILE_BEGIN);
						if (ReadFromDisk(fd_ptr, 32, NULL) != 0) {
							if (fd_ptr->DIR_Name[0] != 0xE5 && fd_ptr->DIR_Name[0] != 0 && fd_ptr->DIR_Name[0] != 0x2E) {
								// 目录0x10，文件0x20，卷标0x28
								if (fd_ptr->DIR_Attr == 0x10) {
									for (int j = 0; j < 11; j++) {
										if (fd_ptr->DIR_Name[j] != ' ') {
											directory[j] = fd_ptr->DIR_Name[j];
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
										recursiveDeleteDirectory(fd_ptr->DIR_FstClus);
										// 删除该文件夹
										SetHeaderOffset(dataBase, NULL, FILE_BEGIN);
										if (WriteToDisk(&del, 1, NULL) != 0) {
											result = recoverClus(fd_ptr->DIR_FstClus); // 传入首簇，回收
											break;
										}
									}
								}
							}
						}
						dataBase += 32;
					}
					if (result) break;
				} while ((FstClus = getFATValue(FstClus)) != 0xFFF && FstClus != 0);
			}
		}
	}
	ShutdownDisk();
	return result;
}

BOOL MySetFilePointer(DWORD dwFileHandle, int nOffset, DWORD dwMoveMethod) {
	FileHandle* hd = dwHandles[dwFileHandle];
	if (hd == NULL || initBPB() == FALSE) return FALSE; // 句柄不存在
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
			hd->offset = fileSize + nOffset;
		}
		break;
	}
	ShutdownDisk();
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
		int loop;
		if (FstClus == 0) {
			// 根目录区偏移
			dataBase = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec;
			loop = RootEntCnt;
		}
		else {
			// 数据区文件首址偏移
			dataBase = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec + RootEntCnt * 32 + (FstClus - 2) * BytsPerSec;
			loop = BytsPerSec / 32;
		}
		for (int i = 0; i < loop; i++) {
			SetHeaderOffset(dataBase, NULL, FILE_BEGIN);
			if (ReadFromDisk(rootEntry_ptr, 32, NULL) != 0) {
				// 目录0x10，文件0x20，卷标0x28
				if (rootEntry_ptr->DIR_Name[0] != 0xE5 && rootEntry_ptr->DIR_Name[0] != 0 && rootEntry_ptr->DIR_Name[0] != 0x2E) {
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
			}
			dataBase += 32;
		}
		if (isExist) break;
	} while ((FstClus = getFATValue(FstClus)) != 0xFFF && FstClus != 0);
	return isExist;
}

u16 isDirectoryExist(char *FolderName, u16 FstClus) {
	char directory[12];
	int dataBase;
	u16 isExist = 0;
	// 遍历当前目录所有项目
	do {
		int loop;
		if (FstClus == 0) {
			// 根目录区偏移
			dataBase = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec;
			loop = RootEntCnt;
		}
		else {
			// 数据区文件首址偏移
			dataBase = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec + RootEntCnt * 32 + (FstClus - 2) * BytsPerSec;
			loop = BytsPerSec / 32;
		}
		for (int i = 0; i < loop; i++) {
			SetHeaderOffset(dataBase, NULL, FILE_BEGIN);
			if (ReadFromDisk(rootEntry_ptr, 32, NULL) != 0) {
				// 目录0x10，文件0x20，卷标0x28
				if (rootEntry_ptr->DIR_Name[0] != 0xE5 && rootEntry_ptr->DIR_Name[0] != 0 && rootEntry_ptr->DIR_Name[0] != 0x2E) {
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
			}
			dataBase += 32;
		}
		if (isExist) break;
	} while ((FstClus = getFATValue(FstClus)) != 0xFFF && FstClus != 0);
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
	int i = 0;
	if (FileAttr == 0x10) {
		FileInfo_ptr->DIR_FileSize = 0;
		while (FileName[i] != '\0' && i < 11) {
			FileInfo_ptr->DIR_Name[i] = FileName[i];
			i++;
		}
		while (i < 11) {
			FileInfo_ptr->DIR_Name[i] = 0x20;
			i++;
		}
	}
	else {
		FileInfo_ptr->DIR_FileSize = FileSize;
		while (FileName[i] != '\0') {
			if (FileName[i] == '.') {
				int j = i;
				while (j < 8) {
					FileInfo_ptr->DIR_Name[j] = 0x20;
					j++;
				}
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
	int clusNum;
	if ((FileSize % BytsPerSec) == 0 && FileSize != 0) {
		clusNum = FileSize / BytsPerSec;
	}
	else {
		clusNum = FileSize / BytsPerSec + 1;
	}
	FileInfo_ptr->DIR_FstClus = setFATValue(clusNum);
}

BOOL writeEmptyClus(u16 FstClus, RootEntry* FileInfo) {
	int dataBase;
	u16 originClus;
	BOOL success = FALSE;
	do {
		int loop;
		originClus = FstClus; // 保存非0xfff簇号
		if (FstClus == 0) {
			// 根目录区偏移
			dataBase = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec;
			loop = RootEntCnt;
		}
		else {
			// 数据区文件首址偏移
			dataBase = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec + RootEntCnt * 32 + (FstClus - 2) * BytsPerSec;
			loop = BytsPerSec / 32;
		}
		for (int i = 0; i < loop; i++) {
			SetHeaderOffset(dataBase, NULL, FILE_BEGIN);
			if (ReadFromDisk(rootEntry_ptr, 32, NULL) != 0) {
				// 说明该目录项可用
				if (rootEntry_ptr->DIR_Name[0] == 0x00 || rootEntry_ptr->DIR_Name[0] == 0xE5) {
					SetHeaderOffset(dataBase, NULL, FILE_BEGIN);
					if (WriteToDisk(FileInfo, 32, NULL) != 0) {
						success = TRUE;
						break;
					}
				}
			}
			dataBase += 32;
		}
		if (success) break;
	} while ((FstClus = getFATValue(FstClus)) != 0xFFF && FstClus != 0);
	if (success == FALSE && FstClus != 0) { // 目录空间不足且不是根目录
		u16 bytes;
		u16* bytes_ptr;
		int fatBase = RsvdSecCnt * BytsPerSec;
		u16 tempClus = setFATValue(1);
		dataBase = SetHeaderOffset((fatBase + originClus * 3 / 2), NULL, FILE_BEGIN); // 尾簇号偏移
		SetHeaderOffset(dataBase, NULL, FILE_BEGIN);
		if (ReadFromDisk(bytes_ptr, 2, NULL) != 0) {
			if (originClus % 2 == 0) {
				bytes = bytes >> 12;
				bytes = bytes << 12; // 保留高四位，低12位为0
				bytes = bytes | tempClus;
			}
			else {
				bytes = bytes << 12;
				bytes = bytes >> 12; // 保留低四位，高12位为0
				bytes = bytes | (tempClus << 4);
			}
			SetHeaderOffset((fatBase + originClus * 3 / 2), NULL, FILE_BEGIN);
			if (WriteToDisk(bytes_ptr, 2, NULL) == 0) {
				return -1;
			}
			dataBase = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec + RootEntCnt * 32 + (tempClus - 2) * BytsPerSec;
			SetHeaderOffset(dataBase, NULL, FILE_BEGIN);
			WriteToDisk(setzero, BytsPerSec, NULL); // 清0
			SetHeaderOffset(dataBase, NULL, FILE_BEGIN);
			if (WriteToDisk(FileInfo, 32, NULL) != 0) {
				success = TRUE;
			}
		}
	}
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
		if (clus % 2 == 0) {
			fatPos++; // 往后偏一个字节
		}
		else {
			fatPos += 2; // 往后偏2个字节
		}
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

DWORD createHandle(RootEntry* FileInfo, u16 parentClus) {
	int i;
	FileHandle* hd = (FileHandle*)malloc(sizeof(FileHandle)); // 统一在这里malloc
	for (i = 1; i < MAX_NUM; i++) {
		if (dwHandles[i] == NULL) {
			memcpy(&hd->fileInfo, FileInfo, 32);
			hd->offset = 0; // 偏移量初始化为0
			hd->parentClus = parentClus;
			dwHandles[i] = hd;
			break;
		}
	}
	return i;
}

BOOL recoverClus(u16 fileClus) {
	// 回收簇
	u16 bytes;
	u16* bytes_ptr = &bytes;
	// 下一簇为末尾簇退出循环
	while (fileClus != 0xFFF) {
		int clusBase = RsvdSecCnt * BytsPerSec + fileClus * 3 / 2;
		u16 tempClus = getFATValue(fileClus); // 暂存下一簇，当前簇内容刷新成0
		SetHeaderOffset(clusBase, NULL, FILE_BEGIN);
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
	return TRUE;
}

void recursiveDeleteDirectory(u16 fClus) {
	u8 del = 0xE5;
	// 递归删除文件夹下的文件和目录
	// fClus 保存待删除文件夹首簇
	RootEntry fdd;
	RootEntry* fdd_ptr = &fdd;
	int fBase = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec + RootEntCnt * 32 + (fClus - 2) * BytsPerSec; // 找到该文件夹的扇区偏移
	// 遍历待删除目录下的所有目录项删除掉
	do {
		for (int k = 0; k < BytsPerSec / 32; k++) {
			SetHeaderOffset(fBase, NULL, FILE_BEGIN);
			if (ReadFromDisk(fdd_ptr, 32, NULL) != 0) {
				// 文件就直接把第一字节改了就成
				if (fdd_ptr->DIR_Name[0] != 0xE5 && fdd_ptr->DIR_Name[0] != 0 && fdd_ptr->DIR_Name[0] != 0x2E) {
					if (fdd_ptr->DIR_Attr == 0x20) {
						SetHeaderOffset(fBase, NULL, FILE_BEGIN);
						WriteToDisk(&del, 1, NULL);
						recoverClus(fdd_ptr->DIR_FstClus); // 回收文件簇
					}
					else if (fdd_ptr->DIR_Attr == 0x10) {
						// 文件夹递归调用删除其底下的目录项
						SetHeaderOffset(fBase, NULL, FILE_BEGIN);
						WriteToDisk(&del, 1, NULL);
						recursiveDeleteDirectory(fdd_ptr->DIR_FstClus); // 递归调用
						recoverClus(fdd_ptr->DIR_FstClus); // 回收目录簇
					}
				}
			}
			fBase += 32;
		}
	} while ((fClus = getFATValue(fClus)) != 0xFFF);
}