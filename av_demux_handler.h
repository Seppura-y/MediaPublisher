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

	int GetVideoIndex();
	int GetAudioIndex();

	int CopyCodecExtraData(uint8_t* buffer, int& size);
	uint8_t* GetSpsData();
	uint8_t* GetPpsData();
	int GetSpsSize();
	int GetPpsSize();
protected:
	virtual void Loop() override;
	virtual void Handle(AVHandlerPackage* pkt) override;

	int64_t DurationScale(int64_t duration,AVRational src_timebase);
private:
	AVDemuxer demuxer_;
	std::string url_;

	int video_index_ = -1;
	int audio_index_ = -1;

	bool is_local_file_ = false;
	bool is_first_packet_ = true;
	int64_t start_time_ = -1;
	int64_t total_duration_ = 0;
};

#endif