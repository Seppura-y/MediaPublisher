#include "av_encode_handler.h"
#include <iostream>

extern"C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
}

#ifdef _WIN32
#pragma comment (lib,"avformat.lib")
#pragma comment (lib,"avcodec.lib")
#pragma comment (lib,"avutil.lib")
#endif

#ifdef _WIN32
#include <Windows.h>
#endif

static long long NowMs()
{
	return (clock() / (CLOCKS_PER_SEC / 1000));
}
using namespace std;

static void PrintError(int err)
{
	char buffer[1024] = { 0 };
	av_strerror(err, buffer, sizeof(buffer));
	cout << buffer << endl;
}

static int GetCpuNumber()
{
	SYSTEM_INFO info;
	GetSystemInfo(&info);
	return info.dwNumberOfProcessors;
}

int AVEncodeHandler::EncodeHandlerInit(AVCodecParameters* param,int out_width, int out_height,AVRational* src_timebase,AVRational* src_frame_rate)
{
	unique_lock<mutex> lock(mtx_);
	int isok = 0;
	AVCodecContext* codec_ctx = encoder_.CreateContext(AV_CODEC_ID_H264, false);
	if (codec_ctx == nullptr)
	{
		cout << "encode handler create context failed" << endl;
		return -1;
	}
	output_width_ = out_width;
	output_height_ = out_height;
	if (param)
	{
		if (param->width != out_width && param->height != out_height)
		{
			is_need_scale_ = true;
			video_scaler_.SetDimension(param->width, param->height, out_width, out_height);
			video_scaler_.InitScale(param->format, param->format);

			scaled_frame_ = av_frame_alloc();
			scaled_frame_->format = param->format;
			scaled_frame_->width = output_width_;
			scaled_frame_->height = output_height_;
			isok = av_frame_get_buffer(scaled_frame_, 0);
			if (isok != 0)
			{
				cout << "set scaled_frame failed : " << flush;
				PrintError(isok);
				return -1;
			}
		}
		else
		{
			is_need_scale_ = false;
			output_width_ = param->width;
			output_height_ = param->height;
		}
	}

	codec_ctx->codec_type = AVMEDIA_TYPE_VIDEO;
	codec_ctx->pix_fmt = AV_PIX_FMT_YUV420P;
	codec_ctx->width = output_width_;
	codec_ctx->height = output_height_;

	codec_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

	if (src_timebase)
	{
		if (!media_src_timebase_)
		{
			media_src_timebase_ = new AVRational();
		}
		media_src_timebase_->num = src_timebase->num;
		media_src_timebase_->den = src_timebase->den;
		codec_ctx->time_base = { media_src_timebase_->num,media_src_timebase_->den };
	}
	else
	{
		codec_ctx->time_base = av_get_time_base_q();
	}

	if (src_frame_rate)
	{
		if (!video_src_frame_rate_)
		{
			video_src_frame_rate_ = new AVRational();
		}
		video_src_frame_rate_->num = src_frame_rate->num;
		video_src_frame_rate_->den = src_frame_rate->den;
		codec_ctx->framerate = { video_src_frame_rate_->num,video_src_frame_rate_->den };
	}
	else
	{
		codec_ctx->framerate = { 25,1 };
	}
	
	codec_ctx->thread_count = GetCpuNumber();

	//AVCodecParameters par;
	codec_ctx->max_b_frames = 0;
	codec_ctx->gop_size = 50;
	codec_ctx->bit_rate = 20 * 1024 * 8;

	encoder_.SetCodecContext(codec_ctx);

	isok = encoder_.SetOption("preset", "ultrafast");
	if (isok != 0)
	{
		return -1;
	}
	isok = encoder_.SetOption("qp", 23);
	if (isok != 0)
	{
		return -1;
	}
	isok = encoder_.SetOption("tune", "zerolatency");
	if (isok != 0)
	{
		return -1;
	}

	isok = encoder_.SetOption("nal-hrd", "cbr");
	if (isok != 0)
	{
		return -1;
	}

	isok = encoder_.OpenContext(false);
	if (isok != 0)
	{
		LOGERROR("encode_.OpenContext failed");
		return -1;
	}
	return 0;
}
int AVEncodeHandler::EncodeHandlerInit(std::shared_ptr<AVParametersWarpper> para)
{
	unique_lock<mutex> lock(mtx_);
	int isok = 0;
	AVCodecContext* codec_ctx = encoder_.CreateContext(AV_CODEC_ID_H264, false);
	if (codec_ctx == nullptr)
	{
		cout << "encode handler create context failed" << endl;
		return -1;
	}
	output_width_ = para->dst_width_;
	output_height_ = para->dst_height_;
	if (para->para)
	{
		if (para->para->width != output_width_ && para->para->height != output_height_)
		{
			is_need_scale_ = true;
			video_scaler_.SetDimension(para->para->width, para->para->height, output_width_, output_height_);
			video_scaler_.InitScale(para->para->format, para->para->format);

			scaled_frame_ = av_frame_alloc();
			scaled_frame_->format = para->para->format;
			scaled_frame_->width = output_width_;
			scaled_frame_->height = output_height_;
			isok = av_frame_get_buffer(scaled_frame_, 0);
			if (isok != 0)
			{
				cout << "set scaled_frame failed : " << flush;
				PrintError(isok);
				return -1;
			}
		}
		else
		{
			is_need_scale_ = false;
			output_width_ = para->para->width;
			output_height_ = para->para->height;
		}
	}

	codec_ctx->codec_type = AVMEDIA_TYPE_VIDEO;
	codec_ctx->pix_fmt = AV_PIX_FMT_YUV420P;
	codec_ctx->width = output_width_;
	codec_ctx->height = output_height_;

	codec_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

	if (para->src_time_base)
	{
		if (!media_src_timebase_)
		{
			media_src_timebase_ = new AVRational();
		}
		media_src_timebase_->num = para->src_time_base->num;
		media_src_timebase_->den = para->src_time_base->den;
		codec_ctx->time_base = { media_src_timebase_->num,media_src_timebase_->den };
	}
	else
	{
		codec_ctx->time_base = av_get_time_base_q();
	}

	if (para->src_framerate)
	{
		if (!video_src_frame_rate_)
		{
			video_src_frame_rate_ = new AVRational();
		}
		video_src_frame_rate_->num = para->src_framerate->num;
		video_src_frame_rate_->den = para->src_framerate->den;
		codec_ctx->framerate = { video_src_frame_rate_->num,video_src_frame_rate_->den };
	}
	else
	{
		codec_ctx->framerate = { 25,1 };
	}

	codec_ctx->thread_count = GetCpuNumber();

	//AVCodecParameters par;
	codec_ctx->max_b_frames = 0;
	codec_ctx->gop_size = 50;
	codec_ctx->bit_rate = 20 * 1024 * 8;

	encoder_.SetCodecContext(codec_ctx);

	isok = encoder_.SetOption("preset", "ultrafast");
	if (isok != 0)
	{
		return -1;
	}
	isok = encoder_.SetOption("qp", 23);
	if (isok != 0)
	{
		return -1;
	}
	isok = encoder_.SetOption("tune", "zerolatency");
	if (isok != 0)
	{
		return -1;
	}

	isok = encoder_.SetOption("nal-hrd", "cbr");
	if (isok != 0)
	{
		return -1;
	}

	isok = encoder_.OpenContext(false);
	if (isok != 0)
	{
		LOGERROR("encode_.OpenContext failed");
		return -1;
	}
	return 0;
}

void AVEncodeHandler::Handle(AVHandlerPackage* pkg)
{
	if (!pkg)
	{
		cout << "pkg is null" << endl;
		return;
	}
	if (pkg->av_type_ == AVHandlerPackageAVType::AVHANDLER_PACKAGE_AV_TYPE_VIDEO &&
		pkg->type_ == AVHandlerPackageType::AVHANDLER_PACKAGE_TYPE_FRAME &&
		pkg->payload_.frame_ == nullptr)
	{
		cout << "AVHandlerPackage payload is null" << endl;
		return;
	}

	if (is_pause_)
	{
		//av_frame_free(&pkg->payload_.frame_);
	}
	else
	{
		frame_list_.Push(pkg->payload_.frame_);
	}
	av_frame_unref(pkg->payload_.frame_);
	return;
}

void AVEncodeHandler::SetEncodePause(bool status)
{
	unique_lock<mutex> lock(mtx_);
	is_pause_ = status;
}

void AVEncodeHandler::Loop()
{
	int ret = -1;
	AVPacket* pkt = av_packet_alloc();
	AVHandlerPackage* pkg = new AVHandlerPackage();
	while (!is_exit_)
	{
		if (is_pause_)
		{
			this_thread::sleep_for(1ms);
			continue;
		}

		AVFrame* frame = frame_list_.Pop();
		if (frame != nullptr && (frame->data[0] == nullptr || frame->linesize[0] == 0))
		{
			av_frame_unref(frame);
			av_frame_free(&frame);
			this_thread::sleep_for(1ms);
			continue;
		}
		if(!frame)
		{
			this_thread::sleep_for(1ms);
			continue;
		}

		int64_t du = frame->pkt_duration;
		frame->pict_type = AV_PICTURE_TYPE_NONE;

		if (is_need_scale_)
		{
			if (!scaled_frame_)
			{
				scaled_frame_ = av_frame_alloc();
				scaled_frame_->format = frame->format;
				scaled_frame_->width = output_width_;
				scaled_frame_->height = output_height_;
				ret = av_frame_get_buffer(scaled_frame_, 0);
				if (ret != 0)
				{
					PrintError(ret);
					continue;
				}
			}
			scaled_frame_->pts = frame->pts;
			scaled_frame_->pkt_pts = frame->pkt_pts;
			scaled_frame_->pkt_dts = frame->pkt_dts;
			scaled_frame_->pkt_duration = frame->pkt_duration;

			if (scaled_frame_->data[0] && scaled_frame_->linesize[0])
			{
				video_scaler_.FrameScale(frame, scaled_frame_);
				ret = encoder_.Send(scaled_frame_);
				av_frame_unref(frame);
				av_frame_free(&frame);
				av_frame_unref(scaled_frame_);
				av_frame_free(&scaled_frame_);
				if (ret != 0)
				{
					//cout << "encode handler : send frame failed " << endl;
					//this_thread::sleep_for(1ms);
					continue;
				}
			}
			else
			{
				continue;
			}

		}
		else
		{
			ret = encoder_.Send(frame);
			av_frame_unref(frame);
			av_frame_free(&frame);
			if (ret != 0)
			{
				//cout << "encode handler : send frame failed " << endl;
				//this_thread::sleep_for(1ms);
				continue;
			}
		}


		//if (media_src_timebase_)
		//{
		//	if (frame->pts)
		//	{
		//		int64_t scaled_pts = ScaleToMsec(frame->pts, *media_src_timebase_);
		//		frame->pts = scaled_pts;
		//	}
		//	if (frame->pkt_pts)
		//	{
		//		int64_t scaled_pkt_pts = ScaleToMsec(frame->pkt_pts, *media_src_timebase_);
		//		frame->pkt_pts = scaled_pkt_pts;
		//	}
		//	if (frame->pkt_dts)
		//	{
		//		int64_t scaled_dts = ScaleToMsec(frame->pkt_dts, *media_src_timebase_);
		//		frame->pkt_dts = scaled_dts;
		//	}
		//	if (frame->pkt_duration)
		//	{
		//		int64_t scaled_duration = ScaleToMsec(frame->pkt_duration, *media_src_timebase_);
		//		frame->pkt_duration = scaled_duration;
		//	}
		//}
		//else
		//{
		//	//frame->pts = (encoded_count_++) * (encoder_.get_codec_ctx()->time_base.den) / (encoder_.get_codec_ctx()->time_base.num);
		//	//frame->pkt_pts = frame->pts;
		//	//frame->pkt_dts = frame->pts;
		//}

		ret = encoder_.Recv(pkt);
		if (ret != 0)
		{
			cout << "encode handler : recv packet failed " << endl;
			av_packet_unref(pkt);
			this_thread::sleep_for(1ms);
			continue;
		}
		if (pkt->buf == nullptr || pkt->size == 0 || pkt->data == nullptr)
		{
			av_packet_unref(pkt);
			this_thread::sleep_for(1ms);
			continue;
		}
		pkt->pts;
		pkt->dts;
		pkt->duration;
		if (pkt->duration == 0)
		{
			pkt->duration = du;
		}
		cache_pkt_list_.Push(pkt);
		av_packet_unref(pkt);

		if (cache_pkt_list_.Size() >= 10)
		{
			cache_avaliable_ = true;
		}
		if (cache_avaliable_)
		{
			if (is_video_callback_enabled_)
			{
				AVPacket* pkt = cache_pkt_list_.Pop();
				if (video_callback_)
				{
					video_callback_(pkt);
					av_packet_unref(pkt);
					av_packet_free(&pkt);
				}
				else
				{
					av_packet_unref(pkt);
					av_packet_free(&pkt);
					continue;
				}
			}
			else
			{
				AVPacket* pkt = cache_pkt_list_.Pop();
				if (GetNextHandler())
				{
					pkg->av_type_ = AVHandlerPackageAVType::AVHANDLER_PACKAGE_AV_TYPE_VIDEO;
					pkg->type_ = AVHandlerPackageType::AVHANDLER_PACKAGE_TYPE_PACKET;
					pkg->payload_.packet_ = pkt;
					GetNextHandler()->Handle(pkg);
					av_packet_unref(pkt);
					av_packet_free(&pkt);
				}
				else
				{
					av_packet_unref(pkt);
					av_packet_free(&pkt);
					continue;
				}
			}
		}
	}
	frame_list_.Clear();
	cache_pkt_list_.Clear();
	encoder_.Send(nullptr);
	while (1)
	{
		encoder_.Recv(pkt);
		if (pkt->data)
		{
			av_packet_unref(pkt);
		}
		else
			break;

	}
	av_packet_free(&pkt);
	delete pkg;
}

int AVEncodeHandler::CopyCodecExtraData(uint8_t* buffer, int& size)
{
	return encoder_.GetCodecExtraData(buffer, size);
}

shared_ptr<AVParamWarpper> AVEncodeHandler::CopyCodecParameters()
{
	return encoder_.CopyCodecParam();
}

std::shared_ptr<AVParametersWarpper> AVEncodeHandler::CopyCodecParameter()
{
	return encoder_.CopyCodecParameters();
}

uint8_t* AVEncodeHandler::GetSpsData()
{
	unique_lock<mutex> lock(mtx_);
	return encoder_.GetSpsData();
}

uint8_t* AVEncodeHandler::GetPpsData()
{
	unique_lock<mutex> lock(mtx_);
	return encoder_.GetPpsData();
}

int AVEncodeHandler::GetSpsSize()
{
	unique_lock<mutex> lock(mtx_);
	return encoder_.GetSpsSize();
}

int AVEncodeHandler::GetPpsSize()
{
	unique_lock<mutex> lock(mtx_);
	return encoder_.GetPpsSize();
}

void AVEncodeHandler::Stop()
{
	{
		unique_lock<mutex> lock(mtx_);
		if (scaled_frame_)
		{
			av_frame_unref(scaled_frame_);
			av_frame_free(&scaled_frame_);
			scaled_frame_ = nullptr;
		}
	}
	IAVBaseHandler::Stop();
}