#include "gist_feature.h"
#include "gist.h"

GistFeature::GistFeature()
{
	gist_data_ = new GistData();
	gist_features_ = NULL;
}

GistFeature::~GistFeature()
{
	delete gist_data_;
	if (gist_features_ != NULL)
		free(gist_features_);
}

void GistFeature::set_parameters(int block_num, int scale_num, std::vector<int> orientation_num_per_scale)
{
	block_num_ = block_num;
	scale_num_ = scale_num;
	orientation_num_per_scale_ = orientation_num_per_scale;
}

float* GistFeature::do_extraction(unsigned char *image_data, int width, int height)
{	
	image_t *im = load_image_from_array(image_data, height, width);
	image_normalize(im);
	{
		if (gist_data_->gabor == NULL){
			int tot_oris = 0;
			for (int i = 0; i < scale_num_; i++)
				tot_oris += orientation_num_per_scale_[i];
			gist_data_ = create_gabor(scale_num_, orientation_num_per_scale_.data(), width, height, gist_data_);
		}
	}
	float *desc = bw_gist_scaletab(im, block_num_, scale_num_, orientation_num_per_scale_.data(), gist_data_->gabor);
	image_delete(im);

	return desc;
}
