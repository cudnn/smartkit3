#include <opencv2/opencv.hpp>
//#include <opencv2/legacy/legacy.hpp>
#include "tld_utils.h"
#include "MedianFlowTracker.h"
#include "FerNNClassifier.h"
#include "PatchGenerator.h"
#include <fstream>
//#include "../IVCSceneWrapper.h"
namespace smart{
//Bounding Boxes
struct BoundingBox : public cv::Rect {
  BoundingBox(){}
  BoundingBox(cv::Rect r): cv::Rect(r){}
public:
  float overlap;        //Overlap with current Bounding Box
  int sidx;             //scale index
};

//Detection structure
struct DetStruct {
    std::vector<int> bb;
    std::vector<std::vector<int> > patt;
    std::vector<float> conf1;
    std::vector<float> conf2;
    std::vector<std::vector<int> > isin;
    std::vector<cv::Mat> patch;
  };
//Temporal structure
  struct TempStruct {
    std::vector<std::vector<int> > patt;
    std::vector<float> conf;
  };

struct OComparator{
  OComparator(const std::vector<BoundingBox>& _grid):grid(_grid){}
  std::vector<BoundingBox> grid;
  bool operator()(int idx1,int idx2){
    return grid[idx1].overlap > grid[idx2].overlap;
  }
};
struct CComparator{
  CComparator(const std::vector<float>& _conf):conf(_conf){}
  std::vector<float> conf;
  bool operator()(int idx1,int idx2){
    return conf[idx1]> conf[idx2];
  }
};


class TLD{
private:
  cv::PatchGenerator generator;
  FerNNClassifier classifier;
  MedianFlowTracker tracker;
  ///Parameters
  int bbox_step;
  int min_win;
  int patch_size;
  //initial parameters for positive examples
  int num_closest_init;
  int num_warps_init;
  int noise_init;
  float angle_init;
  float shift_init;
  float scale_init;
  //update parameters for positive examples
  int num_closest_update;
  int num_warps_update;
  int noise_update;
  float angle_update;
  float shift_update;
  float scale_update;
  //parameters for negative examples
  float bad_overlap;
  float bad_patches;
  ///Variables
//Integral Images
  cv::Mat iisum;
  cv::Mat iisqsum;
  float var;
//Training data
  std::vector<std::pair<std::vector<int>,int> > pX; //positive ferns <features,labels=1>
  std::vector<std::pair<std::vector<int>,int> > nX; // negative ferns <features,labels=0>
  cv::Mat pEx;  //positive NN example
  cv::Mat pExR;
  cv::Mat pExG;
  cv::Mat pExB;
  std::vector<cv::Mat> nEx; //negative NN examples
//Test data
  std::vector<std::pair<std::vector<int>,int> > nXT; //negative data to Test
  std::vector<cv::Mat> nExT; //negative NN examples to Test
//Last frame data
  BoundingBox lastbox;
  bool lastvalid;
  float lastconf;
//Current frame data
  //Tracker data
  bool tracked;
  BoundingBox tbb;
  cv::Rect_<float> ftbb;     //Subpixel tracking results
  bool tvalid;
  float tconf;
  //Detector data
  TempStruct tmp;
  DetStruct dt;
  std::vector<BoundingBox> dbb;
  std::vector<bool> dvalid;
  std::vector<float> dconf;
  bool detected;
  bool isTrackerInit;
  int  detect_interval;			//Re-detection interval
  long  framecnt;
  float scale_ratio;
  int  det_window;				//Detection window size during tracking

  //Bounding Boxes
  std::vector<BoundingBox> grid;
  std::vector<cv::Size> scales;
  std::vector<int> good_boxes; //indexes of bboxes with overlap > 0.6
  std::vector<int> bad_boxes; //indexes of bboxes with overlap < 0.2
  BoundingBox bbhull; // hull of good_boxes
  BoundingBox best_box; // maximum overlapping bbox

  cv::Mat prev_image_;	//Store previous images
  cv::Mat prev_image_Cb;
  cv::Mat prev_image_Cr;
public:
  //Constructors
  TLD();
  TLD(const cv::FileNode& file);
  void read(const cv::FileNode& file);
  //Methods
  void init(const cv::Mat& frame1, const cv::Mat& frame1Cb, const cv::Mat& frame1Cr,const cv::Rect &initbox, FILE* bb_file);
  void generatePositiveData(const cv::Mat& frame, const cv::Mat& frameCb, const cv::Mat& frameCr, int num_warps);
  void generateNegativeData(const cv::Mat& frame);
  void processFrame(/*const cv::Mat& img1,*/const cv::Mat& img, const cv::Mat& imgCb, const cv::Mat& imgCr,/*std::vector<cv::Point2f>& points1,std::vector<cv::Point2f>& points2,*/
      BoundingBox& bbnext,bool& lastboxfound, bool tl);
  void track(/*const cv::Mat& img1,*/ const cv::Mat& img2/*,std::vector<cv::Point2f>& points1,std::vector<cv::Point2f>& points2*/);
  void detect(const cv::Mat& frame, const cv::Mat& frame1Cb, const cv::Mat& frame1Cr);
  void clusterConf(const std::vector<BoundingBox>& dbb,const std::vector<float>& dconf,std::vector<BoundingBox>& cbb,std::vector<float>& cconf);
  void evaluate();
  void learn(const cv::Mat& img, const cv::Mat& imgCb, const cv::Mat& imgCr);
  //Tools
  void buildGrid(const cv::Mat& img, const cv::Rect& box);
  float bbOverlap(const BoundingBox& box1,const BoundingBox& box2);
  void getOverlappingBoxes(const cv::Rect& box1,int num_closest);
  void getBBHull();
  void getPattern(const cv::Mat& img, cv::Mat& pattern,cv::Scalar& mean,cv::Scalar& stdev);
  void bbPoints(std::vector<cv::Point2f>& points, const BoundingBox& bb);
  void bbPredict(const std::vector<cv::Point2f>& points1,const std::vector<cv::Point2f>& points2,
      const BoundingBox& bb1,BoundingBox& bb2);
  double getVar(const BoundingBox& box,const cv::Mat& sum,const cv::Mat& sqsum);
  bool bbComp(const BoundingBox& bb1,const BoundingBox& bb2);
  int clusterBB(const std::vector<BoundingBox>& dbb,std::vector<int>& indexes);

  void init_clear();	//Clean and release all the vectors and buffers for reinitialization
  int  get_pEx_size(); // return the pEx size() of tld;
};

} //namespace end
