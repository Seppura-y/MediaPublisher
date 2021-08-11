#include "ivideo_view.h"
#include "sdl_view.h"

#include <iostream>
extern "C"
{
#include <libavformat/avformat.h>
}
#ifdef WIN32

#pragma comment(lib,"avformat.lib")

#endif

using namespace std;

IVideoView* IVideoView::CreateView(RenderType type)
{
	switch (type)
	{
	case RenderType::RENDER_TYPE_SDL:
	{
		IVideoView* view = new SDLView();
		return view;
	}
	case RenderType::RENDER_TYPE_OPENGL:
		break;
	default:
		break;
	}
	return nullptr;
}

int IVideoView::InitView(AVCodecParameters* param)
{
	if (!param)
	{
		return -1;
	}

	width_ = param->width;
	height_ = param->height;
	switch (param->format)
	{
	case AV_PIX_FMT_YUV420P:
	case AV_PIX_FMT_YUVJ420P:
	{
		pix_fmt_ = PixFormat::PIX_FORMAT_YUV420P;
		break;
	}
	case AV_PIX_FMT_RGBA:
	{
		pix_fmt_ = PixFormat::PIX_FORMAT_RGBA;
		break;
	}
	case AV_PIX_FMT_BGRA:
	{
		pix_fmt_ = PixFormat::PIX_FORMAT_BGRA;
		break;
	}
	case AV_PIX_FMT_ARGB:
	{
		pix_fmt_ = PixFormat::PIX_FORMAT_ARGB;
		break;
	}
	default:
		break;
	}

	int ret = InitView(width_, height_, pix_fmt_, window_id_);
	return ret;
}

int IVideoView::DrawFrame(AVFrame* frame)
{
	if (!frame || !frame->data[0])
	{
		cout << "DrawView : frame or frame->data is nullptr" << endl;
		return -1;
	}
	switch (frame->format)
	{
	case AV_PIX_FMT_YUV420P:
		return DrawView(frame->data[0], frame->linesize[0], frame->data[1], frame->linesize[1], frame->data[2], frame->linesize[2]);
	case AV_PIX_FMT_RGBA:
	case AV_PIX_FMT_ARGB:
	case AV_PIX_FMT_BGRA:
		return DrawView(frame->data[0], frame->linesize[0]);
	default:
		return -1;
	}
}

int IVideoView::ScaleView(int width, int height)
{
	unique_lock<mutex> lock(mtx_);
	scaled_width_ = width;
	scaled_height_ = height;

	return 0;
}