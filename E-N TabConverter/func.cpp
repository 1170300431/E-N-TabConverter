#pragma once
#include "Dodo.h"
#include "global.h"
#include "Cuckoo.h"
#include "opencv.hpp"
#include "swan.h"
#include "tools.h"

using namespace std;
using namespace cv;

#define imdebug(img, title) imshow((img), (title)); cv::waitKey()

GlobalPool *global = NULL;
constexpr const char* PROJECT = "E-N TabConverter";

extern notify<int> progress;
extern notify<string> notification;

void TrainMode() {
	train();
}

int go(string f, bool isCut) {
	bool flag = false;
	Mat img = imread(f.c_str(), 0);
	Mat trimmed = threshold(img);
	if (img.empty()) {
		err ex = { 3,__LINE__,"Wrong format." };
		throw ex;
	}
	
	trimmed = trim(trimmed);
	float screenCols = 1919 / 1.25f;				//1919, �����ʾ��ȣ�1.25�� win10 ϵͳ���ű�
													//TODO����֪����ô��ȡQAQ
	if (trimmed.cols > screenCols) {
		float toRows = screenCols / trimmed.cols * trimmed.rows;
		trimmed = perspect(trimmed, screenCols, (int) toRows);
		trimmed = threshold(trimmed);
	}
	vector<Mat> piece;
	
	splitter piccut(trimmed);
	piccut.start(piece);
	
	progress = 1;
	notification = "�����㷨����";
	
	global = new GlobalPool(cfgPath,trimmed.cols);
	vector<measure> sections;					//���д洢
	vector<Mat> info;							//������Ϣ
	vector<Mat> notes;							//С��
	

	size_t n = piece.size();
	Mat toOCR;

	for (Mat& i: piece) {
		if (i.empty()) continue;
		vector<Vec4i> rows;
		vector<int> thick;
		findRow(i, CV_PI / 18, rows,thick);
		if (rows.size() == 6) {
			flag = true;

			vector<Vec4i> lines;
			int max = min(rows[5][1], rows[5][3]);
			int min = std::max(rows[0][1], rows[0][3]);

			findCol(i, CV_PI / 18 * 8, max, min, thick, lines);				//
			vector<Mat> origin;												//�и�洢
			if (lines.size()) cut(i, lines, 0, origin, true);				//

			global->rowLenth += i.rows;

			for (Mat& j: origin) {
				try {
					measure newSec(j, rows, sections.size() + 1);
					if (newSec.id) sections.emplace_back(newSec);
				}
				catch (err ex) {
					switch (ex.id){
					case 1:
					default: throw ex; break;
					}
				}
			}
		}
		else {
			if(flag) notes.emplace_back(i);
			else info.emplace_back(i);
		}
		progress = progress + 79 / (int)n;
	}
	piece.clear();

	notification = "ɨ����ɣ�׼��д���ļ�";
	progress = 80;
	delete global;
	char name[256] = "";
	fname(f.c_str(),name);
	saveDoc finish(name,"unknown","unknown","unknown",PROJECT,"Internet");
	for (measure& i : sections) {
		if(SUCCEED(i.id))
		try { 
			finish.saveMeasure(i); 
		}
		catch (err ex) {
			switch (ex.id)
			{
			case 3: break;				//unexpected timeValue
										//impossible
			default: break;
			}
		}
		progress = progress + 20 / (int)sections.size();
	}
	string fn = name;
	fn += ".xml";
	if (isExist(fn)) if (prompt(NULL, fn + " �Ѿ����ڣ�Ҫ�滻��", PROJECT, 0x34) == 7) {
		notification = "�û������˱���";
		return 1;
	}
	finish.save(fn);
	progress = 100;
	notification = "Success";
	destroyAllWindows();
	return 0;
}