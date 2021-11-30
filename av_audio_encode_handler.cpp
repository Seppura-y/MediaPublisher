#include "av_audio_encode_handler.h"
#include "av_audio_resampler.h"

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

//int AVAudioEncodeHandler::AudioEncodeHandlerInit(AVCodecParameters* param, int out_width, int out_height, AVRational* src_timebase, AVRational* src_frame_rate)
//{
//	return 0;
//}

int AVAudioEncodeHandler::AudioEncodeHandlerInit(AVCodecParameters* param, int sample_rate, int sample_fmt, int nb_samples, int64_t channel_layout)
{
	lock_guard<mutex> lock(mtx_);
	nb_samples_ = nb_samples;
	int isok = 0;
	AVCodecContext* codec_ctx = encoder_.CreateContext(AV_CODEC_ID_AAC, false);
	if (codec_ctx == nullptr)
	{
		cout << "encode handler create context failed" << endl;
		return -1;
	}
	param->channel_layout;
	param->sample_rate;
	param->format;

	if (param)
	{
		if (param->channel_layout != channel_layout || param->sample_rate != sample_rate || param->format != sample_fmt)
		{
			is_need_resample_ = true;
			AudioResamplerParams resampler_params;
			resampler_params.src_sample_rate = param->sample_rate;
			resampler_params.src_sample_fmt = (AVSampleFormat)param->format;
			resampler_params.src_channel_layout = param->channel_layout;

			resampler_params.dst_sample_rate = sample_rate;
			resampler_params.dst_sample_fmt = (AVSampleFormat)sample_fmt;
			resampler_params.dst_channel_layout = channel_layout;

			//int ret = audio_resampler_.AVAudioResamplerInit(resampler_params);
		}
		else
		{
			is_need_resample_ = false;
			//output_width_ = param->width;
			//output_height_ = param->height;
		}
	}

	codec_ctx->codec_type = AVMEDIA_TYPE_AUDIO;
	codec_ctx->sample_fmt = AV_SAMPLE_FMT_FLTP;
	codec_ctx->channel_layout = channel_layout;
	codec_ctx->sample_rate = sample_rate;
	codec_ctx->bit_rate = 20 * 1024 * 8;
	codec_ctx->thread_count = GetCpuNumber();

	codec_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;



	encoder_.SetCodecContext(codec_ctx);

	isok = encoder_.OpenContext(false);
	if (isok != 0)
	{
		LOGERROR("encode_.OpenContext failed");
		return -1;
	}
	return 0;
}

int AVAudioEncodeHandler::AudioEncodeHandlerInit(std::shared_ptr<AVParametersWarpper> param)
{
	lock_guard<mutex> lock(mtx_);
	nb_samples_ = param->dst_nb_samples_;
	int isok = 0;
	AVCodecContext* codec_ctx = encoder_.CreateContext(AV_CODEC_ID_AAC, false);
	if (codec_ctx == nullptr)
	{
		cout << "encode handler create context failed" << endl;
		return -1;
	}
	param->para->channel_layout;
	param->para->sample_rate;
	param->para->format;

	if (param->dst_channel_layout_  && param->dst_sample_rate_ && param->dst_sample_fmt_)
	{
		if (param->para)
		{
			if (param->para->channel_layout != param->dst_channel_layout_ || param->para->sample_rate != param->dst_sample_rate_ || param->para->format != param->dst_sample_fmt_)
			{
				is_need_resample_ = true;
				AudioResamplerParams resampler_params;
				resampler_params.src_sample_rate = param->para->sample_rate; ;
				resampler_params.src_sample_fmt = (AVSampleFormat)param->para->format;
				resampler_params.src_channel_layout = param->para->channel_layout;

				resampler_params.dst_sample_rate = param->dst_sample_rate_;
				resampler_params.dst_sample_fmt = (AVSampleFormat)param->dst_sample_fmt_;
				resampler_params.dst_channel_layout = param->dst_channel_layout_;

				int ret = audio_resampler_.AVAudioResamplerInit(resampler_params);
			}
			else
			{
				is_need_resample_ = false;
			}
		}
	}
	else
	{
		is_need_resample_ = false;
	}

	codec_ctx->codec_type = AVMEDIA_TYPE_AUDIO;
	codec_ctx->sample_fmt = (AVSampleFormat)param->dst_sample_fmt_;
	codec_ctx->channel_layout = param->dst_channel_layout_;
	codec_ctx->sample_rate = param->dst_sample_rate_;
	//codec_ctx->bit_rate = 20 * 1024 * 8;
	codec_ctx->thread_count = GetCpuNumber();

	codec_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

	encoder_.SetCodecContext(codec_ctx);
	isok = encoder_.OpenContext(false);
	if (isok != 0)
	{
		LOGERROR("encode_.OpenContext failed");
		return -1;
	}
	return 0;
}

void AVAudioEncodeHandler::Handle(AVHandlerPackage* pkg)
{
	if (!pkg)
	{
		cout << "pkg is null" << endl;
		return;
	}
	if (pkg->av_type_ == AVHandlerPackageAVType::AVHANDLER_PACKAGE_AV_TYPE_VIDEO &&
		pkg->type_ == AVHandlerPackageType::AVHANDLER_PACKAGE_TYPE_FRAME)
	{
		cout << "AudioEncodeHandler handle wrong payload type" << endl;
		if (pkg->payload_.frame_)
		{
			av_frame_unref(pkg->payload_.frame_);
		}
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

void AVAudioEncodeHandler::SetEncodePause(bool status)
{
	unique_lock<mutex> lock(mtx_);
	is_pause_ = status;
}

void AVAudioEncodeHandler::Loop()
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
		if (!frame)
		{
			this_thread::sleep_for(1ms);
			continue;
		}

		//int64_t du = frame->pkt_duration;

		if (is_need_resample_)
		{
			//audio_resampler_.SendAudioFrame(frame);

			//resampled_frame_ = audio_resampler_.ReceiveResampledFrame(nb_samples_);
			if (!resampled_frame_.get())
			{
				continue;
			}

			ret = encoder_.Send(resampled_frame_.get());
			av_frame_unref(frame);
			av_frame_free(&frame);
			if (ret != 0)
			{
				//cout << "encode handler : send frame failed " << endl;
				//this_thread::sleep_for(1ms);
				continue;
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
				continue;
			}
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
		//pkt->pts;
		//pkt->dts;
		//pkt->duration;
		//if (pkt->duration == 0)
		//{
		//	pkt->duration = du;
		//}

		if (is_video_callback_enabled_)
		{
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
	frame_list_.Clear();
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

int AVAudioEncodeHandler::CopyCodecExtraData(uint8_t* buffer, int& size)
{
	return encoder_.GetCodecExtraData(buffer, size);
}

shared_ptr<AVParamWarpper> AVAudioEncodeHandler::CopyCodecParameters()
{
	return encoder_.CopyCodecParam();
}

shared_ptr<AVParametersWarpper> AVAudioEncodeHandler::CopyCodecParameter()
{
	return encoder_.CopyCodecParameters();
}

//uint8_t* AVAudioEncodeHandler::GetSpsData()
//{
//	unique_lock<mutex> lock(mtx_);
//	return encoder_.GetSpsData();
//}
//
//uint8_t* AVAudioEncodeHandler::GetPpsData()
//{
//	unique_lock<mutex> lock(mtx_);
//	return encoder_.GetPpsData();
//}
//
//int AVAudioEncodeHandler::GetSpsSize()
//{
//	unique_lock<mutex> lock(mtx_);
//	return encoder_.GetSpsSize();
//}
//
//int AVAudioEncodeHandler::GetPpsSize()
//{
//	unique_lock<mutex> lock(mtx_);
//	return encoder_.GetPpsSize();
//}

void AVAudioEncodeHandler::Stop()
{
	{
		unique_lock<mutex> lock(mtx_);
		if (resampled_frame_)
		{
			resampled_frame_.reset();
		}
	}
	IAVBaseHandler::Stop();
}
