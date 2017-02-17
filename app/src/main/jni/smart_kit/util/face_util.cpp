#include "face_util.h"
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

namespace util {



int imgMean(unsigned char *yuv420sp , int xstart , int ystart , int xend , int yend , int height , int width)
{
	int i, j , subheight , subwidth ;
	int meanface = 0;
	int tempv  = 0;
	int yp = 0;

	if(xstart < 0)
		xstart = 0 ;
	if(ystart < 0)
		ystart = 0 ;
	if(xend >= width)
		xend = width -1 ;
	if(yend >= height)
		yend = height-1 ;

	subheight = yend - ystart + 1 ;
	subwidth  = xend - xstart + 1 ;

	int subLength = subwidth * subheight;

	if( subLength > 0 ){
		for ( j = ystart ; j <= yend; j++ )
		{
			yp  = j*width + xstart;
			for( i = xstart ; i <= xend ; i++){
				tempv = tempv + yuv420sp[yp] ;
				yp++;
			}
		}
		meanface = tempv / subLength;
	}else{
		meanface = 90;
	}

	return meanface;
}

unsigned char imgMaxVal(unsigned char *yuv420sp, int ystart, int xstart,
		int yend, int xend, int height, int width) {
	//imgShow(yuv420sp,ystart,xstart,yend,xend,height,width);
	int i, j, yp, subheight, subwidth;
	unsigned char maxvalue = 0;

	subheight = yend - ystart + 1;
	subwidth = xend - xstart + 1;

	for (j = ystart, yp = 0; j <= yend; j++) {
		yp = j * width;
		yp = yp + xstart;

		for (i = xstart; i <= xend; i++, yp++) {
			unsigned char Y = yuv420sp[yp];

			if (Y > maxvalue) {
				maxvalue = Y;
			}
		}
	}
	return maxvalue;
}


bool check_face_YUV(const cv::Mat &mat_face) {
	std::vector<float> hist = util::get_color_histogram_of_image_YUV(mat_face);

	if (hist.empty()) {
		return false;
	}

	int count[3] = { 8, 8, 8 }; // bins per channel

	float _max[3] = { 0, 0, 0 };
	for (int i = 0; i < count[0]; ++i) {
		if (hist[i] > _max[0]) {
			_max[0] = hist[i];
		}
	}
	for (int i = count[0]; i < count[0] + count[1]; ++i) {
		if (hist[i] > _max[1]) {
			_max[1] = hist[i];
		}
	}
	for (int i = count[0] + count[1]; i < (int) hist.size(); ++i) {
		if (hist[i] > _max[2]) {
			_max[2] = hist[i];
		}
	}

	return _max[0] > 0.5 && _max[1] > 0.5 && _max[2] < 0.45;
}

std::vector<float> get_color_histogram_of_image_YUV(const cv::Mat &img) {
	std::vector<float> result;

	if (img.channels() != 3) { // must have 3 channels
		return result;
	}

	/// Separate the image in 3 places ( B, G and R )
	std::vector<cv::Mat> yuv_planes;
	split(img, yuv_planes);

	/// Establish the number of bins
	int histSize = 8;

	/// Set the ranges ( for Y,U,V) )
	float range[] = { 0, 256 }; //0~255 the upper boundary is exclusive
	const float * histRange = { range };
	bool uniform = true;
	bool accumulate = false;
	cv::Mat b_hist, g_hist, r_hist;

	/// Compute the histograms:
	cv::calcHist(&yuv_planes[0], 1, 0, cv::Mat(), b_hist, 1, &histSize, &histRange,
			uniform, accumulate);
	cv::calcHist(&yuv_planes[1], 1, 0, cv::Mat(), g_hist, 1, &histSize, &histRange,
			uniform, accumulate);
	cv::calcHist(&yuv_planes[2], 1, 0, cv::Mat(), r_hist, 1, &histSize, &histRange,
			uniform, accumulate);

	/// stored in result
	for (int i = 0; i < histSize; i++)
		result.push_back(r_hist.at<float>(i, 0));
	for (int i = 0; i < histSize; i++)
		result.push_back(g_hist.at<float>(i, 0));
	for (int i = 0; i < histSize; i++)
		result.push_back(b_hist.at<float>(i, 0));

	util::normalized(result);

	return result;
}

void normalized(std::vector<float> &a) {
	float length = 0;
	for (size_t i = 0; i < a.size(); i++)
		length += a[i] * a[i];
	length = sqrtf(length);

	if (length < 1e-6) {
		return;
	}

	for (size_t i = 0; i < a.size(); i++)
		a[i] /= length;
}

} //namespace util
