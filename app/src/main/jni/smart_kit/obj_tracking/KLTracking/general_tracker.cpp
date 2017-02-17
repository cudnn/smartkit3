#include "general_tracker.h"

//#include <GL/glew.h>
//#include <GL/glut.h>
//#include "../Libs/opencvar/acgl.h"
//#include "../Libs/opencvar/acmath.h"

using namespace std;
using namespace cv;


GeneralTracker::GeneralTracker() : mTracker(NULL), mFrameNo(0)
{
//	glbGeneralARObj = this;
}

GeneralTracker::~GeneralTracker()
{
	delete mTracker;
	mTracker = NULL;
}

void GeneralTracker::InitTracker(const cv::Mat& gray)
{
	if (mTracker == NULL)
		mTracker = new LKTracker();

	int MaxNumFeatures = 2000;
	mTracker->DetectFeatures(gray, MaxNumFeatures);
}


float GeneralTracker::evaluate_tracked_markers_with_detected_ones(const cv::Point2f* markers, const cv::Point2f* trackedMarkers) const
{
	float dist[GLB_NUM_MARKERS] = { 0.0 };
	float sumdist2 = 0.0;
	Vec2f s;
	for (int i = 0; i < GLB_NUM_MARKERS; i++)
	{
		if (i == GLB_NUM_MARKERS - 2) // THIS MARKER WAS PREDICED, NOT DEPENDABLE.
			continue;

		s = markers[i] - trackedMarkers[i];
		dist[i] = s.dot(s);
		sumdist2 = max(sumdist2, dist[i]);
		//sumdist2 += dist[i];
	}
	//sumdist2 /= (NUM_QRCODE_MARKERS - 1);
	return sumdist2;
}

