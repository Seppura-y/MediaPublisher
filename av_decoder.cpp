#include "av_decoder.h"

#include <iostream>
extern"C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
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

#define PRINT_ERR_P(err) if(err != 0) {PrintError(err);return nullptr;}
#define PRINT_ERR_I(err) if(err != 0) {PrintError(err);return -1;}

int AVDecoder::Send(AVPacket* pkt)
{
	unique_lock<mutex> lock(mtx_);
	if (codec_ctx_ == nullptr)
	{
		cout << "Send Packet failed : codec_ctx_ is null" << endl;
		return -1;
	}

	int ret = avcodec_send_packet(codec_ctx_, pkt);
	if (ret == AVERROR(EAGAIN) || ret == AVERROR(EOF))
	{
		cout << "avcodec_send_packet failed : ret == AVERROR(EAGAIN) || ret == AVERROR(EOF)" << endl;
		PrintError(ret);
		return -1;
	}
	else if (ret == 0)
	{
		return ret;
	}
	else
	{
		//PrintError(ret);
		return -1;
	}
}

int AVDecoder::Recv(AVFrame* frame)
{
	unique_lock<mutex> lock(mtx_);
	if (codec_ctx_ == nullptr)
	{
		cout << "Receive frame failed : codec_ctx_ is null" << endl;
		return -1;
	}
	int ret = avcodec_receive_frame(codec_ctx_, frame);
	if (ret != 0)
	{
		//cout << "avcodec_receive_frame failed" << endl;
		//PrintError(ret);
		return -1;
	}
	return ret;
}