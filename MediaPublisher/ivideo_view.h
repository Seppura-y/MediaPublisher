#ifndef IVIDEO_VIEW_H
#define IVIDEO_VIEW_H


#include <mutex>

struct AVCodecParameters;
struct AVFrame;

enum class PixFormat
{
	PIX_FORMAT_YUV420P = 0,
	PIX_FORMAT_RGBA = 1,
	PIX_FORMAT_ARGB = 2,
	PIX_FORMAT_BGRA = 3
};

enum class RenderType
{
	RENDER_TYPE_SDL = 0,
	RENDER_TYPE_OPENGL
};

class IVideoView
{
public:
	static IVideoView* CreateView(RenderType type = RenderType::RENDER_TYPE_SDL);
	virtual int InitView(AVCodecParameters* param);
	virtual int DrawFrame(AVFrame* frame);
	virtual int ScaleView(int width, int height);
	virtual int InitView(int width, int height, PixFormat fmt, void* win_id = nullptr) = 0;
	virtual int DestoryView() = 0;
	virtual int ResetView() = 0;
	virtual void SetWindowId(void* win);
protected:
	virtual int DrawView(uint8_t* buffer, int size) = 0;
	virtual int DrawView(uint8_t* y_buffer, int y_size, uint8_t* u_buffer, int u_size, uint8_t* v_buffer, int v_size) = 0;
protected:
	std::mutex mtx_;
	int width_ = 0;
	int height_ = 0;
	int scaled_width_ = 0;
	int scaled_height_ = 0;
	void* window_id_ = nullptr;
	PixFormat pix_fmt_ = PixFormat::PIX_FORMAT_YUV420P;
};

#endif