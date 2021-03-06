#include "pch.h"
#include "Cuckoo.h"
#include "Dodo.h"
#include <atomic>
#include "../E-N TabConverter/global.h"

using namespace cv;
using namespace std;

#if _DEBUG
#define Showdenoise if(0)
#else
#define Showdenoise /##/
#endif

template<class cls>
vector<vector<cls>> classifyContinuous(const vector<cls>& arr, function<const int(cls)> getInt);

void Splitter::start(vector<Mat>& piece) {
	const Mat hkernel = getStructuringElement(MORPH_RECT, Size(max(org.cols / 2, 1), 1)); 
	auto hMORPH = [this, &hkernel](Mat& r, short threadNum) {
		//terriblly slow... so multi-thread...
		atomic_int cnt = threadNum;
		int y = 0;
		int step = org.rows / threadNum;
		auto forthread = [&](const int y, const int len) {
			const Rect roi(0, y, org.cols, len);
			morphologyEx(255 - org(roi), r(roi), MORPH_CLOSE, hkernel);		//TODO: unknown bug... 
			cnt--;
		};
		for (short i = 1; i < threadNum; i++) {
			thread t(forthread, y, step);
			t.detach();
			y += step;
		}
		thread t(forthread, y, org.rows - y);
		t.join();
		while (cnt > 0) this_thread::yield();
	};
	
	Mat r(org.rows, org.cols, org.type());
	hMORPH(r, max(thread::hardware_concurrency(), 2u));
	r = Morphology(r, org.cols / 100, false, true);
	Mat ccolor;
	cvtColor(org, ccolor, CV_GRAY2BGR);
	vector<vector<Point>> cont;
	findContours(r, cont, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
	drawContours(ccolor, cont, -1, Scalar(0, 0, 255));

	size_t n = cont.size();
	vector<Rect> region(n);
	piece.resize(n);

	std::transform(cont.begin(), cont.end(), region.begin(), [](vector<Point> x) -> Rect {return boundingRect(x); });
	std::sort(region.begin(), region.end(), [](const Rect x, const Rect y) -> bool {return x.y < y.y; });

	std::transform(region.begin(), region.end(), piece.begin(), [this](Rect x) -> Mat {return org(x).clone(); });

	piece.erase(remove_if(piece.begin(), piece.end(), [](Mat & x) -> bool {return x.empty(); }), piece.end());
	piece.shrink_to_fit();
}

Mat Denoiser::denoise_morphology() {
	Mat mask1, mask2, r;
	auto fullfill = [](Mat & mask) {
		vector<vector<Point>> cont;
		findContours(mask, cont, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
		size_t n = cont.size();
		vector<Rect> region(n);
		
		for (int i = 0; i < n; i++)  rectangle(mask, boundingRect(cont[i]), Scalar(255), -1);
	};
	mask1 = Morphology(255 - org, org.cols / 10, true, false);
	fullfill(mask1);
	mask2 = Morphology(255 - org, org.rows / 6, false, false);
	fullfill(mask2);
	r = mask1 | mask2 | org;

	Showdenoise imdebug("denoise(Morphology)", r);
	return r;
}

Mat Denoiser::denoise_inpaint(vector<Vec4i> lines, double radius) {
	Mat r, mask = Mat(org.size(), CV_8UC1, Scalar::all(0));
	for (Vec4i& i : lines) line(mask, Point(i[0], i[1]), Point(i[2], i[3]), Scalar(255));
	
	inpaint(r, org, mask);
	threshold(r, r, 200, 255, THRESH_BINARY);

	Showdenoise imdebug("denoise(Inpaint)", r);

	return r;
}

/*
	fix the pixels that may be cleared by morphology-denoise. 
	requires: lines.size() == width.size() ==  6. 
	only the line i that width[i] == 1 is fixed. 
	@param img, back-ground is black, fore-ground is white. 
*/
void Denoiser::inpaint(cv::Mat& img, std::vector<cv::Vec4i> lines, std::vector<int> width)
{
	vector<int> fix;
	for (int i = 0; i < width.size(); i++) if (width[i] == 1) fix.emplace_back(lines[i][1]);
	const auto l = [](const uchar* up, const uchar* down, int c) -> const uchar {
		return min(up[c - 1], max(down[c], down[c + 1]));
	};
	const auto r = [](const uchar* up, const uchar* down, int c) -> const uchar {
		return min(up[c + 1], max(down[c], down[c - 1]));
	};
	const auto m = [](const uchar* up, const uchar* down, int c) -> const uchar {
		return min(up[c], max({ down[c - 1], down[c], down[c + 1] }));
	};

	for (int i : fix) {
		if (i <= 1 || i >= img.rows - 1) continue;
		uchar* up = img.ptr<uchar>(i - 1);
		uchar* cur = img.ptr<uchar>(i);
		uchar* down = img.ptr<uchar>(i + 1);
		
		int j = 0;
		cur[j] = max({ cur[j], min(up[j], max(down[j], down[j + 1])), min(up[j + 1], down[j]) });
		for (j = 1; j < img.cols - 1; j++) {
			cur[j] = max({ cur[j], l(up, down, j), r(up, down, j), m(up, down, j) });
		}
		cur[j] = max({ cur[j], min(up[j - 1], down[j]), min(up[j], max(down[j - 1], down[j])) });
	}
}


/*
	* @brief			概率霍夫变换提取横线
	* @param[in]		lines，筛选后的结果
	*/
void LineFinder::findRow(vector<Vec4i> & lines) {
	HoughLinesP(255 - org, lines, 1, CV_PI / 180, 100, org.cols * 2 / 3, org.cols / 50);


	//角度筛选
	double t = tan(range);
	lines.erase(remove_if(lines.begin(), lines.end(), [t, this](const Vec4i x) -> bool {
		return x[2] == x[0] 
			|| abs(((double)x[3] - x[1]) / (x[2] - x[0])) > t
			|| isDotLine(x);
	}), lines.end());

	//sort
	if (!lines.size()) return;
	sort(lines.begin(), lines.end(), [](const Vec4i x, const Vec4i y) ->bool {
		return x[1] < y[1];
	});


	//去除连续
	auto rcls = classifyContinuous<Vec4i>(lines, [](const Vec4i x) -> int {return x[1]; });
	vector<Vec4i> copy(lines);
	thickness.resize(rcls.size());
	lines.resize(rcls.size());
	std::transform(rcls.begin(), rcls.end(), thickness.begin(), [this](const vector<Vec4i> x) -> int { int i = (int)x.size();  const_cast<GlobalUnit&>(global["lineThickness"]) += i; return int(i); });
	std::transform(rcls.begin(), rcls.end(), lines.begin(), [](const vector<Vec4i> x) -> Vec4i {return x[x.size() / 2]; });

	upper = std::min(lines[5][1], lines[5][3]);
	lower = std::max(lines[0][1], lines[0][3]);
#if 0
	Mat color;
	cvtColor(org, color, CV_GRAY2BGR);
	for (Vec4i& i : lines) {
		line(color, Point(i[0], i[1]), Point(i[2], i[3]), Scalar(0, 0, 255));
	}
	imdebug("Cuckoo Debug: Show rows", color);
#endif
}


/*
	 * 函数：findCol
	 * @brief				概率霍夫变换提取竖线
	 * param[in]			lines，筛选后的结果
	*/
void LineFinder::findCol(vector<Vec4i> & lines) {
	int length = upper - lower;
	int thick = max(1, *max_element(thickness.begin(), thickness.end()));
	Mat toScan = org(Range(lower, upper), Range::all()).clone();
	toScan = Morphology(toScan, max(thick, 3), false, false);
	Mat inv = 255 - Morphology(toScan, max(toScan.rows / 2, 1), false, true);


	//80:140
	HoughLinesP(inv, lines, 1, CV_PI / 180, (toScan.rows - 2 * thick) / 2, toScan.rows - 2 * thick, thick + 2);

	//range filter
	double t = tan(CV_PI / 2.0 - range);
	lines.erase(remove_if(lines.begin(), lines.end(), [t](const Vec4i x) -> bool {
		return x[0] != x[2] && abs(((double)x[3] - x[1]) / (x[2] - x[0])) < t; 
	}), lines.end());

	//sort it
	sort(lines.begin(), lines.end(), [](const Vec4i x, const Vec4i y) ->bool {
		return x[0] < y[0];
	});


	//去除连续
	auto rcls = classifyContinuous<Vec4i>(lines, [](const Vec4i x) ->int {return x[0]; });
	lines.resize(rcls.size());
	std::transform(rcls.begin(), rcls.end(), lines.begin(), [](const vector<Vec4i> x) -> Vec4i {return x[x.size() / 2]; });
	lines.shrink_to_fit();


	for (size_t i = 1; i < lines.size(); i++) {
		if (lines[i][0] - lines[i - 1][0] >= max(16, global["colLenth"] / 5)) {
			const_cast<GlobalUnit&>(global["colLenth"]) += lines[i][0] - lines[i - 1][0];
		}
	}

#if 0
	Mat ccolor;
	cvtColor(org, ccolor, CV_GRAY2BGR);
	for (Vec4i& i : lines) {
		line(ccolor, Point(i[0], 0), Point(i[2], org.rows), Scalar(0, 0, 255));
	}
	imdebug("Cuckoo Debug: Show cols", ccolor);
#endif
}


bool LineFinder::isDotLine(cv::Vec4i line) {
	/*
	 * @brief 判断一条直线是否为虚线
	*/
	auto y = [line](int x) -> int {
		return line[1] + (line[3] - line[1]) / (line[2] - line[0]) * (x - line[0]);
	};
	int sum = 0;
	for (int i = line[0]; i <= line[2]; i++) {
		if (!org.at<uchar>(y(i), i)) {
			sum++;
		}
	}
	if (sum < 0.7 * (line[2] - line[0])) {
		return true;
	}
	return false;
}

template<class cls>
vector<vector<cls>> classifyContinuous(const vector<cls>& arr, function<const int(cls)> getInt) {
	size_t n = arr.size();
	if (n < 1) return vector<vector<cls>>();

	int t, t1 = getInt(arr[0]);
	vector<vector<cls>> r{ {arr[0]} };

	for (size_t i = 1; i < n; i++) {
		t = t1;
		t1 = getInt(arr[i]);
		if (t + 1 >= t1) r.back().emplace_back(arr[i]);
		else r.emplace_back(vector<cls>{ arr[i] });
	}

	return r;
}