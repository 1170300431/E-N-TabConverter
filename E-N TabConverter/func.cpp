#include "stdafx.h"
#include "../Cuckoo/Dodo.h"
#include "eagle.h"
#include "global.h"
#include "../Cuckoo/Cuckoo.h"
#include "swan.h"
#include "tools.h"
#include <functional>

using namespace std;
using namespace cv;

constexpr const char* PROJECT = "E-N TabConverter";
GlobalPool global(cfgPath);

void TrainMode() {
	NumReader::train(defaultCSV);
}
int go(string f, bool isCut, function<void(string)> notify, function<void(int)> progress) {
	atomic_int prog = 0;
	bool flag = false;
	Mat img = imread(f.c_str(), 0);
	Mat trimmed = threshold(img);
	if (img.empty()) raiseErr("Wrong format.", 3);
	
	trimmed = trim(trimmed);
	float screenCols = 1919 / 1.25f;				//1919, �����ʾ��ȣ�1.25�� win10 ϵͳ���ű�
													//TODO����֪����ô��ȡQAQ
	if (trimmed.cols > screenCols) {
		float toRows = screenCols / trimmed.cols * trimmed.rows;
		trimmed = perspect(trimmed, int(screenCols), int(toRows));
		trimmed = threshold(trimmed);
	}
	vector<Mat> piece;
	
	Splitter piccut(trimmed);
	piccut.start(piece);
	
	progress(1);
	notify("�����㷨����");
	
	global.setCol(trimmed.cols);
	size_t n = piece.size();
	vector<measure> sections;					//���д洢
	vector<Mat> info;							//������Ϣ
	
	
	for (Mat& i: piece) {
		vector<Vec4i> rows;
		vector<int> thick;
		LineFinder finder(i, 10);
		finder.findRow(rows);
		if (rows.size() == 6) {
			vector<Vec4i> lines;

			finder.findCol(lines);											//
			vector<Mat> origin;												//�и�洢
			if (lines.size()) cut(i, lines, 0, origin, true);				//

			getkey(rowLenth) += i.rows;

			/*atomic_int cnt = int(origin.size());
			auto measureStart = [&cnt, &rows](measure& x) {
				x.start(rows);
				cnt--;
			};*/
			for (Mat j : origin) {
				measure newSec(j, sections.size() + 1);
				if (newSec.id) sections.emplace_back(newSec); 
				else {
					//cnt--; 
					continue;
				};
				try {
					thread t(&measure::start, newSec, rows);
					t.join();
				}
				catch (Err ex) {
					//cnt--;
					switch (ex.id) {
					case 1:
					default: throw ex; break;
					}
				}
				
			}
			//while (cnt > 0) this_thread::yield();
		}
		else {
			info.emplace_back(i);
		}
		prog += 80 / int(n);
		progress(prog);
	}
	piece.clear(); piece.shrink_to_fit();

	//sort(sections.begin(), sections.end());
	notify("ɨ����ɣ�׼��д���ļ�");
	progress((prog = 80));
	global.save();
	string name = fname(f);
	saveDoc finish(name, "unknown", "unknown", "unknown", PROJECT, "Internet");
	for (measure& i : sections) {
		if(SUCCEED(i.id))
		try { 
			finish.saveMeasure(i.getNotes()); 
		}
		catch (Err ex) {
			switch (ex.id)
			{
			case 3: break;				//unexpected timeValue
										//impossible
			default: break;
			}
		}
		prog += 20 / (int)sections.size();
		progress(prog);
	}
	string fn = name;
	fn += ".xml";
	if (isExist(fn)) if (prompt(NULL, fn + " �Ѿ����ڣ�Ҫ�滻��", PROJECT, 0x34) == 7) {
		notify("�û������˱���");
		return 1;
	}
	finish.save(fn);
	progress(100);
	notify("Success");
	cv::destroyAllWindows();
	return 0;
}