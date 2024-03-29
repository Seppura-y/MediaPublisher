#include "av_decode_handler.h"
#include <iostream>

extern"C"
{
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
}

#ifdef _WIN32
#pragma comment (lib,"avcodec.lib")
#pragma comment (lib,"avutil.lib")

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


int AVDecodeHandler::Open(AVCodecParameters* param)
{
	AVCodecContext* codec_ctx = decoder_.CreateContext(param->codec_id, true);
	if (codec_ctx == nullptr)
	{
		return -1;
	}

	int ret = avcodec_parameters_to_context(codec_ctx, param);
	if (ret < 0)
	{
		cout << "avcodec_parameters_to_context failed : " << endl;
		PrintError(ret);
		avcodec_free_context(&codec_ctx);
		return -1;
	}

	decoder_.SetCodecContext(codec_ctx);

	ret = decoder_.OpenContext();
	if (ret != 0)
	{
		cout << "decoder_.SetCodecContext(codec_ctx) failed" << endl;
		return -1;
	}
}

void AVDecodeHandler::Handle(AVHandlerPackage* pkg)
{
	av_type_ = pkg->av_type_;
	pkt_list_.Push(pkg->payload_.packet_);
}

void AVDecodeHandler::Loop()
{
	AVHandlerPackage package;
	int ret = -1;
	CreateFrame();
	while (!is_exit_)
	{
		AVPacket* pkt = pkt_list_.Pop();
		if (pkt == nullptr)
		{
			this_thread::sleep_for(1ms);
			continue;
		}

		do
		{
			ret = decoder_.Send(pkt);
			this_thread::sleep_for(1ms);
		} while (ret == 0);
		av_packet_unref(pkt);

		ret = decoder_.Recv(decoded_frame_);
		if (ret != 0)
		{
			this_thread::sleep_for(1ms);
			continue;
		}

		if (is_need_play_)
		{
			if (play_frame_ == nullptr)
			{
				play_frame_ = av_frame_alloc();
			}
			av_frame_ref(play_frame_, decoded_frame_);
		}

		IAVBaseHandler* next = GetNextHandler();
		if (next)
		{
			package.av_type_ = this->av_type_;
			package.payload_.frame_ = decoded_frame_;//decodec_frame_ ------> will be unref by ffmpeg before receiving new frame from the decoder
			package.type_ = AVHandlerPackageType::AVHANDLER_PACKAGE_TYPE_FRAME;
			next->Handle(&package);
		}
	}

	if (decoded_frame_)
	{
		unique_lock<mutex> lock(mtx_);
		av_frame_free(&decoded_frame_);
	}
}

void AVDecodeHandler::CreateFrame()
{
	unique_lock<mutex> lock(mtx_);
	if (!decoded_frame_)
	{
		decoded_frame_ = av_frame_alloc();
	}
}

void AVDecodeHandler::GetPlayFrame(AVFrame* frame)
{
	unique_lock<mutex> lock(mtx_);
	if (play_frame_ != nullptr || is_need_play_)
	{
		int ret = av_frame_ref(frame, play_frame_);//decodec_frame_    -------->  ref count = 2
		if (ret != 0)
		{
			cout << "GetPlayFrame failed" << endl;
		}

		av_frame_unref(play_frame_);//decodec_frame_    -------->  ref count = 1
	}
}