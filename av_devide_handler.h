#ifndef AV_DEVIDE_HANDLER_H
#define AV_DEVIDE_HANDLER_H

#include "iav_base_handler.h"
#include "av_data_tools.h"
class AVDevideHandler : public IAVBaseHandler
{
public :
	virtual void Handle(AVHandlerPackage* pkt) override;
protected:
	virtual void Loop() override;
private:
	IAVBaseHandler* audio_next_handler_ = nullptr;
	IAVBaseHandler* video_next_handler_ = nullptr;
	AVPacketDataList audio_pkt_list_;
	AVPacketDataList video_pkt_list_;
};

#endif