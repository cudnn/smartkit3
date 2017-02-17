/*
 * FerNNClassifier.cpp
 *
 *  Created on: Jun 14, 2011
 *      Author: alantrrs
 */

#include "FerNNClassifier.h"
#include "time.h"
//#include <cpu-features.h>
//#include <arm_neon.h>
using namespace cv;
using namespace std;
//using namespace cv::ocl;




inline float IVC_calculate_CV_TM_CCORR_NORMED1(const cv::Mat &prev, const cv::Mat &pred)
{
	//int sum0 = 0;
	//int sum1 = 0;
	//int sum2 = 0;
	//LOGI("fern xc : ncc start");
	float sum0 = 0.0f,sum1 = 0.0f,sum2 = 0.0f;
	/*
	#ifdef NEON
	float32x4_t pred_t,prev_t,vv_result,dd_result,dv_result,vv_temp,dv_temp,dd_temp;

	vv_result = vdupq_n_f32((float)0.0f);
	dd_result = vdupq_n_f32((float)0.0f);
	dv_result = vdupq_n_f32((float)0.0f);
	for (int r = 0; r < prev.rows; r++) {
	    const float* pd = pred.ptr<float>(r);
	    const float* pv = prev.ptr<float>(r);
		for (int c = 0; c < prev.cols; c+=4) {

            pred_t = vld1q_f32((float32_t*)(pd+c));
            prev_t = vld1q_f32((float32_t*)(pv+c));
            vv_temp = vmulq_f32(prev_t,prev_t);    // v[0,1,2,3]*v[0,1,2,3]
            dd_temp = vmulq_f32(pred_t,pred_t);
            dv_temp = vmulq_f32(pred_t,prev_t);
            vv_result = vaddq_f32(vv_result,vv_temp);
            dd_result = vaddq_f32(dd_result,dd_temp);
            dv_result = vaddq_f32(dv_result,dv_temp);
		}

	}
	sum0 += vgetq_lane_f32(vv_result,0);
    sum0 += vgetq_lane_f32(vv_result,1);
    sum0 += vgetq_lane_f32(vv_result,2);
    sum0 += vgetq_lane_f32(vv_result,3);
    sum1 += vgetq_lane_f32(dd_result,0);
    sum1 += vgetq_lane_f32(dd_result,1);
    sum1 += vgetq_lane_f32(dd_result,2);
    sum1 += vgetq_lane_f32(dd_result,3);
    sum2 += vgetq_lane_f32(dv_result,0);
    sum2 += vgetq_lane_f32(dv_result,1);
    sum2 += vgetq_lane_f32(dv_result,2);
    sum2 += vgetq_lane_f32(dv_result,3);
	#else
	*/
	for (int r = 0; r < prev.rows; r++) {
		for (int c = 0; c < prev.cols; c++) {
			short pred_t = (short)pred.at<float>(r, c);
			short prev_t = (short)prev.at<float>(r, c);
			sum0 += (prev_t )*(prev_t );
			sum1 += (pred_t )*(pred_t );
			sum2 += (prev_t )*(pred_t );
		}
	}
	/*
	#endif
	*/
	//LOGD("fern xc :s0,s1,s2 :%f,%f,%f\n",sum0,sum1,sum2);
	if (sum0<= 0 && sum1<= 0)
		return 0.1f;
	else if (sum0 <= 0 || sum1 <= 0)
		return 0.0f;
	else
		return sum2 / (float)(sqrt((double)sum0)*sqrt((double)sum1));
}

inline float calculate_CV_TM_CCORR_NORMED0(const cv::Mat &prev, const cv::Mat &pred)
{
	float sum0 = 0;
	float sum1 = 0;
	float sum2 = 0;
	float prev_mean = 0.0f;
	float pred_mean = 0.0f;
	for (int r = 0; r < prev.rows; r++) {
		for (int c = 0; c < prev.cols; c++) {
			sum0 += (float)prev.at<float>(r, c);
			sum1 += (float)prev.at<float>(r, c);
		}
	}
	prev_mean = sum0 / (prev.cols * prev.rows);
	pred_mean = sum1 / (pred.cols * pred.rows);

	for (int r = 0; r < prev.rows; r++) {
		for (int c = 0; c < prev.cols; c++) {
			float pred_t = (float)pred.at<float>(r, c);
			float prev_t = (float)prev.at<float>(r, c);
			sum0 += (prev_t - prev_mean)*(prev_t-prev_mean);
			sum1 += (pred_t - pred_mean)*(pred_t - pred_mean);
			sum2 += (prev_t - prev_mean)*(pred_t - pred_mean);
		}
	}
	if (sum0< 0.1 && sum1< 0.1) 
		return 0.1f;
	else if (sum0 < 0.1 || sum1 < 0.1)
		return 0.0f;
	else
		return sum2 / (float)(sqrt((double)sum0)*sqrt((double)sum1));
}
void FerNNClassifier::init(){
	///Classifier Parameters initialization
	valid = 0.5f;
	ncc_thesame = 0.95f;
	nstructs = 10;
	structSize = 13;
	thr_fern = 0.5f;
	thr_nn = 0.65f;
	thr_nn_valid = 0.7f;
/*
#ifdef OPENCVOCL
    LOGI("fern xc : ocl start");
    //PlatformsInfo platforms;
    PlatformsInfo platforms;
    getOpenCLPlatforms(platforms);

    for(size_t i = 0; i < platforms.size(); i ++)
    {
        LOGI("fern xc : platform info = %s\n",platforms[i]->platformVendor.c_str());
    }
    DevicesInfo device_info;
    getOpenCLDevices(device_info,CVCL_DEVICE_TYPE_GPU);
    LOGI("fern xc : device size = %d\n",device_info.size());
    LOGI("fern xc : device type = %d,max work group size = %d\n",device_info[0]->deviceType,device_info[0]->maxWorkGroupSize);
    ocl::setDevice(device_info[0]);
#endif
*/

}

void FerNNClassifier::read(const FileNode& file){
  ///Classifier Parameters
  valid = (float)file["valid"];
  ncc_thesame = (float)file["ncc_thesame"];
  nstructs = (int)file["num_trees"];
  structSize = (int)file["num_features"];
  thr_fern = (float)file["thr_fern"];
  thr_nn = (float)file["thr_nn"];
  thr_nn_valid = (float)file["thr_nn_valid"];
}

void FerNNClassifier::setParameters(float valid_, 
								    float ncc_thesame_,
								    int nstructs_,
								    int structSize_,
								    float thr_fern_,
								    float thr_nn_,
								    float thr_nn_valid_){
	valid = valid_;
	ncc_thesame = ncc_thesame_;
	nstructs = nstructs_;
	structSize = structSize_;
	thr_fern = thr_fern_;
	thr_nn = thr_nn_;
	thr_nn_valid = thr_nn_valid_;
}

void FerNNClassifier::init_clear(){
	features.clear();
	nCounter.clear();
	pCounter.clear();
	posteriors.clear();
	pEx.clear();
	pExB.clear();
	pExG.clear();
	pExR.clear();
	nEx.clear();
}

void FerNNClassifier::prepare(const vector<Size>& scales){
	init_clear();
	acum = 0;
	//Initialize test locations for features
	int totalFeatures = nstructs*structSize;
	features = vector<vector<Feature> >(scales.size(), vector<Feature>(totalFeatures));
	RNG& rng = theRNG();
	float x1f, x2f, y1f, y2f;
	int x1, x2, y1, y2;
	for (int i = 0; i < totalFeatures; i++){
		x1f = (float)rng;
		y1f = (float)rng;
		x2f = (float)rng;
		y2f = (float)rng;
		for (int s = 0; s < scales.size(); s++){
			x1 = x1f * scales[s].width;
			y1 = y1f * scales[s].height;
			x2 = x2f * scales[s].width;
			y2 = y2f * scales[s].height;
			features[s][i] = Feature(x1, y1, x2, y2);
		}

  }
  //Thresholds
  thrN = 0.5*nstructs;

  //Initialize Posteriors
  for (int i = 0; i<nstructs; i++) {
      posteriors.push_back(vector<float>(pow(2.0,structSize), 0));
      pCounter.push_back(vector<int>(pow(2.0,structSize), 0));
      nCounter.push_back(vector<int>(pow(2.0,structSize), 0));
  }
}

void FerNNClassifier::getFeatures(const cv::Mat& image,const int& scale_idx, vector<int>& fern){
  int leaf;
  int stride = 0;
  for (int t=0;t<nstructs;t++){
      leaf=0;

      for (int f=0; f<structSize; f++){
          leaf = (leaf << 1) + features[scale_idx][stride+f](image);
      }
	  stride += nstructs;
      fern[t]=leaf;
  }
}

float FerNNClassifier::measure_forest(vector<int> fern) {
  float votes = 0;
  for (int i = 0; i < nstructs; i++) {
      votes += posteriors[i][fern[i]];
  }
  return votes;
}

void FerNNClassifier::update(const vector<int>& fern, int C, int N) {
  int idx;
  for (int i = 0; i < nstructs; i++) {
      idx = fern[i];
      (C==1) ? pCounter[i][idx] += N : nCounter[i][idx] += N;
      if (pCounter[i][idx]==0) {
          posteriors[i][idx] = 0;
      } else {
          posteriors[i][idx] = ((float)(pCounter[i][idx]))/(pCounter[i][idx] + nCounter[i][idx]);
      }
  }
}

void FerNNClassifier::trainF(const vector<std::pair<vector<int>,int> >& ferns,int resample){
  // Conf = function(2,X,Y,Margin,Bootstrap,Idx)
  //                 0 1 2 3      4         5
  //  double *X     = mxGetPr(prhs[1]); -> ferns[i].first
  //  int numX      = mxGetN(prhs[1]);  -> ferns.size()
  //  double *Y     = mxGetPr(prhs[2]); ->ferns[i].second
  //  double thrP   = *mxGetPr(prhs[3]) * nTREES; ->threshold*nstructs
  //  int bootstrap = (int) *mxGetPr(prhs[4]); ->resample
  thrP = thr_fern*nstructs;                                                          // int step = numX / 10;
  //for (int j = 0; j < resample; j++) {                      // for (int j = 0; j < bootstrap; j++) {
      for (int i = 0; i < ferns.size(); i++){               //   for (int i = 0; i < step; i++) {
                                                            //     for (int k = 0; k < 10; k++) {
                                                            //       int I = k*step + i;//box index
                                                            //       double *x = X+nTREES*I; //tree index
          if(ferns[i].second==1){                           //       if (Y[I] == 1) {
              if(measure_forest(ferns[i].first)<=thrP)      //         if (measure_forest(x) <= thrP)
                update(ferns[i].first,1,1);                 //             update(x,1,1);
          }else{                                            //        }else{
              if (measure_forest(ferns[i].first) >= thrN)   //         if (measure_forest(x) >= thrN)
                update(ferns[i].first,0,1);                 //             update(x,0,1);
          }
      }
  //}
}

void FerNNClassifier::trainNN(const vector<cv::Mat>& nn_examples, const cv::Mat& pExb, const cv::Mat& pExg, const cv::Mat& pExr){
  float conf,dummy , _conf;
  vector<int> y(nn_examples.size(),0);
  y[0]=1;
  vector<int> isin , _isin;
  cv::Mat img_clone;

  for (int i=0;i<nn_examples.size();i++){                          //  For each example
      NNConf(nn_examples[i],isin,conf,dummy);                      //  Measure Relative similarity
      if (y[i]==1 && conf<=thr_nn){                                //    if y(i) == 1 && conf1 <= tld.model.thr_nn % 0.65
          if (isin[1]<0){                                          //      if isnan(isin(2))
              pEx = vector<Mat>(1,nn_examples[i]);                 //        tld.pex = x(:,i);
			  img_clone = pExb.clone();
			  pExB = vector<Mat>(1, img_clone);
			  img_clone = pExg.clone();
			  pExG = vector<Mat>(1, img_clone);
			  img_clone = pExr.clone();
			  pExR = vector<Mat>(1, img_clone);
              continue;                                            //        continue;
          }                                                        //      end
          //pEx.insert(pEx.begin()+isin[1],nn_examples[i]);        //      tld.pex = [tld.pex(:,1:isin(2)) x(:,i) tld.pex(:,isin(2)+1:end)]; % add to model
		  //*

//		  std::vector<cv::Mat> mv;

//		  mv.push_back(pExb);
//		  mv.push_back(pExg);
//		  mv.push_back(pExr);

//		  cv::Mat img_rgb;
//		  cv::merge(mv, img_rgb);
//		  img_rgb.convertTo(img_rgb, CV_32FC3);

//		  NNConfBGR(nn_examples[i], img_rgb, _isin, _conf, dummy);  //  Evaluate nearest neighbour classifier

//		  if (conf <= thr_nn)
//		  {
//			  pEx.push_back(nn_examples[i]);
//			  img_clone = pExb.clone();
//			  pExB.push_back(img_clone);
//			  img_clone = pExg.clone();
//			  pExG.push_back(img_clone);
//			  img_clone = pExr.clone();
//			  pExR.push_back(img_clone);
//		  }

		  //*

		  pEx.push_back(nn_examples[i]);
		  img_clone = pExb.clone();
		  pExB.push_back(img_clone);
		  img_clone = pExg.clone();
		  pExG.push_back(img_clone);
		  img_clone = pExr.clone();
		  pExR.push_back(img_clone);
		   LOGD("bob-hello : color learing , size =[ %d , %d , %d , %d]",pEx.size(),pExB.size(),pExG.size(),pExR.size());
      }                                                            //    end
      if(y[i]==0 && conf>0.3)                                      //  if y(i) == 0 && conf1 > 0.5
        nEx.push_back(nn_examples[i]);                             //    tld.nex = [tld.nex x(:,i)];

  }                                                                 //  end
  acum++;
//  printf("%d. Trained NN examples: %d positive %d negative\n",acum,(int)pEx.size(),(int)nEx.size());
}                                                                  //  end


void FerNNClassifier::NNConf(const Mat& example, vector<int>& isin,float& rsconf,float& csconf){
  /*Inputs:
   * -NN Patch
   * Outputs:
   * -Relative Similarity (rsconf), Conservative Similarity (csconf), In pos. set|Id pos set|In neg. set (isin)
   */
   clock_t start,end;
  isin=vector<int>(3,-1);
  if (pEx.empty()){ //if isempty(tld.pex) % IF positive examples in the model are not defined THEN everything is negative
      rsconf = 0; //    conf1 = zeros(1,size(x,2));
      csconf=0;
      return;
  }
  if (nEx.empty()){ //if isempty(tld.nex) % IF negative examples in the model are not defined THEN everything is positive
      rsconf = 1;   //    conf1 = ones(1,size(x,2));
      csconf=1;
      return;
  }
  Mat ncc(1,1,CV_32F);
  float fncc = 0.0f;
  float nccP = 0.0f,sum0 = 0.0f,sum1 = 0.0f,sum2 = 0.0f, csmaxP = 0.0f, maxP = 0.0f;
  bool anyP=false;
  int maxPidx,validatedPart = ceil(pEx.size()*valid);
  float nccN = 0.0f, maxN=0;
  bool anyN=false;

  start = clock();
  /*
#ifdef OPENCVOCL
    ocl::oclMat ocl_example(example);
    ocl::oclMat ocl_ncc(ncc);
    ocl::matchTemplate(ocl_example,ocl_example,ocl_ncc,CV_TM_CCORR);
    ocl_ncc.download(ncc);
    sum2=((float*)ncc.data)[0];
#endif
*/
  for (int i=0;i<pEx.size();i++){

/*
#ifdef OPENCVOCL

      ocl::oclMat ocl_pEx(pEx[i]);
      ocl::matchTemplate(ocl_pEx,ocl_example,ocl_ncc,CV_TM_CCORR);
      ocl_ncc.download(ncc);
      sum0=((float*)ncc.data)[0];

      ocl::matchTemplate(ocl_pEx,ocl_pEx,ocl_ncc,CV_TM_CCORR);
      ocl_ncc.download(ncc);
      sum1=((float*)ncc.data)[0];

      nccP = sum0/(float)(sqrt((double)sum1)*sqrt((double)sum2));
      nccP = (nccP + 1)*0.5;

       LOGD("boo-tld-clrunning.\n");
#else

	  //cv::Mat tmp = pEx[i].clone();
	  //cv::Mat tmp2 = example.clone();
	  //int test = tmp2.at<float>(1, 1);

	  //int test = tmp.at<uchar>(1, 1);
#ifndef MATCHTEMP
*/
	  fncc = IVC_calculate_CV_TM_CCORR_NORMED1(pEx[i], example);
	  nccP = (fncc + 1)*0.5;
      LOGD("boo-tld-tianxiangrunning.\n");
/*
#else
	  matchTemplate(pEx[i],example,ncc,CV_TM_CCORR_NORMED);      // measure NCC to positive examples
      nccP=(((float*)ncc.data)[0]+1)*0.5;
      LOGD("boo-tld-opencvrunning.\n");
#endif
      //matchTemplate(pEx[i],example,ncc,CV_TM_CCORR_NORMED);      // measure NCC to positive examples
#endif
*/

      if (nccP>ncc_thesame)
        anyP=true;
      if(nccP > maxP){
          maxP=nccP;
          maxPidx = i;
          if(i<validatedPart)
            csmaxP=maxP;
      }
  }
  //end = clock();
  //LOGD("fern xc : match1 time = %f\n",(end-start));
  for (int i=0;i<nEx.size();i++){
/*
#ifdef OPENCVOCL

      ocl::oclMat ocl_nEx(nEx[i]);
      ocl::matchTemplate(ocl_nEx,ocl_example,ocl_ncc,CV_TM_CCORR);
      ocl_ncc.download(ncc);
      sum0=((float*)ncc.data)[0];

      ocl::matchTemplate(ocl_nEx,ocl_nEx,ocl_ncc,CV_TM_CCORR);
      ocl_ncc.download(ncc);
      sum1=((float*)ncc.data)[0];

      nccN = sum0/(float)(sqrt((double)sum1)*sqrt((double)sum2));
      nccN = (nccN + 1)*0.5;
#else
*/
	  fncc = IVC_calculate_CV_TM_CCORR_NORMED1(nEx[i], example);
	  //LOGD("fern xc : nccN = %f\n",fncc);
	  nccN = (fncc + 1)*0.5;
/*
#endif
*/
	  //matchTemplate(nEx[i],example,ncc,CV_TM_CCORR_NORMED);     //measure NCC to negative examples
	 // nccN=(((float*)ncc.data)[0]+1)*0.5;
      if (nccN>ncc_thesame)
        anyN=true;
      if(nccN > maxN)
        maxN=nccN;
  }
  end = clock();
  LOGD("fern xc : match2 time = %f\n",(end-start));
  //set isin
  if (anyP) isin[0]=1;  //if he query patch is highly correlated with any positive patch in the model then it is considered to be one of them
  isin[1]=maxPidx;      //get the index of the maximall correlated positive patch
  if (anyN) isin[2]=1;  //if  the query patch is highly correlated with any negative patch in the model then it is considered to be one of them
  //Measure Relative Similarity
  float dN=1-maxN;
  float dP=1-maxP;
  rsconf = (float)dN/(dN+dP);
  //Measure Conservative Similarity
  dP = 1 - csmaxP;
  csconf =(float)dN / (dN + dP);
}

void FerNNClassifier::NNConfBGR(const Mat& example, const Mat& example_bgr,  vector<int>& isin, float& rsconf, float& csconf){

	isin = vector<int>(3, -1);
	if (pEx.empty()){ //if isempty(tld.pex) % IF positive examples in the model are not defined THEN everything is negative
		rsconf = 0; //    conf1 = zeros(1,size(x,2));
		csconf = 0;
		return;
	}
	if (nEx.empty()){ //if isempty(tld.nex) % IF negative examples in the model are not defined THEN everything is positive
		rsconf = 1;   //    conf1 = ones(1,size(x,2));
		csconf = 1;
		return;
	}
	Mat ncc(1, 1, CV_32F);
	float fncc = 0.0f;
	float fncc_bgr = 0.0f;
	float nccP = .0f, csmaxP = 0.0f, maxP = 0.0f;
	bool anyP = false;
	int maxPidx, validatedPart = ceil(pEx.size()*valid);
	float nccN, maxN = 0;
	bool anyN = false;	

	std::vector<cv::Mat> img_bgr;
/*	imshow("example_bgr", example_bgr)*/;
	
	cv::split(example_bgr, img_bgr);
	//imshow("img_bgr[0]", img_bgr[0]);
	//imshow("img_bgr[1]", img_bgr[1]);
	//imshow("img_bgr[2]", img_bgr[2]);

	for (int i = 0; i<pEx.size(); i++){
		cv::Mat tmp = pEx[i].clone();
		cv::Mat tmp2 = example.clone();	
		fncc = calculate_CV_TM_CCORR_NORMED0(tmp, tmp2);

		tmp = pExB[i].clone();
		tmp2 = img_bgr[0].clone();
		fncc_bgr = calculate_CV_TM_CCORR_NORMED0(tmp, tmp2);
		//matchTemplate(tmp, tmp2, ncc, CV_TM_CCORR_NORMED);
		

		tmp = pExG[i].clone();
		tmp2 = img_bgr[1].clone();
		fncc_bgr *= calculate_CV_TM_CCORR_NORMED0(tmp, tmp2);
		//matchTemplate(tmp, tmp2, ncc, CV_TM_CCORR_NORMED);
		

		tmp = pExR[i].clone();
		tmp2 = img_bgr[2].clone();
		fncc_bgr *= calculate_CV_TM_CCORR_NORMED0(tmp, tmp2);
		//matchTemplate(tmp, tmp2, ncc, CV_TM_CCORR_NORMED);

		fncc = powf(fncc_bgr,0.33333);

		nccP = (fncc + 1)*0.5;
		//matchTemplate(pEx[i],example,ncc,CV_TM_CCORR_NORMED);      // measure NCC to positive examples
		//nccP=(((float*)ncc.data)[0]+1)*0.5;
		if (nccP>ncc_thesame)
			anyP = true;
		if (nccP > maxP){
			maxP = nccP;
			maxPidx = i;
			if (i<validatedPart)
				csmaxP = maxP;
		}
	}
	for (int i = 0; i<nEx.size(); i++){
		fncc = calculate_CV_TM_CCORR_NORMED0(nEx[i], example);
		nccN = (fncc + 1)*0.5;
		//matchTemplate(nEx[i],example,ncc,CV_TM_CCORR_NORMED);     //measure NCC to negative examples
		// nccN=(((float*)ncc.data)[0]+1)*0.5;
		if (nccN>ncc_thesame)
			anyN = true;
		if (nccN > maxN)
			maxN = nccN;
}
//set isin
if (anyP) isin[0] = 1;  //if the query patch is highly correlated with any positive patch in the model then it is considered to be one of them
isin[1] = maxPidx;      //get the index of the maximall correlated positive patch
if (anyN) isin[2] = 1;  //if  the query patch is highly correlated with any negative patch in the model then it is considered to be one of them
//Measure Relative Similarity
float dN = 1 - maxN;
float dP = 1 - maxP;
rsconf = (float)dN / (dN + dP);
//Measure Conservative Similarity
dP = 1 - csmaxP;
csconf = (float)dN / (dN + dP);
}

void FerNNClassifier::evaluateTh(const vector<pair<vector<int>,int> >& nXT , const vector<cv::Mat>& nExT){
float fconf;
  for (int i=0;i<nXT.size();i++){
    fconf = (float) measure_forest(nXT[i].first)/nstructs;
    if (fconf>thr_fern)
      thr_fern=fconf;
}
  vector <int> isin;
  float conf,dummy;
  for (int i=0;i<nExT.size();i++){
      NNConf(nExT[i],isin,conf,dummy);
      if (conf>thr_nn)
        thr_nn=conf;
  }
  if (thr_nn>thr_nn_valid)
    thr_nn_valid = thr_nn;
}

void FerNNClassifier::show(){
  Mat examples((int)pEx.size()*pEx[0].rows,pEx[0].cols,CV_8U);
  double minval;
    char fname[128] = {0};
   // sprintf(fname,"/sdcard/lenovo/alwaysinfocus/pos/pos.jpg");

  Mat ex(pEx[0].rows,pEx[0].cols,pEx[0].type());
  for (int i=0;i<pEx.size();i++) {
      minMaxLoc(pEx[i], &minval);
      pEx[i].copyTo(ex);
      ex = ex - minval;
      Mat tmp = examples.rowRange(Range(i * pEx[i].rows, (i + 1) * pEx[i].rows));
      ex.convertTo(tmp, CV_8U);
  }
    imwrite(fname,examples);
  //imshow("Examples",examples);
//  Mat examples2((int)nEx.size()*nEx[0].rows, nEx[0].cols, CV_8U);
//  double minval2;
//  Mat ex2(nEx[0].rows, nEx[0].cols, nEx[0].type());
//  for (int i = 0; i<nEx.size(); i++){
//	  minMaxLoc(nEx[i], &minval2);
//	  nEx[i].copyTo(ex2);
//	  ex2 = ex2 - minval2;
//	  Mat tmp = examples2.rowRange(Range(i*nEx[i].rows, (i + 1)*nEx[i].rows));
//	  ex2.convertTo(tmp, CV_8U);
//  }
//  //imshow("Examples2", examples2);
}
