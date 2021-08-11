#include "av_encoder.h"
#include <iostream>

extern"C"
{
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
}

#ifdef _WIN32
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

#define PRINT_ERR_P(err) if(err != 0) {PrintError(err);return nullptr;}
#define PRINT_ERR_I(err) if(err != 0) {PrintError(err);return -1;}


int AVEncoder::Send(AVFrame* frame)
{
	unique_lock<mutex> lock(mtx_);
	if (codec_ctx_ == nullptr)
	{
		cout << "codec_ctx_  is nullptr" << endl;
		return -1;
	}

	int ret = avcodec_send_frame(codec_ctx_, frame);
	if (ret == AVERROR(EAGAIN) || ret == AVERROR(EOF))
	{
		PrintError(ret);
		return -1;
	}
	else if (ret == 0)
	{
		return 0;
	}
	else
	{
		PrintError(ret);
		return -1;
	}
}


int AVEncoder::Recv(AVPacket* packet)
{
	unique_lock<mutex> lock(mtx_);
	if (codec_ctx_ == nullptr)
	{
		cout << "codec_ctx_ is nullptr" << endl;
		return -1;
	}

	int ret = avcodec_receive_packet(codec_ctx_, packet);
	if (ret == AVERROR(EAGAIN) || ret == AVERROR(EOF))
	{
		return -1;
	}
}