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
	if (!audio_src_timebase_)
	{
		audio_src_timebase_ = new AVRational();
		//audio_timebase_->den
	}
	if (!video_src_timebase_)
	{
		video_src_timebase_ = new AVRational();
	}
	if (!video_src_frame_rate_)
	{
		video_src_frame_rate_ = new AVRational();
	}
}

AVFormatBase::~AVFormatBase()
{
	if (audio_src_timebase_)
	{
		delete audio_src_timebase_;
		audio_src_timebase_ = nullptr;
	}
	if (video_src_timebase_)
	{
		delete video_src_timebase_;
		video_src_timebase_ = nullptr;
	}
	if (video_src_frame_rate_)
	{
		delete video_src_frame_rate_;
		video_src_frame_rate_ = nullptr;
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
	last_proc_time_ = GetCurrentTimeMsec();

	if (timeout_threshold_ > 0 && is_timeout_proc_enabled_)
	{
		AVIOInterruptCB cb = { TimeoutCallback,this };
		fmt_ctx_->interrupt_callback = cb;
	}

	audio_index_ = -1;
	video_index_ = -1;
	for (int i = 0; i < fmt_ctx_->nb_streams; i++)
	{
		if (fmt_ctx_->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
		{
			audio_index_ = fmt_ctx_->streams[i]->index;
			audio_src_timebase_->den = fmt_ctx_->streams[i]->time_base.den;
			audio_src_timebase_->num = fmt_ctx_->streams[i]->time_base.num;
		}

		if (fmt_ctx_->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			video_index_ = i;
			video_src_timebase_->den = fmt_ctx_->streams[i]->time_base.den;
			video_src_timebase_->num = fmt_ctx_->streams[i]->time_base.num;

			video_src_frame_rate_->num = fmt_ctx_->streams[i]->avg_frame_rate.num;
			video_src_frame_rate_->den = fmt_ctx_->streams[i]->avg_frame_rate.den;
			//get sps pps data
			if (fmt_ctx_->streams[i]->codecpar->codec_id == AV_CODEC_ID_H264 && fmt_ctx_->streams[i]->codecpar->extradata && fmt_ctx_->streams[i]->codecpar->extradata_size > 0)
			{
				int data_size = fmt_ctx_->streams[i]->codecpar->extradata_size;
				uint8_t* data = fmt_ctx_->streams[i]->codecpar->extradata;
				uint8_t num_of_sps;
				uint8_t num_of_pps;
				uint8_t* sps;
				uint8_t* pps;
				uint8_t sps_index = -1;
				uint8_t pps_index = -1;
				sps_size_ = 0;
				pps_size_ = 0;
				for (int i = 1; i < data_size; i++)
				{
					if (data[ i ] == 0x67 && data[ i - 2 ] == 0x00)
					{
						num_of_sps = data[i - 3] & 0x1f;
						uint8_t tmp_size = data[ i - 1 ];
						sps = &data[ i ];
						sps_index = sps - data;
						do
						{
							sps_data_.append((const char*)sps,tmp_size);
							sps_size_ += tmp_size;
							sps = sps + tmp_size;
							tmp_size = *(sps + 2);
						} while (--num_of_sps);

						//num_of_pps = *(sps + 1);
						break;
					}
				}
				pps_index = sps_index + sps_size_;
				num_of_pps = data[pps_index];
				uint8_t tmp_size = data[pps_index + 2];
				pps = &data[pps_index + 3];
				do
				{
					pps_data_.append((const char*)pps, tmp_size);
					pps_size_ += tmp_size;
					pps = pps + tmp_size;
					tmp_size = *(pps + 2);
				} while (--num_of_pps);
			}
		}
	}

}

//void AVFormatBase::SetTimeout(int ms)
//{
//	unique_lock<mutex> lock(mtx_);
//	timeout_threshold_ = ms;
//	if (fmt_ctx_)
//	{
//		AVIOInterruptCB cb = { TimeoutCallback,this };
//		fmt_ctx_->interrupt_callback = cb;
//	}
//}

void AVFormatBase::SetTimeout(int ms,bool status)
{
	unique_lock<mutex> lock(mtx_);
	timeout_threshold_ = ms;
	is_timeout_proc_enabled_ = status;
}

void AVFormatBase::SetTimeoutEnable(bool status)
{
	unique_lock<mutex> lock(mtx_);
	is_timeout_proc_enabled_ = status;
}

int AVFormatBase::TimeoutCallback(void* opaque)
{
	AVFormatBase* fmt_ctx = (AVFormatBase*)opaque;
	if (fmt_ctx->IsTimeout())
	{
		cout << "timeout callback" << endl;
		return 1;
	}
	return 0;
}

bool AVFormatBase::IsTimeout()
{
	//unique_lock<mutex> lock(mtx_);
	int64_t tmp = GetCurrentTimeMsec();
	if (is_timeout_proc_enabled_ && (tmp - last_proc_time_ > timeout_threshold_))
	{
		is_network_connected_ = false;
		last_proc_time_ = GetCurrentTimeMsec();
		return true;
	}
	//is_network_connected_ = true;
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
			avio_closep(&fmt_ctx_->pb);
		}
		else if (fmt_ctx_->iformat)
		{
			avformat_close_input(&fmt_ctx_);
		}
		else
		{

		}
		avformat_free_context(fmt_ctx_);
		fmt_ctx_ = nullptr;
	}
	is_network_connected_ = false;
	return 0;
}

int AVFormatBase::CloseIOContext()
{
	unique_lock<mutex> lock(mtx_);
	if (fmt_ctx_)
	{
		if (fmt_ctx_->oformat)
		{
			avio_close(fmt_ctx_->pb);
		}
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
	//unique_lock<mutex> lock(mtx_);
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

void AVFormatBase::SetProtocolType(AVProtocolType type)
{
	unique_lock<mutex> lock(mtx_);
	protocol_type_ = type;
}

uint8_t* AVFormatBase::GetSpsData()
{
	return (uint8_t*)sps_data_.c_str();
}

uint8_t* AVFormatBase::GetPpsData()
{
	return (uint8_t*)pps_data_.c_str();
}

int AVFormatBase::GetSpsSize()
{
	return sps_size_;
}

int AVFormatBase::GetPpsSize()
{
	return pps_size_;
}

int AVFormatBase::GetCodecExtraData(uint8_t* buffer, int& size)
{
	unique_lock<mutex> lock(mtx_);
	if (!fmt_ctx_)
	{
		cout << "GetCodecExtraData failed : fmt_ctx_ is null" << endl;
		return -1;
	}
	if (fmt_ctx_->streams[video_index_]->codecpar->codec_id == AV_CODEC_ID_H264)
	{
		memcpy(buffer, fmt_ctx_->streams[video_index_]->codecpar->extradata, fmt_ctx_->streams[video_index_]->codecpar->extradata_size);
		size = fmt_ctx_->streams[video_index_]->codecpar->extradata_size;
	}
	else
	{
		return -1;
	}
	return 0;
}

AVRational* AVFormatBase::GetVideoTimebase()
{
	unique_lock<mutex> lock(mtx_);
	return video_src_timebase_;
}

AVRational* AVFormatBase::GetAudioTimebase()
{
	unique_lock<mutex> lock(mtx_);
	return audio_src_timebase_;
}
AVRational* AVFormatBase::GetVideoFrameRate()
{
	unique_lock<mutex> lock(mtx_);
	return video_src_frame_rate_;
}

int AVFormatBase::TimeScale(int index, AVPacket* pkt, AVRational src, long long pts)
{
	if (!pkt)
	{
		cout << "TimeScale failed : (!pkt)" << endl;
		return -1;
	}
	unique_lock<mutex> lock(mtx_);
	if (!fmt_ctx_)
	{
		cout << "TimeScale failed : (!fmt_ctx_)" << endl;
		return -1;
	}
	AVStream* stream = fmt_ctx_->streams[index];
	pkt->pts = av_rescale_q_rnd(pkt->pts - pts, src, stream->time_base, (AVRounding)(AV_ROUND_INF | AV_ROUND_PASS_MINMAX));
	pkt->dts = av_rescale_q_rnd(pkt->dts - pts, src, stream->time_base, (AVRounding)(AV_ROUND_INF | AV_ROUND_PASS_MINMAX));
	pkt->duration = av_rescale_q_rnd(pkt->duration, src, stream->time_base, (AVRounding)(AV_ROUND_INF | AV_ROUND_PASS_MINMAX));
	pkt->pos = -1;
	return 0;
}