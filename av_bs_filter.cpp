#include "av_bs_filter.h"

#include <iostream>

extern"C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
}

#ifdef _WIN32
#pragma comment(lib,"avformat.lib")
#pragma comment(lib,"avcodec.lib")
#pragma comment(lib,"avutil.lib")
#endif

using namespace std;

static void PrintError(int err)
{
	char buffer[1024] = { 0 };
	av_strerror(err, buffer, sizeof(buffer));
	cout << buffer << endl;
}

AVBSFilter::AVBSFilter()
{

}

AVBSFilter::~AVBSFilter()
{
	if (bsf_ctx_)
	{
		av_bsf_free(&bsf_ctx_);
		bsf_ctx_ = nullptr;
	}
}

AVBSFContext* AVBSFilter::CreateBSFContext(AVCodecParameters* param)
{
	const AVBitStreamFilter* bs_filter = av_bsf_get_by_name("h264_mp4toannexb");
	if (!bs_filter)
	{
		cout << "create bsf context failed : bs_filter is null" << endl;
		return nullptr;
	}

	AVBSFContext* bsf_ctx = nullptr;
	int ret = av_bsf_alloc(bs_filter, &bsf_ctx);
	if (ret != 0)
	{
		cout << "av_bsf_alloc failed : " << flush;
		PrintError(ret);
		return nullptr;
	}

	ret = avcodec_parameters_copy(bsf_ctx->par_in, param);
	if (ret >= 0)
	{
		cout << "avcodec_parameters_copy failed: " << flush;
		PrintError(ret);
		return nullptr;
	}
	return bsf_ctx;
}

int AVBSFilter::SetBSFContext(AVBSFContext* ctx)
{
	unique_lock<mutex> lock(mtx_);
	bsf_ctx_ = ctx;

	int ret = av_bsf_init(bsf_ctx_);
	if (ret < 0)
	{
		cout << "av_bsf_init failed : " << flush;
		PrintError(ret);
		return ret;
	}
	return 0;
}

int AVBSFilter::Send(AVPacket* pkt)
{
	unique_lock<mutex> lock(mtx_);
	if (!bsf_ctx_)
	{
		cout << "send packet failed : bit_stream_filter_context_ is null " << endl;
		return -1;
	}

	int ret = av_bsf_send_packet(bsf_ctx_, pkt);
	if (ret == AVERROR(EAGAIN) || ret == AVERROR(EOF))
	{
		cout << "av_bsf_send_packet failed : ret == AVERROR(EAGAIN) || ret == AVERROR(EOF)" << endl;
		PrintError(ret);
		return -1;
	}
	else if (ret == 0)
	{
		return ret;
	}
	else
	{
		PrintError(ret);
		return -1;
	}
}

int AVBSFilter::Recv(AVPacket* pkt)
{
	unique_lock<mutex> lock(mtx_);
	if (!bsf_ctx_)
	{
		cout << "receive frame failed : bsf_ctx_ is null" << endl;
		return -1;
	}

	int ret = av_bsf_receive_packet(bsf_ctx_, pkt);
	if (ret == AVERROR(EAGAIN) || ret == AVERROR(EOF))
	{
		cout << "av_bsf_receive_packet failed : ret == AVERROR(EAGAIN) || ret == AVERROR(EOF)" << endl;
		PrintError(ret);
		return -1;
	}
	else if (ret == 0)
	{
		return ret;
	}
	else
	{
		PrintError(ret);
		return -1;
	}
}