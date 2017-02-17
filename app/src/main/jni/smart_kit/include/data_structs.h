#ifndef SMART_KIT_DATA_STRUCTS_H_
#define SMART_KIT_DATA_STRUCTS_H_

#include "smart_type.h"
#include "log.h"
#include <string.h>

namespace smart {

struct Image {
	Image(unsigned char* data_, const int width_, const int height_, int mem_size = -1) {
		width = width_;
		height = height_;
		is_allocate = 0;
		if (mem_size == -1) memory_size = width * height;
		else memory_size = mem_size;
		
		data = data_;
//		switch (TYPE) {
//			case IMAGE_TYPE_Y:
//			case IMAGE_TYPE_YUV420SP:
//				data = data_;
//				break;
//			default:
//				break;
//		}

	}
	Image(const int width_, const int height_, int mem_size = -1) {
		width = width_;
		height = height_;
		is_allocate = 1;
		if (mem_size == -1) memory_size = width * height;
		else memory_size = mem_size;

		data = new unsigned char[memory_size];
//		switch (TYPE) {
//			case IMAGE_TYPE_Y:
//				data = new unsigned char[width_ * height_];
//				break;
//			case IMAGE_TYPE_YUV420SP:
//				data = new unsigned char[width_ * height_ * 3 / 2];
//				break;
//			default:
//				break;
//		}
	}

	~Image() {
		if (is_allocate) delete[] data;
		SMART_DEBUG("xxx---------->Image deleted");
	}

	unsigned char* data;
	int width;
	int height;
	int is_allocate;
//	const int k_image_type;
	int memory_size;
};

}/* namespace smart */

#endif /* SMART_KIT_DATA_STRUCTS_H_ */
