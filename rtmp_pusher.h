#ifndef RTMP_PUSHER_H
#define RTMP_PUSHER_H


#include "rtmp_base.h"
#include "base_handler.h"

class RtmpPusher : public RtmpBase, public BaseHandler
{
public:
	RtmpPusher(int max_queue_size = 30);
protected:
	int SendPacket(unsigned int type,unsigned char* data,unsigned int size,unsigned int timestamp);
	int SendMetaData(FlvOnMetaData* data);
	int SendAudioSequence(AudioSequenceHeaderMessage* asequence);
	int SendAudioRawData(AudioRawData* araw);
	int SendVideoSequence(VideoSequenceHeaderMessage* vsequence);
	int SendVideoRawData(NALUStruct* nalu);
	bool GetAudioSpecificConfig(uint8_t* data, uint8_t profile, uint8_t channels, uint32_t sample_rate);
private:
	virtual void Handle(MessagePayloadType what,MessageBase* msg) override;
	virtual void AddMsg(MessagePayload* obj,bool flush) override;

	enum
	{
		FLV_CODECID_H264 = 7,
		FLV_CODECID_AAC = 10,
	};
	bool is_first_metadata_sent_ = false;
	bool is_first_video_seq_sent_ = false;
	bool is_first_audio_seq_sent_ = false;
	bool is_first_audio_raw_sent_ = false;
	bool is_first_video_raw_sent_ = false;

	int64_t time_ = 0;
};

#endif