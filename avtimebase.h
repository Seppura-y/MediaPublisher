#ifndef AVTIMEBASE_H
#define AVTIMEBASE_H


#include <stdint.h>

class AVPublishTime
{
public:
	~AVPublishTime(){}

	typedef enum PTS_STRATEGY
	{
		PTS_RECTIFY,
		PTS_REAL_TIME
	}PTS_STRATEGY;

	static AVPublishTime* GetInstance()
	{
		if (!publish_time_)
		{
			publish_time_ = new AVPublishTime();
		}
		return publish_time_;
	}

	void set_audio_frame_duration(double duration);
	void set_video_frame_duration(double duration);
	void set_audio_pts_strategy(PTS_STRATEGY strategy);
	void set_video_pts_strategy(PTS_STRATEGY strategy);

	void ResetTime();
	uint32_t GetAudioPts();
	uint32_t GetVideoPts();

protected:

private:
	AVPublishTime()
	{
		start_time_ = GetCurrentTimeMSec();
	}
	int64_t GetCurrentTimeMSec();


	static AVPublishTime* publish_time_;
	int64_t start_time_;

	PTS_STRATEGY audio_pts_strategy_ = PTS_STRATEGY::PTS_RECTIFY;
	double audio_frame_duration_ = 21.3333;
	double audio_previous_pts_ = 0;
	uint32_t audio_pts_threshold_ = (uint32_t)(audio_frame_duration_ / 2);

	PTS_STRATEGY video_pts_strategy_ = PTS_STRATEGY::PTS_RECTIFY;
	double video_frame_duration_ = 40;
	double video_previous_pts_ = 0;
	uint32_t video_pts_threshold_ = (uint32_t)(video_frame_duration_ / 2);

};

#endif