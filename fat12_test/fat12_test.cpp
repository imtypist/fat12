#include <iostream>
#include <windows.h>
using namespace std;

typedef DWORD (*MyCreateFile)(char *, char *);

int main() {
	HMODULE h = LoadLibraryA("..\\Debug\\fat12.dll");
	if (NULL == h)
	{
		cout << "can not load library..." << endl;
		return 1;
	}
	MyCreateFile MCF = (MyCreateFile)GetProcAddress(h, "MyCreateFile");
	MCF("C:\\DRAFTS","DOS.TXT");
	FreeLibrary(h);
	return 0;
}