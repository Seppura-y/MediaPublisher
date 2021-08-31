#pragma once
#include <d3d11.h>
#include <dxgi1_2.h>
#include <mutex>

#pragma comment(lib, "D3D11.lib")
#pragma comment(lib, "DXGI.lib")

#define RESET_OBJECT(x) {if (x) x->Release(); x = nullptr;}

struct AVFrame;
struct SwsContext;
class AVScreenCapturer
{
public:

	bool CaptureInit();
	bool ScaleInit();

	bool CaptureImgData(void* pImgData, INT& nImgSize);
	AVFrame* ImgDataScale(void* data, int* linesize);
	AVFrame* GetCapturedFrame();

	void CaptureRotatelImg(unsigned char* pImgData);
	void CaptureSaveBmpFile(const char* fileName, unsigned char* pImgData, int imgLength);

	int getInputWidth();
	int getInputHeight();
	void setOutputSize(int width, int height);

private:
	int screen_width_ = -1;
	int screen_height_ = -1;
	int out_width_ = -1;
	int out_height_ = -1;
	SwsContext* sws_ctx_ = nullptr;
	unsigned char* pImgData_ = nullptr;
	int* plinesize_ = nullptr;
	std::mutex mtx_;
};

