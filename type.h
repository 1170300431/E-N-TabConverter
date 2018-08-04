#pragma once
#include "cv.h"                             
#include "highgui.h"
#include "cvaux.h"
#include "cxcore.h"
#include "opencv2/opencv.hpp"
#include "opencv2/imgproc.hpp"
#include "tinyxml2.h"
#include <string>
#include<vector>

typedef struct space {
	int start;
	int length;
}space;

typedef struct err {
	int id;
	int line;
	std::string description;
}err;

enum Value {
	whole = 1,
	half = 2,
	quarter = 4,
	eighth = 8,
	sixteenth = 16,
	thirty_second = 32
};
typedef struct technical {
	int string = 1;									//��
	int fret = 0;									//Ʒ
}technical;

typedef struct notations {
	char* dynamics = (char*)"mf";						//����
	technical technical;
}notations;

typedef struct note {
	bool chord = false;							//������ǣ�Ϊtrue����һ������ͬʱ����
	std::vector<int> possible;					//�������ܵ�Ʒ��
	Value timeValue = whole;					//ʱֵ
	bool dot = false;							//����
	int voice = 1;								//��������
	int duration = 1;							//����ֵ
	notations notation = { (char*)"mf",{1,0} };

	int pos;									//x����
}note;

typedef struct Time {
	int beats = 4;								//ÿС������
	int beat_type = 4;							//������������һ��
}Time;

typedef struct key {
	int fifth = 0;								//��������
	char* mode = (char*)"major";				//1-��׼��������
}key;

class measure {
private:
	int noteBottom = 0;
	int maxCharacterWidth = 0;
	void recNum(cv::Mat section, std::vector<cv::Vec4i> rows);
	void recTime(std::vector<cv::Vec4i> rows);
	cv::Mat org;
public:
	int id;
	unsigned int number = 1;						//С����
	Time time;
	std::vector<note> notes;
	key key;
	measure(cv::Mat org, cv::Mat img, std::vector<cv::Vec4i> rows,int id);
};

class saveDoc {
private:
	tinyxml2::XMLDocument backup;
public:
	saveDoc(char* title, const char* composer, const char* lyricist, const char* artist, const char* tabber, const char* irights);
	int save(const char* xmlPath);
	void saveMeasure(measure toSave);
};

template<typename va>
class notify {
private:
	va v;
	void (*Set)(va newNoti) = NULL;
public:
	va operator = (const va newV) {
		assert(Set);
		Set(newV);
		v = newV;
		return newV;
	}
	operator va() {
		return v;
	}
	notify(void (*pSet)(va newV))
	{
		Set = pSet;
	}
};

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

	//GlobalUnit(int init) {
	//	value = init;
	//	initialed = true;
	//}
	//GlobalUnit() {}
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

	GlobalPool(std::string,int);
	~GlobalPool();

};