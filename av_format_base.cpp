#include "av_format_base.h"
#include "av_data_tools.h"

extern"C"
{
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
}

#include <iostream>

#ifdef _WIN32
#pragma comment(lib,"avformat.lib")
#pragma comment(lib,"avutil.lib")
#endif

using namespace std;

AVFormatBase::AVFormatBase()
{
	if (!audio_timebase_)
	{
		audio_timebase_ = new AVRational();
		//audio_timebase_->den
	}
	if (!video_timebase_)
	{
		video_timebase_ = new AVRational();
	}
}

AVFormatBase::~AVFormatBase()
{
	if (audio_timebase_)
	{
		delete audio_timebase_;
		audio_timebase_ = nullptr;
	}
	if (video_timebase_)
	{
		delete video_timebase_;
		video_timebase_ = nullptr;
	}

	CloseContext();
}

int AVFormatBase::SetFormatContext(AVFormatContext* ctx)
{
	CloseContext();
	unique_lock<mutex> lock(mtx_);
	fmt_ctx_ = ctx;
	if (fmt_ctx_ == nullptr)
	{
		is_network_connected_ = false;
		return 0;
	}
	is_network_connected_ = true;

	audio_index_ = -1;
	video_index_ = -1;
	for (int i = 0; i < fmt_ctx_->nb_streams; i++)
	{
		if (fmt_ctx_->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
		{
			audio_index_ = fmt_ctx_->streams[i]->index;
			audio_timebase_->den = fmt_ctx_->streams[i]->time_base.den;
			audio_timebase_->num = fmt_ctx_->streams[i]->time_base.num;
		}
		if (fmt_ctx_->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			video_index_ = i;
			video_timebase_->den = fmt_ctx_->streams[i]->time_base.den;
			video_timebase_->num = fmt_ctx_->streams[i]->time_base.num;
		}
	}

}

void AVFormatBase::SetTimeout(int ms)
{
	unique_lock<mutex> lock(mtx_);
	connect_timeout_ = ms;
	if (fmt_ctx_)
	{
		AVIOInterruptCB cb = { TimeoutCallback,this };
		fmt_ctx_->interrupt_callback = cb;
	}
}

int AVFormatBase::TimeoutCallback(void* opaque)
{
	AVFormatBase* fmt_ctx = (AVFormatBase*)opaque;
	if (fmt_ctx->IsTimeout())
	{
		return 1;
	}
	return 0;
}

bool AVFormatBase::IsTimeout()
{
	//unique_lock<mutex> lock(mtx_);
	if (GetCurrentTimeMsec() - last_read_time_ > connect_timeout_)
	{
		is_network_connected_ = false;
		//last_read_time_ = GetCurrentTimeMsec();  ???
		return true;
	}
	return false;
}

int AVFormatBase::CloseContext()
{
	unique_lock<mutex> lock(mtx_);
	if (fmt_ctx_)
	{
		if (fmt_ctx_->oformat)
		{
			/* -muxing: set by the user before avformat_write_header().The caller must
			* take care of closing / freeing the IO context.
			*/
			avio_close(fmt_ctx_->pb);
			avformat_free_context(fmt_ctx_);
		}
		else if (fmt_ctx_->iformat)
		{
			avformat_close_input(&fmt_ctx_);
		}
		else
		{
			avformat_free_context(fmt_ctx_);
		}

		fmt_ctx_ = nullptr;
	}
	return 0;
}

bool AVFormatBase::is_network_connected()
{
	unique_lock<mutex> lock(mtx_);
	return is_network_connected_;
}


bool AVFormatBase::HasVideo()
{
	unique_lock<mutex> lock(mtx_);
	if (video_index_ != -1) return true;
	else return false;
}


bool AVFormatBase::HasAudio()
{
	unique_lock<mutex> lock(mtx_);
	if (audio_index_ != -1) return true;
	else return false;
}

int AVFormatBase::get_audio_index()
{
	unique_lock<mutex> lock(mtx_);
	return audio_index_;
}
int AVFormatBase::get_video_index()
{
	unique_lock<mutex> lock(mtx_);
	return video_index_;
}

std::shared_ptr<AVParamWarpper> AVFormatBase::CopyVideoParameters()
{
	shared_ptr<AVParamWarpper> shr_ptr;
	unique_lock<mutex> lock(mtx_);
	if (!HasVideo() || fmt_ctx_ == nullptr)
	{
		return shr_ptr;
	}
	shr_ptr.reset(AVParamWarpper::Create());
	avcodec_parameters_copy(shr_ptr->para, fmt_ctx_->streams[video_index_]->codecpar);
	*shr_ptr->time_base = fmt_ctx_->streams[video_index_]->time_base;
	return shr_ptr;
}


std::shared_ptr<AVParamWarpper> AVFormatBase::CopyAudioParameters()
{
	shared_ptr<AVParamWarpper> shr_ptr;
	unique_lock<mutex> lock(mtx_);
	if (!HasAudio() || fmt_ctx_ == nullptr)
	{
		return shr_ptr;
	}
	shr_ptr.reset(AVParamWarpper::Create());
	avcodec_parameters_copy(shr_ptr->para, fmt_ctx_->streams[audio_index_]->codecpar);
	*shr_ptr->time_base = fmt_ctx_->streams[audio_index_]->time_base;
	return shr_ptr;
}