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

	ret = decoder_.OpenContext(true);
	if (ret != 0)
	{
		cout << "decoder_.SetCodecContext(codec_ctx) failed" << endl;
		return -1;
	}
}

void AVDecodeHandler::Handle(AVHandlerPackage* pkg)
{
	if (!pkg)
	{
		cout << "pkg is null" << endl;
		return;
	}
	if (pkg->av_type_ == AVHandlerPackageAVType::AVHANDLER_PACKAGE_AV_TYPE_VIDEO &&
		pkg->type_ == AVHandlerPackageType::AVHANDLER_PACKAGE_TYPE_PACKET &&
		pkg->payload_.packet_ == nullptr)
	{
		cout << "AVHandlerPackage payload is null" << endl;
		return;
	}
	else
	{
		pkt_list_.Push(pkg->payload_.packet_);
	}
	av_packet_unref(pkg->payload_.packet_);
	return;
}

void AVDecodeHandler::Loop()
{
	AVHandlerPackage package;
	int ret = -1;
	if (!decoded_frame_)
	{
		decoded_frame_ = av_frame_alloc();
	}
	while (!is_exit_)
	{
		AVPacket* pkt = pkt_list_.Pop();

		if (pkt == nullptr) 
		{
			//this_thread::sleep_for(1ms);
			continue;
		}
		int64_t pts = pkt->pts;
		int64_t dts = pkt->dts;
		int64_t duration = pkt->duration;
		ret = decoder_.Send(pkt);
		//this_thread::sleep_for(1ms);
		av_packet_unref(pkt);

		while (1)
		{
			ret = decoder_.Recv(decoded_frame_);
			if (ret != 0)
			{
				av_frame_unref(decoded_frame_);
				//this_thread::sleep_for(1ms);
				//continue;
				break;
			}
			decoded_frame_->pts;
			decoded_frame_->pkt_pts;
			decoded_frame_->pkt_dts;
			decoded_frame_->pkt_duration;

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
				package.payload_.frame_ = decoded_frame_;
				package.type_ = AVHandlerPackageType::AVHANDLER_PACKAGE_TYPE_FRAME;
				next->Handle(&package);
			}
			av_frame_unref(decoded_frame_);
		}

	}

	if (decoded_frame_)
	{
		unique_lock<mutex> lock(mtx_);
		av_frame_free(&decoded_frame_);
	}
	if (play_frame_)
	{
		av_frame_free(&play_frame_);
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

AVFrame* AVDecodeHandler::GetPlayFrame()
{
	unique_lock<mutex> lock(mtx_);
	if (play_frame_ && is_need_play_)
	{
		//AVFrame* frame = av_frame_alloc();
		//av_frame_move_ref(frame, play_frame_);
		//return frame;
		return play_frame_;
	}
	return nullptr;
}

//void AVDecodeHandler::GetPlayFrame(AVFrame* frame)
//{
//	unique_lock<mutex> lock(mtx_);
//	if (play_frame_ != nullptr && is_need_play_)
//	{
//		int ret = av_frame_ref(frame, play_frame_);//decodec_frame_    -------->  ref count = 2
//		if (ret != 0)
//		{
//			cout << "GetPlayFrame failed" << endl;
//		}
//
//		av_frame_unref(play_frame_);//decodec_frame_    -------->  ref count = 1
//	}
//}


uint8_t* AVDecodeHandler::GetSpsData()
{
	unique_lock<mutex> lock(mtx_);
	return decoder_.GetSpsData();
}

uint8_t* AVDecodeHandler::GetPpsData()
{
	unique_lock<mutex> lock(mtx_);
	return decoder_.GetPpsData();
}

int AVDecodeHandler::GetSpsSize()
{
	unique_lock<mutex> lock(mtx_);
	return decoder_.GetSpsSize();
}

int AVDecodeHandler::GetPpsSize()
{
	unique_lock<mutex> lock(mtx_);
	return decoder_.GetPpsSize();
}

void AVDecodeHandler::SetNeedPlay(bool status)
{
	unique_lock<mutex> lock(mtx_);
	is_need_play_ = status;
}