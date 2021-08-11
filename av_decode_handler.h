#ifndef AV_DECODE_HANDLER_H
#define AV_DECODE_HANDLER_H


#include "iav_base_handler.h"
#include "av_decoder.h"

struct AVFrame;
struct AVCodecParameters;
class AVDecodeHandler : public IAVBaseHandler
{
public:
	int Open(AVCodecParameters* param);
	virtual void Handle(AVHandlerPackage* pkg) override;
	void GetPlayFrame(AVFrame* frame);
protected:
	virtual void Loop() override;
	void CreateFrame();
protected:

private:
	bool is_need_play_ = false;
	AVFrame* play_frame_ = nullptr;
	AVFrame* decoded_frame_ = nullptr;
	AVDecoder decoder_;
	AVPacketDataList pkt_list_;
	AVHandlerPackageAVType av_type_;
};

#endif