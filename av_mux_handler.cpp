#include "av_mux_handler.h"

#include <iostream>

extern"C"
{
#include <libavformat/avformat.h>
#include <libavutil/time.h>
#include <libavutil/avutil.h>
}

#ifdef _WIN32
#pragma comment (lib,"avformat.lib")
#pragma comment (lib,"avutil.lib")

#endif
//static long long NowMs()
//{
//	return (clock() / (CLOCKS_PER_SEC / 1000));
//}

using namespace std;

int AVMuxHandler::MuxerInit(std::string url, AVCodecParameters* v_param, AVRational* v_timebase, AVCodecParameters* a_param, AVRational* a_timebase, uint8_t* extra_data, int extra_data_size)
{
	//if (!url.c_str())
	//{
	//	cout << "Muxer init failed : url error" << endl;
	//	return -1;
	//}

	//if (!a_param && !v_param && !a_timebase && !v_timebase)
	//{
	//	cout << "Muxer init failed : a_timebase and v_timebase are null" << endl;
	//	return -1;
	//}

	//unique_lock<mutex> lock(mtx_);
	//if (strstr(url.c_str(), "rtsp"))
	//{
	//	protocol_type_ = AVProtocolType::AV_PROTOCOL_TYPE_RTSP;
	//}
	//else if (strstr(url.c_str(), "rtmp"))
	//{
	//	protocol_type_ = AVProtocolType::AV_PROTOCOL_TYPE_RTMP;
	//}
	//else
	//{
	//	protocol_type_ = AVProtocolType::AV_PROTOCOL_TYPE_FILE;
	//}

	//url_ = url;
	//has_audio_ = false;
	//has_video_ = false;

	//if (!audio_param_)
	//{
	//	audio_param_ = AVParamWarpper::Create();
	//}
	//if (!video_param_)
	//{
	//	video_param_ = AVParamWarpper::Create();
	//}

	//if (!a_param || !a_timebase)
	//{
	//	has_audio_ = false;
	//	audio_param_->para = nullptr;
	//	audio_param_->time_base = nullptr;
	//}
	//else if (a_param && a_timebase)
	//{
	//	has_audio_ = true;
	//	//audio_param_->para = a_param;
	//	avcodec_parameters_copy(audio_param_->para, a_param);
	//	audio_param_->time_base = new AVRational(*a_timebase);
	//}

	//if (!v_param || !v_timebase)
	//{
	//	has_video_ = false;
	//	video_param_->para = nullptr;
	//	video_param_->time_base = nullptr;
	//}
	//else if (v_param && v_timebase)
	//{
	//	has_video_ = true;
	//	//video_param_->para = v_param;
	//	avcodec_parameters_copy(video_param_->para, v_param);
	//	video_param_->time_base = new AVRational(*v_timebase);
	//}
	//if (extradata_)
	//{
	//	av_free(extradata_);
	//	extradata_ = nullptr;
	//}
	//extradata_size_ = extra_data_size;
	//extradata_ = (uint8_t*)av_malloc(extradata_size_);
	//memcpy(extradata_, extra_data, extradata_size_);

	//mux_info_.max_pts = INT64_MIN;
	//mux_info_.min_pts = INT64_MAX;
	return 0;
}

int AVMuxHandler::MuxerInit(std::string url, std::shared_ptr<AVParametersWarpper>v_para, std::shared_ptr<AVParametersWarpper>a_para, uint8_t* extra_data, int extra_data_size)
{

	if (!url.c_str())
	{
		cout << "Muxer init failed : url error" << endl;
		return -1;
	}

	if (!v_para->para && !a_para->para && !v_para->dst_time_base && !a_para->dst_time_base)
	{
		cout << "Muxer init failed : a_timebase and v_timebase are null" << endl;
		return -1;
	}

	lock_guard<mutex> lock(mtx_);
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

	if (!audio_para_)
	{
		audio_para_ = new AVParametersWarpper();
	}
	if (!video_para_)
	{
		video_para_ = new AVParametersWarpper();
	}

	if (a_para)
	{
		if (!a_para->para || !a_para->dst_time_base)
		{
			has_audio_ = false;
			audio_para_->para = nullptr;
			audio_para_->dst_time_base = nullptr;
		}
		else if (a_para->para && a_para->dst_time_base)
		{
			has_audio_ = true;
			avcodec_parameters_copy(audio_para_->para, a_para->para);
			audio_para_->dst_time_base = new AVRational(*a_para->dst_time_base);
		}
	}


	if (!v_para->para || !v_para->dst_time_base)
	{
		has_video_ = false;
		video_para_->para = nullptr;
		video_para_->dst_time_base = nullptr;
	}
	else if (v_para->para && v_para->dst_time_base)
	{
		has_video_ = true;
		avcodec_parameters_copy(video_para_->para, v_para->para);
		video_para_->dst_time_base = new AVRational(*v_para->dst_time_base);
	}
	if (extradata_)
	{
		av_free(extradata_);
		extradata_ = nullptr;
	}
	extradata_size_ = extra_data_size;
	extradata_ = (uint8_t*)av_malloc(extradata_size_);
	memcpy(extradata_, extra_data, extradata_size_);

	//mux_info_.max_pts = INT64_MIN;
	//mux_info_.min_pts = INT64_MAX;
	return 0;
}

int AVMuxHandler::Open()
{
	unique_lock<mutex> lock(mtx_);
	AVFormatContext* fmt_ctx = muxer_.OpenContext(url_.c_str(), video_para_->para, audio_para_->para, protocol_type_);
	if (!fmt_ctx)
	{
		cout << "mux handler open failed : open context return null context" << endl;
		return -1;
	}
	int video_stream_index = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
	if (video_stream_index >= 0)
	{
		mux_info_.vs_index = video_stream_index_ = video_stream_index;
	}

	int audio_stream_index = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
	if (audio_stream_index >= 0)
	{
		mux_info_.vs_index = video_stream_index_ = video_stream_index;
	}
	if (audio_stream_index < 0 && video_stream_index < 0)
	{
		cout << "audio_stream_index < 0 && video_stream_index < 0" << endl;
		return -1;
	}

	memcpy(fmt_ctx->streams[video_stream_index]->codecpar->extradata, extradata_, extradata_size_);
	fmt_ctx->streams[video_stream_index]->codecpar->extradata_size = extradata_size_;
	av_dump_format(fmt_ctx, video_stream_index, url_.c_str(), 1);

	muxer_.SetFormatContext(fmt_ctx);
	muxer_.SetProtocolType(protocol_type_);


	if (has_audio_)
	{
		muxer_.SetAudioTimebase(audio_para_->dst_time_base);
	}
	if (has_video_)
	{
		muxer_.SetVideoTimebase(video_para_->dst_time_base);
	}


	return 0;
}


//int AVMuxHandler::Open(std::string url, AVCodecParameters* v_param, AVRational* v_timebase, AVCodecParameters* a_param, AVRational* a_timebase, uint8_t* extra_data, int extra_data_size)
//{
//	if (!url.c_str())
//	{
//		cout << "Muxer handler Open failed : url error" << endl;
//		return -1;
//	}
//	AVProtocolType protocol_type;
//	if (strstr(url.c_str(), "rtsp"))
//	{
//		protocol_type = AVProtocolType::AV_PROTOCOL_TYPE_RTSP;
//	}
//	else if (strstr(url.c_str(), "rtmp"))
//	{
//		protocol_type = AVProtocolType::AV_PROTOCOL_TYPE_RTMP;
//	}
//	else
//	{
//		protocol_type = AVProtocolType::AV_PROTOCOL_TYPE_FILE;
//	}
//
//	unique_lock<mutex> lock(mtx_);
//
//	url_ = url;
//	if (!audio_param_)
//	{
//		audio_param_ = AVParamWarpper::Create();
//	}
//
//	if (!video_param_)
//	{
//		video_param_ = AVParamWarpper::Create();
//	}
//
//	audio_param_->para = a_param;
//	video_param_->para = v_param;
//
//	AVFormatContext* fmt_ctx = muxer_.OpenContext(url.c_str(), video_param_->para, audio_param_->para, protocol_type);
//	if (!fmt_ctx)
//	{
//		cout << "mux handler open failed : open context return null context" << endl;
//		return -1;
//	}
//	int video_stream_index = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
//	if (video_stream_index < 0)
//	{
//		cout << "mux handler open failed : video stream not found " << endl;
//		return -1;
//	}
//	memcpy(fmt_ctx->streams[video_stream_index]->codecpar->extradata, extra_data, extra_data_size);
//	fmt_ctx->streams[video_stream_index]->codecpar->extradata_size = extra_data_size;
//	av_dump_format(fmt_ctx, video_stream_index, url_.c_str(), 1);
//
//	muxer_.SetFormatContext(fmt_ctx);
//	muxer_.SetProtocolType(protocol_type);
//	//muxer_.SetTimeout(1000, true);
//
//	if (!a_timebase && !v_timebase)
//	{
//		cout << "mux handler open failed : a_timebase and v_timebase are null" << endl;
//		return -1;
//	}
//	else if (a_timebase && !v_timebase)
//	{
//		audio_param_->time_base = a_timebase;
//		muxer_.SetAudioTimebase(audio_param_->time_base);
//	}
//	else if (!a_timebase && v_timebase)
//	{
//		video_param_->time_base = v_timebase;
//		muxer_.SetVideoTimebase(video_param_->time_base);
//	}
//	else
//	{
//		audio_param_->time_base = a_timebase;
//		video_param_->time_base = v_timebase;
//		muxer_.SetAudioTimebase(audio_param_->time_base);
//		muxer_.SetVideoTimebase(video_param_->time_base);
//	}
//	muxer_.ResetAudioBeginPts();
//	muxer_.ResetVideoBeginPts();
//	return 0;
//}

void AVMuxHandler::Handle(AVHandlerPackage* pkg)
{
	if (pkg->type_ == AVHandlerPackageType::AVHANDLER_PACKAGE_TYPE_FRAME)
	{
		cout << "mux handler handle failed : type error(frame)" << endl;
	}
	unique_lock<mutex> lock(mtx_);
	if ((video_stream_index_ == -1) && pkg->av_type_ == AVHandlerPackageAVType::AVHANDLER_PACKAGE_AV_TYPE_VIDEO)
	{
		video_stream_index_ = pkg->payload_.packet_->stream_index;
	}
	else if ((audio_stream_index_ == -1) && pkg->av_type_ == AVHandlerPackageAVType::AVHANDLER_PACKAGE_AV_TYPE_AUDIO)
	{
		audio_stream_index_ = pkg->payload_.packet_->stream_index;
	}
	this->pkt_list_.Push(pkg->payload_.packet_);
}

void AVMuxHandler::Loop()
{
	AVPacket* pkt = nullptr;
	muxer_.ResetAudioBeginPts();
	muxer_.ResetVideoBeginPts();
	muxer_.WriteHeader();
	AVRational video_src_timebase = *(video_para_->dst_time_base);
	AVRational audio_src_timebase = *(audio_para_->dst_time_base);
	while (!is_exit_)
	{
		if (!muxer_.is_network_connected())
		{
			muxer_.SetFormatContext(nullptr);
			int ret = Open();
			if (ret != 0)
			{
				continue;
			}
			muxer_.WriteHeader();
		}
		pkt = pkt_list_.Pop();
		if (!pkt || !pkt->data /*|| pkt->size ==0*/)
		{
			av_packet_free(&pkt);
			continue;
		}

		int64_t cur_dts_ = -1;

		if (pkt->stream_index == video_stream_index_)
		{
			if (mux_info_.vs_nb_packets == 1)
			{
				//side data
			}
			av_rescale_q(pkt->dts, av_get_time_base_q(), video_src_timebase);
			av_rescale_q(pkt->pts, av_get_time_base_q(), video_src_timebase);
			av_rescale_q(pkt->duration, av_get_time_base_q(), video_src_timebase);
		}
		else if (pkt->stream_index == audio_stream_index_)
		{
			av_rescale_q(pkt->dts, av_get_time_base_q(),audio_src_timebase);
			av_rescale_q(pkt->pts, av_get_time_base_q(), audio_src_timebase);
			av_rescale_q(pkt->duration, av_get_time_base_q(), audio_src_timebase);
		}

		int ret = muxer_.WriteData(pkt);
		av_packet_free(&pkt);
	}

	muxer_.WriteTrailer();
	muxer_.CloseContext();
	muxer_.SetFormatContext(nullptr);
	pkt_list_.Clear();
}

//AVMuxHandler::Loop()的循环推流时间戳叠加版本，具体实现移至demux handler
//void AVMuxHandler::Loop()
//{
//	AVPacket* pkt = nullptr;
//	muxer_.ResetAudioBeginPts();
//	muxer_.ResetVideoBeginPts();
//	muxer_.WriteHeader();
//	AVRational video_src_timebase = *(video_para_->dst_time_base);
//	AVRational audio_src_timebase = *(audio_para_->dst_time_base);
//	while (!is_exit_)
//	{
//		if (!muxer_.is_network_connected())
//		{
//			//cout << "time out" << endl;
//			//muxer_.CloseContext();
//			muxer_.SetFormatContext(nullptr);
//			int ret = Open();
//			if (ret != 0)
//			{
//				continue;
//			}
//			muxer_.WriteHeader();
//		}
//		//unique_lock<mutex> lock(mtx_);
//		pkt = pkt_list_.Pop();
//		if (!pkt || !pkt->data /*|| pkt->size ==0*/)
//		{
//			av_packet_free(&pkt);
//			continue;
//		}
//
//		int64_t cur_dts_ = -1;
//		int64_t duration = av_rescale_q(mux_info_.duration, *mux_info_.time_base, video_src_timebase);
//		if (pkt->pts != AV_NOPTS_VALUE)
//		{
//			pkt->pts += duration;
//		}
//		if (pkt->dts != AV_NOPTS_VALUE)
//		{
//			pkt->dts += duration;
//		}
//
//		if (pkt->stream_index == video_stream_index_)
//		{
//			if (mux_info_.vs_nb_packets == 1)
//			{
//				//side data
//			}
//			mux_info_.vs_nb_packets++;
//			int64_t pkt_duration = av_rescale_q(pkt->duration, video_src_timebase, av_get_time_base_q());
//
//			mux_info_.vs_min_pts = FFMIN(mux_info_.vs_min_pts, pkt->pts);
//			mux_info_.vs_max_pts = FFMAX(mux_info_.vs_max_pts, pkt->pts);
//		}
//		else if (pkt->stream_index == audio_stream_index_)
//		{
//			av_rescale_q(pkt->dts, audio_src_timebase, av_get_time_base_q());
//		}
//
//		if (is_cycling() && cur_dts_ <= mux_info_.last_dts)
//		{
//			//muxer_.WriteTrailer();
//			//muxer_.CloseContext();
//			//muxer_.SetFormatContext(nullptr);
//			//Open();
//			mux_info_.ts_delta += mux_info_.duration;
//			//muxer_.WriteHeader();
//		}
//
//
//
//		pkt->dts += av_rescale_q(mux_info_.ts_delta, av_get_time_base_q(), video_src_timebase);
//		pkt->pts += av_rescale_q(mux_info_.ts_delta, av_get_time_base_q(), video_src_timebase);
//
//		int diff = GetCurrentTimeMsec() - last_proc_time_;
//
//		int ret = muxer_.WriteData(pkt);
//		if (ret == 0)
//		{
//			last_proc_time_ = GetCurrentTimeMsec();
//		}
//
//		av_packet_free(&pkt);
//	}
//
//	muxer_.WriteTrailer();
//	muxer_.CloseContext();
//	muxer_.SetFormatContext(nullptr);
//	pkt_list_.Clear();
//}


//单独视频流
//void AVMuxHandler::Loop()
//{
//	AVPacket* pkt = nullptr;
//	muxer_.ResetAudioBeginPts();
//	muxer_.ResetVideoBeginPts();
//	muxer_.WriteHeader();
//	AVRational src_rational = *(video_param_->time_base);
//	AVRational audio_src_rational = *(audio_param_->time_base);
//	while (!is_exit_)
//	{
//		if (!muxer_.is_network_connected())
//		{
//			//cout << "time out" << endl;
//			//muxer_.CloseContext();
//			muxer_.SetFormatContext(nullptr);
//			int ret = Open();
//			if (ret != 0)
//			{
//				continue;
//			}
//			muxer_.WriteHeader();
//		}
//		//unique_lock<mutex> lock(mtx_);
//		pkt = pkt_list_.Pop();
//		if (!pkt || !pkt->data /*|| pkt->size ==0*/)
//		{
//			av_packet_free(&pkt);
//			continue;
//		}
//
//		int64_t cur_dts_ = -1; 
//		if (pkt->stream_index == video_stream_index_)
//		{
//			av_rescale_q(pkt->dts, src_rational, av_get_time_base_q());
//		}
//		else if (pkt->stream_index == audio_stream_index_)
//		{
//
//		}
//		
//		if (is_cycling() && cur_dts_ <= mux_info_.last_dts)
//		{
//			//muxer_.WriteTrailer();
//			//muxer_.CloseContext();
//			//muxer_.SetFormatContext(nullptr);
//			//Open();
//			mux_info_.ts_delta += mux_info_.duraion;
//			//muxer_.WriteHeader();
//		}
//
//		mux_info_.last_dts = av_rescale_q(pkt->dts, src_rational, av_get_time_base_q());
//		mux_info_.last_pts = av_rescale_q(pkt->pts, src_rational, av_get_time_base_q());
//
//		int64_t pkt_duration = av_rescale_q(pkt->duration, src_rational, av_get_time_base_q());
//		mux_info_.next_pts = mux_info_.last_pts + pkt_duration;
//		mux_info_.next_dts = mux_info_.last_dts + pkt_duration;
//
//		mux_info_.duraion += pkt_duration;
//
//		pkt->dts += av_rescale_q(mux_info_.ts_delta, av_get_time_base_q(), src_rational);
//		pkt->pts += av_rescale_q(mux_info_.ts_delta, av_get_time_base_q(), src_rational);
//
//		int diff = GetCurrentTimeMsec() - last_proc_time_;
//
//		int ret = muxer_.WriteData(pkt);
//		if (ret == 0)
//		{
//			last_proc_time_ = GetCurrentTimeMsec();
//		}
//
//		av_packet_free(&pkt);
//	}
//
//	muxer_.WriteTrailer();
//	muxer_.CloseContext();
//	muxer_.SetFormatContext(nullptr);
//	pkt_list_.Clear();
//}

//void AVMuxHandler::Loop()
//{
//	AVPacket* pkt = nullptr;
//	muxer_.ResetAudioBeginPts();
//	muxer_.ResetVideoBeginPts();
//	muxer_.WriteHeader();
//	AVRational src_rational = *(video_param_->time_base);
//	while (!is_exit_)
//	{
//		if (!muxer_.is_network_connected())
//		{
//			//cout << "time out" << endl;
//			//muxer_.CloseContext();
//			muxer_.SetFormatContext(nullptr);
//			int ret = Open();
//			if (ret != 0)
//			{
//				continue;
//			}
//			muxer_.WriteHeader();
//		}
//		//unique_lock<mutex> lock(mtx_);
//		pkt = pkt_list_.Pop();
//		if (!pkt || !pkt->data /*|| pkt->size ==0*/)
//		{
//			av_packet_free(&pkt);
//			continue;
//		}
//
//		if (is_first_packet_)
//		{
//			start_time_ = av_rescale_q(pkt->dts, src_rational, av_get_time_base_q());
//			is_first_packet_ = false;
//		}
//
//		int64_t cur_dts_ = av_rescale_q(pkt->dts, src_rational, av_get_time_base_q());
//		if (is_cycling() && cur_dts_ <= mux_info_.last_dts)
//		{
//			//muxer_.WriteTrailer();
//			//muxer_.CloseContext();
//			//muxer_.SetFormatContext(nullptr);
//			//Open();
//			mux_info_.ts_delta += mux_info_.duraion;
//			//muxer_.WriteHeader();
//		}
//
//		mux_info_.last_dts = av_rescale_q(pkt->dts, src_rational, av_get_time_base_q());
//		mux_info_.last_pts = av_rescale_q(pkt->pts, src_rational, av_get_time_base_q());
//
//		int64_t pkt_duration = av_rescale_q(pkt->duration, src_rational, av_get_time_base_q());
//		mux_info_.next_pts = mux_info_.last_pts + pkt_duration;
//		mux_info_.next_dts = mux_info_.last_dts + pkt_duration;
//
//		mux_info_.duraion += pkt_duration;
//
//		pkt->dts += av_rescale_q(mux_info_.ts_delta, av_get_time_base_q(), src_rational);
//		pkt->pts += av_rescale_q(mux_info_.ts_delta, av_get_time_base_q(), src_rational);
//
//		int diff = GetCurrentTimeMsec() - last_proc_time_;
//
//		int ret = muxer_.WriteData(pkt);
//		if (ret == 0)
//		{
//			last_proc_time_ = GetCurrentTimeMsec();
//		}
//
//		av_packet_free(&pkt);
//
//
//		
//	}
//
//	muxer_.WriteTrailer();
//	muxer_.CloseContext();
//	muxer_.SetFormatContext(nullptr);
//	pkt_list_.Clear();
//}