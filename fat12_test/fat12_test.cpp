#include <iostream>
#include "fat12.h"
#pragma comment(lib, "..\\Debug\\fat12.lib")
using namespace std;

int main() {
	DWORD res;
	// 删除文件
	/*
	res = MyDeleteFile("c:\\", "ramdom1.doc");
	cout << "MyDeleteFile => return " << res << endl;
	//*/

	// 打开文件
	/*
	res = MyOpenFile("c:\\", "random.txt");
	cout << "MyOpenFile => return " << res << endl;
	//*/

	// 关闭文件
	/*
	MyCloseFile(res);
	cout << "MyCloseFile => void"<< endl;
	//*/

	// 创建文件
	//*
	res = MyCreateFile("c:", "random1.txt");
	cout << "MyCreateFile => return " << res << endl;
	//*/
	
	// 移动文件指针
	/*
	MySetFilePointer(res, 1024, MY_FILE_BEGIN);
	res = MySetFilePointer(res, -512, MY_FILE_CURRENT);
	cout << "MySetFilePointer => return " << res << endl;
	//*/

	// 写文件
	/*
	char pBuffer[768] = { 0 };
	for (int i = 0; i < 768; i++) {
		pBuffer[i] = 'a';
	}
	res = MyWriteFile(res, &pBuffer, 768);
	cout << "MyWriteFile => return " << res << endl;
	//*/

	// 读文件
	/*
	char rBuffer[1025] = { 0 };
	res = MyReadFile(res, &rBuffer, 1024);
	cout << "MyReadFile => return " << res << endl;
	cout << "rBuffer => " << rBuffer << endl;
	cout << "rBuffer length => " << strlen(rBuffer) << endl;
	//*/

	// 创建目录
	/*
	res = MyCreateDirectory("c:\\huangjun", "qin");
	cout << "MyCreateDirectory => return " << res << endl;
	//*/

	// 删除目录
	/*
	res = MyDeleteDirectory("c:\\", "drafts");
	cout << "MyDeleteDirectory => return " << res << endl;
	//*/
	return 0;
}