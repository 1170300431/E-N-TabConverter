#pragma once
#include "type.h"
#include "swan.h"
#include <map>
#include <algorithm>
#include <fstream>

class GlobalUnit {
private:
	int value = 0;
	bool initialed = false;
	float studyRate = 0.5;
public:
	void operator +=(int newVa) {
		value = initialed ? (int)((1 - studyRate) * value + studyRate * newVa) : (initialed = true, newVa);
	}
	operator int() {
		return value;
	}

	void setStudyRate(float newVa) {
		studyRate = newVa;
	}

};

class GlobalPool {
private:
	std::string path;
	int col;								//���׿��
public:
	GlobalUnit lineThickness;				//�ߴ�ϸ
	GlobalUnit rowLenth;					//���߿��
	GlobalUnit colLenth;					//С�ڿ��
	GlobalUnit valueSignalLen;
	GlobalUnit characterWidth;				//�ַ����


	GlobalPool(std::string, int);
	~GlobalPool();

};


extern GlobalPool* global;