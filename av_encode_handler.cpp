#include "av_encode_handler.h"
#include <iostream>
#include <Windows.h>

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

using namespace std;

static int GetCpuNumber()
{
	SYSTEM_INFO info;
	GetSystemInfo(&info);
	return info.dwNumberOfProcessors;
}

int AVEncodeHandler::EncoderInit(int out_width, int out_height,AVRational* src_timebase)
{
	unique_lock<mutex> lock(mtx_);
	AVCodecContext* codec_ctx = encoder_.CreateContext(AV_CODEC_ID_H264, false);
	if (codec_ctx == nullptr)
	{
		cout << "encode handler create context failed" << endl;
		return -1;
	}

	codec_ctx->codec_type = AVMEDIA_TYPE_VIDEO;
	codec_ctx->pix_fmt = AV_PIX_FMT_YUV420P;
	codec_ctx->width = out_width;
	codec_ctx->height = out_height;

	codec_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
	codec_ctx->time_base = { 1,25 };
	codec_ctx->framerate = { 25,1 };
	codec_ctx->thread_count = GetCpuNumber();

	codec_ctx->max_b_frames = 0;
	codec_ctx->gop_size = 25;
	codec_ctx->bit_rate = 500 * 1024;

	encoder_.SetCodecContext(codec_ctx);

	int isok = encoder_.SetOption("preset", "ultrafast");
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

	isok = encoder_.OpenContext();
	if (isok != 0)
	{
		LOGERROR("encode_.OpenContext failed");
		return -1;
	}

	if (src_timebase)
	{
		if (!media_src_timebase_)
		{
			media_src_timebase_ = new AVRational();
		}
		media_src_timebase_->num = src_timebase->num;
		media_src_timebase_->den = src_timebase->den;
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
			//av_frame_free(&frame);
			av_frame_unref(frame);
			this_thread::sleep_for(1ms);
			continue;
		}
		if(!frame)
		{
			this_thread::sleep_for(1ms);
			continue;
		}
		if (frame->pict_type == AV_PICTURE_TYPE_B)
		{
			frame->pict_type = AV_PICTURE_TYPE_I;
		}
		if (media_src_timebase_)
		{
			if (frame->pts)
			{
				int64_t scaled_pts = ScaleToMsec(frame->pts, *media_src_timebase_);
				frame->pts = scaled_pts;
			}
			if (frame->pkt_pts)
			{
				int64_t scaled_pkt_pts = ScaleToMsec(frame->pkt_pts, *media_src_timebase_);
				frame->pkt_pts = scaled_pkt_pts;
			}
			if (frame->pkt_dts)
			{
				int64_t scaled_dts = ScaleToMsec(frame->pkt_dts, *media_src_timebase_);
				frame->pkt_dts = scaled_dts;
			}
			if (frame->pkt_duration)
			{
				int64_t scaled_duration = ScaleToMsec(frame->pkt_duration, *media_src_timebase_);
				frame->pkt_duration = scaled_duration;
			}
		}
		else
		{
			frame->pts = (encoded_count_++) * (encoder_.get_codec_ctx()->time_base.den) / (encoder_.get_codec_ctx()->time_base.num);
			frame->pkt_pts = frame->pts;
			frame->pkt_dts = frame->pts;
		}

		ret = encoder_.Send(frame);

		int64_t du = frame->pkt_duration;
		//av_frame_free(&frame);
		av_frame_unref(frame);
		if (ret != 0)
		{
			//cout << "encode handler : send frame failed " << endl;
			//this_thread::sleep_for(1ms);
			continue;
		}

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
		if (pkt->duration == 0)
		{
			pkt->duration = du;
		}
		pkt->duration;

		cache_pkt_list_.Push(pkt);
		av_packet_unref(pkt);

		if (cache_pkt_list_.Size() >= 100)
		{
			start_push_ = true;
		}
		if (start_push_)
		{
			if (is_callback_enable_)
			{
				AVPacket* pkt = cache_pkt_list_.Pop();
				if (callable_object_)
				{
					callable_object_(pkt);
					av_packet_unref(pkt);
				}
				else
				{
					av_packet_unref(pkt);
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
				}
				else
				{
					av_packet_unref(pkt);
					continue;
				}
			}
		}
		this_thread::sleep_for(1ms);
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