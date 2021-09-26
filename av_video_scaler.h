#pragma once
#include <mutex>

struct SwsContext;
struct AVFrame;
class AVVideoScaler
{
public:
	AVVideoScaler();
	virtual ~AVVideoScaler();

	int InitScale(int in_fmt, int out_fmt);
	int InitScale();
	void SetDimension(int input_width, int input_height, int output_width, int output_height);
	void SetOutputSize(int width, int height);
	AVFrame* ImgDataScale(void* data, int* linesize);
	int FrameScale(AVFrame* input, AVFrame* output);
protected:

private:
	int input_width_ = -1;
	int input_height_ = -1;
	int output_width_ = -1;
	int output_height_ = -1;
	int src_width_ = -1;
	int src_height_ = -1;

	int* in_linesize_ = nullptr;
	int* out_linesize_ = nullptr;
	SwsContext* sws_ctx_ = nullptr;

	std::mutex mtx_;
};

