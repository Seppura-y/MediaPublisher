#pragma once
#include "iav_base_handler.h"
#include "av_encoder.h"
class AVEncodeHandler : public IAVBaseHandler
{
public:
	int EncoderInit(int out_width, int out_height,AVRational* src_timebase,AVRational* src_frame_rate);
	virtual void Handle(AVHandlerPackage* pkg) override;
	void SetEncodePause(bool status);
	std::shared_ptr<AVParamWarpper> CopyCodecParameters();

	int CopyCodecExtraData(uint8_t* buffer, int& size);
	uint8_t* GetSpsData();
	uint8_t* GetPpsData();
	int GetSpsSize();
	int GetPpsSize();
protected:
	virtual void Loop() override;
private:
	bool is_pause_ = true;
	bool cache_avaliable_ = false;
	int encoded_count_ = 0;
	AVEncoder encoder_;
	AVFrameDataList frame_list_;
	AVCachedPacketDataList cache_pkt_list_;

	AVRational* media_src_timebase_ = nullptr;
	AVRational* video_src_frame_rate_ = nullptr;
};

