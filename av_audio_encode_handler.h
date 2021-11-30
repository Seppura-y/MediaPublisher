#pragma once
#include "iav_base_handler.h"
#include "av_encoder.h"
#include "av_audio_resampler.h"

class AVAudioEncodeHandler : public IAVBaseHandler
{
public:
	//int AudioEncodeHandlerInit(AVCodecParameters* param, int channels, int channel_layouts, AVRational* src_timebase,AVRational* sample_rate);
	int AudioEncodeHandlerInit(AVCodecParameters* param, int channels, int sample_fmt, int nb_samples, int64_t channel_layouts);
	int AudioEncodeHandlerInit(std::shared_ptr<AVParametersWarpper> param);
	virtual void Handle(AVHandlerPackage* pkg) override;
	virtual void Stop()override;
	void SetEncodePause(bool status);
	std::shared_ptr<AVParamWarpper> CopyCodecParameters();
	std::shared_ptr<AVParametersWarpper> CopyCodecParameter();

	int CopyCodecExtraData(uint8_t* buffer, int& size);

protected:
	virtual void Loop() override;
private:
	bool is_pause_ = true;
	//bool cache_avaliable_ = false;
	int encoded_count_ = 0;
	AVEncoder encoder_;
	AVFrameDataList frame_list_;
	//AVCachedPacketDataList cache_pkt_list_;

	AVAudioResampler audio_resampler_;

	int nb_samples_ = -1;
	bool is_need_resample_ = false;
	std::shared_ptr<AVFrame> resampled_frame_ = nullptr;

};

