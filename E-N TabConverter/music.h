#pragma once
#include <vector>

class Value {
private:
	float v = 1;
	Value(const float init) {
		v = init;
	}
public:
	enum predef {
		none = 0,
		whole = 1,
		half = 2,
		quarter = 4
	};

	bool dot = false;
	Value(const int init)  {
		v = 1.0f / init;
	}
	Value() {}
	const Value operator= (const int x) {
		v = 1.0f / x;
		return x;
	}
	const Value operator= (const Value x) {
		v = x.v;
		return x;
	}
	Value operator+(const int x) {
		return Value(v + 1.0f / x);
	}
	Value operator+= (const int x) {
		v += 1.0f / x;
		return *this;
	}
	Value operator-(const int x) {
		return Value(abs(v - 1.0f / x));
	}
	const Value operator*(const int x) {
		return Value(v * x);
	}
	const Value operator/(const int x) {
		return Value(v / x);
	}
	bool operator<(const int x) {
		return v < 1.0f / x;
	}
	bool operator>(const int x) {
		return v > 1.0f / x;
	}
	bool operator==(const int x){
		return x == int(round(1 / v));
	}
	bool operator==(const Value& x) const {
		return x.v == int(round(1 / v)) && x.dot == dot;
	}
	operator int() {
		return int(round(1 / v));
	}
};

typedef struct {
	int string = 1;									//��
	int fret = 0;									//Ʒ
}technical;

typedef struct {
	char* dynamics = (char*)"mf";					//����
	technical technical;
	std::string notation;
}notations;

class note {
public:
	bool chord = false;							//������ǣ�Ϊtrue����һ������ͬʱ����
	std::vector<int> possible;					//�������ܵ�Ʒ��
	Value timeValue;							//ʱֵ
	bool dot = false;							//����
	int voice = 1;								//��������
	int duration = 1;							//����ֵ
	notations notation = { (char*)"mf", {1,0}, ""};

	int pos;									//x����
	bool operator==(const note x) {
		return pos == x.pos &&
			notation.technical.string == x.notation.technical.string &&
			notation.technical.fret == x.notation.technical.fret;
	}
};

typedef struct{
	int beats = 4;											//ÿС������
	Value beat_type = 4;									//������������һ��
}Time;

typedef struct{
	int fifth = 0;											//��������
	std::string mode = "major";									//1-��׼��������
}key;

typedef struct  {
	size_t id;
	std::vector<note> notes;
	Time time;
	key key;
}MusicMeasure;