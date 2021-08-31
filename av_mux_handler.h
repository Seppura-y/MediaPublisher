#pragma once
#include "iav_base_handler.h"
#include "av_muxer.h"
class AVMuxHandler : public IAVBaseHandler
{
public:
	int Open();
	int MuxerInit(std::string url, AVCodecParameters* v_param, AVRational* v_timebase, AVCodecParameters* a_param, AVRational* a_timebase, uint8_t* extra_data, int extra_data_size);
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

	uint8_t* extradata_ = nullptr;
	int extradata_size_ = -1;

	bool has_audio_ = false;
	bool has_video_ = false;
	AVProtocolType protocol_type_ = AVProtocolType::AV_PROTOCOL_TYPE_RTMP;
};

