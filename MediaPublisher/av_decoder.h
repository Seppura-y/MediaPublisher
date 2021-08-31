#ifndef AV_DECODER_H
#define AVDECODER_H


#include "av_codec_base.h"
struct AVPacket;
struct AVFrame;
class AVDecoder : public AVCodecBase
{
public:
	int Send(AVPacket* pkt);
	int Recv(AVFrame* frame);

	//int Parse(AVPacket* pkt);
protected:
	AVFrame* decoded_frame_ = nullptr;
private:

};

#endif