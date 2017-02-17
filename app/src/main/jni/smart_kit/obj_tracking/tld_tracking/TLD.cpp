#include "TLD.h"
#include <stdio.h>

using namespace cv;
using namespace std;

namespace smart {
TLD::TLD() {
	min_win = 16;
	patch_size = 16;
	num_closest_init = 10;
	num_warps_init = 20;
	noise_init = 5;
	angle_init = 20;
	shift_init = 0.02f;
	scale_init = 0.02f;
	//update parameters for positive examples
	num_closest_update = 10;
	num_warps_update = 10;
	noise_update = 5;
	angle_update = 10;
	shift_update = 0.02f;
	scale_update = 0.02f;
	//parameters for negative examples
	bad_overlap = 0.2f;
	bad_patches = 100;
	//parameters for algorithm logic adjustment
	detect_interval = 2;	//Perform detection for each 2 frames
	framecnt = 0;			//frame counter
	scale_ratio = 0.05;
	classifier.init();
}
TLD::TLD(const FileNode& file) {
	read(file);
}

void TLD::read(const FileNode& file) {
	///Bounding Box Parameters
	min_win = (int) file["min_win"];
	///Genarator Parameters
	//initial parameters for positive examples
	patch_size = (int) file["patch_size"];
	num_closest_init = (int) file["num_closest_init"];
	num_warps_init = (int) file["num_warps_init"];
	noise_init = (int) file["noise_init"];
	angle_init = (float) file["angle_init"];
	shift_init = (float) file["shift_init"];
	scale_init = (float) file["scale_init"];
	//update parameters for positive examples
	num_closest_update = (int) file["num_closest_update"];
	num_warps_update = (int) file["num_warps_update"];
	noise_update = (int) file["noise_update"];
	angle_update = (float) file["angle_update"];
	shift_update = (float) file["shift_update"];
	scale_update = (float) file["scale_update"];
	//parameters for negative examples
	bad_overlap = (float) file["overlap"];
	bad_patches = (int) file["num_patches"];
	classifier.read(file);
}

void TLD::init_clear() {
	grid.clear();
	iisum.release();
	iisqsum.release();
	dconf.clear();
	dbb.clear();
	tmp.conf.clear();
	tmp.patt.clear();
	good_boxes.clear();
	bad_boxes.clear();
	pEx.release();
	pExR.release();
	pExG.release();
	pExB.release();
	pX.clear();
	nX.clear();
	nEx.clear();
	nXT.clear();
	dvalid.clear();
	dt.bb.clear();
	dt.conf1.clear();
	dt.conf2.clear();
	dt.isin.clear();
	dt.patch.clear();
	dt.patt.clear();
}

void TLD::init(const Mat& frame1, const Mat& frame1Cb, const Mat& frame1Cr,const Rect& initbox, FILE* bb_file) {
	//bb_file = fopen("bounding_boxes.txt","w");
	//Protection for min ROI

//	cv::Scalar meanCb = cv::mean(frame1Cb);
//	cv::Scalar meanCr = cv::mean(frame1Cr);

	LOGD("boo-tld-step-1");
	init_clear();
	Rect box;
	box = initbox;
	if (initbox.width < min_win || initbox.height < min_win) {
		box.width = MAX(initbox.width, min_win);
		box.height = MAX(initbox.height, min_win);
		float centre_x = (float) initbox.x + (float) initbox.width / 2;
		float centre_y = (float) initbox.y + (float) initbox.height / 2;
		box.x = (int) (centre_x - (float) box.width / 2 + 0.5f);
		box.y = (int) (centre_y - (float) box.height / 2 + 0.5f);
	}
	det_window = (int) (MAX(frame1.cols, frame1.rows) / 10);
	//isTrackerInit = false;
	tbb = box;
	//Get Bounding Boxes
	buildGrid(frame1, box);
	printf("Created %d bounding boxes\n", (int) grid.size());
	///Preparation
	//allocation
	iisum.create(frame1.rows + 1, frame1.cols + 1, CV_32F);
	iisqsum.create(frame1.rows + 1, frame1.cols + 1, CV_64F);
	dconf.reserve(100);
	dbb.reserve(100);
	bbox_step = 7;
	//tmp.conf.reserve(grid.size());
	tmp.conf = vector<float>(grid.size());
	tmp.patt = vector < vector<int> > (grid.size(), vector<int>(10, 0));
	//tmp.patt.reserve(grid.size());
	dt.bb.reserve(grid.size());
	good_boxes.reserve(grid.size());
	bad_boxes.reserve(grid.size());
	pEx.create(patch_size, patch_size, CV_64FC1);
	pExR.create(patch_size, patch_size, CV_32FC1);
	pExG.create(patch_size, patch_size, CV_32FC1);
	pExB.create(patch_size, patch_size, CV_32FC1);
	//Init Generator
	generator = PatchGenerator(0, 0, noise_init, true, 1 - scale_init,
			1 + scale_init, -angle_init * CV_PI / 180, angle_init * CV_PI / 180,
			-angle_init * CV_PI / 180, angle_init * CV_PI / 180);
	getOverlappingBoxes(box, num_closest_init);
	printf("Found %d good boxes, %d bad boxes\n", (int) good_boxes.size(),
			(int) bad_boxes.size());
	printf("Best Box: %d %d %d %d\n", best_box.x, best_box.y, best_box.width,
			best_box.height);
	printf("Bounding box hull: %d %d %d %d\n", bbhull.x, bbhull.y, bbhull.width,
			bbhull.height);
	//Correct Bounding Box
	lastbox = best_box;
	lastconf = 1;
	lastvalid = true;
	//Print
	//fprintf(bb_file, "_*_*_*_*_*_*_*_*\n");
	//fprintf(bb_file,"%d,%d,%d,%d,%f\n",lastbox.x,lastbox.y,lastbox.br().x,lastbox.br().y,lastconf);
	//Prepare Classifier
	classifier.prepare(scales);
	///Generate Data
	// Generate positive data

	generatePositiveData(frame1, frame1Cb, frame1Cr, num_warps_init);
	LOGD("boo-tld-step-2");
	// Set variance threshold
	Scalar stdev, mean;
	meanStdDev(frame1(best_box), mean, stdev);
	integral(frame1, iisum, iisqsum);
	var = pow(stdev.val[0], 2) * 0.5; //getVar(best_box,iisum,iisqsum);
	cout << "variance: " << var << endl;
	//check variance
	double vr = getVar(best_box, iisum, iisqsum) * 0.5;
	cout << "check variance: " << vr << endl;
	// Generate negative data
	generateNegativeData(frame1);
	//Split Negative Ferns into Training and Testing sets (they are already shuffled)
	int half = (int) nX.size() * 0.5f;
	nXT.assign(nX.begin() + half, nX.end());
	nX.resize(half);
	///Split Negative NN Examples into Training and Testing sets
	half = (int) nEx.size() * 0.5f;
	nExT.assign(nEx.begin() + half, nEx.end());
	nEx.resize(half);
	//Merge Negative Data with Positive Data and shuffle it
	vector < pair<vector<int>, int> > ferns_data(nX.size() + pX.size());
	vector<int> idx = index_shuffle(0, ferns_data.size());
	int a = 0;
	for (int i = 0; i < pX.size(); i++) {
		ferns_data[idx[a]] = pX[i];
		a++;
	}
	for (int i = 0; i < nX.size(); i++) {
		ferns_data[idx[a]] = nX[i];
		a++;
	}
	//Data already have been shuffled, just putting it in the same vector
	vector<cv::Mat> nn_data(nEx.size() + 1);
	nn_data[0] = pEx;
	for (int i = 0; i < nEx.size(); i++) {
		nn_data[i + 1] = nEx[i];
	}
	///Training
	classifier.trainF(ferns_data, 2); //bootstrap = 2

	classifier.trainNN(nn_data, pExB, pExG, pExR);	
	LOGD("boo-tld-step-3");
	///Threshold Evaluation on testing sets
	classifier.evaluateTh(nXT, nExT);

	//Initialize MFT tracker
	tracker.start_tracking(frame1, initbox);
}

/* Generate Positive data
 * Inputs:
 * - good_boxes (bbP)
 * - best_box (bbP0)
 * - frame (im0)
 * Outputs:
 * - Positive fern features (pX)
 * - Positive NN examples (pEx)
 */
void TLD::generatePositiveData(const Mat& frame, const Mat& frameCb,const Mat& frameCr, int num_warps) {
	Scalar mean;
	Scalar stdev;

	getPattern(frame(best_box), pEx, mean, stdev);
	LOGD("boo-tld-step-1-1");

	cv::Mat dstImg = cv::Mat(best_box.height, best_box.width, CV_8UC3);

	cv::Mat img_y = frame(best_box);

	LOGD("boo-tld-step-1-1-1");
	cv::Scalar meanCb = cv::mean(frameCb);
	cv::Scalar meanCr = cv::mean(frameCr);

	LOGD("meanCb = %f  , [ %d , %d ]  meanCr = %f , [ %d , %d] ", meanCb[0],
			frameCb.cols, frameCb.rows, meanCr[0], frameCr.cols, frameCr.rows);
	LOGD("bestbox = [%d ,%d ,%d ,%d ]", best_box.x, best_box.y, best_box.height,
			best_box.width);

	LOGD("boo-tld-step-1-1-2");
	cv::Mat img_cb = frameCb(best_box);

	LOGD("boo-tld-step-1-1-3");

	cv::Mat img_cr = frameCr(best_box);

	std::vector<cv::Mat> mv;
	mv.push_back(img_y);
	mv.push_back(img_cr);
	mv.push_back(img_cb);
	cv::merge(mv, dstImg);

	cv::cvtColor(dstImg, dstImg, CV_YCrCb2RGB);
	LOGD("boo-tld-step-1-2");
	//cv::imshow("dstImg", dstImg);
	dstImg.convertTo(dstImg, CV_32FC3);

	std::vector<cv::Mat> img_bgr;
	cv::split(dstImg, img_bgr);

	cv::resize(img_bgr[0], pExB, cv::Size(patch_size, patch_size));
	cv::resize(img_bgr[1], pExG, cv::Size(patch_size, patch_size));
	cv::resize(img_bgr[2], pExR, cv::Size(patch_size, patch_size));
	LOGD("boo-tld-step-1-3");
	//Get Fern features on warped patches
	Mat img;
	Mat warped;
	GaussianBlur(frame, img, Size(9, 9), 1.5);
	warped = img(bbhull);
	RNG& rng = theRNG();
	Point2f pt(bbhull.x + (bbhull.width - 1) * 0.5f,
			bbhull.y + (bbhull.height - 1) * 0.5f);
	vector<int> fern(classifier.getNumStructs());
	pX.clear();
	Mat patch;
	if (pX.capacity() < num_warps * good_boxes.size())
		pX.reserve(num_warps * good_boxes.size());
	int idx;
	for (int i = 0; i < num_warps; i++) {
		if (i > 0)
			generator(frame, pt, warped, bbhull.size(), rng);
		for (int b = 0; b < good_boxes.size(); b++) {
			idx = good_boxes[b];
			patch = img(grid[idx]);
			classifier.getFeatures(patch, grid[idx].sidx, fern);
			pX.push_back(make_pair(fern, 1));
		}
	}
	LOGD("boo-tld-step-1-4");
	//LOGD("Positive examples generated: ferns:%d NN:1\n",(int)pX.size());
}

void TLD::getPattern(const Mat& img, Mat& pattern, Scalar& mean,
		Scalar& stdev) {
	//Output: resized Zero-Mean patch
	resize(img, pattern, Size(patch_size, patch_size));
	meanStdDev(pattern, mean, stdev);
	pattern.convertTo(pattern, CV_32F);
	pattern = pattern - mean.val[0];
}

void TLD::generateNegativeData(const Mat& frame) {
	/* Inputs:
	 * - Image
	 * - bad_boxes (Boxes far from the bounding box)
	 * - variance (pEx variance)
	 * Outputs
	 * - Negative fern features (nX)
	 * - Negative NN examples (nEx)
	 */
	random_shuffle(bad_boxes.begin(), bad_boxes.end()); //Random shuffle bad_boxes indexes
	int idx;
	//Get Fern Features of the boxes with big variance (calculated using integral images)
	int a = 0;
	//int num = std::min((int)bad_boxes.size(),(int)bad_patches*100); //limits the size of bad_boxes to try
	//LOGD("negative data generation started.\n");
	vector<int> fern(classifier.getNumStructs());
	nX.reserve(bad_boxes.size());
	Mat patch;
	for (int j = 0; j < bad_boxes.size(); j++) {
		idx = bad_boxes[j];
		if (getVar(grid[idx], iisum, iisqsum) < var * 0.5f)
			continue;
		patch = frame(grid[idx]);
		classifier.getFeatures(patch, grid[idx].sidx, fern);
		nX.push_back(make_pair(fern, 0));
		a++;
	}
	//LOGD("Negative examples generated: ferns: %d ",a);
	//random_shuffle(bad_boxes.begin(),bad_boxes.begin()+bad_patches);//Randomly selects 'bad_patches' and get the patterns for NN;
	Scalar dum1, dum2;
	nEx = vector < Mat > (bad_patches);
	for (int i = 0; i < bad_patches; i++) {
		idx = bad_boxes[i];
		patch = frame(grid[idx]);
		getPattern(patch, nEx[i], dum1, dum2);
	}
	//LOGD("NN: %d\n",(int)nEx.size());
}

double TLD::getVar(const BoundingBox& box, const Mat& sum, const Mat& sqsum) {
	double brs = sum.at<int>(box.y + box.height, box.x + box.width);
	double bls = sum.at<int>(box.y + box.height, box.x);
	double trs = sum.at<int>(box.y, box.x + box.width);
	double tls = sum.at<int>(box.y, box.x);
	double brsq = sqsum.at<double>(box.y + box.height, box.x + box.width);
	double blsq = sqsum.at<double>(box.y + box.height, box.x);
	double trsq = sqsum.at<double>(box.y, box.x + box.width);
	double tlsq = sqsum.at<double>(box.y, box.x);
	double mean = (brs + tls - trs - bls) / ((double) box.area());
	double sqmean = (brsq + tlsq - trsq - blsq) / ((double) box.area());
	return sqmean - mean * mean;
}

void TLD::processFrame(/*const cv::Mat& img1,*/const cv::Mat& img,const cv::Mat& imgCb, const cv::Mat& imgCr,/*vector<Point2f>& points1,vector<Point2f>& points2,*/BoundingBox& bbnext, bool& lastboxfound, bool tl) {
	vector<BoundingBox> cbb;
	vector<float> cconf;
	int confident_detections = 0;
	int didx; //detection index
	cv::Rect_<float> fbbNext;
	///Track
	LOGD("boo-tld-step-4");
	if (lastboxfound && tl) {
		LOGD("tld : track ...");
		track(img);
	} else {
		tracked = false;

	}
	LOGD("boo-tld-step-5");
	///Integration

	if (tracked) {
		fbbNext = ftbb;
		lastconf = tconf;
		lastvalid = tvalid;
		if (framecnt % detect_interval == 0) {
			///Detect
			LOGD("tld : detect ...");
			detect(img, imgCb, imgCr);

			//t = (double)getTickCount() - t;
			//printf("TLD-Detection %gms\n", t * 1000 / getTickFrequency());

			//framecnt = 0;

			//printf("Tracked\n");
			if (detected) {                                     //   if Detected
				clusterConf(dbb, dconf, cbb, cconf);     //   cluster detections
				//LOGD("tld :Found %d clusters\n", (int)cbb.size());
				for (int i = 0; i < cbb.size(); i++) {
					if (bbOverlap(tbb, cbb[i]) < 0.5 && cconf[i] > tconf) { //  Get index of a clusters that is far from tracker and are more confident than the tracker
						confident_detections++;
						didx = i; //detection index
					}
				}
				if (confident_detections == 1) { //if there is ONE such a cluster, re-initialize the tracker
					//LOGD("tld :Found a better match..reinitializing tracking\n");

					//float dist = sqrt(img.cols*img.cols + img.rows*img.rows);
					float tdx = ((ftbb.x + ftbb.width / 2)	- (lastbox.x + lastbox.width / 2));
					float tdy = ((ftbb.y + ftbb.height / 2)- (lastbox.y + lastbox.height / 2));

					float ddx = (((float) cbb[didx].x	+ (float) cbb[didx].width / 2)- (lastbox.x + lastbox.width / 2));
					float ddy = (((float) cbb[didx].y+ (float) cbb[didx].height / 2)- (lastbox.y + lastbox.height / 2));

					float tdist = sqrt(tdx * tdx + tdy * tdy);
					float ddist = sqrt(ddx * ddx + ddy * ddy);

					float tweight = (1 - tdist / (tdist + ddist));

					//float weight = 1 - 5 * sqrt(dx*dx + dy*dy)/dist;     //Give more weights to tracking result if the offset btw previous and prediction is close (thres = 1/10 diag)
					//weight = MAX(weight, 0.0f);

					fbbNext.x = ftbb.x * tweight + cbb[didx].x * (1 - tweight);
					fbbNext.y = ftbb.y * tweight + cbb[didx].y * (1 - tweight);
					fbbNext.width = ftbb.width * tweight+ cbb[didx].width * (1 - tweight);
					fbbNext.height = ftbb.height * tweight+ cbb[didx].height * (1 - tweight);

					lastconf = tconf * tweight + cconf[didx] * (1 - tweight);

					//bbnext = cbb[didx];
					//lastconf = cconf[didx];
					lastvalid = false;
				} else {
					LOGD("tld :%d confident cluster was found\n",confident_detections);
					int cx = 0, cy = 0, cw = 0, ch = 0;
					int close_detections = 0;
					float weight_sum = 0.0f;
					for (int i = 0; i < dbb.size(); i++) {
						if (bbOverlap(tbb, dbb[i]) > 0.7) { // Get weighted mean of close detections
							cx += dbb[i].x * dconf[i];
							cy += dbb[i].y * dconf[i];
							cw += dbb[i].width * dconf[i];
							ch += dbb[i].height * dconf[i];
							close_detections++;
							weight_sum += dconf[i];
							LOGD("weighted detection: %d %d %d %d\n", dbb[i].x,	dbb[i].y, dbb[i].width, dbb[i].height);
						}
					}
					if (close_detections > 0) {
						float det_width = cw / weight_sum;
						float det_height = ch / weight_sum;
						//float det_cx = cx / weight_sum;
						//float det_cy = cy / weight_sum;
						float ratio_x = det_width / lastbox.width;
						float ratio_y = det_height / lastbox.height;

						if (ratio_x < 1.01 && ratio_x > 0.99)
							det_width = ftbb.width;
						if (ratio_y < 1.01 && ratio_y > 0.99)
							det_height = ftbb.height;

						//float tdx = ((ftbb.x + ftbb.width / 2) - (lastbox.x + lastbox.width / 2));
						//float tdy = ((ftbb.y + ftbb.height / 2) - (lastbox.y + lastbox.height / 2));

						//float ddx = ((det_cx + det_width / 2) - (lastbox.x + lastbox.width / 2));
						//float ddy = ((det_cy + det_height / 2) - (lastbox.y + lastbox.height / 2));

						//float tdist = sqrt(tdx*tdx + tdy*tdy);
						//float ddist = sqrt(ddx*ddx + ddy*ddy);

						//float tweight = (1 - tdist / (tdist + ddist));
						//
						//fbbNext.x = tweight * ftbb.x + (1 - tweight)*cx / weight_sum;									// weighted average trackers trajectory with the close detections
						//fbbNext.y = tweight * ftbb.y + (1 - tweight)*cy / weight_sum;
						//fbbNext.width = tweight* ftbb.width + (1 - tweight)*det_width;
						//fbbNext.height = tweight * ftbb.height + (1 - tweight)* det_height;

						fbbNext.x = (2 * ftbb.x + cx / weight_sum) / 3;	// weighted average trackers trajectory with the close detections
						fbbNext.y = (2 * ftbb.y + cy / weight_sum) / 3;
						fbbNext.width = (2 * ftbb.width + det_width) / 3;
						fbbNext.height = (2 * ftbb.height + det_height) / 3;

						//fbbNext.x = ((float)(10 * ftbb.x + cx) / (float)(10 + close_detections));   // weighted average trackers trajectory with the close detections
						//fbbNext.y = ((float)(10 * ftbb.y + cy) / (float)(10 + close_detections));
						//fbbNext.width = ((float)(10 * ftbb.width + cw) / (float)(10 + close_detections));
						//fbbNext.height = ((float)(10 * ftbb.height + ch) / (float)(10 + close_detections));
						//LOGD("Tracker bb: %d %d %d %d\n", tbb.x, tbb.y, tbb.width, tbb.height);
						//LOGD("Average bb: %d %d %d %d\n", bbnext.x, bbnext.y, bbnext.width, bbnext.height);
						//LOGD("Weighting %d close detection(s) with tracker..\n", close_detections);
					} else {
						//LOGD("%d close detections were found\n", close_detections);

					}
				}
			}
		}
	} else {                                       //   If NOT tracking
		LOGD("Not tracking..\n");
		lastboxfound = false;
		lastvalid = false;
		///Detect
		detect(img, imgCb, imgCr);
		if (detected) {                           //  and detector is defined
			clusterConf(dbb, dconf, cbb, cconf);   //  cluster detections
			LOGD("Found %d clusters\n", (int )cbb.size());
			if (cconf.size() == 1) {
				fbbNext = cbb[0];
				lastconf = cconf[0];
				LOGD("Confident detection..reinitializing tracker\n");
				lastboxfound = true;
				tracker.start_tracking(img, cbb[0]);//Re-initialize the tracker
			}
		}
	}
	bbnext.x = cvRound(fbbNext.x);
	bbnext.y = cvRound(fbbNext.y);
	bbnext.height = cvRound(fbbNext.height);
	bbnext.width = cvRound(fbbNext.width);
	//lastbox = bbnext;
	tracker.reset_boundingbox(fbbNext);
	framecnt++;
	LOGD("boo-tld-step-6 , fbbNext [%f , %f ,%f , %f]", fbbNext.x, fbbNext.y,
			fbbNext.width, fbbNext.height);
	//if (lastboxfound)
	//  fprintf(bb_file,"%d,%d,%d,%d,%f\n",lastbox.x,lastbox.y,lastbox.br().x,lastbox.br().y,lastconf);
	//else
	//  fprintf(bb_file,"NaN,NaN,NaN,NaN,NaN\n");
	if (lastvalid && tl && framecnt % detect_interval == 0) {
		LOGD("tld : learn ...");
		//learn(img);
		learn(prev_image_, prev_image_Cb, prev_image_Cr);
		//t = (double)getTickCount() - t;
		//printf("TLD-Learning %gms\n", t * 1000 / getTickFrequency());
	}
	LOGD("boo-tld-step-7");
	prev_image_ = img.clone();//Speed up by seperating learn and detection into different frames
	prev_image_Cb = imgCb.clone();//Speed up by seperating learn and detection into different frames
	prev_image_Cr = imgCr.clone();//Speed up by seperating learn and detection into different frames
	lastbox = bbnext;
}

void TLD::track(const Mat& img/*,vector<Point2f>& points1,vector<Point2f>& points2*/) {
	clock_t start, end;
	start = clock();
	/*Inputs:
	 * -current frame(img2), last frame(img1), last Bbox(bbox_f[0]).
	 *Outputs:
	 *- Confidence(tconf), Predicted bounding box(tbb),Validity(tvalid), points2 (for display purposes only)
	 */

	//if (!isTrackerInit){
	//	tracker.start_tracking(img, tbb);
	//	ftbb = tbb;
	//	tracker.setParameter(scale_ratio);
	//	//isTrackerInit = true;
	//}
	//else{
	//Generate points
	//bbPoints(points1,lastbox);
	//if (points1.size()<1){
	//    LOGD("BB= %d %d %d %d, Points not generated\n",lastbox.x,lastbox.y,lastbox.width,lastbox.height);
	//    tvalid=false;
	//    tracked=false;
	//    return;
	//}
	//vector<Point2f> points = points1;
	//Frame-to-frame tracking with forward-backward error cheking
	//tracked = tracker.trackf2f(img1,img2,points,points2);
	//LOGD("tld : predict 0 %d,%d,%d,%d\n",tbb.x,tbb.y,tbb.br().x,tbb.br().y);
	tracked = tracker.predict(img, tbb);
	//LOGD("tld : predict 1 %d,%d,%d,%d\n",tbb.x,tbb.y,tbb.br().x,tbb.br().y);
	ftbb = tracker.get_subpixel_res();
	if (tracked) {
		if (tracker.getErr() > 10 || tbb.x > img.cols || tbb.y > img.rows || tbb.br().x < 1 || tbb.br().y < 1 || tbb.x <= 0 || tbb.y <= 0 || (tbb.x + tbb.width) >= img.cols || (tbb.y + tbb.height) >= img.rows) {
			tvalid = false; //too unstable prediction
			tracked = false;
			LOGD("tld :Too unstable predictions FB error=%f\n",tracker.getErr());
			return;
		}
		LOGD("boo-tld-1 :tbb-[ %d , %d , %d , %d]\n", tbb.x, tbb.y, tbb.width,tbb.height);
		//Estimate Confidence and Validity
		Mat pattern;
		Scalar mean, stdev;
		BoundingBox bb;
		bb.x = max(tbb.x, 0);
		bb.y = max(tbb.y, 0);
		bb.width = min(min(img.cols - tbb.x, tbb.width),min(tbb.width, tbb.br().x));
		bb.height = min(min(img.rows - tbb.y, tbb.height),	min(tbb.height, tbb.br().y));
		if (tbb.x <= 0)
			bb.x = 0;
		if (tbb.y <= 0)
			bb.y = 0;
		if (tbb.x + tbb.width > img.cols - 1)
			bb.width = img.cols - 1 - bb.x;
		if (tbb.y + tbb.height > img.rows - 1)
			bb.height = img.rows - 1 - bb.y;

		LOGD("boo-tld-2 :tbb-[ %d , %d , %d , %d]\n", tbb.x, tbb.y, tbb.width,
				tbb.height);

		getPattern(img(bb), pattern, mean, stdev);
		vector<int> isin;
		float dummy;
		classifier.NNConf(pattern, isin, dummy, tconf); //Conservative Similarity
		tvalid = lastvalid;
		if (tconf > classifier.thr_nn_valid) {
			tvalid = true;
		}
	} else
		printf("No points tracked\n");
	//}

	end = clock();
	LOGD("tldt : track time :%d ms\n", (end - start) / 1000);

}

void TLD::bbPoints(vector<cv::Point2f>& points, const BoundingBox& bb) {
	int max_pts = 10;
	int margin_h = 0;
	int margin_v = 0;
	int stepx = ceil((bb.width - 2 * margin_h) / max_pts);
	int stepy = ceil((bb.height - 2 * margin_v) / max_pts);
	for (int y = bb.y + margin_v; y < bb.y + bb.height - margin_v; y += stepy) {
		for (int x = bb.x + margin_h; x < bb.x + bb.width - margin_h; x +=stepx) {
			points.push_back(Point2f(x, y));
		}
	}
}

void TLD::bbPredict(const vector<cv::Point2f>& points1,const vector<cv::Point2f>& points2, const BoundingBox& bb1,BoundingBox& bb2) {
	int npoints = (int) points1.size();
	vector<float> xoff(npoints);
	vector<float> yoff(npoints);
	LOGD("tld :tracked points : %d\n", npoints);
	for (int i = 0; i < npoints; i++) {
		xoff[i] = points2[i].x - points1[i].x;
		yoff[i] = points2[i].y - points1[i].y;
	}
	float dx = median(xoff);
	float dy = median(yoff);
	float s;
	if (npoints > 1) {
		vector<float> d;
		d.reserve(npoints * (npoints - 1) / 2);
		for (int i = 0; i < npoints; i++) {
			for (int j = i + 1; j < npoints; j++) {
				d.push_back(norm(points2[i] - points2[j])	/ norm(points1[i] - points1[j]));
			}
		}
		s = median(d);
	} else {
		s = 1.0;
	}
	float s1 = 0.5 * (s - 1) * bb1.width;
	float s2 = 0.5 * (s - 1) * bb1.height;
	//LOGD("s= %f s1= %f s2= %f \n",s,s1,s2);
	bb2.x = round(bb1.x + dx - s1);
	bb2.y = round(bb1.y + dy - s2);
	bb2.width = round(bb1.width * s);
	bb2.height = round(bb1.height * s);
	//LOGD("predicted bb: %d %d %d %d\n",bb2.x,bb2.y,bb2.br().x,bb2.br().y);
}

void TLD::detect(const cv::Mat& frame, const cv::Mat& frameCb,const cv::Mat& frameCr) {
	//cleaning
	clock_t start, end, start2, end2;
	start = clock();
	dbb.clear();
	dconf.clear();
	dt.bb.clear();
	double t = (double) getTickCount();
	Mat img(frame.rows, frame.cols, CV_8U);
	integral(frame, iisum, iisqsum);
	GaussianBlur(frame, img, Size(9, 9), 1.5);
	int numtrees = classifier.getNumStructs();
	float fern_th = classifier.getFernTh();
	vector<int> ferns(10);
	float conf;
	int a = 0;
	int sc_idx = 0;
	std::vector<int> sc;
	int sc_constraint = 1;		//Constraint the scale +-1 near tracking result
	Mat patch;
	if (tracked) {//Speed up the detection process during tracking by limiting the scales
		float diff = FLT_MAX;
		float dist = 0.0f;
		for (int i = 0; i < scales.size(); i++) {
			dist = sqrt(
					(scales[i].height - ftbb.height)
							* (scales[i].height - ftbb.height)
							+ (scales[i].width - ftbb.width)
									* (scales[i].width - ftbb.width));
			if (dist < diff) {
				diff = dist;
				sc_idx = i;
			}
		}
		for (int i = 0; i < scales.size(); i++) {
			if (abs(i - sc_idx) <= sc_constraint)
				sc.push_back(i);
		}
	}

	for (int i = 0; i < grid.size(); i++) {					//FIXME: BottleNeck
		bool isCandidate = false;
		BoundingBox grid_tmp = grid[i];
		if (tracked) {
			for (int j = 0; j < sc.size(); j++) {
				if (sc[j] == grid_tmp.sidx) {
					float centre_x = (float) grid_tmp.x+ (float) grid_tmp.width * 0.5f;
					float centre_y = (float) grid_tmp.y+ (float) grid_tmp.height * 0.5f;
					float centre_x_t = tbb.x + (float) tbb.width * 0.5f;
					float centre_y_t = tbb.y + (float) tbb.height * 0.5f;
					float dist = sqrt((centre_x_t - centre_x) * (centre_x_t - centre_x)+ (centre_y_t - centre_y)	* (centre_y_t - centre_y));
					if (dist < det_window) {
						if (getVar(grid_tmp, iisum, iisqsum) >= var) {
							a++;
							patch = img(grid_tmp);
							classifier.getFeatures(patch, grid_tmp.sidx, ferns);
							conf = classifier.measure_forest(ferns);
							tmp.conf[i] = conf;
							tmp.patt[i] = ferns;
							isCandidate = true;
							if (conf > numtrees * fern_th) {
								dt.bb.push_back(i);
							}
						}
					}
				}
			}
		} else {
			float centre_x = (float) grid_tmp.x + (float) grid_tmp.width * 0.5f;
			float centre_y = (float) grid_tmp.y+ (float) grid_tmp.height * 0.5f;
			float centre_x_t = tbb.x + (float) tbb.width * 0.5f;
			float centre_y_t = tbb.y + (float) tbb.height * 0.5f;
			int interval = (int) (MAX(frame.cols, frame.rows) / 3);
			int x_min = 0, y_min = 0, x_max = 0, y_max = 0;
			if (framecnt % 3 == 0) {	//Region 1: left
				x_max = interval;
				y_max = frame.rows;
			} else if (framecnt % 3 == 1) {	//Region 2: middle
				x_min = interval;
				x_max = 2 * interval;
				y_max = frame.rows;
			} else {							//Region 3: right
				x_min = 2 * interval;
				x_max = frame.cols;
				y_max = frame.rows;
			}
			if (centre_x > x_min && centre_x < x_max && centre_y > y_min&& centre_y < y_max) {
				if (getVar(grid_tmp, iisum, iisqsum) >= var) {
					a++;
					patch = img(grid_tmp);
					classifier.getFeatures(patch, grid_tmp.sidx, ferns);
					conf = classifier.measure_forest(ferns);
					tmp.conf[i] = conf;
					tmp.patt[i] = ferns;
					if (conf > numtrees * fern_th) {
						LOGD("boo-tld-det-2 : grid.indx= %d\n", i);
						dt.bb.push_back(i);
						isCandidate = true;
					}
				}
			}
		}
		if (!isCandidate)
			tmp.conf[i] = 0.0f;

	}
	int detections = dt.bb.size();
	//LOGD("%d Bounding boxes passed the variance filter\n",a);
	//LOGD("%d Initial detection from Fern Classifier\n",detections);
#ifdef NO_ALLSORT

	if (detections > 30) {
		for (int i = 0; i < 30; i++) {
			float max_conf = 0.0;
			int max_idx = 0;
			for (int j = i; j < dt.bb.size(); j++) {
				if (tmp.conf[dt.bb[j]] > max_conf) {
					max_conf = tmp.conf[dt.bb[j]];
					max_idx = j;
				}
			}
			int tmp = dt.bb[i];
			dt.bb[i] = dt.bb[max_idx];
			dt.bb[max_idx] = tmp;

		}

		dt.bb.resize(30);
		detections = 30;
	}
#else
	if (detections>30) {
		nth_element(dt.bb.begin(), dt.bb.begin() + 30, dt.bb.end(), CComparator(tmp.conf));
		dt.bb.resize(30);
		detections = 30;
	}
#endif
	//for (int i=0;i<detections;i++){
	//      drawBox(img,grid[dt.bb[i]]);
	//  }
	//imshow("detections",img);
	if (detections == 0) {
		detected = false;
		return;
	}
	//LOGD("Fern detector made %d detections ",detections);
	//t=(double)getTickCount()-t;
	//LOGD("in %gms\n", t*1000/getTickFrequency());
	end = clock();
	LOGD("tldt : detect time 1 :%d ms\n", (end - start) / 1000);
	//  Initialize detection structure
	dt.patt = vector < vector<int> > (detections, vector<int>(10, 0)); //  Corresponding codes of the Ensemble Classifier
	dt.conf1 = vector<float>(detections); //  Relative Similarity (for final nearest neighbour classifier)
	dt.conf2 = vector<float>(detections); //  Conservative Similarity (for integration with tracker)
	dt.isin = vector < vector<int> > (detections, vector<int>(3, -1)); //  Detected (isin=1) or rejected (isin=0) by nearest neighbour classifier
	dt.patch = vector < Mat > (detections, Mat(patch_size, patch_size, CV_32F)); //  Corresponding patches
	int idx;
	Scalar mean, stdev;
	float nn_th = classifier.getNNTh();
	for (int i = 0; i < detections; i++) {     //  for every remaining detection
		idx = dt.bb[i];                  //  Get the detected bounding box index
		patch = frame(grid[idx]);
		getPattern(patch, dt.patch[i], mean, stdev); //  Get pattern within bounding box

		classifier.NNConf(dt.patch[i], dt.isin[i], dt.conf1[i], dt.conf2[i]); //  Evaluate nearest neighbour classifier
		dt.patt[i] = tmp.patt[idx];
		//LOGD("nn_th = %f\n",nn_th);
		//LOGD("Testing feature %d, conf:%f isin:(%d|%d|%d)\n",i,dt.conf1[i],dt.isin[i][0],dt.isin[i][1],dt.isin[i][2]);
		if (dt.conf1[i] > nn_th) { //  idx = dt.conf1 > tld.model.thr_nn; % get all indexes that made it through the nearest neighbour

			cv::Mat img_rgb = cv::Mat(best_box.height, best_box.width, CV_8UC3);

			cv::Mat img_y = frame(grid[idx]);
			cv::Mat img_cb = frameCb(grid[idx]);
			cv::Mat img_cr = frameCr(grid[idx]);

			std::vector<cv::Mat> mv;
			mv.push_back(img_y);
			mv.push_back(img_cr);
			mv.push_back(img_cb);
			cv::merge(mv, img_rgb);

			cv::cvtColor(img_rgb, img_rgb, CV_YCrCb2RGB);
			cv::resize(img_rgb, img_rgb, cv::Size(patch_size, patch_size));
			img_rgb.convertTo(img_rgb, CV_32FC3);

			float _conf1 = 0.0;
			float _conf2 = 0.0;
			vector<int> _isin = vector<int>(3, -1);
			classifier.NNConfBGR(dt.patch[i], img_rgb, _isin, _conf1, _conf2);

			if (_conf1 > 0.5 /*0.54*/) {
				dbb.push_back(grid[idx]);
				dconf.push_back(dt.conf2[i]);
				LOGD("bob-hello : color detecting 10 , yconf=%f , cconf=%f ",
						dt.conf1[i], _conf1);
			}

//			dbb.push_back(grid[idx]);                                         //  BB    = dt.bb(:,idx); % bounding boxes
//			dconf.push_back(dt.conf2[i]);                                     //  Conf  = dt.conf2(:,idx); % conservative confidences
		}
	}                                                                    //  end
	if (dbb.size() > 0) {
		//LOGD("Found %d NN matches\n",(int)dbb.size());
		detected = true;
	} else {
		//LOGD("No NN matches found.\n");
		detected = false;
	}
	end2 = clock();
	LOGD("tldt fern : detect time 3 :%d ms\n", (end2 - end) / 1000);
}

void TLD::evaluate() {
}

void TLD::learn(const Mat& img, const Mat& imgCb, const Mat& imgCr) {
	clock_t start, end;
	start = clock();

	///Check consistency
	BoundingBox bb;
	bb.x = max(lastbox.x, 0);
	bb.y = max(lastbox.y, 0);
	bb.width = min(min(img.cols - bb.x, lastbox.width),	min(lastbox.width, lastbox.br().x));
	bb.height = min(min(img.rows - bb.y, lastbox.height),min(lastbox.height, lastbox.br().y));
	//Original code, crash if lastbox.x or lastbox.y are negative
	//bb.width = min(min(img.cols-lastbox.x,lastbox.width),min(lastbox.width,lastbox.br().x));
	//bb.height = min(min(img.rows-lastbox.y,lastbox.height),min(lastbox.height,lastbox.br().y));
	Scalar mean, stdev;
	Mat pattern;
	getPattern(img(bb), pattern, mean, stdev);
	vector<int> isin;
	float dummy, conf;
	classifier.NNConf(pattern, isin, conf, dummy);
	if (conf < 0.5) {
		//LOGD("Fast change..not training\n");
		lastvalid = false;
		return;
	}
	if (pow(stdev.val[0], 2) < var) {
		//LOGD("Low variance..not training\n");
		lastvalid = false;
		return;
	}
	if (isin[2] == 1) {
		//LOGD("Patch in negative data..not traing");
		lastvalid = false;
		return;
	}
/// Data generation
	for (int i = 0; i < grid.size(); i++) {
		grid[i].overlap = bbOverlap(lastbox, grid[i]);
	}
	vector < pair<vector<int>, int> > fern_examples;
	good_boxes.clear();
	bad_boxes.clear();
	getOverlappingBoxes(lastbox, num_closest_update);
	if (good_boxes.size() > 0)
		generatePositiveData(img, imgCb, imgCr, num_warps_update);
	else {
		lastvalid = false;
		//LOGD("No good boxes..Not training");
		return;
	}
	fern_examples.reserve(pX.size() + bad_boxes.size());
	fern_examples.assign(pX.begin(), pX.end());
	int idx;

	//int num = MIN(bad_boxes.size(),200);

	for (int i = 0; i < bad_boxes.size(); i++) {
		idx = bad_boxes[i];
		if (tmp.conf[idx] >= 1) {
			fern_examples.push_back(make_pair(tmp.patt[idx], 0));
		}
	}
	vector<Mat> nn_examples;
	nn_examples.reserve(dt.bb.size() + 1);
	nn_examples.push_back(pEx);
	for (int i = 0; i < dt.bb.size(); i++) {
		idx = dt.bb[i];
		if (bbOverlap(lastbox, grid[idx]) < bad_overlap)
			nn_examples.push_back(dt.patch[i]);
	}
	/// Classifiers update
	classifier.trainF(fern_examples, 2);
	classifier.trainNN(nn_examples, pExB, pExG, pExR);
	//classifier.show();
	end = clock();
	LOGD("tldt : Learning time :%d ms\n", (end - start) / 1000);
}

void TLD::buildGrid(const cv::Mat& img, const cv::Rect& box) {
	const float SHIFT = 0.1;
	const float SCALES[] = { 0.16151, 0.19381, 0.23257, 0.27908, 0.33490,
			0.40188, 0.48225, 0.57870, 0.69444, 0.83333, 1, 1.20000, 1.44000,
			1.72800, 2.07360, 2.48832, 2.98598, 3.58318, 4.29982, 5.15978,
			6.19174 };
	int width, height, min_bb_side;
	//Rect bbox;
	BoundingBox bbox;
	Size scale;
	int sc = 0;
	for (int s = 0; s < 21; s++) {
		width = round(box.width * SCALES[s]);
		height = round(box.height * SCALES[s]);
		min_bb_side = min(height, width);
		if (min_bb_side < min_win || width > img.cols || height > img.rows)
			continue;
		scale.width = width;
		scale.height = height;
		scales.push_back(scale);
		for (int y = 1; y < img.rows - height;	y += round(SHIFT * min_bb_side)) {
			for (int x = 1; x < img.cols - width;	x += round(SHIFT * min_bb_side)) {
				bbox.x = x;
				bbox.y = y;
				bbox.width = width;
				bbox.height = height;
				bbox.overlap = bbOverlap(bbox, BoundingBox(box));
				bbox.sidx = sc;
				grid.push_back(bbox);
			}
		}
		sc++;
	}
}

float TLD::bbOverlap(const BoundingBox& box1, const BoundingBox& box2) {
	if (box1.x > box2.x + box2.width) {
		return 0.0;
	}
	if (box1.y > box2.y + box2.height) {
		return 0.0;
	}
	if (box1.x + box1.width < box2.x) {
		return 0.0;
	}
	if (box1.y + box1.height < box2.y) {
		return 0.0;
	}

	float colInt = min(box1.x + box1.width, box2.x + box2.width)
			- max(box1.x, box2.x);
	float rowInt = min(box1.y + box1.height, box2.y + box2.height)
			- max(box1.y, box2.y);

	float intersection = colInt * rowInt;
	float area1 = box1.width * box1.height;
	float area2 = box2.width * box2.height;
	return intersection / (area1 + area2 - intersection);
}

void TLD::getOverlappingBoxes(const cv::Rect& box1, int num_closest) {
	float max_overlap = 0.0f;
	for (int i = 0; i < grid.size(); i++) {
		if (grid[i].overlap > max_overlap) {
			max_overlap = grid[i].overlap;
			best_box = grid[i];
		}
		if (grid[i].overlap > 0.6) {
			good_boxes.push_back(i);
		} else if (grid[i].overlap < bad_overlap) {
			bad_boxes.push_back(i);
		}
	}
	//Get the best num_closest (10) boxes and puts them in good_boxes
	int good_boxes_tmp[10];
	int erase_idx = 0;
	if (good_boxes.size() > num_closest) {
#define NO_SORT
#ifdef NO_SORT
		for (int i = 0; i < num_closest; i++) {
			max_overlap = 0.0f;
			for (int j = 0; j < good_boxes.size(); j++) {
				int idx = good_boxes[j];
				if (grid[idx].overlap > max_overlap) {
					erase_idx = j;
					max_overlap = grid[idx].overlap;
					good_boxes_tmp[i] = idx;
				}
			}
			good_boxes.erase(good_boxes.begin() + erase_idx);
		}
		good_boxes.clear();
		for (int i = 0; i < num_closest; i++)
			good_boxes.push_back(good_boxes_tmp[i]);
#else
		std::nth_element(good_boxes.begin(),good_boxes.begin()+num_closest,good_boxes.end(),OComparator(grid));
		good_boxes.resize(num_closest);
#endif
	}
	getBBHull();
}

void TLD::getBBHull() {
	int x1 = INT_MAX, x2 = 0;
	int y1 = INT_MAX, y2 = 0;
	int idx;
	for (int i = 0; i < good_boxes.size(); i++) {
		idx = good_boxes[i];
		x1 = min(grid[idx].x, x1);
		y1 = min(grid[idx].y, y1);
		x2 = max(grid[idx].x + grid[idx].width, x2);
		y2 = max(grid[idx].y + grid[idx].height, y2);
	}
	bbhull.x = x1;
	bbhull.y = y1;
	bbhull.width = x2 - x1;
	bbhull.height = y2 - y1;
}

bool bbcomp(const BoundingBox& b1, const BoundingBox& b2) {
	TLD t;
	if (t.bbOverlap(b1, b2) < 0.5)
		return false;
	else
		return true;
}
int TLD::clusterBB(const vector<BoundingBox>& dbb, vector<int>& indexes) {
	//FIXME: Conditional jump or move depends on uninitialised value(s)
	const int c = dbb.size();
	//1. Build proximity matrix
	Mat D(c, c, CV_32F);
	float d;
	for (int i = 0; i < c; i++) {
		for (int j = i + 1; j < c; j++) {
			d = 1 - bbOverlap(dbb[i], dbb[j]);
			D.at<float>(i, j) = d;
			D.at<float>(j, i) = d;
		}
	}
	//2. Initialize disjoint clustering
	// float L[c-1]; //Level
	float *L = new float[c - 1];
	// int nodes[c-1][2];
	int **nodes = new int *[c - 1];
	for (int i = 0; i < 2; i++)
		nodes[i] = new int[c - 1];

	// int belongs[c];
	int *belongs = new int[c];
	int m = c;
	for (int i = 0; i < c; i++) {
		belongs[i] = i;
	}
	for (int it = 0; it < c - 1; it++) {
		//3. Find nearest neighbor
		float min_d = 1;
		int node_a, node_b;
		for (int i = 0; i < D.rows; i++) {
			for (int j = i + 1; j < D.cols; j++) {
				if (D.at<float>(i, j) < min_d && belongs[i] != belongs[j]) {
					min_d = D.at<float>(i, j);
					node_a = i;
					node_b = j;
				}
			}
		}
		if (min_d > 0.5) {
			int max_idx = 0;
			bool visited;
			for (int j = 0; j < c; j++) {
				visited = false;
				for (int i = 0; i < 2 * c - 1; i++) {
					if (belongs[j] == i) {
						indexes[j] = max_idx;
						visited = true;
					}
				}
				if (visited)
					max_idx++;
			}
			return max_idx;
		}

		//4. Merge clusters and assign level
		L[m] = min_d;
		nodes[it][0] = belongs[node_a];
		nodes[it][1] = belongs[node_b];
		for (int k = 0; k < c; k++) {
			if (belongs[k] == belongs[node_a] || belongs[k] == belongs[node_b])
				belongs[k] = m;
		}
		m++;
	}

	delete[] L;
	L = NULL;
	for (int i = 0; i < 2; ++i) {
		delete[] nodes[i];
		nodes[i] = NULL;
	}
	delete[] nodes;
	nodes = NULL;
	delete[] belongs;
	belongs = NULL;

	return 1;

}

void TLD::clusterConf(const vector<BoundingBox>& dbb,
		const vector<float>& dconf, vector<BoundingBox>& cbb,
		vector<float>& cconf) {
	int numbb = dbb.size();
	vector<int> T;
	float space_thr = 0.5;
	int c = 1;
	switch (numbb) {
	case 1:
		cbb = vector < BoundingBox > (1, dbb[0]);
		cconf = vector<float>(1, dconf[0]);
		return;
		break;
	case 2:
		T = vector<int>(2, 0);
		if (1 - bbOverlap(dbb[0], dbb[1]) > space_thr) {
			T[1] = 1;
			c = 2;
		}
		break;
	default:
		T = vector<int>(numbb, 0);
		c = partition(dbb, T, (*bbcomp));
		//c = clusterBB(dbb,T);
		break;
	}
	cconf = vector<float>(c);
	cbb = vector < BoundingBox > (c);
	//LOGD("Cluster indexes: ");
	BoundingBox bx;
	for (int i = 0; i < c; i++) {
		float cnf = 0;
		int N = 0, mx = 0, my = 0, mw = 0, mh = 0;
		for (int j = 0; j < T.size(); j++) {
			if (T[j] == i) {
				//LOGD("%d ", i);
				cnf = cnf + dconf[j];
				mx = mx + dbb[j].x;
				my = my + dbb[j].y;
				mw = mw + dbb[j].width;
				mh = mh + dbb[j].height;
				N++;
			}
		}
		if (N > 0) {
			cconf[i] = cnf / N;
			bx.x = cvRound(mx / N);
			bx.y = cvRound(my / N);
			bx.width = cvRound(mw / N);
			bx.height = cvRound(mh / N);
			cbb[i] = bx;
		}
	}
	//LOGD("\n");
}

int TLD::get_pEx_size(){
	return classifier.pEx.size();
}

}
