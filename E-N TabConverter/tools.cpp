/*
	��ͼ�����޹صĹ���
*/
#include <Windows.h>
#include <vector>
#include "resource.h"
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

bool FreeResFile(DWORD dwResName, LPCSTR lpResType, LPCSTR lpFilePathName){
	/*
		���ܣ��ͷ���Դ�ļ�
		������
			DWORD dwResName   ��ԴID����IDR_ML_CSV1
			LPCSTR lpResType ָ���ͷŵ���Դ����Դ���ͣ���"ML_CSV"
			LPCSTR lpFilePathName �ͷ�·���������ļ�����

		����ֵ���ɹ��򷵻�TRUE,ʧ�ܷ���FALSE
	*/
	HMODULE hInstance = ::GetModuleHandle(NULL);//�õ�����ʵ�����  

	HRSRC hResID = ::FindResource(hInstance, MAKEINTRESOURCE(dwResName), lpResType);//������Դ  
	HGLOBAL hRes = ::LoadResource(hInstance, hResID);//������Դ  
	LPVOID pRes = ::LockResource(hRes);//������Դ  

	if (pRes == NULL)//����ʧ��  
	{
		return false;
	}
	DWORD dwResSize = ::SizeofResource(hInstance, hResID);//�õ����ͷ���Դ�ļ���С  
	HANDLE hResFile = CreateFile(lpFilePathName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);//�����ļ�  

	if (INVALID_HANDLE_VALUE == hResFile)
	{
		//TRACE("�����ļ�ʧ�ܣ�");  
		return false;
	}

	DWORD dwWritten = 0;//д���ļ��Ĵ�С     
	WriteFile(hResFile, pRes, dwResSize, &dwWritten, NULL);//д���ļ�  
	CloseHandle(hResFile);//�ر��ļ����  

	return (dwResSize == dwWritten);//��д���С�����ļ���С�����سɹ�������ʧ��  
}

void ls(const char* lpPath, std::vector<std::string> &fileList){
	char szFind[MAX_PATH];
	WIN32_FIND_DATA FindFileData;

	strcpy_s(szFind, lpPath);
	strcat_s(szFind, "\\*.jpg");

	HANDLE hFind = ::FindFirstFile(szFind, &FindFileData);
	if (INVALID_HANDLE_VALUE == hFind)    return;

	while (true)
	{
		if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (FindFileData.cFileName[0] != '.')
			{
				char szFile[MAX_PATH];
				strcpy_s(szFile, lpPath);
				strcat_s(szFile, "\\");
				strcat_s(szFile, (char*)(FindFileData.cFileName));
				ls(szFile, fileList);
			}
		}
		else
		{
			//std::cout << FindFileData.cFileName << std::endl;  
			char szFile[MAX_PATH];
			strcpy_s(szFile, lpPath);
			strcat_s(szFile, "\\");
			strcat_s(szFile, FindFileData.cFileName);
			fileList.push_back(std::string(szFile));
		}
		if (!FindNextFile(hFind, &FindFileData))    break;
	}
	FindClose(hFind);
}