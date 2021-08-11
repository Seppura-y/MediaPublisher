#ifndef AV_FORMATTER_H
#define AV_FORMATTER_H


#include <mutex>
#include "av_data_tools.h"

struct AVFormatContext;
class AVFormatBase
{
public:
	AVFormatBase();
	virtual ~AVFormatBase();

	int SetFormatContext(AVFormatContext* ctx);
	int CloseContext();

	std::shared_ptr<AVParamWarpper> CopyVideoParameters();
	std::shared_ptr<AVParamWarpper> CopyAudioParameters();

	bool is_network_connected();
	bool HasVideo();
	bool HasAudio();
	void SetTimeout(int ms);

	int get_audio_index();
	int get_video_index();
protected:
	bool IsTimeout();
	static int TimeoutCallback(void* opaque);

protected:
	std::mutex mtx_;
	AVFormatContext* fmt_ctx_ = nullptr;

	AVRational* audio_timebase_{ nullptr };
	AVRational* video_timebase_{ nullptr };

	int audio_index_ = -1;
	int video_index_ = -1;
	int connect_timeout_ = -1;
	int last_read_time_ = -1;
	bool is_network_connected_ = false;

private:

};

#endif