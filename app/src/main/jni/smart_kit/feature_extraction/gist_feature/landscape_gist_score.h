#ifndef GIST_SCORE_H
#define GIST_SCORE_H
#include "gist_feature.h"

namespace smart {

class LandscapeGistScore
{
public:
	LandscapeGistScore();
	~LandscapeGistScore();
	float get_score();
	float compute_gist_score(unsigned char* image_data, int width, int height, int orientation);

private:
	GistFeature *gist_feature_;
	unsigned char *imageData_;
	int height_;
	int width_;
	int feature_length_;
	float gist_score_;
};

} /* namespace smart */

#endif
