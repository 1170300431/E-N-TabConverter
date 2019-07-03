#include "pch.h"
#include "Dodo.h"
#include "eagle.h"
#include "tools.h"
#include "type.h"
#include <fstream>

#define IDR_ML_CSV1 103

using namespace std;
using namespace cv;

/**
	ʶ������ַ�. 
	@name CharReader::rec
	@param	character Mat, the character's image. 
				requires size of the Mat equals to preferWidth * preferHeight. 
	@param	threshold, float, if the possibility is greater than the threshold, the result will
				be pushed into the return val. 
	@retval	map of(-1->-1) is returned if 'reshape' throws an error; or no result has a possibility 
				greater than threshold. 
*/
map<char, float> knnReader::rec(Mat character, float threshold) {
	cvtColor(character, character, CV_GRAY2BGR);
	character = perspect(character, getPreferWidth(), getPreferHeight());
	
	Mat res, tmp, neighbour, dist;
	//dist: wrong recgonization, 33.244, 47.31, 45.299
	(character.isContinuous() ? character : character.clone()).
		reshape(1, 1).convertTo(tmp, CV_32FC1, 1.0 / 255.0);
	
	load(defaultCSV);					//lazy load
	assert(knn->isTrained());			//post condition
	knn->findNearest(tmp, 5, res, neighbour, dist);

	map<char, float> ret;
	for (int j = 0; j < dist.cols ; j++) {
		float a = dist.at<float>(0, j);
		if (a < threshold) {
			ret.insert(make_pair(static_cast<char>(neighbour.at<float>(0, j)), a));
		}
		else break;
	}
	return ret.empty() ? map<char, float>({ {-1, -1.0f} }) : ret;
}

void CharReader::train(string save) {
	//trainData ����*��С
	//Labels ����*10
	Mat trainData, Label ,CSV;
	//¼��ѵ�������ͱ��
	vector<string> fileList;
	string path = samplePath;

	for (char i: readable) {
		ls((path + i).c_str(), fileList);
		for (int i = 0; i < (int)fileList.size(); i++) {
			Mat tmp = imread(string(fileList[i]));
			trainData.push_back(tmp.reshape(1, 1));
			Label.push_back(i);										//��trainData��Ӧ�ı��
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

/*
	load train data from a csv file. 
	@name CharReader::load
	@param csv	string, path of the csv file. 
		
		effects: this->knn->isTrain() == true
*/
void knnReader::load(string csv) {
	if (knn->isTrained()) return;									//return if trained. 
	
	if(!isExist(csv)) FreeResFile(IDR_ML_CSV1, "ML_CSV", csv);

	Ptr<TrainData> trainData = TrainData::loadFromCSV(csv, 0, 0, -1);

	if (trainData.empty()) {										//csv is destroyed. 
		if (onDestroyed(csv)) return;
	}
	knn->setDefaultK(5);
	knn->setIsClassifier(true);
	Mat Label;
	trainData->getResponses().convertTo(Label,CV_32S);
	knn->train(trainData->getSamples(),ROW_SAMPLE,Label);
}

/*
	the strategy when train data is lost or destroyed. 
	@param csv	string, the csv file path. 
	@retval true if the strategy suggests exit. 
*/
bool CharReader::onDestroyed(const string& csv) noexcept {
	FreeResFile(IDR_ML_CSV1, "ML_CSV", csv);
	Ptr<TrainData> trainData = TrainData::loadFromCSV(csv, 0, 0, -1);
	return false;
}

CharReader::~CharReader() {}			//virtual dtor. 