#include "sdl_view.h"
#include<sdl/SDL.h>
#include <iostream>
extern"C"
{
#include <libavformat/avformat.h>
}
#ifdef WIN32

#pragma comment(lib,"SDL2.lib")
#pragma comment(lib,"avformat.lib")

#endif

using namespace std;

int SDLView::InitSdlLibrary()
{
	unique_lock<mutex> lock(mtx_);
	if (!is_init_lib_)
	{
		int ret = SDL_Init(SDL_INIT_VIDEO);
		if (ret != 0)
		{
			cout << "SDL_Init failed" << SDL_GetError() << endl;
			return -1;
		}

		ret = SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
		//if (ret != 0)
		//{
		//	cout << "SDL_SetHint failed" << SDL_GetError() << endl;
		//	return -1;
		//}

		is_init_lib_ = true;
	}
	return 0;
}

int SDLView::InitView(int width,int height,PixFormat fmt,void* win_id)
{
	if (InitSdlLibrary() != 0)
	{
		cout << "InitSdlLibrary failed" << endl;
		return -1;
	}

	int format = -1;
	unique_lock<mutex> lock(mtx_);

	width_ = width;
	height_ = height;
	pix_fmt_ = fmt;
	window_id_ = win_id;

	if (!win_id)
	{
		window_ = SDL_CreateWindow("win",
			SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
			width, height, SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
	}
	else
	{
		window_ = SDL_CreateWindowFrom(win_id);
	}
	if (!window_)
	{
		cout << "SDL_CreateWindowFrom failed" << endl;
		return -1;
	}

	renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE);
	if (!renderer_)
	{
		cout << "SDL_CreateRenderer failed : " << SDL_GetError() << endl;
		return -1;
	}

	switch (pix_fmt_)
	{
		case PixFormat::PIX_FORMAT_YUV420P:
			format = SDL_PIXELFORMAT_IYUV;
			break;
		case PixFormat::PIX_FORMAT_ARGB:
			format = SDL_PIXELFORMAT_ARGB32;
			break;
		case PixFormat::PIX_FORMAT_BGRA:
			format = SDL_PIXELFORMAT_BGRA32;
			break;
		case PixFormat::PIX_FORMAT_RGBA:
			format = SDL_PIXELFORMAT_RGBA32;
			break;
	}

	texture_ = SDL_CreateTexture(renderer_, format, SDL_TEXTUREACCESS_STREAMING, width, height);
	if (!texture_)
	{
		cout << "SDL_CreateTexture failed : " << SDL_GetError() << endl;
		return -1;
	}
	return 0;
}

int SDLView::DrawView(uint8_t* buffer, int size)
{
	unique_lock<mutex> lock(mtx_);
	if (!window_ || !renderer_ || !texture_)
	{
		cout << "DrawView failed : (!window_ || !renderer_ || !texture_)" << endl;
		return -1;
	}

	if (width_ == 0 || height_ == 0)
	{
		cout << "DrawView failed : (width_ == 0 || height_ == 0)" << endl;
		return -1;
	}

	if (size <= 0)
	{
		switch (pix_fmt_)
		{
		case PixFormat::PIX_FORMAT_YUV420P:
			size = width_;
			break;
		case PixFormat::PIX_FORMAT_ARGB:
		case PixFormat::PIX_FORMAT_BGRA:
		case PixFormat::PIX_FORMAT_RGBA:
			size = width_ * 4;
			break;
		default:
			cout << "DrawView : could not find a linesize" << endl;
			return -1;
		}
	}

	SDL_RenderClear(renderer_);

	int ret = SDL_UpdateTexture(texture_, nullptr, buffer, size);
	if (ret != 0)
	{
		cout << "SDL_UpdateTexture failed : " << SDL_GetError() << endl;
		return -1;
	}

	SDL_Rect rect;
	SDL_Rect* pRect = nullptr;
	if (scaled_width_ > 0 && scaled_height_ > 0)
	{
		rect.x = 0;
		rect.y = 0;
		rect.w = scaled_width_;
		rect.h = scaled_height_;
		pRect = &rect;
	}
	ret = SDL_RenderCopy(renderer_, texture_, nullptr, pRect);
	if (ret != 0)
	{
		cout << "SDL_RenderCopy failed : " << SDL_GetError() << endl;
		return -1;
	}
	SDL_RenderPresent(renderer_);
	return 0;
}

int SDLView::DrawView(uint8_t* y_buffer, int y_size, uint8_t* u_buffer, int u_size, uint8_t* v_buffer, int v_size)
{

	if (!y_buffer || y_size <= 0 || !u_buffer || u_size <= 0 || !v_buffer || v_size <= 0)
	{
		cout << "DrawView failed : (!y_buffer || y_size <= 0 || !u_buffer || u_size || !v_buffer || v_size)" << endl;
		return -1;
	}

	unique_lock<mutex> lock(mtx_);

	if (!window_ || !texture_ || !renderer_)
	{
		cout << "DrawView failed : (!window_ || !texture_ || !renderer_)" << endl;
		return -1;
	}

	if (width_ == 0 || height_ == 0)
	{
		cout << "DrawView failed : (width_ == 0 || height_ == 0)" << endl;
		return -1;
	}

	SDL_RenderClear(renderer_);
	int ret = SDL_UpdateYUVTexture(texture_, nullptr, y_buffer, y_size, u_buffer, u_size, v_buffer, v_size);
	if (ret != 0)
	{
		cout << "SDL_UpdateYUVTexture failed : " << SDL_GetError() << endl;
		return -1;
	}

	SDL_Rect rect;
	SDL_Rect* pRect = nullptr;

	if (scaled_width_ > 0 && scaled_width_ > 0)
	{
		rect.x = 0;
		rect.y = 0;
		rect.w = scaled_width_;
		rect.h = scaled_height_;
		pRect = &rect;
	}

	ret = SDL_RenderCopy(renderer_, texture_, nullptr, pRect);
	if (ret != 0)
	{
		cout << "SDL_RenderCopy failed : " << SDL_GetError() << endl;
		return -1;
	}

	SDL_RenderPresent(renderer_);
	return 0;
}

int SDLView::DestoryView()
{
	unique_lock<mutex> lock(mtx_);
	if (texture_)
	{
		SDL_DestroyTexture(texture_);
	}

	if (renderer_)
	{
		SDL_DestroyRenderer(renderer_);
	}

	if (window_)
	{
		SDL_DestroyWindow(window_);
	}
	texture_ = nullptr;
	renderer_ = nullptr;
	window_ = nullptr;
	return 0;
}

int SDLView::ResetView()
{
	unique_lock<mutex> lock(mtx_);
	if (texture_)
	{
		SDL_DestroyTexture(texture_);
	}

	if (renderer_)
	{
		SDL_DestroyRenderer(renderer_);
	}

	texture_ = nullptr;
	renderer_ = nullptr;
	return 0;
}