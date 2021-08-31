#include "av_mux_handler.h"

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
static long long NowMs()
{
	return (clock() / (CLOCKS_PER_SEC / 1000));
}

using namespace std;

int AVMuxHandler::MuxerInit(std::string url, AVCodecParameters* v_param, AVRational* v_timebase, AVCodecParameters* a_param, AVRational* a_timebase, uint8_t* extra_data, int extra_data_size)
{
	if (!url.c_str())
	{
		cout << "Muxer init failed : url error" << endl;
		return -1;
	}

	if (!a_param && !v_param && !a_timebase && !v_timebase)
	{
		cout << "Muxer init failed : a_timebase and v_timebase are null" << endl;
		return -1;
	}

	unique_lock<mutex> lock(mtx_);
	if (strstr(url.c_str(), "rtsp"))
	{
		protocol_type_ = AVProtocolType::AV_PROTOCOL_TYPE_RTSP;
	}
	else if (strstr(url.c_str(), "rtmp"))
	{
		protocol_type_ = AVProtocolType::AV_PROTOCOL_TYPE_RTMP;
	}
	else
	{
		protocol_type_ = AVProtocolType::AV_PROTOCOL_TYPE_FILE;
	}

	url_ = url;
	has_audio_ = false;
	has_video_ = false;

	if (!audio_param_)
	{
		audio_param_ = AVParamWarpper::Create();
	}
	if (!video_param_)
	{
		video_param_ = AVParamWarpper::Create();
	}

	if (!a_param || !a_timebase)
	{
		has_audio_ = false;
		audio_param_->para = nullptr;
		audio_param_->time_base = nullptr;
	}
	else if (a_param && a_timebase)
	{
		has_audio_ = true;
		audio_param_->para = a_param;
		audio_param_->time_base = a_timebase;
	}

	if (!v_param || !v_timebase)
	{
		has_video_ = false;
		video_param_->para = nullptr;
		video_param_->time_base = nullptr;
	}
	else if (v_param && v_timebase)
	{
		has_video_ = true;
		video_param_->para = v_param;
		video_param_->time_base = v_timebase;
	}
	if (extradata_)
	{
		av_free(extradata_);
		extradata_ = nullptr;
	}
	extradata_size_ = extra_data_size;
	extradata_ = (uint8_t*)av_malloc(extradata_size_);
	memcpy(extradata_, extra_data, extradata_size_);
}

int AVMuxHandler::Open()
{
	unique_lock<mutex> lock(mtx_);

	AVFormatContext* fmt_ctx = muxer_.OpenContext(url_.c_str(), video_param_->para, audio_param_->para, protocol_type_);
	if (!fmt_ctx)
	{
		cout << "mux handler open failed : open context return null context" << endl;
		return -1;
	}
	int video_stream_index = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
	if (video_stream_index < 0)
	{
		cout << "mux handler open failed : video stream not found " << endl;
		return -1;
	}
	memcpy(fmt_ctx->streams[video_stream_index]->codecpar->extradata, extradata_, extradata_size_);
	fmt_ctx->streams[video_stream_index]->codecpar->extradata_size = extradata_size_;
	av_dump_format(fmt_ctx, video_stream_index, url_.c_str(), 1);

	muxer_.SetFormatContext(fmt_ctx);
	muxer_.SetProtocolType(protocol_type_);


	if (has_audio_)
	{
		muxer_.SetAudioTimebase(audio_param_->time_base);
	}
	if (has_video_)
	{
		muxer_.SetVideoTimebase(video_param_->time_base);
	}

	muxer_.SetTimeout(1000);
	return 0;
}


int AVMuxHandler::Open(std::string url, AVCodecParameters* v_param, AVRational* v_timebase, AVCodecParameters* a_param, AVRational* a_timebase, uint8_t* extra_data, int extra_data_size)
{
	if (!url.c_str())
	{
		cout << "Muxer handler Open failed : url error" << endl;
		return -1;
	}
	AVProtocolType protocol_type;
	if (strstr(url.c_str(), "rtsp"))
	{
		protocol_type = AVProtocolType::AV_PROTOCOL_TYPE_RTSP;
	}
	else if (strstr(url.c_str(), "rtmp"))
	{
		protocol_type = AVProtocolType::AV_PROTOCOL_TYPE_RTMP;
	}
	else
	{
		protocol_type = AVProtocolType::AV_PROTOCOL_TYPE_FILE;
	}

	unique_lock<mutex> lock(mtx_);

	url_ = url;
	if (!audio_param_)
	{
		audio_param_ = AVParamWarpper::Create();
	}

	if (!video_param_)
	{
		video_param_ = AVParamWarpper::Create();
	}

	audio_param_->para = a_param;
	video_param_->para = v_param;

	AVFormatContext* fmt_ctx = muxer_.OpenContext(url.c_str(), video_param_->para, audio_param_->para, protocol_type);
	if (!fmt_ctx)
	{
		cout << "mux handler open failed : open context return null context" << endl;
		return -1;
	}
	int video_stream_index = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
	if (video_stream_index < 0)
	{
		cout << "mux handler open failed : video stream not found " << endl;
		return -1;
	}
	memcpy(fmt_ctx->streams[video_stream_index]->codecpar->extradata, extra_data, extra_data_size);
	fmt_ctx->streams[video_stream_index]->codecpar->extradata_size = extra_data_size;
	av_dump_format(fmt_ctx, video_stream_index, url_.c_str(), 1);

	muxer_.SetFormatContext(fmt_ctx);
	muxer_.SetProtocolType(protocol_type);

	if (!a_timebase && !v_timebase)
	{
		cout << "mux handler open failed : a_timebase and v_timebase are null" << endl;
		return -1;
	}
	else if (a_timebase && !v_timebase)
	{
		audio_param_->time_base = a_timebase;
		muxer_.SetAudioTimebase(audio_param_->time_base);
	}
	else if (!a_timebase && v_timebase)
	{
		video_param_->time_base = v_timebase;
		muxer_.SetVideoTimebase(video_param_->time_base);
	}
	else
	{
		audio_param_->time_base = a_timebase;
		video_param_->time_base = v_timebase;
		muxer_.SetAudioTimebase(audio_param_->time_base);
		muxer_.SetVideoTimebase(video_param_->time_base);
	}
	return 0;
}

void AVMuxHandler::Handle(AVHandlerPackage* pkg)
{
	if (pkg->type_ == AVHandlerPackageType::AVHANDLER_PACKAGE_TYPE_FRAME)
	{
		cout << "mux handler handle failed : type error(frame)" << endl;
	}
	this->pkt_list_.Push(pkg->payload_.packet_);
}

void AVMuxHandler::Loop()
{
	AVPacket* pkt = nullptr;
	muxer_.WriteHeader();
	while (!is_exit_)
	{
		unique_lock<mutex> lock(mtx_);
		pkt = pkt_list_.Pop();
		if (!pkt || !pkt->data /*|| pkt->size ==0*/)
		{
			av_packet_free(&pkt);
			//this_thread::sleep_for(1ms);
			continue;
		}

		int diff = NowMs() - last_proc_time_;
		cout << "mux ms : " << diff << " " << endl;
		last_proc_time_ = NowMs();

		//cout << "w" << pkt->size <<" " << flush;
		muxer_.WriteData(pkt);
		av_packet_free(&pkt);

		this_thread::sleep_for(1ms);

		if (!muxer_.is_network_connected())
		{
			cout << "time out" << endl;
			int ret = Open();
		}
	}

	muxer_.WriteTrailer();
	muxer_.SetFormatContext(nullptr);
	pkt_list_.Clear();
}