#ifndef AV_DEMUX_HANDLER_H
#define AV_DEMUX_HANDLER_H
#include "iav_base_handler.h"
#include "av_demuxer.h"
class AVDemuxHandler : public IAVBaseHandler
{
public:
	bool OpenAVSource(const char* url,int timeout = 1000);
	virtual void Stop() override;

	std::shared_ptr<AVParamWarpper> CopyVideoParameters();
	std::shared_ptr<AVParamWarpper> CopyAudioParameters();
protected:
	virtual void Loop() override;
	virtual void Handle(AVHandlerPackage* pkt) override;
private:
	AVDemuxer demuxer_;
	std::string url_;
};

#endif