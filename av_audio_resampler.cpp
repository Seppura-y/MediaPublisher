#pragma once
#include "av_data_tools.h"
#include "av_audio_resampler.h"

#include <iostream>
extern"C"
{
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
#include <libavutil/avutil.h>
#include <libavutil/audio_fifo.h>
#include <libswresample/swresample.h>
}

#ifdef _WIN32
#pragma comment(lib,"avcodec.lib")
#pragma comment(lib,"avutil.lib")
#pragma comment(lib,"swresample.lib")
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

void foo()
{
	PrintError(0);
}

int AVAudioResampler::AVAudioResamplerInit(const AudioResamplerParams& param)
{
	is_init_ = false;

	resampler_params_ = param;

	src_channels_ = av_get_channel_layout_nb_channels(resampler_params_.src_channel_layout);
	dst_channels_ = av_get_channel_layout_nb_channels(resampler_params_.dst_channel_layout);

	audio_samples_fifo_ = av_audio_fifo_alloc(resampler_params_.dst_sample_fmt, dst_channels_, 1);
	if (!audio_samples_fifo_)
	{
		cout << "av_audio_fifo_alloc failed" << endl;
		return -1;
	}

	if (resampler_params_.dst_channel_layout == resampler_params_.src_channel_layout &&
		resampler_params_.dst_sample_fmt == resampler_params_.src_sample_fmt &&
		resampler_params_.dst_sample_rate == resampler_params_.src_sample_rate)
	{
		is_audio_fifo_only_ = true;
		return 0;
	}

	swr_ctx_ = swr_alloc();
	if (!swr_ctx_)
	{
		cout << "swr context alloc failed" << endl;
		return -1;
	}

	int ret = av_opt_set_int(swr_ctx_, "in_channel_layout", resampler_params_.src_channel_layout, 0);	if (ret != 0) { PRINT_ERR_I(ret); }
	ret = av_opt_set_int(swr_ctx_, "in_sample_rate", resampler_params_.src_sample_rate, 0); 	if (ret != 0) { PRINT_ERR_I(ret); }
	ret = av_opt_set_sample_fmt(swr_ctx_, "in_sample_fmt", resampler_params_.src_sample_fmt, 0);	if (ret != 0) { PRINT_ERR_I(ret); }

	ret = av_opt_set_int(swr_ctx_, "out_channel_layout", resampler_params_.dst_channel_layout, 0);	 if (ret != 0) { PRINT_ERR_I(ret); }
	ret = av_opt_set_int(swr_ctx_, "out_sample_rate", resampler_params_.dst_sample_rate, 0);	if (ret != 0) { PRINT_ERR_I(ret); }
	ret = av_opt_set_sample_fmt(swr_ctx_, "out_sample_fmt", resampler_params_.dst_sample_fmt, 0);	if (ret != 0) { PRINT_ERR_I(ret); }

	ret = swr_init(swr_ctx_);
	if (ret != 0)
	{
		PRINT_ERR_I(ret);
	}

	src_nb_samples_ = 1024;
	max_dst_nb_samples_ = dst_nb_samples_ = av_rescale_rnd(src_nb_samples_, resampler_params_.dst_sample_rate, resampler_params_.src_sample_rate, AV_ROUND_UP);

	is_init_ = true;
	return 0;
}

shared_ptr<AVFrame> AVAudioResampler::OutputFrameAlloc(const int nb_samples)
{
	shared_ptr<AVFrame> frame = shared_ptr<AVFrame>(av_frame_alloc(), [](AVFrame* frame) {if (frame) { av_frame_free(&frame); } });
	if (!frame)
	{
		cout << "av_frame_alloc failed" << endl;
		return {};
	}

	frame->format = resampler_params_.dst_sample_fmt;
	frame->nb_samples = nb_samples;
	frame->sample_rate = resampler_params_.dst_sample_rate;
	frame->channel_layout = resampler_params_.dst_channel_layout;
	int ret = av_frame_get_buffer(frame.get(), 0);
	if (ret != 0)
	{
		cout << "av_frame_get_buffer failed " << endl;
		return {};
	}
	return frame;
}

shared_ptr<AVFrame> AVAudioResampler::GetOneResampledFrame(const int desired_size)
{
	shared_ptr<AVFrame> frame = OutputFrameAlloc(desired_size);
	if (frame)
	{
		av_audio_fifo_read(audio_samples_fifo_, (void**)frame->data, desired_size);
		frame->pts = current_pts_;
		current_pts_ += desired_size;
		total_duration_ += desired_size;
	}
	return frame;
}

shared_ptr<AVFrame> AVAudioResampler::ReceiveResampledFrame(const int desired_size)
{
	lock_guard<mutex> lock(mtx_);
	int read_size = desired_size == 0 ? av_audio_fifo_size(audio_samples_fifo_) : desired_size;
	if (av_audio_fifo_size(audio_samples_fifo_) < desired_size || read_size == 0)
	{
		cout << "audio fifo lack data" << endl;
		return {};
	}
	return GetOneResampledFrame(read_size);
}

int AVAudioResampler::ReceiveResampledFrames(vector<shared_ptr<AVFrame>>& vec, int desired_size)
{
	int ret = 0;
	lock_guard<mutex> lock(mtx_);
	int read_size = desired_size == 0 ? av_audio_fifo_size(audio_samples_fifo_) : desired_size;
	while (1)
	{
		if (av_audio_fifo_size(audio_samples_fifo_) < read_size)
		{
			break;
		}

		auto frame = GetOneResampledFrame(desired_size);
		if (frame)
		{
			vec.push_back(frame);
		}
		else if (!frame)
		{
			ret = AVERROR(ENOMEM);
			break;
		}
	}
	return ret;
}

int AVAudioResampler::InitResampledBuffer()
{
	if (resampled_buffer_)
	{
		av_freep(resampled_buffer_[0]);
	}
	av_freep(resampled_buffer_);

	int ret = av_samples_alloc_array_and_samples(&resampled_buffer_, &dst_linesize_, dst_channels_, dst_nb_samples_, resampler_params_.dst_sample_fmt, 0);
	if (ret < 0)
	{
		cout << "av_samples_alloc_array_and_samples failed" << endl;
		return -1;
	}

	return 0;
}

int AVAudioResampler::SendRawPcmData(uint8_t* raw_pcm, int size)
{
	shared_ptr<AVFrame> frame = shared_ptr<AVFrame>(av_frame_alloc(), [](AVFrame* frame) {if (frame)av_frame_free(&frame); });
	if (!frame)
	{
		cout << "SendRawPcmData : av_frame_alloc failed" << endl;
		return -1;
	}
	frame->channel_layout = resampler_params_.src_channel_layout;
	frame->sample_rate = resampler_params_.src_sample_rate;
	int channels = av_get_channel_layout_nb_channels(resampler_params_.src_channel_layout);
	frame->nb_samples = size / av_get_bytes_per_sample(resampler_params_.src_sample_fmt) / channels;

	int ret = avcodec_fill_audio_frame(frame.get(), channels, resampler_params_.src_sample_fmt, raw_pcm, size, 0);

	ret = SendAudioFrame(frame.get());

	return ret;
}


int AVAudioResampler::SendAudioFrame(AVFrame* frame)
{
	lock_guard<mutex> lock(mtx_);
	int src_nb_samples = 0;
	uint8_t** src_data = nullptr;
	if (frame)
	{
		src_nb_samples = frame->nb_samples;
		src_data = frame->extended_data;
		if (start_pts_ == AV_NOPTS_VALUE && frame->pts != AV_NOPTS_VALUE)
		{
			start_pts_ = frame->pts;
			current_pts_ = frame->pts;
		}
	}
	else
	{
		is_flushed_ = true;
	}

	if (is_audio_fifo_only_) 
	{
		return src_data ? av_audio_fifo_write(audio_samples_fifo_, (void**)src_data, src_nb_samples) : 0;
	}
	int delay = swr_get_delay(swr_ctx_, resampler_params_.src_sample_rate);
	dst_nb_samples_ = av_rescale_rnd(delay + src_nb_samples,
														resampler_params_.dst_sample_rate,
														resampler_params_.src_sample_rate,
														AV_ROUND_UP);

	if (dst_nb_samples_ > max_dst_nb_samples_) 
	{
		av_freep(&resampled_buffer_[0]);
		int ret = av_samples_alloc(resampled_buffer_, &dst_linesize_, dst_channels_,
												dst_nb_samples_, resampler_params_.dst_sample_fmt, 1);
		if (ret < 0) 
		{
			cout << "av_samples_allco failed" << endl;
			return 0;
		}
		max_dst_nb_samples_ = dst_nb_samples_;
	}
	int nb_samples = swr_convert(swr_ctx_, resampled_buffer_, dst_nb_samples_,(const uint8_t**)src_data, src_nb_samples);

	int dst_bufsize = av_samples_get_buffer_size(&dst_linesize_, dst_channels_,nb_samples, resampler_params_.dst_sample_fmt, 1);
	// dump
	//
	static FILE* s_swr_fp = fopen("swr.pcm", "wb");
	fwrite(resampled_buffer_[0], 1, dst_bufsize, s_swr_fp);
	fflush(s_swr_fp);

	return av_audio_fifo_write(audio_samples_fifo_, (void**)resampled_buffer_, nb_samples);
}