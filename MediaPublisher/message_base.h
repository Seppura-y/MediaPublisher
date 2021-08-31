#ifndef MESSAGE_BASE_H
#define MESSAGE_BASE_H

#include "media_base.h"

#include <iostream>
#include <stdint.h>

enum class MessagePayloadType
{
	MESSAGE_PAYLOAD_TYPE_UNDEFINE = -1,
	MESSAGE_PAYLOAD_TYPE_METADATA,
	MESSAGE_PAYLOAD_TYPE_NALU,
	MESSAGE_PAYLOAD_TYPE_VIDEO_SEQ,
	MESSAGE_PAYLOAD_TYPE_ADTS_HEADER,
	MESSAGE_PAYLOAD_TYPE_AUDIO_RAW
};

class MessageBase
{
public:
	MessageBase(){}
	virtual ~MessageBase(){}
};

typedef struct MessagePayload
{
	MessagePayloadType what;
	MessageBase* message;
	bool is_exit;
} MessagePayload;

class NALUStruct : public MessageBase
{
public:
	NALUStruct(int size);
	NALUStruct(uint8_t* buf, int size);
	~NALUStruct();

	NALUType type_ = NALU_TYPE_UNDEFINE_1;
	int size_ = 0;
	uint8_t* data_ = nullptr;
	uint32_t pts_ = 0;
};

class VideoSequenceHeaderMessage : public MessageBase
{
public:
	VideoSequenceHeaderMessage(uint8_t* sps_data,int sps_size,uint8_t* pps_data,int pps_size)
	{
		sps_size_ = sps_size;
		sps_ = (uint8_t*)malloc(sps_size_);
		if (sps_ == nullptr)
		{
			std::cout << "sps_ malloc failed" << std::endl;
			return;
		}
		else
		{
			memcpy(sps_, sps_data, sps_size_);
		}

		pps_size_ = pps_size;
		pps_ = (uint8_t*)malloc(pps_size_);
		if (pps_ == nullptr)
		{
			std::cout << "pps_ malloc failed" << std::endl;
			return;
		}
		else
		{
			memcpy(pps_, pps_data, pps_size_);
		}
	}
	virtual ~VideoSequenceHeaderMessage()
	{
		if (sps_)
			free(sps_);
		if (pps_)
			free(pps_);
	}
	uint8_t* get_sps()
	{
		return this->sps_;
	}
	uint8_t* get_pps()
	{
		return this->pps_;
	}
	int get_sps_size()
	{
		return sps_size_;
	}
	int get_pps_size()
	{
		return pps_size_;
	}

//public:
//	unsigned int width_ = -1;
//	unsigned int height_ = -1;
//	unsigned int frame_rate_ = -1;
//	unsigned int bit_per_sec_ = -1;
//	int64_t pts_ = -1;
private:
	uint8_t* sps_ = nullptr;
	uint8_t* pps_ = nullptr;
	int sps_size_ = 0;
	int pps_size_ = 0;
};

class AudioSequenceHeaderMessage : public MessageBase
{
public:
	AudioSequenceHeaderMessage(uint8_t profile, uint8_t channel_num, uint32_t samplerate)
	{
		profile_ = profile;
		channels_ = channel_num;
		sample_rate_ = samplerate;
	}
	virtual ~AudioSequenceHeaderMessage() {}

	uint8_t get_profile()
	{
		return profile_;
	}

	uint8_t get_channels()
	{
		return channels_;
	}

	uint32_t get_sample_rate()
	{
		return sample_rate_;
	}

private:
	uint8_t profile_ = 2;   //2 : AAC LC(Low Complexity)
	uint8_t channels_ = 2;
	uint32_t sample_rate_ = 48000;
	int64_t pts_;
};

class AudioRawData : public MessageBase
{
public:
	AudioRawData(uint8_t* data,uint32_t data_size)
	{
		this->size_ = data_size;
		this->data_ = (uint8_t*)malloc(data_size);
		memcpy(data_, data, data_size);
	}
	~AudioRawData()
	{
		if (data_)
		{
			free(data_);
		}
	}

	uint8_t* data_ = nullptr;
	uint32_t pts_ = 0;
	uint32_t size_ = 0;
};

class FlvOnMetaData : public MessageBase
{
public:
	FlvOnMetaData() {};
	~FlvOnMetaData() {};

	bool has_video_ = false;
	double width_ = 0;
	double height_ = 0;
	double video_codec_id_ = -1;
	double video_data_rate_ = 0;
	double frame_rate_ = 0;

	bool has_audio_ = false;
	bool stereo_ = true;
	double audio_codec_id_ = -1;
	double audio_data_rate_ = 0;
	double audio_sample_rate_ = 0;
	double audio_sample_size_ = 0;

	int channels_ = 0;
	double duration_ = 0;

	int64_t pts_ = 0;
	int64_t file_size_ = 0;
	bool can_seek_to_end_ = false;
	std::string creation_time_;
};

class YUVStruct : public MessageBase
{
public:
	YUVStruct(int w, int h, int size);
	YUVStruct(uint8_t* buff,int w, int h, int size);
	virtual ~YUVStruct();

	int width_ = 0;
	int height_ = 0;
	int size_ = 0;
	uint8_t* data_ = nullptr;
};

class YUV420P : public YUVStruct
{
public:
	YUV420P(int w, int h, int size);
	YUV420P(uint8_t* buff, int w, int h, int size);
	~YUV420P();
	
	uint8_t* y_slice_ = nullptr;
	uint8_t* u_slice_ = nullptr;
	uint8_t* v_slice_ = nullptr;
};

#endif