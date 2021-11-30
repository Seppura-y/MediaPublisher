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
	AVRational* GetVideoSrcTimebase();
	AVRational* GetAudioSrcTimebase();
	AVRational* GetVideoSrcFrameRate();

	int GetVideoIndex();
	int GetAudioIndex();

	IAVBaseHandler* GetNextVideoHandler();
	IAVBaseHandler* GetNextAudioHandler();

	int CopyCodecExtraData(uint8_t* buffer, int& size);
	uint8_t* GetSpsData();
	uint8_t* GetPpsData();
	int GetSpsSize();
	int GetPpsSize();

	bool is_end_of_file() { std::unique_lock<std::mutex> lock(mtx_); return is_end_of_file_; }
	void set_is_cyling(bool status) { std::unique_lock<std::mutex> lock(mtx_); is_cycling_ = status; }
	bool is_cycling() { std::unique_lock<std::mutex> lock(mtx_); return is_cycling_; }
protected:
	virtual void Loop() override;
	virtual void Handle(AVHandlerPackage* pkt) override;

	int64_t ScaleToMsec(int64_t duration,AVRational src_timebase);

	std::function<void(void)> restart_callback_ = nullptr;
private:
	AVDemuxer demuxer_;
	std::string url_;

	IAVBaseHandler* next_video_handler_ = nullptr;
	IAVBaseHandler* next_audio_handler_ = nullptr;

	int time_out_ = -1;
	int video_index_ = -1;
	int audio_index_ = -1;
	//AVStream** input_streams = nullptr;

	bool is_local_file_ = false;
	bool is_cycling_ = false;
	bool is_first_packet_ = true;
	bool is_end_of_file_ = false;
	int64_t start_time_ = -1;
	int64_t last_pts_ = -1;
	int64_t last_dts_ = -1;
	int64_t next_pts_ = -1;
	int64_t next_dts_ = -1;
	int64_t last_proc_time_ = -1;
	int64_t total_duration_ = 0;

	bool is_first_audio_packet_ = true;
	int64_t audio_start_time_ = -1;
	int64_t audio_last_pts_ = -1;
	int64_t audio_last_dts_ = -1;
	int64_t audio_next_pts_ = -1;
	int64_t audio_next_dts_ = -1;
};

#endif