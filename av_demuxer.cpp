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

static void* grow_array(void* array, int elem_size, int* size, int new_size)
{
	if (new_size >= INT_MAX / elem_size) {
		av_log(NULL, AV_LOG_ERROR, "Array too big.\n");
		return nullptr;
	}
	if (*size < new_size) {
		uint8_t* tmp = (uint8_t*)av_realloc_array(array, new_size, elem_size);
		if (!tmp) {
			av_log(NULL, AV_LOG_ERROR, "Could not alloc buffer.\n");
			return nullptr;
		}
		memset(tmp + *size * elem_size, 0, (new_size - *size) * elem_size);
		*size = new_size;
		return tmp;
	}
	return array;
}

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

int AVDemuxer::AddInputStreams(AVFormatContext* ctx)
{
	for (int i = 0; i < ctx->nb_streams; i++)
	{
		AVStream* st = ctx->streams[i];
		AVCodecParameters* par = st->codecpar;
		InputStream* ist = (InputStream*)av_mallocz(sizeof(*ist));
		//char* framerate = NULL, * hwaccel_device = NULL;
		//const char* hwaccel = NULL;
		//char* hwaccel_output_format = NULL;
		//char* codec_tag = NULL;
		//char* next;
		//char* discard_str = NULL;
		//const AVClass* cc = avcodec_get_class();
		//const AVOption* discard_opt = av_opt_find(&cc, "skip_frame", NULL, 0, 0);

		if (!ist)
			//exit_program(1);
			return -1;

		input_streams = (InputStream**)grow_array(input_streams, sizeof(input_streams), &nb_input_streams,nb_input_streams + 1);
		if (!input_streams)
		{
			cout << "grow array failed" << endl;
			return -1;
		}
		input_streams[nb_input_streams - 1] = ist;

		ist->st = st;
		//ist->file_index = nb_input_files;
		//ist->discard = 1;
		//st->discard = AVDISCARD_ALL;
		//ist->nb_samples = 0;
		//ist->min_pts = INT64_MAX;
		//ist->max_pts = INT64_MIN;

		//ist->ts_scale = 1.0;
		//MATCH_PER_STREAM_OPT(ts_scale, dbl, ist->ts_scale, ic, st);

		//ist->autorotate = 1;
		//MATCH_PER_STREAM_OPT(autorotate, i, ist->autorotate, ic, st);

		//MATCH_PER_STREAM_OPT(codec_tags, str, codec_tag, ic, st);
		//if (codec_tag) {
		//	uint32_t tag = strtol(codec_tag, &next, 0);
		//	if (*next)
		//		tag = AV_RL32(codec_tag);
		//	st->codecpar->codec_tag = tag;
		//}

		//ist->dec = choose_decoder(o, ic, st);
		//ist->decoder_opts = filter_codec_opts(o->g->codec_opts, ist->st->codecpar->codec_id, ic, st, ist->dec);

		//ist->reinit_filters = -1;
		//MATCH_PER_STREAM_OPT(reinit_filters, i, ist->reinit_filters, ic, st);

	}
}

int AVDemuxer::AddAVStreams(AVFormatContext* ctx)
{
	for (int i = 0; i < ctx->nb_streams; i++)
	{

	}
	return 0;
}


