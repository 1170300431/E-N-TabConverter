#include "cv.h"
#include "imgproc.hpp"
#include "opencv.hpp"

using namespace std;
using namespace cv;

#define useInpaint 1
#if _DEBUG
#define ShowDenoise 0
#endif
int cut(Mat img, vector<Vec4i> divideBy, int direction, vector<Mat> &container, bool includeAll) {
	//direction: 0����ֱ�ü�, 1��ˮƽ�ü�
	if (direction > 1) return 0;
	int from = (int)container.size();
	switch (direction) {
	case 0:
		//��ֱ�ü�

		if (includeAll) {
			if (divideBy[0][0] > 5) {
				Vec4i h = { 0,0,0,img.rows };
				divideBy.insert(divideBy.begin(), h);
			}
			if (divideBy.back()[0] < img.cols - 5) {
				Vec4i h = { img.cols,0,img.cols,img.rows };
				divideBy.insert(divideBy.begin() + divideBy.size(), h);
			}
		}
		for (int i = 0; i < (int)divideBy.size() - 1; i++) {
			Mat tmp;
			img(Range::all(), Range(divideBy[i][0], divideBy[i + 1][0])).copyTo(tmp);
			container.push_back(tmp);
		}
		break;
	case 1:
		//ˮƽ�ü�

		if (includeAll) {
			if (divideBy[0][1] > 5) {
				Vec4i h = { 0,0,img.cols,0 };
				divideBy.insert(divideBy.begin(), h);
			}
			if (divideBy.back()[1] < img.rows - 5) {
				Vec4i h = { 0,img.rows,img.cols,img.rows };
				divideBy.insert(divideBy.begin() + divideBy.size(), h);
			}
		}
		for (int i = 0; i < (int)divideBy.size() - 1; i++) {
			Mat tmp;
			img(Range(divideBy[i][1], divideBy[i + 1][1]), Range::all()).copyTo(tmp);
			container.push_back(tmp);
		}
	}
	return (int)container.size() - from;
}

inline void extractNum(vector<Vec4i> &pos, vector<Mat> &nums, vector<Mat> section, vector<Vec4i> rows,int &bottom,int range) {
	/*
	 * ������extractNum
	 * ���ܣ��Ӵ���ͼ������ȡ���ֵ�
	 * ������	pos��Vec4i���ֱ���λ����Ϣ���� �� �� ��
				nums��Mat����ȡ����ͼ��
				section��Mat������ͼ��
				rows��Vec4i�������������Ϣ�����ߣ�
				bottom��int����ȡ���ֵ��½磬������ʱֵɨ�躯��
				range��int����sectionβ����ʼ��Ҫ����ķ�Χ
	*/
	bottom = 0;
	for (size_t j = section.size() - range; j < section.size(); j++) {
		vector<vector<cv::Point>> cont;
		cv::Mat inv = 255 - section[j];
		cv::findContours(inv, cont, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
		for (int q = 0; q < cont.size(); q++) {
			cv::Vec4i tmp = { section.back().cols,section.back().rows,0,0 };
			for (int k = 0; k < cont[q].size(); k++) {
				tmp[0] = min(tmp[0], cont[q][k].x);
				tmp[2] = max(tmp[2], cont[q][k].x);
				tmp[1] = min(tmp[1], cont[q][k].y);
				tmp[3] = max(tmp[3], cont[q][k].y);

				pos.push_back(tmp);
			}
			//�޶�ɸѡ
			if (tmp[3] - tmp[1] < rows[1][1] - rows[0][1]					//�����޶�
				&& tmp[3] - tmp[1] > tmp[2] - tmp[0])						//��״�޶�
			{
				bottom = max(bottom, tmp[3]);
				nums.push_back(section[j](cv::Range(tmp[1], tmp[3] + 1), cv::Range(tmp[0], tmp[2] + 1)));
			}
		}
	}
}

Mat Denoise(Mat img,vector<Vec4i> lines, double radius) {
	//ΪOCRȥ������
	//����̬ѧ��ʴ�õ�mask ��mask�ϵĵ���0
#if !useInpaint
	Mat dilated;
	dilated = 255 - Morphology(img, img.cols / 50, true, true);
	dilated = Morphology(dilated, 2, true, true);
	//imshow("2", dilated); cvWaitKey();
	return cv::max(dilated, img);
#endif
	Mat mask = Mat(img.size(), CV_8UC1, Scalar::all(0));
	for (size_t i = 0; i < lines.size(); i++) {
		line(mask, Point(lines[i][0], lines[i][1]), Point(lines[i][2], lines[i][3]), Scalar(255));
		
	}
	inpaint(img, mask, img, radius, INPAINT_TELEA);
	threshold(img, img, 217, 255, THRESH_BINARY);
#if ShowDenoise
	imdebug("2", img);
#endif
	return img;
}