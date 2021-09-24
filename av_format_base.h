#ifndef AV_FORMATTER_H
#define AV_FORMATTER_H


#include <mutex>
#include "av_data_tools.h"

struct AVFormatContext;
struct AVRational;

enum class AVProtocolType
{
	AV_PROTOCOL_TYPE_FILE,
	AV_PROTOCOL_TYPE_RTMP,
	AV_PROTOCOL_TYPE_RTSP
};

class AVFormatBase
{
public:
	AVFormatBase();
	virtual ~AVFormatBase();

	int SetFormatContext(AVFormatContext* ctx);
	int CloseContext();
	int CloseIOContext();

	std::shared_ptr<AVParamWarpper> CopyVideoParameters();
	std::shared_ptr<AVParamWarpper> CopyAudioParameters();

	bool is_network_connected();
	bool HasVideo();
	bool HasAudio();
	void SetTimeout(int ms,bool status);
	void SetTimeoutEnable(bool status);

	int get_audio_index();
	int get_video_index();

	int TimeScale(int index, AVPacket* pkt, AVRational src, long long pts);

	AVRational* GetVideoTimebase();
	AVRational* GetAudioTimebase();
	AVRational* GetVideoFrameRate();
	int GetCodecExtraData(uint8_t* buffer, int& size);
	uint8_t* GetSpsData();
	uint8_t* GetPpsData();
	int GetSpsSize();
	int GetPpsSize();

	virtual void SetProtocolType(AVProtocolType type);
protected:
	bool IsTimeout();
	static int TimeoutCallback(void* opaque);

protected:
	std::mutex mtx_;
	AVFormatContext* fmt_ctx_ = nullptr;

	AVRational* audio_src_timebase_{ nullptr };
	AVRational* video_src_timebase_{ nullptr };
	AVRational* video_src_frame_rate_{ nullptr };

	int sps_size_ = -1;
	int pps_size_ = -1;
	std::string sps_data_;
	std::string pps_data_;

	int audio_index_ = -1;
	int video_index_ = -1;
	int timeout_threshold_ = -1;
	int last_proc_time_ = -1;
	bool is_network_connected_ = false;
	bool is_timeout_proc_enabled_ = true;

	AVProtocolType protocol_type_ = AVProtocolType::AV_PROTOCOL_TYPE_FILE;
private:

};

#endif