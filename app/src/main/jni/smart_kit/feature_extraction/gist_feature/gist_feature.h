/* Lear's GIST implementation, version 1.1, (c) INRIA 2009, Licence: PSFL */
#ifndef GIST_FEATURE_HH
#define GIST_FEATURE_HH
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <iostream>
#include <vector>

#include "standalone_image.h"
class GistFeature
{
public:
	GistFeature();
	~GistFeature();
	void set_parameters(int block_num, int scale_num, std::vector<int> orientation_num_per_scale);
	float* do_extraction(unsigned char *image_data, int width, int height);	
private:
	GistData *gist_data_;
	float *gist_features_;
	int block_num_;
	int scale_num_;
	std::vector<int> orientation_num_per_scale_;
};

#endif

