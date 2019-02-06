#pragma once
#include <vector>
#include "cv.h" 

using namespace std;

#define picFolder "C:\\Users\\Administrator\\Desktop\\oh"

enum value {
	zero = 0,
	whole = 1,
	half = 2,
	quarter = 4,
	eighth = 8,
	sixteenth = 16,
	thirty_second = 32
};

class Value {
private:
	int v = whole;
public:
	bool dot = false;
	Value(const value init) :v(init) {}
	Value() {}
	Value operator= (const value x) {
		v = x;
		return x;
	}
	Value operator+(const value x) {
		if (!v) v = x;
		else if (!x) return *this;
		else return value(v * x / (v + x));
	}
	Value operator+= (const value x) {
		if (!v) v = x;
		else if (!x) return *this;
		else v = v * x / (v + x);
		return *this;
	}
	Value operator-(const value x) {
		if (x == v) return zero;
		if (!v) return x;
		if (!x) return (value)v;
		else return value(x * v / abs(x - v));
	}
	Value operator*(const float x) {
		assert(x);
		return (value)(int)(v / x);
	}
	Value operator/(const int x) {
		return (value)(v * x);
	}
	bool operator<(const value x) {
		return x < this->v;
	}
	bool operator>(const value x) {
		return x > this->v;
	}
	bool operator==(const value x) {
		return x == this->v;
	}
	operator value() {
		return (value)v;
	}
};

typedef struct {
	int pos;
	unsigned string, fret;
	vector<int> possible;
	Value time;
}easynote;

typedef struct {
	vector<easynote> chords;
	int avrpos;
}ChordSet;

typedef struct{
	int string = 1;									//��
	int fret = 0;									//Ʒ
}technical;

typedef struct{
	char* dynamics = (char*)"mf";					//����
	technical technical;
}notations;

class note{
public:
	bool chord = false;							//������ǣ�Ϊtrue����һ������ͬʱ����
	std::vector<int> possible;					//�������ܵ�Ʒ��
	Value timeValue;							//ʱֵ
	bool dot = false;							//����
	int voice = 1;								//��������
	int duration = 1;							//����ֵ
	notations notation = { (char*)"mf",{1,0} };

	int pos;									//x����
	bool operator==(const note x) {
		return pos == x.pos &&
			notation.technical.string == x.notation.technical.string &&
			notation.technical.fret == x.notation.technical.fret;
	}
};

typedef struct Time {
	int beats = 4;								//ÿС������
	Value beat_type = quarter;					//������������һ��
}Time;

typedef struct key {
	int fifth = 0;								//��������
	char* mode = (char*)"major";				//1-��׼��������
}key;

class measure {
private:
	int maxCharacterWidth = 0;
	int maxCharacterHeight = 0;
	void recNum(cv::Mat section, vector<cv::Vec4i> rows);
	void recTime(vector<cv::Vec4i> rows);
	cv::Mat org;
	vector<ChordSet> notes;
public:
	int id;
	unsigned int number = 1;						//С����
	Time time;
	key key;
	measure(cv::Mat org, cv::Mat img, vector<cv::Vec4i> rows, int id);
	vector<note> getNotes();
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
};