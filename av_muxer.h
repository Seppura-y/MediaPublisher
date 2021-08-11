#ifndef AV_MUXER_H
#define AV_MUXER_H

#include "av_format_base.h"



enum class AVMuxerType 
{
	AV_MUXER_TYPE_FILE,
	AV_MUXER_TYPE_RTMP,
	AV_MUXER_TYPE_RTSP
};
class AVMuxer : public AVFormatBase
{
public:
	static AVFormatContext* OpenContext(const char* url, AVCodecParameters* vparam,AVCodecParameters* aparam,AVMuxerType type);
	int WriteHeader();
	int WriteTrailer();
	int WriteData(AVPacket* pkt);
	//int TimeScale(int index,AVPacket* pkt,AVRational* src,long long pts);
	int TimeScale(int index, AVPacket* pkt, AVRational src, long long pts);

	int SetVideoTimebase(AVRational* src);
	int SetAudioTimebase(AVRational* src);
protected:

private:
	std::string url_;
	long long v_begin_pts_ = -1;
	long long a_begin_pts_ = -1;
	AVRational* v_src_timebase_ = nullptr;
	AVRational* a_src_timebase_ = nullptr;
};

#endif