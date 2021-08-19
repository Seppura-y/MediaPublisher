#include "av_demux_handler.h"
#include "avtimebase.h"

#include <iostream>

extern"C"
{
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
}

#ifdef _WIN32
#pragma comment (lib,"avformat.lib")
#pragma comment (lib,"avutil.lib")

#endif

using namespace std;
bool AVDemuxHandler::OpenAVSource(const char* url,int timeout)
{
	demuxer_.CloseContext();
	AVFormatContext* fmt_ctx = nullptr;

	fmt_ctx = demuxer_.OpenContext(url);
	demuxer_.SetFormatContext(fmt_ctx);
	url_ = url;
	video_index_ = demuxer_.get_video_index();
	audio_index_ = demuxer_.get_audio_index();
	if (strstr(url, "mp4") || strstr(url,"flv"))
	{
		is_local_file_ = true;
		return true;
	}
	demuxer_.SetTimeout(timeout);
	return true;
}


void AVDemuxHandler::Loop()
{
	AVPacket* pkt = av_packet_alloc();
	while (!is_exit_)
	{
		if (demuxer_.Read(pkt) >= 0)
		{
			if (is_local_file_)
			{
				if (pkt->data && pkt->size > 0 && is_first_packet_)
				{
					//start_time_ = AVPublishTime::GetInstance()->GetCurrentTimeMSec();
					start_time_ = GetCurrentTimeMsec();
					is_first_packet_ = false;
				}
				AVRational src_rational;
				src_rational.den = demuxer_.GetVideoTimebase()->den;
				src_rational.num = demuxer_.GetVideoTimebase()->num;
				int64_t duration = DurationScale(pkt->duration, src_rational);
				total_duration_ += duration;
				//SleepForMsec(duration);
				//SleepForMsec(40);
				

				//int64_t current_time = AVPublishTime::GetInstance()->GetCurrentTimeMSec();
				//int64_t current_time = GetCurrentTimeMsec();
				//int64_t diff = current_time - start_time_;
				//if (diff < total_duration_)
				//{
				//	//this_thread::sleep_for(40ms);
				//	//cout << "demux handler continue" << endl;
				//	continue;
				//}

				this_thread::sleep_for(30ms);
			}
			
			if (is_callback_enable_)
			{
				callable_object_(pkt);
			}
			if (GetNextHandler() && pkt->stream_index == video_index_)
			{
				AVHandlerPackage payload;
				payload.type_ = AVHandlerPackageType::AVHANDLER_PACKAGE_TYPE_PACKET;
				payload.av_type_ = AVHandlerPackageAVType::AVHANDLER_PACKAGE_AV_TYPE_VIDEO;
				payload.payload_.packet_ = pkt;
				if (GetNextHandler())
				{
					GetNextHandler()->Handle(&payload);
				}
				av_packet_unref(pkt);
			}
		}
		else
		{
			cout << "demux handler exit " << endl;
			//is_exit_ = true;
			cout << total_duration_;
		}
		if (!demuxer_.is_network_connected())
		{
			demuxer_.OpenContext(url_.c_str());
		}
	}
	av_packet_free(&pkt);
}


void AVDemuxHandler::Handle(AVHandlerPackage* pkt)
{
	//first node : nothing to do
	return;
}

void AVDemuxHandler::Stop()
{
	IAVBaseHandler::Stop();
	demuxer_.CloseContext();
}


std::shared_ptr<AVParamWarpper> AVDemuxHandler::CopyVideoParameters()
{
	return demuxer_.CopyVideoParameters();
}

std::shared_ptr<AVParamWarpper> AVDemuxHandler::CopyAudioParameters()
{
	return demuxer_.CopyAudioParameters();
}

int AVDemuxHandler::GetVideoIndex()
{
	return demuxer_.get_video_index();
}

int AVDemuxHandler::GetAudioIndex()
{
	return demuxer_.get_audio_index();
}

uint8_t* AVDemuxHandler::GetSpsData()
{
	unique_lock<mutex> lock(mtx_);
	return demuxer_.GetSpsData();
}

uint8_t* AVDemuxHandler::GetPpsData()
{
	unique_lock<mutex> lock(mtx_);
	return demuxer_.GetPpsData();
}

int AVDemuxHandler::GetSpsSize()
{
	unique_lock<mutex> lock(mtx_);
	return demuxer_.GetSpsSize();
}

int AVDemuxHandler::GetPpsSize()
{
	unique_lock<mutex> lock(mtx_);
	return demuxer_.GetPpsSize();
}

int AVDemuxHandler::CopyCodecExtraData(uint8_t* buffer, int& size)
{
	return demuxer_.GetCodecExtraData(buffer, size);
}

int64_t AVDemuxHandler::DurationScale(int64_t duration,AVRational src_timebase)
{
	AVRational dst = { 1,1000 };
	return av_rescale_q(duration, src_timebase, dst);
}