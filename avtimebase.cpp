#include "avtimebase.h"
#include <stdint.h>
#ifdef _WIN32
#include <WinSock2.h>
#include <time.h>
#else
#include <sys/time.h>
#endif

AVPublishTime* AVPublishTime::publish_time_ = nullptr;

int64_t AVPublishTime::GetCurrentTimeMSec()
{
#ifdef _WIN32
	struct timeval time_val;
	struct tm time;
	time_t sec;
	SYSTEMTIME sys_time;
	GetLocalTime(&sys_time);
	time.tm_year = sys_time.wYear - 1900;
	time.tm_mon = sys_time.wMonth;
	time.tm_mday = sys_time.wDay;
	time.tm_hour = sys_time.wHour;
	time.tm_min = sys_time.wMinute;
	time.tm_sec = sys_time.wSecond;
	time.tm_isdst = -1;
	sec = mktime(&time);
	time_val.tv_sec = sec;
	time_val.tv_usec = sys_time.wMilliseconds * 1000;
	return ( (time_val.tv_sec * 1000) + (time_val.tv_usec / 1000) );
#else
	struct timeval tv;
	gettimeofday(&tv,NULL);
	return ((tv.tv_sec * 1000) + (tv.tv_usec / 1000));
#endif
}

void AVPublishTime::set_audio_frame_duration(double duration)
{
	audio_frame_duration_ = duration;
	audio_pts_threshold_ = (uint32_t)(audio_frame_duration_ / 2);
}

void AVPublishTime::set_video_frame_duration(double duration)
{
	video_frame_duration_ = duration;
	video_pts_threshold_ = (uint32_t)(video_frame_duration_ / 2);
}

void AVPublishTime::set_audio_pts_strategy(PTS_STRATEGY strategy)
{
	audio_pts_strategy_ = strategy;
}

void AVPublishTime::set_video_pts_strategy(PTS_STRATEGY strategy)
{
	video_pts_strategy_ = strategy;
}

void AVPublishTime::ResetTime()
{
	start_time_ = GetCurrentTimeMSec();
}

uint32_t AVPublishTime::GetAudioPts()
{
	uint64_t cur = GetCurrentTimeMSec() - start_time_;
	uint32_t diff = (uint32_t)abs((long long)cur - (long long)(audio_previous_pts_ + audio_frame_duration_));
	if (audio_pts_strategy_ == PTS_RECTIFY)
	{
		if (diff < audio_pts_threshold_)
		{
			audio_previous_pts_ += audio_frame_duration_;
			return (uint32_t)( (uint64_t)audio_previous_pts_ % 0xffffffff );
		}
		else
		{
			audio_previous_pts_ = (double)cur;
			return (uint32_t)((uint64_t)audio_previous_pts_ % 0xffffffff);
		}
	}
	else
	{
		audio_previous_pts_ = (double)cur;
		return (uint32_t)((uint64_t)audio_previous_pts_ % 0xffffffff);
	}
}

uint32_t AVPublishTime::GetVideoPts()
{
	uint64_t cur = GetCurrentTimeMSec() - start_time_;
	uint32_t diff = (uint32_t)abs((long long)cur - (long long)(video_previous_pts_ + video_frame_duration_));
	if (video_pts_strategy_ == PTS_RECTIFY)
	{
		if (diff < video_pts_threshold_)
		{
			video_previous_pts_ += video_frame_duration_;
			return (uint32_t)((uint64_t)video_previous_pts_ % 0xffffffff);
		}
		else
		{
			video_previous_pts_ = (double)cur;
			return (uint32_t)((uint64_t)video_previous_pts_ % 0xffffffff);
		}
	}
	else
	{
		video_previous_pts_ = (double)cur;
		return (uint32_t)((uint64_t)video_previous_pts_ % 0xffffffff);
	}
}

