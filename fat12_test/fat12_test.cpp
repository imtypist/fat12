#include <iostream>
#include "fat12.h"
#pragma comment(lib, "..\\Debug\\fat12.lib")
using namespace std;

int main() {
	DWORD res;
	// ɾ���ļ�
	/*
	res = MyDeleteFile("c:\\", "ramdom1.doc");
	cout << "MyDeleteFile => return " << res << endl;
	//*/

	// ���ļ�
	/*
	res = MyOpenFile("c:\\", "random.txt");
	cout << "MyOpenFile => return " << res << endl;
	//*/

	// �ر��ļ�
	/*
	MyCloseFile(res);
	cout << "MyCloseFile => void"<< endl;
	//*/

	// �����ļ�
	//*
	res = MyCreateFile("c:", "random1.txt");
	cout << "MyCreateFile => return " << res << endl;
	//*/
	
	// �ƶ��ļ�ָ��
	/*
	MySetFilePointer(res, 1024, MY_FILE_BEGIN);
	res = MySetFilePointer(res, -512, MY_FILE_CURRENT);
	cout << "MySetFilePointer => return " << res << endl;
	//*/

	// д�ļ�
	/*
	char pBuffer[768] = { 0 };
	for (int i = 0; i < 768; i++) {
		pBuffer[i] = 'a';
	}
	res = MyWriteFile(res, &pBuffer, 768);
	cout << "MyWriteFile => return " << res << endl;
	//*/

	// ���ļ�
	/*
	char rBuffer[1025] = { 0 };
	res = MyReadFile(res, &rBuffer, 1024);
	cout << "MyReadFile => return " << res << endl;
	cout << "rBuffer => " << rBuffer << endl;
	cout << "rBuffer length => " << strlen(rBuffer) << endl;
	//*/

	// ����Ŀ¼
	/*
	res = MyCreateDirectory("c:\\huangjun", "qin");
	cout << "MyCreateDirectory => return " << res << endl;
	//*/

	// ɾ��Ŀ¼
	/*
	res = MyDeleteDirectory("c:\\", "drafts");
	cout << "MyDeleteDirectory => return " << res << endl;
	//*/
	return 0;
}