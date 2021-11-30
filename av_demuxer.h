#ifndef AV_FORAMT_BASE_H
#define AV_FORMAT_BASE_H

#include "av_format_base.h"
#include "av_ffmpeg.h"

struct AVPacket;
struct AVStream;
class AVDemuxer : public AVFormatBase
{
public:
	AVFormatContext* OpenContext(const char* url);
	int Read(AVPacket* pkt);
	int SeekToBeginning();
	bool is_end_of_file() { std::unique_lock<std::mutex> lock(mtx_); return is_end_of_file_; }

protected:
	int AddInputStreams(AVFormatContext* ctx);
	int AddAVStreams(AVFormatContext* ctx);
private:
	bool is_end_of_file_ = false;
	int nb_input_streams = 0;
	InputStream** input_streams = nullptr;

	AVStream* video_stream = nullptr;
	AVStream* audio_stream = nullptr;
};
#endif
