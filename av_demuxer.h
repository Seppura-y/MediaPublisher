#ifndef AV_FORAMT_BASE_H
#define AV_FORMAT_BASE_H

#include "av_format_base.h"

struct AVPacket;
class AVDemuxer : public AVFormatBase
{
public:
	static AVFormatContext* OpenContext(const char* url);
	int Read(AVPacket* pkt);

	bool is_end_of_file() { std::unique_lock<std::mutex> lock(mtx_); return is_end_of_file_; }
protected:

private:
	bool is_end_of_file_ = false;
};
#endif
