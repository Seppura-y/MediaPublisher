#ifndef AV_ENCODER_H
#define AV_ENCODER_H

#include "av_codec_base.h"

struct AVFrame;
struct AVPacket;
class AVEncoder : public AVCodecBase
{
public:
	int Send(AVFrame* frame);
	int Recv(AVPacket* packet);
protected:
	AVPacket* encoded_packet_ = nullptr;
private:

};

#endif