#pragma once
#include "iav_base_handler.h"
#include "av_muxer.h"
class AVMuxHandler : public IAVBaseHandler
{
public:
	int Open(std::string url,AVCodecParameters* v_param,AVRational* v_timebase,AVCodecParameters* a_param,AVRational* a_timebase,uint8_t* extra_data,int extra_data_size);
	virtual void Handle(AVHandlerPackage* pkg) override;
protected:
	virtual void Loop() override;
private:
	std::string url_{ "" };
	AVMuxer muxer_;
	AVParamWarpper* audio_param_{ nullptr };
	AVParamWarpper* video_param_{ nullptr };
	AVPacketDataList pkt_list_;

};

