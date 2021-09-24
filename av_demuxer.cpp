#include "av_demuxer.h"
#include "av_data_tools.h"

#include <iostream>
#ifdef _WIN32
extern"C"
{
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
}

#pragma comment(lib,"avformat.lib")
#pragma comment(lib,"avutil.lib")
#endif

using namespace std;

static void PrintError(int err)
{
	char buffer[1024] = { 0 };
	av_strerror(err, buffer, sizeof(buffer));
	cout << buffer << endl;
}

#define PRINT_ERR_P(err) if(err != 0) {PrintError(err);return nullptr;}
#define PRINT_ERR_I(err) if(err != 0) {PrintError(err);return -1;}


//AVFormatContext* AVDemuxer::OpenContext(const char* url)
//{
//	//mutex mtx;
//	//unique_lock<mutex> lock(mtx);
//	AVFormatContext* fmt_ctx = nullptr;
//	AVDictionary* opt = nullptr;
//	av_dict_set(&opt, "stimeout", "1000000", 0);
//	int ret = avformat_open_input(&fmt_ctx, url, nullptr, &opt);
//	PRINT_ERR_P(ret)
//
//	ret = avformat_find_stream_info(fmt_ctx, nullptr);
//	PRINT_ERR_P(ret)
//
//	av_dump_format(fmt_ctx, 0, url, 0);
//
//	return fmt_ctx;
//}

AVFormatContext* AVDemuxer::OpenContext(const char* url)
{
	//mutex mtx;
	//unique_lock<mutex> lock(mtx);
	AVFormatContext* fmt_ctx = avformat_alloc_context();
	fmt_ctx->flags |= AVFMT_FLAG_NONBLOCK;

	//AVIOInterruptCB cb = { TimeoutCallback,this };
	//fmt_ctx->interrupt_callback = cb;

	AVDictionary* opt = nullptr;
	av_dict_set(&opt, "stimeout", "1000000", 0);
	int ret = avformat_open_input(&fmt_ctx, url, nullptr, &opt);
	PRINT_ERR_P(ret)

		ret = avformat_find_stream_info(fmt_ctx, nullptr);
	PRINT_ERR_P(ret)

		av_dump_format(fmt_ctx, 0, url, 0);

	return fmt_ctx;
}

int AVDemuxer::Read(AVPacket* pkt)
{
	unique_lock<mutex> lock(mtx_);
	if (!fmt_ctx_)
	{
		cout << "fmt_ctx_ is null" << endl;
		return -1;
	}

	int ret = av_read_frame(fmt_ctx_, pkt);
	if (ret == AVERROR_EOF)
	{
		is_end_of_file_ = true;
		cout << "read frame end of file " << endl;
		return -1;
	}
	//PRINT_ERR_I(ret);

	last_proc_time_ = GetCurrentTimeMsec();
	return 0;
}

int AVDemuxer::SeekToBeginning()
{
	unique_lock<mutex> lock(mtx_);
	if (!fmt_ctx_)
	{
		cout << "fmt_ctx_ is null seek failed" << endl;
		return -1;
	}

	int ret = av_seek_frame(fmt_ctx_, video_index_, fmt_ctx_->start_time, AVSEEK_FLAG_FRAME | AVSEEK_FLAG_BACKWARD);
	if (ret < 0)
	{
		cout << "av_seek_frame failed" << endl;
		PrintError(ret);
		return -1;
	}

	return 0;
}



