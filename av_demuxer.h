#ifndef AV_FORAMT_BASE_H
#define AV_FORMAT_BASE_H

#include "av_format_base.h"

struct AVPacket;
class AVDemuxer : public AVFormatBase
{
public:
	static AVFormatContext* OpenContext(const char* url);
	int Read(AVPacket* pkt);
protected:

private:

};
#endif
