#ifndef AV_DEMUX_HANDLER_H
#define AV_DEMUX_HANDLER_H
#include "iav_base_handler.h"
#include "av_demuxer.h"

struct DemuxInfo
{
	int64_t last_pts;
	int64_t last_dts;
	int64_t next_pts;
	int64_t next_dts;

	int64_t ts_delta;

	int64_t duration;

	int64_t vs_max_pts;
	int64_t vs_min_pts;
	uint64_t vs_nb_packets;

	int64_t as_max_pts;
	int64_t as_min_pts;
	uint64_t as_nb_packets;

	AVRational time_base;
};

class AVDemuxHandler : public IAVBaseHandler
{
public:
	AVDemuxHandler();
	~AVDemuxHandler();
	bool OpenAVSource(const char* url,int timeout = 1000);
	virtual void Stop() override;
#if 0
	std::shared_ptr<AVParamWarpper> CopyVideoParameters();
	std::shared_ptr<AVParamWarpper> CopyAudioParameters();
#else
	std::shared_ptr<AVParametersWarpper> CopyVideoParameters();
	std::shared_ptr<AVParametersWarpper> CopyAudioParameters();
#endif
	AVRational* GetVideoSrcTimebase();
	AVRational* GetAudioSrcTimebase();
	AVRational* GetVideoSrcFrameRate();

	int GetVideoIndex();
	int GetAudioIndex();
	bool HasAudio() { return demuxer_.HasAudio(); }
	bool HasVideo() { return demuxer_.HasVideo(); }

	void SetNextVideoHandler(IAVBaseHandler* handler);
	void SetNextAudioHandler(IAVBaseHandler* handler);
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

	void set_nb_samples(int samples) { std::lock_guard<std::mutex> lock(mtx_); nb_samples_ = samples; }
	void set_sample_rate(int sample_rate) { std::lock_guard<std::mutex>lock(mtx_); sample_rate_ = sample_rate; }
	void set_frame_rate(AVRational rate) {
		std::lock_guard<std::mutex> lock(mtx_);
		frame_rate_ = rate;
	}
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
	AVRational frame_rate_ = { 0,0 };

	int audio_index_ = -1;
	int nb_samples_ = -1;
	int sample_rate_ = -1;
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

	DemuxInfo demux_info_;
};

#endif