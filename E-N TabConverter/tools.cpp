/*
	��ͼ�����޹صĹ���
*/
#include <Windows.h>
#include <string>

using namespace std;

string GBKToUTF8(const char* strGBK) {
	/*
		��������GBKToUTF8
		���ܣ���GBK����ת��UTF8����ҪӦ��tinyxml���봦�������
	*/
	int len = MultiByteToWideChar(CP_ACP, 0, strGBK, -1, NULL, 0);
	wchar_t* wstr = new wchar_t[len + 1];
	memset(wstr, 0, len + 1);
	MultiByteToWideChar(CP_ACP, 0, strGBK, -1, wstr, len);
	len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
	char* str = new char[len + 1];
	memset(str, 0, len + 1);
	WideCharToMultiByte(CP_UTF8, 0, wstr, -1, str, len, NULL, NULL);
	string strTemp = str;
	if (wstr) delete[] wstr;
	if (str) delete[] str;
	return strTemp;
}

string getPath() {
	/*
		��������getPath
		���ܣ��õ���������Ŀ¼
	*/
	char szFilePath[MAX_PATH + 1] = { 0 };
	GetModuleFileNameA(NULL, szFilePath, MAX_PATH);
	(strrchr(szFilePath, '\\'))[0] = 0;								// ɾ���ļ�����ֻ���·��
	string path = szFilePath;

	return path;
}

void fname(const char* path, char* name) {
	/*
		��������fname
		���ܣ��õ�����ȫ·��ָ����ļ���
		����ֵ�� char* name
	*/
	int start, end;
	int n = (int)strnlen_s(path, 260);
	for (int i = n - 1; i >= 0; i--) {
		if (path[i] == '.' || path[i] == '\\') {
			end = i;
			for (i = i - 1; i >= 0; i--) {
				if (path[i] == '\\') {
					break;
				}
			}
			start = i + 1;
			goto copy;
		}
	}
	strncpy(name, path, n);
	return;
copy:
	strncpy(name, &path[start], end - start);
	return;
}