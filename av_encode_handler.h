#pragma once
#include "iav_base_handler.h"
#include "av_encoder.h"
#include "av_video_scaler.h"
class AVEncodeHandler : public IAVBaseHandler
{
public:
	int EncodeHandlerInit(AVCodecParameters* param,int out_width, int out_height,AVRational* src_timebase,AVRational* src_frame_rate);
	int EncodeHandlerInit(std::shared_ptr<AVParametersWarpper> para);
	virtual void Handle(AVHandlerPackage* pkg) override;
	virtual void Stop()override;
	void SetEncodePause(bool status);
	std::shared_ptr<AVParamWarpper> CopyCodecParameters();
	std::shared_ptr<AVParametersWarpper> CopyCodecParameter();

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

	bool is_need_scale_ = false;
	int output_width_ = -1;
	int output_height_ = -1;
	AVFrame* scaled_frame_ = nullptr;
	AVVideoScaler video_scaler_;

	AVRational* media_src_timebase_ = nullptr;
	AVRational* video_src_frame_rate_ = nullptr;
};

