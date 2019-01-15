#pragma once
#include <vector>
#include "cv.h" 

using namespace std;

#define savepic 0
#define picFolder "C:\\Users\\Administrator\\Desktop\\oh"

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
	char* dynamics = (char*)"mf";					//����
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
	void recNum(cv::Mat section, vector<cv::Vec4i> rows);
	void recTime(vector<cv::Vec4i> rows);
	cv::Mat org;
public:
	int id;
	unsigned int number = 1;						//С����
	Time time;
	vector<note> notes;
	key key;
	measure(cv::Mat org, cv::Mat img, vector<cv::Vec4i> rows, int id);
};

typedef struct{
	unsigned start;
	unsigned length;
}space;

class splitter{
public:
	splitter(cv::Mat img);
	void start(vector<cv::Mat>& piece);

private:
	cv::Mat org;
	vector<space> collection;
	int split();
	void interCheck(vector<int> &f);
	void KClassify(vector<bool> &classifier);
};

/*
class Denoiser {
private:
	double radius;
};
*/