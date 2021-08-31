//#pragma once
//#include <mutex>
//
//struct SwsContext;
//struct AVFrame;
//class AVVideoScaler
//{
//public:
//	AVVideoScaler();
//	~AVVideoScaler();
//
//	int InitScale();
//	void SetOutputSize(int width, int height);
//	AVFrame* ImgDataScale(void* data, int* linesize);
//protected:
//
//private:
//	int output_width_ = -1;
//	int output_height_ = -1;
//	int src_width_ = -1;
//	int src_height_ = -1;
//	SwsContext* sws_ctx_ = nullptr;
//
//	std::mutex mtx_;
//};
//
