/*
 * FerNNClassifier.h
 *
 *  Created on: Jun 14, 2011
 *      Author: alantrrs
 */

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
//#include <opencv2/ocl/ocl.hpp>
#include "tld_utils.h"
class FerNNClassifier{
private:
  float thr_fern;
  int structSize;
  int nstructs;
  float valid;
  float ncc_thesame;
  float thr_nn;
  int acum;
  void init_clear();
public:
  //Parameters
  float thr_nn_valid;
  void setParameters(float valid,float ncc_thesame, int nstructs, int structSize, float thr_fern, float thr_nn, float thr_nn_valid);
  void init();
  void read(const cv::FileNode& file);
  void prepare(const std::vector<cv::Size>& scales);
  void getFeatures(const cv::Mat& image,const int& scale_idx,std::vector<int>& fern);
  void update(const std::vector<int>& fern, int C, int N);
  float measure_forest(std::vector<int> fern);
  void trainF(const std::vector<std::pair<std::vector<int>,int> >& ferns,int resample);
  void trainNN(const std::vector<cv::Mat>& nn_examples, const cv::Mat& pExb, const cv::Mat& pExg, const cv::Mat& pExr);
  void NNConf(const cv::Mat& example,std::vector<int>& isin,float& rsconf,float& csconf);
  void NNConfBGR(const cv::Mat& example, const cv::Mat& example_bgr, std::vector<int>& isin, float& rsconf, float& csconf);
  void evaluateTh(const std::vector<std::pair<std::vector<int>,int> >& nXT,const std::vector<cv::Mat>& nExT);
  void show();
  //Ferns Members
  int getNumStructs(){return nstructs;}
  float getFernTh(){return thr_fern;}
  float getNNTh(){return thr_nn;}
  struct Feature
      {
          uchar x1, y1, x2, y2;
          Feature() : x1(0), y1(0), x2(0), y2(0) {}
          Feature(int _x1, int _y1, int _x2, int _y2)
          : x1((uchar)_x1), y1((uchar)_y1), x2((uchar)_x2), y2((uchar)_y2)
          {}
          bool operator ()(const cv::Mat& patch) const
          { return patch.at<uchar>(y1,x1) > patch.at<uchar>(y2, x2); }
      };
  std::vector<std::vector<Feature> > features; //Ferns features (one std::vector for each scale)
  std::vector< std::vector<int> > nCounter; //negative counter
  std::vector< std::vector<int> > pCounter; //positive counter
  std::vector< std::vector<float> > posteriors; //Ferns posteriors
  float thrN; //Negative threshold
  float thrP;  //Positive thershold
  //NN Members
  std::vector<cv::Mat> pEx; //NN positive examples
  std::vector<cv::Mat> pExB; //NN positive examples
  std::vector<cv::Mat> pExG; //NN positive examples
  std::vector<cv::Mat> pExR; //NN positive examples
  std::vector<cv::Mat> nEx; //NN negative examples
};
