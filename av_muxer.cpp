#include "av_muxer.h"
#include <iostream>
extern"C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
}

#ifdef _WIN32
#pragma comment(lib,"avformat.lib")
#pragma comment(lib,"avcodec.lib")
#pragma comment(lib,"avutil.lib")
#endif

using namespace std;

static void PrintError(int err)
{
	char buffer[1024] = { 0 };
	av_strerror(err, buffer, sizeof(buffer));
	cout << buffer << endl;
}

#define PRINT_ERR_P(err) if(err != 0) {PrintError(err);return nullptr;}
#define PRINT_ERR_I(err) if(err != 0) {PrintError(err);return -1;}


AVFormatContext* AVMuxer::OpenContext(const char* url, AVCodecParameters* vparam, AVCodecParameters* aparam, AVProtocolType type)
{
	if (!url || (!vparam && !aparam))
	{
		cout << "(!url || (!vparam && !aparam))" << endl;
		return nullptr;
	}
	AVFormatContext* fmt_ctx = nullptr;
	int ret = -1;
	switch (type)
	{
		case(AVProtocolType::AV_PROTOCOL_TYPE_FILE):
		{
			ret = avformat_alloc_output_context2(&fmt_ctx, nullptr, nullptr, url);
			break;
		}
		case(AVProtocolType::AV_PROTOCOL_TYPE_RTMP):
		{
			ret = avformat_alloc_output_context2(&fmt_ctx, nullptr, "flv", url);
			break;
		}
		case(AVProtocolType::AV_PROTOCOL_TYPE_RTSP):
		{
			cout << "rtsp is not supported yet" << endl;
			break;
		}
		default:
		{
			cout << "muxer type not found" << endl;
			break;
		}
	}

	if (ret < 0)
	{
		PrintError(ret);
		return nullptr;
	}

	if (aparam)
	{
		AVStream* a_stream = avformat_new_stream(fmt_ctx, nullptr);
		if (!a_stream)
		{
			cout << "audio : avformat_new_stream error" << endl;
			avformat_free_context(fmt_ctx);
			return nullptr;
		}
		a_stream->codecpar->codec_tag = 0;
		a_stream->codecpar->codec_type = AVMEDIA_TYPE_AUDIO;
		ret = avcodec_parameters_copy(a_stream->codecpar, aparam);
		if (ret < 0)
		{
			cout << "audio : avcodec_paramters_copy failed" << endl;
			PrintError(ret);
			avformat_free_context(fmt_ctx);
			return nullptr;
		}
	}

	if (vparam)
	{
		AVStream* v_stream = avformat_new_stream(fmt_ctx, nullptr);
		if (!v_stream)
		{
			cout << "video : avformat_new_stream failed" << endl;
			PrintError(ret);
			avformat_free_context(fmt_ctx);
			return nullptr;
		}
		v_stream->codecpar->codec_tag = 0;
		v_stream->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
		ret = avcodec_parameters_copy(v_stream->codecpar, vparam);
		if (ret < 0)
		{
			cout << "video : avcodec_parameters_copy failed" << endl;
			PrintError(ret);
			avformat_free_context(fmt_ctx);
			return nullptr;
		}
	}

	ret = avio_open(&fmt_ctx->pb, url, AVIO_FLAG_WRITE);
	if (ret < 0)
	{
		cout << "avio_open failed " << endl;
		PrintError(ret);
		avformat_free_context(fmt_ctx);
		return nullptr;
	}

	return fmt_ctx;
}

int AVMuxer::WriteHeader()
{
	unique_lock<mutex> lock(mtx_);
	if (!fmt_ctx_)
	{
		cout << "writeheader failed : fmt_ctx_ is null" << endl;
		return -1;
	}
	int ret = avformat_write_header(fmt_ctx_, nullptr);
	if (ret < 0)
	{
		cout << "avformat_write_header failed" << endl;
		PrintError(ret);
		return -1;
	}
	last_proc_time_ = GetCurrentTimeMsec();
	return 0;
}

int AVMuxer::WriteTrailer()
{
	unique_lock<mutex> lock(mtx_);
	if (!fmt_ctx_)
	{
		cout << "write trailer failed : fmt_ctx_ is null" << endl;
		return -1;
	}
	int ret = av_write_trailer(fmt_ctx_);
	if (ret != 0)
	{
		cout << "av_write_tailer failed" << endl;
		PrintError(ret);
		return -1;
	}
	last_proc_time_ = GetCurrentTimeMsec();
	return 0;
}

int AVMuxer::WriteData(AVPacket* pkt)
{
	{
		unique_lock<mutex> lock(mtx_);
		if (!fmt_ctx_)
		{
			cout << "write data failed : fmt_ctx_ is null" << endl;
			return -1;
		}
	}
	if (pkt->pts == AV_NOPTS_VALUE)
	{
		pkt->pts = 0;
		pkt->dts = 0;
	}
	if (pkt->stream_index == get_audio_index())
	{
		if (a_begin_pts_ < 0)
		{
			a_begin_pts_ = pkt->pts;
		}
		TimeScale(get_audio_index(),pkt,*a_src_timebase_,a_begin_pts_);
	}

	if (pkt->stream_index == get_video_index())
	{
		if (v_begin_pts_ < 0)
		{
			v_begin_pts_ = pkt->pts;
		}
		TimeScale(get_video_index(), pkt, *v_src_timebase_, v_begin_pts_);
	}

	{
		unique_lock<mutex> lock(mtx_);
		int ret = -1;
		if (protocol_type_ == AVProtocolType::AV_PROTOCOL_TYPE_FILE)
		{
			ret = av_interleaved_write_frame(fmt_ctx_, pkt);
			if (ret != 0)
			{
				cout << "av_interleaved_write_frame faield" << endl;
				PrintError(ret);
				return -1;
			}
		}
		else
		{
			ret = av_write_frame(fmt_ctx_, pkt);
			if (ret != 0)
			{
				cout << "av_write_frame failed : ";
				PrintError(ret);
				return -1;
			}
		}
		last_proc_time_ = GetCurrentTimeMsec();
	}
	return 0;
}
//
//int AVMuxer::WriteData(AVPacket* pkt)
//{
//	{
//		if (!pkt)return -1;
//		unique_lock<mutex> lock(mtx_);
//		if (!fmt_ctx_) return -1;
//		if (pkt->pts == AV_NOPTS_VALUE)
//		{
//			pkt->pts = 0;
//			pkt->dts = 0;
//		}
//	}
//	if (pkt->stream_index == get_video_index())
//	{
//		if (v_begin_pts_ < 0)
//		{
//			v_begin_pts_ = pkt->pts;
//		}
//
//		AVRational time_base;
//		time_base.den = v_src_timebase_->den;
//		time_base.num = v_src_timebase_->num;
//
//		TimeScale(get_video_index(), pkt, time_base, v_begin_pts_);
//		//TimeScale(pkt, v_src_timebase_, vbegin_pts_);
//
//	}
//	if (pkt->stream_index == get_audio_index())
//	{
//		if (a_begin_pts_ < 0)
//		{
//			a_begin_pts_ = pkt->pts;
//		}
//		AVRational time_base;
//		time_base.den = a_src_timebase_->den;
//		time_base.num = a_src_timebase_->num;
//
//		TimeScale(get_audio_index(), pkt, time_base, a_begin_pts_);
//		//TimeScale(pkt, a_src_timebase_, abegin_pts_);
//
//	}
//
//	{
//		unique_lock<mutex> lock(mtx_);
//		int ret = av_interleaved_write_frame(fmt_ctx_, pkt);
//		if(ret != 0)PrintError(ret);
//	}
//
//	//last_read_time_ = NowMs();
//	//IERR(ret);
//
//	return 0;
//}



int AVMuxer::TimeScale(int index, AVPacket* pkt, AVRational src, long long pts)
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


//int AVMuxer::TimeScale(int index, AVPacket* pkt, AVRational src, long long pts)
//{
//	if (!pkt)
//	{
//		cout << "TimeScale failed : (!pkt)" << endl;
//		return -1;
//	}
//	{
//		unique_lock<mutex> lock(mtx_);
//		if (!fmt_ctx_) return -1;
//	}
//	if (index == get_video_index())
//	{
//		pkt->pts = av_rescale_q_rnd(pkt->pts - pts, src, fmt_ctx_->streams[get_video_index()]->time_base, (AVRounding)(AV_ROUND_INF | AV_ROUND_PASS_MINMAX));
//		pkt->dts = av_rescale_q_rnd(pkt->dts - pts, src, fmt_ctx_->streams[get_video_index()]->time_base, (AVRounding)(AV_ROUND_INF | AV_ROUND_PASS_MINMAX));
//		pkt->duration = av_rescale_q_rnd(pkt->duration, src, fmt_ctx_->streams[get_video_index()]->time_base, (AVRounding)(AV_ROUND_INF | AV_ROUND_PASS_MINMAX));
//	}
//	else if (index == get_audio_index())
//	{
//		pkt->pts = av_rescale_q_rnd(pkt->pts - pts, src, fmt_ctx_->streams[get_audio_index()]->time_base, (AVRounding)(AV_ROUND_INF | AV_ROUND_PASS_MINMAX));
//		pkt->dts = av_rescale_q_rnd(pkt->dts - pts, src, fmt_ctx_->streams[get_audio_index()]->time_base, (AVRounding)(AV_ROUND_INF | AV_ROUND_PASS_MINMAX));
//		pkt->duration = av_rescale_q_rnd(pkt->duration, src, fmt_ctx_->streams[get_audio_index()]->time_base, (AVRounding)(AV_ROUND_INF | AV_ROUND_PASS_MINMAX));
//	}
//	pkt->pos = -1;
//	return 0;
//}

int AVMuxer::SetAudioTimebase(AVRational* src)
{
	if (!src) return -1;
	unique_lock<mutex> lock(mtx_);
	if (!a_src_timebase_)
	{
		a_src_timebase_ = new AVRational();
	}
	*a_src_timebase_ = *src;
	return 0;
}

int AVMuxer::SetVideoTimebase(AVRational* src)
{
	if (!src) return -1;
	unique_lock<mutex> lock(mtx_);
	if (!v_src_timebase_)
	{
		v_src_timebase_ = new AVRational();
	}
	*v_src_timebase_ = *src;
	return 0;
}