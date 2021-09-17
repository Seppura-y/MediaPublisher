#include "av_demux_handler.h"
#include "avtimebase.h"

#include <iostream>

extern"C"
{
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/time.h>
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
	time_out_ = timeout;
	video_index_ = demuxer_.get_video_index();
	audio_index_ = demuxer_.get_audio_index();
	if (strstr(url, "mp4") || strstr(url,"flv"))
	{
		is_local_file_ = true;
		return true;
	}
	demuxer_.SetTimeout(timeout);

	last_pts_ = -1;
	last_dts_ = -1;
	next_pts_ = -1;
	next_dts_ = -1;
	is_first_packet_ = true;
	return true;
}


void AVDemuxHandler::Loop()
{
	AVPacket* demux_pkt = av_packet_alloc();

	while (!is_exit_)
	{
		if (start_time_ >= 0)
		{
			int64_t now_in_tb_q = av_gettime_relative() - start_time_;
			if (next_dts_ > now_in_tb_q)
			{
				continue;
			}
		}
		if (demuxer_.Read(demux_pkt) >= 0)
		{
			if (is_local_file_)
			{
				if (demux_pkt->data && (demux_pkt->size > 0) && (demux_pkt->stream_index == video_index_))
				{
					unique_lock<mutex> lock(mtx_);
					if (is_first_packet_)
					{
						start_time_ = av_gettime_relative();
						is_first_packet_ = false;
					}
					AVRational src_rational;
					src_rational.den = demuxer_.GetVideoTimebase()->den;
					src_rational.num = demuxer_.GetVideoTimebase()->num;
					//int64_t duration = ScaleToMsec(demux_pkt->duration, src_rational);
					//SleepForMsec(40);

					last_dts_ = av_rescale_q(demux_pkt->dts, src_rational, av_get_time_base_q());
					last_pts_ = av_rescale_q(demux_pkt->pts, src_rational, av_get_time_base_q());

					int64_t duration = av_rescale_q(demux_pkt->duration, src_rational, av_get_time_base_q());
					next_pts_ = last_pts_ + duration;
					next_dts_ = last_dts_ + duration;
				}
				else if (demux_pkt->data && (demux_pkt->size > 0) && (demux_pkt->stream_index == audio_index_))
				{
					av_packet_unref(demux_pkt);
					continue;
				}
				else
				{
					av_packet_unref(demux_pkt);
					continue;
				}
			}

			demux_pkt->pts;
			demux_pkt->dts;
			demux_pkt->duration;
			
			//if (is_callback_enable_)
			//{
			//	callable_object_(demux_pkt);
			//}
			if (GetNextHandler() && demux_pkt->stream_index == video_index_)
			{
				AVHandlerPackage payload;
				payload.type_ = AVHandlerPackageType::AVHANDLER_PACKAGE_TYPE_PACKET;
				payload.av_type_ = AVHandlerPackageAVType::AVHANDLER_PACKAGE_AV_TYPE_VIDEO;
				payload.payload_.packet_ = demux_pkt;
				if (GetNextHandler())
				{
					GetNextHandler()->Handle(&payload);
				}
				av_packet_unref(demux_pkt);
			}
		}
		else if (demuxer_.is_end_of_file())
		{
			if (is_cycling())
			{
				demuxer_.SeekToBeginning();
				
				is_first_packet_ = true;
				start_time_ = -1;
			}
			else
			{
				is_exit_ = true;
			}
		}

		if (!demuxer_.is_network_connected())
		{
			demuxer_.OpenContext(url_.c_str());
		}
	}
	av_packet_free(&demux_pkt);

	last_pts_ = -1;
	last_dts_ = -1;
	next_pts_ = -1;
	next_dts_ = -1;
	is_first_packet_ = true;
}

void AVDemuxHandler::Set_Restart_Callback(std::function<void(void)> cb)
{
	unique_lock<mutex> lock(mtx_);
	restart_callback_ = cb;
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
	demuxer_.SetFormatContext(nullptr);
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

int64_t AVDemuxHandler::ScaleToMsec(int64_t duration,AVRational src_timebase)
{
	AVRational dst = { 1,1000 };
	return av_rescale_q(duration, src_timebase, dst);
}

AVRational* AVDemuxHandler::GetVideoSrcTimebase()
{
	return demuxer_.GetVideoTimebase();
}

AVRational* AVDemuxHandler::GetAudioSrcTimebase()
{
	return demuxer_.GetAudioTimebase();
}

AVRational* AVDemuxHandler::GetVideoSrcFrameRate()
{
	return demuxer_.GetVideoFrameRate();
}