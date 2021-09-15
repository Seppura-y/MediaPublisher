#ifndef AV_MUXER_H
#define AV_MUXER_H

#include "av_format_base.h"

class AVMuxer : public AVFormatBase
{
public:
	static AVFormatContext* OpenContext(const char* url, AVCodecParameters* vparam,AVCodecParameters* aparam,AVProtocolType type);
	int WriteHeader();
	int WriteTrailer();
	int WriteData(AVPacket* pkt);
	//int TimeScale(int index,AVPacket* pkt,AVRational* src,long long pts);
	int TimeScale(int index, AVPacket* pkt, AVRational src, long long pts);

	int SetVideoTimebase(AVRational* src);
	int SetAudioTimebase(AVRational* src);

	void ResetVideoBeginPts() { std::unique_lock<std::mutex> lock(mtx_); v_begin_pts_ = -1; }
	void ResetAudioBeginPts() { std::unique_lock<std::mutex> lock(mtx_); a_begin_pts_ = -1; }
protected:

private:
	std::string url_;
	int64_t v_begin_pts_ = -1;
	int64_t a_begin_pts_ = -1;
	AVRational* v_src_timebase_ = nullptr;
	AVRational* a_src_timebase_ = nullptr;

};

#endif