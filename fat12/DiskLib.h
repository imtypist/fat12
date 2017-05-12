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
��;������Ӳ�̡���API����������������API���ã������Ѵ���·���µ��ļ�����
	������ļ����Ѵ��ڣ�������Ϊ�ļ�ϵͳ�򿪣�
	��������ڣ��򴴽����ļ���
	���ļ����ڴ洢��ʵ�ֵ�FAT12�ļ�ϵͳ��������Ϲ涨��FAT12�ļ���ʽ��
	����ʱ���ǻ�Ը��ļ���ʽ���м�顣
����������ļ�·����ע�⡰\��Ҫ��"\\"ת�塣
����ֵ���ɹ�����TRUE��ʧ�ܷ���FALSE��
ע�⣺��������У��ļ�·��������ڣ��ļ������Դ��ڣ���ʾ��һ���Ѵ��ڵ��ļ�ϵͳ����Ҳ���Բ����ڣ���ʾ�½�һ���յ��ļ�ϵͳ��
*/
BOOL StartupDisk(LPCTSTR lpszFile);

int StartupDiskA(LPCSTR lpszFile);

int StartupDiskW(LPCWSTR lpszFile);

/*
void ShutdownDisk();
��;���ر�Ӳ�̡���API����ɶ��ļ�ϵͳ�Ĳ��������
*/
void ShutdownDisk();

/*
ReadFromDisk(LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead)
��;�����ļ�ϵͳ�ж�ȡָ�����ȵ����ݡ�
���������
	lpBuffer���������ݵĻ����������㴴���ú���
	nNumberOfBytesToRead��Ҫ��ȡ�����ݳ��ȣ�С�ڵ����㴴���Ļ�������С
	lpNumberOfBytesRead��ָ�룬ָ��һ��DWORD���ͱ�������ʾʵ�ʶ��������ݡ���ѡ������Ҫ�Ļ���ΪNULL�ͺá�
����ֵ�����ʾʧ�ܣ������ʾ�ɹ�
*/
int ReadFromDisk(LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead);

/*
int WriteToDisk(LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten)
��;�������������д��ָ�����ȵ����ݡ�
���������
lpBuffer����д���������ڵĻ�����
nNumberOfBytesToWrite����д�����ݵĳ���
lpNumberOfBytesWritten��ָ�룬ָ��һ��DWORD���ͱ�������ʾʵ��д������ݡ���ѡ������Ҫ�Ļ���ΪNULL�ͺá�
����ֵ��
�����ʾ�ɹ������ʾʧ�ܡ�
*/
int WriteToDisk(LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten);

/*
int SetHeaderOffset(LONG lDistanceToMove, PLONG lpDistanceToMoveHigh, DWORD dwMoveMethod);
��;���ƶ���ͷ����д��ͬλ��
lDistanceToMove:DWORD���ͣ�����ָ��ƫ��λ�á������һ������ΪNULL����ò�������ָ��32λ�з�������ƫ�Ʒ�Χ�������һ��������ΪNULL����ò�������һ������ָ��64λƫ���еĵ�32λ��
lpDistanceToMoveHigh��ָ��һ��DWORD���ͣ����ΪNULL������һ����������ָ��32λƫ�Ʒ�Χ������ò�����ΪNULL������ǰһ��������ָͬ��һ��64λ��ƫ�ơ�
dwMoveMethod��ƫ�Ƶ���ʼλ�ã��������������Ϳ�ѡ��
FILE_BEGIN����ͷ����ʼ����ƫ��
FILE_CURRENT���ӵ�ǰ��ͷλ�ÿ�ʼ����ƫ��
FILE_END����ĩβ��ʼ����ƫ��
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

