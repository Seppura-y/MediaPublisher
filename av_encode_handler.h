#pragma once
#include "iav_base_handler.h"
#include "av_encoder.h"
class AVEncodeHandler : public IAVBaseHandler
{
public:
	int EncoderInit(int out_width, int out_height);
	virtual void Handle(AVHandlerPackage* pkg) override;
	void SetEncodePause(bool status);
	std::shared_ptr<AVParamWarpper> CopyCodecParameters();
	int CopyCodecExtraData(uint8_t* buffer, int& size);
protected:
	virtual void Loop() override;
private:
	bool is_pause_ = true;
	int encoded_count_ = 0;
	AVEncoder encoder_;
	AVFrameDataList frame_list_;
};

