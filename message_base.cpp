#include "message_base.h"
#include <stdlib.h>
#include <string.h>

NALUStruct::NALUStruct(int size)
{
	size_ = size;
	type_ = NALU_TYPE_UNDEFINE_1;
	data_ = (uint8_t*)malloc(size_ * sizeof(uint8_t));
}
NALUStruct::NALUStruct(const char* buf, int size)
{
	size_ = size;
	type_ = (NALUType)(buf[4] & 0x1f);
	data_ = (uint8_t*)malloc(size_ * sizeof(uint8_t));
	memcpy(this->data_, buf, size);
}
NALUStruct::~NALUStruct()
{
	if (data_)
	{
		free(data_);
	}
	data_ = nullptr;
}



YUVStruct::YUVStruct(int w, int h, int size)
{
	width_ = w;
	height_ = h;
	size_ = size;
	data_ = (uint8_t*)malloc(size_ * sizeof(uint8_t));
}

YUVStruct::YUVStruct(uint8_t* buff,int w, int h, int size)
{
	width_ = w;
	height_ = h;
	size_ = size;
	data_ = (uint8_t*)malloc(size_ * sizeof(uint8_t));
	memcpy(data_, buff, size);
}

YUVStruct::~YUVStruct()
{
	if (data_)
	{
		free(data_);
	}
	data_ = nullptr;
}

YUV420P::YUV420P(int w, int h, int size) : YUVStruct(w,h,size)
{
	int frame_size = width_ * height_;
	y_slice_ = data_;
	u_slice_ = data_ + frame_size;
	v_slice_ = data_ + frame_size * 5 / 4;
}

YUV420P::YUV420P(uint8_t* buff, int w, int h, int size) : YUVStruct(buff,w,h,size)
{
	int frame_size = width_ * height_;
	y_slice_ = data_;
	u_slice_ = data_ + frame_size;
	v_slice_ = data_ + frame_size * 5 / 4;
}

YUV420P::~YUV420P()
{

}