#include "av_screen_cap_handler.h"
extern"C"
{
#include "libavcodec/avcodec.h"
}

#pragma comment(lib,"avcodec.lib")

using namespace std;

static long long NowMs()
{
	return (clock() / (CLOCKS_PER_SEC / 1000));
}

void AVScreenCapHandler::Loop()
{
	while (!is_exit_)
	{
		{
			unique_lock<mutex> lock(mtx_);
			if (frame_)
			{
				//av_frame_free(&frame_);
				av_frame_unref(frame_);
				frame_ = nullptr;
			}
		}
		frame_ = capturer_.GetCapturedFrame();
		if (!frame_)
		{
			this_thread::sleep_for(1ms);
			continue;
		}
		//long long now_sec = NowMs() * 1000;
		//frame_->pts = now_sec;

		is_need_play_ = true;
		AVFrame* frame = av_frame_alloc();
		av_frame_ref(frame, frame_);

		if (!next_)
		{
			this_thread::sleep_for(1ms);
			av_frame_unref(frame);
			av_frame_free(&frame);
			continue;
		}

		AVHandlerPackage pkg;
		pkg.av_type_ = AVHandlerPackageAVType::AVHANDLER_PACKAGE_AV_TYPE_VIDEO;
		pkg.type_ = AVHandlerPackageType::AVHANDLER_PACKAGE_TYPE_FRAME;
		pkg.payload_.frame_ = frame;
		IAVBaseHandler* next = GetNextHandler();
		if (next)
		{
			next->Handle(&pkg);
		}
		this_thread::sleep_for(1ms);
	}

}

bool AVScreenCapHandler::Init()
{
	if (!capturer_.CaptureInit())
	{
		return false;
	}
	else
	{
		return true;
	}
}

bool AVScreenCapHandler::InitScale(int oWidth, int oHeight)
{
	capturer_.setOutputSize(oWidth, oHeight);
	if (!capturer_.ScaleInit())
	{
		return false;
	}
	else
	{
		return true;
	}
}

AVFrame* AVScreenCapHandler::GetFrame()
{
	unique_lock<mutex> lock(mtx_);
	if (!frame_ || !is_need_play_)return nullptr;
	AVFrame* frame = av_frame_alloc();
	int ret = av_frame_ref(frame, frame_);
	if (ret != 0)
	{
		av_frame_free(&frame);
		return nullptr;
	}

	return frame;
}

int AVScreenCapHandler::getInputWidth()
{
	return capturer_.getInputWidth();
}

int AVScreenCapHandler::getInputHeight()
{
	return capturer_.getInputHeight();
}

void AVScreenCapHandler::setOutputSize(int width, int height)
{
	capturer_.setOutputSize(width, height);
}

void AVScreenCapHandler::Handle(AVHandlerPackage* pkg)
{
	//do nothing 
	return;
}