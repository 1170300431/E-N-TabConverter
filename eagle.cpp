#include "ml.hpp"
#include "opencv.hpp"
#include "type.h"
#include <fstream>
#include <iterator> 
#include <string>

#define IDR_ML_CSV1 103
#define samplePath "C:\\Users\\Administrator\\Desktop\\E-N TabConverter\\sample_classified\\"
#define defaultCSV "C:\\Users\\Administrator\\Desktop\\E-N TabConverter\\tData.csv"

typedef unsigned long       DWORD;
typedef _Null_terminated_ const char *LPCSTR;

using namespace cv;
using namespace cv::ml;
using namespace std;

static cv::Ptr<cv::ml::KNearest> &load(string csv, Ptr<KNearest> &knn);

extern void ls(const char* lpPath, vector<string> &fileList);
extern bool FreeResFile(DWORD dwResName, LPCSTR lpResType, LPCSTR lpFilePathName);
extern bool isExist(string filepath);

int rec(Mat character,vector<int> &possible) {
	Mat res,tmp,neighbour;
	character.reshape(1, 1).convertTo(tmp, CV_32FC1, 1.0 / 255.0);
	static Ptr<KNearest> knn = KNearest::create();
	knn = load(defaultCSV, knn);
	if (!knn->isTrained()) {
		err ex = {5,__LINE__,"knn�����ȡʧ��"};
		throw ex;
		return -1;
	}
	knn->findNearest(tmp, 5, res, neighbour);
	possible.clear();
	for (int j = 0; j < neighbour.rows; j++) {
		possible.push_back((int)neighbour.at<float>(j, 0));
	}
	sort(possible.begin(), possible.end());
	possible.erase(unique(possible.begin(),possible.end()),possible.end());		//ȥ��
	return (int)res.at<float>(0, 0);
}

void train(string save) {
	//trainData ����*��С
	//Labels ����*10
	Mat trainData, Label ,CSV;
	//¼��ѵ�������ͱ��
	int num;															//num ��������ʲô����
	vector<string> fileList;
	string path = samplePath;
	for (num = 0; num < 10; num++) {
		char c = num + '0';
		ls((path + c).c_str(), fileList);
		for (int i = 0; i < (int)fileList.size(); i++) {
			Mat tmp = imread(string(fileList[i]));
			trainData.push_back(tmp.reshape(1, 1));
			Label.push_back(num);										//��trainData��Ӧ�ı��
		}
		fileList.clear();
	}
	Label.convertTo(Label,CV_32F);
	trainData.convertTo(trainData, CV_32FC1, 1.0 / 255.0);
	hconcat(Label, trainData, CSV);
	//Ptr<TrainData> tData = TrainData::create(trainData, ROW_SAMPLE, Label);
	ofstream file(save);
	file << format(CSV, Formatter::FMT_CSV);
	file.close();
}

Ptr<KNearest> &load(string csv, Ptr<KNearest> &knn) {
	if (knn->isTrained()) {
		return knn;											//�����ظ�
	}
	if(!isExist(csv)) FreeResFile(IDR_ML_CSV1, (char*)"ML_CSV", (char*)defaultCSV);

	Ptr<TrainData> trainData = TrainData::loadFromCSV(csv, 0, 0, -1);

	if (trainData.empty()) {
		FreeResFile(IDR_ML_CSV1, (char*)"ML_CSV", (char*)defaultCSV);
		Ptr<TrainData> trainData = TrainData::loadFromCSV(csv, 0, 0, -1);
	}
	knn->setDefaultK(5);
	knn->setIsClassifier(true);
	Mat Label;
	trainData->getResponses().convertTo(Label,CV_32S);
	knn->train(trainData->getSamples(),ROW_SAMPLE,Label);
	return knn;
}

