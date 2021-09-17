#include "av_screen_cap_handler.h"
extern"C"
{
#include "libavcodec/avcodec.h"
#include "libavutil/time.h"
}

#pragma comment(lib,"avcodec.lib")

using namespace std;

static long long NowMs()
{
	return (clock() / (CLOCKS_PER_SEC / 1000));
}

void AVScreenCapHandler::Loop()
{
	start_time_ = av_gettime_relative();
	while (!is_exit_)
	{
		{
			unique_lock<mutex> lock(mtx_);
			if (frame_)
			{
				//av_frame_free(&frame_);
				av_frame_unref(frame_);
				av_frame_free(&frame_);
				frame_ = nullptr;
			}
		}
		frame_ = capturer_.GetCapturedFrame();
		if (!frame_)
		{
			continue;
		}

		int64_t now = av_gettime_relative();
		frame_->pts = now - start_time_;
		frame_->pkt_dts = now - start_time_;
		frame_->pkt_pts = now - start_time_;
		frame_->pkt_duration = av_rescale_q(40, AVRational{ 1,1000 }, av_get_time_base_q());

		is_need_play_ = true;
		AVFrame* frame = av_frame_alloc();
		av_frame_ref(frame, frame_);

		if (!next_)
		{
			//this_thread::sleep_for(1ms);
			av_frame_unref(frame);
			av_frame_free(&frame);
			this_thread::sleep_for(40ms);
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
			av_frame_unref(frame);
			av_frame_free(&frame);
		}
		this_thread::sleep_for(40ms);
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