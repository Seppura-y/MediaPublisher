#include "rtmp_pusher.h"
#include "timeutil.h"

#include <iostream>
using namespace std;

char* put_byte(char* dst, const char src)
{
	dst[0] = src;
	return dst + 1;
}

char* put_be16(char* dst, const uint16_t src)
{
	dst[0] = src >> 8;
	dst[1] = src & 0xff;
	return dst + 2;
}

char* put_be32(char* dst, const uint32_t src)
{
	dst[0] = src >> 24;
	dst[1] = (src >> 16) & 0xff;
	dst[2] = (src >> 8) & 0xff;
	dst[3] = src & 0xff;
	return dst + 4;
}

char* put_amf_string(char* dst, const char* src)
{
	uint16_t len = strlen(src);
	dst = put_be16(dst, len);
	memcpy(dst, src, len);
	return dst + len;
}

char* put_amf_double(char* dst, const double src)
{
	*(dst++) = AMF_NUMBER;
	unsigned char* ci = (unsigned char*)&src;
	unsigned char* co = (unsigned char*)dst;
	co[0] = ci[7];
	co[1] = ci[6];
	co[2] = ci[5];
	co[3] = ci[4];
	co[4] = ci[3];
	co[5] = ci[2];
	co[6] = ci[1];
	co[7] = ci[0];
	return dst + 8;
}

RtmpPusher::RtmpPusher(RtmpBaseType type, string url,int max_queue_size) : RtmpBase(type,url) ,BaseHandler(max_queue_size)
{

}

void RtmpPusher::Handle(MessagePayloadType what, MessageBase* msg)
{
	if (!IsConnected())
	{
		if (!Connect())
		{
			cout << "rtmp pusher handle : connect failed" << endl;
			return;
		}
	}
	switch (what)
	{
		case MessagePayloadType::MESSAGE_PAYLOAD_TYPE_METADATA:
		{
			if (!is_first_metadata_sent_)
			{
				is_first_metadata_sent_ = true;
				//write log
			}
			FlvOnMetaData* meta = (FlvOnMetaData*)msg;
			if (!SendMetaData(meta))
			{
				cout << "RtmpPusher::Handle FLV_TAG_TYPE_ONMETADATA failed" << endl;
			}

			delete meta;
			break;
		}
		case MessagePayloadType::MESSAGE_PAYLOAD_TYPE_ADTS_HEADER:
		{
			if (!is_first_audio_seq_sent_)
			{
				is_first_audio_seq_sent_ = true;
				//write log
			}
			AudioSequenceHeaderMessage* aseq = (AudioSequenceHeaderMessage*)msg;
			if (!SendAudioSequence(aseq))
			{
				cout << "RtmpPusher::Handle FLV_TAG_TYPE_AAC_SEQUENCE_HEADER failed" << endl;
			}
			delete aseq;
			break;
		}
		case MessagePayloadType::MESSAGE_PAYLOAD_TYPE_AUDIO_RAW:
		{
			if (!is_first_audio_raw_sent_)
			{
				is_first_audio_raw_sent_ = true;
				//write log to record the first time that aac raw data is sent
			}
			AudioRawData* araw = (AudioRawData*)msg;
			if (!SendAudioRawData(araw))
			{
				cout << "RtmpPusher::Handle RTMP_PACKET_TYPE_AUDIO failed" << endl;
			}
			delete araw;
			break;
		}
		case MessagePayloadType::MESSAGE_PAYLOAD_TYPE_VIDEO_SEQ:
		{
			if (!is_first_video_seq_sent_)
			{
				is_first_video_seq_sent_ = true;
				//write log
			}
			VideoSequenceHeaderMessage* vseq = (VideoSequenceHeaderMessage*)msg;
			if (!SendVideoSequence(vseq))
			{
				cout << "RtmpPusher::Handle SendVideoSequence failed" << endl;
			}
			delete vseq;
			break;
		}
		case MessagePayloadType::MESSAGE_PAYLOAD_TYPE_NALU:
		{
			if (!is_first_video_raw_sent_)
			{
				is_first_video_raw_sent_ = true;
				//write log
			}

			NALUStruct* nalu = (NALUStruct*)msg;

			if (!SendVideoRawData(nalu))
			{
				cout << "RtmpPusher::Handle SendVideoRawData failed" << endl;
			}
			delete nalu;
			break;
		}
		default:
		{
			break;
		}
	}
}

void RtmpPusher::AddMsg(MessagePayload* msg, bool flush)
{
	mtx_.lock();
	if (flush)
	{
		message_queue_.clear();
	}

	if (message_queue_.size() >= max_queue_size_)
	{
		while (message_queue_.size() > 0)
		{
			MessagePayload* obj = message_queue_.front();
			if (obj->what == MessagePayloadType::MESSAGE_PAYLOAD_TYPE_NALU && ((NALUStruct*)(obj->message))->type_ == NALU_TYPE_IDR)
			{
				break;
			}
			message_queue_.pop_front();
			delete obj->message;
			delete obj;
		}
	}

	message_queue_.push_back(msg);
	mtx_.unlock();

	msg_avaliable_->Post(1);
}

int RtmpPusher::SendMetaData(FlvOnMetaData* metadata)
{
	if (!metadata)
	{
		return FALSE;
	}
	char buffer[1024] = { 0 };
	char* p = buffer;

	put_byte(p, AMF_STRING);
	put_amf_string(p, "onMetaData");

	put_byte(p, AMF_OBJECT);
	put_amf_string(p, "object");

	if (metadata->has_audio_)
	{

		if (metadata->has_video_)
		{
			p = put_amf_string(p, "width");
			p = put_amf_double(p, metadata->width_);

			p = put_amf_string(p, "height");
			p = put_amf_double(p, metadata->height_);

			p = put_amf_string(p, "framerate");
			p = put_amf_double(p, metadata->frame_rate_);

			p = put_amf_string(p, "videodatarate");
			p = put_amf_double(p, metadata->video_data_rate_);

			p = put_amf_string(p, "videocodecid");
			p = put_amf_double(p, FLV_CODECID_H264);
		}
		if (metadata->has_audio_)
		{
			p = put_amf_string(p, "audiodatarate");
			p = put_amf_double(p, (double)metadata->audio_data_rate_);

			p = put_amf_string(p, "audiosamplerate");
			p = put_amf_double(p, (double)metadata->audio_sample_rate_);

			p = put_amf_string(p, "audiosamplesize");
			p = put_amf_double(p, (double)metadata->audio_sample_size_);

			p = put_amf_string(p, "stereo");
			p = put_amf_double(p, (double)metadata->channels_);

			p = put_amf_string(p, "audiocodecid");
			p = put_amf_double(p, (double)FLV_CODECID_AAC);
		}
		p = put_amf_string(p, "");
		p = put_byte(p, AMF_OBJECT_END);

		return SendPacket(RTMP_PACKET_TYPE_INFO, (unsigned char*)buffer, p - buffer, 0);
	}
}

int RtmpPusher::SendAudioSequence(AudioSequenceHeaderMessage* asequence)
{
	uint8_t data[4];
	data[0] = 0xAF;
	data[1] = 0x00;
	if (GetAudioSpecificConfig(&data[2], asequence->get_profile(), asequence->get_sample_rate(), asequence->get_channels()))
	{
		return SendPacket(RTMP_PACKET_TYPE_AUDIO, (unsigned char*)data, sizeof(data), 0);
	}
	else
	{
		return -1;
	}
}

int RtmpPusher::SendAudioRawData(AudioRawData* araw)
{
	return SendPacket(RTMP_PACKET_TYPE_AUDIO, araw->data_, araw->size_, araw->pts_);
}

int RtmpPusher::SendVideoSequence(VideoSequenceHeaderMessage* vsequence)
{

	if (vsequence == NULL)
	{
		return false;
	}
	uint8_t body[1024] = { 0 };

	int i = 0;
	body[i++] = 0x17; // 1:keyframe  7:AVC
	body[i++] = 0x00; // AVC sequence header

	body[i++] = 0x00;
	body[i++] = 0x00;
	body[i++] = 0x00; // fill in 0;   0

	// AVCDecoderConfigurationRecord.
	body[i++] = 0x01;									// configurationVersion
	body[i++] = vsequence->get_sps()[1];	// AVCProfileIndication
	body[i++] = vsequence->get_sps()[2];	// profile_compatibility
	body[i++] = vsequence->get_sps()[3];	// AVCLevelIndication
	body[i++] = 0xff;									// lengthSizeMinusOne

	// sps nums
	body[i++] = 0xE1;                 //&0x1f
	// sps data length
	body[i++] = (vsequence->get_sps_size() >> 8) & 0xff;;
	body[i++] = vsequence->get_sps_size() & 0xff;
	// sps data
	memcpy(&body[i], vsequence->get_sps(), vsequence->get_sps_size());
	i = i + vsequence->get_sps_size();

	// pps nums
	body[i++] = 0x01; //&0x1f
	// pps data length
	body[i++] = (vsequence->get_pps_size() >> 8) & 0xff;;
	body[i++] = vsequence->get_pps_size() & 0xff;
	// sps data
	memcpy(&body[i], vsequence->get_pps(), vsequence->get_pps_size());
	i = i + vsequence->get_pps_size();

	time_ = TimesUtil::GetTimeMillisecond();
	//    time_ = Tim
	return SendPacket(RTMP_PACKET_TYPE_VIDEO, (unsigned char*)body, i, 0);
}

int RtmpPusher::SendVideoRawData(NALUStruct* nalu)
{
	int size = nalu->size_;
	bool is_keyframe = nalu->type_ == 0x05 ? true : false;
	if (!nalu && size < 11)
	{
		return false;
	}

	unsigned char* body = new unsigned char[size + 9];

	int i = 0;
	if (is_keyframe)
	{
		body[i++] = 0x17;// 1:Iframe  7:AVC
	}
	else
	{
		body[i++] = 0x27;// 2:Pframe  7:AVC
	}
	body[i++] = 0x01;// AVC NALU
	body[i++] = 0x00;
	body[i++] = 0x00;
	body[i++] = 0x00;

	// NALU size
	body[i++] = size >> 24;
	body[i++] = size >> 16;
	body[i++] = size >> 8;
	body[i++] = size & 0xff;;

	// NALU data
	memcpy(&body[i], nalu->data_, size);

	bool bRet = SendPacket(RTMP_PACKET_TYPE_VIDEO, body, i + size, nalu->pts_);
	delete[] body;
	return bRet;
}

int RtmpPusher::SendPacket(unsigned int msg_id, unsigned char* data, unsigned int size, unsigned int timestamp)
{
	if (!rtmp_)
	{
		return FALSE;
	}
	RTMPPacket pkt;
	RTMPPacket_Reset(&pkt);
	RTMPPacket_Alloc(&pkt, size);

	if (msg_id == RTMP_PACKET_TYPE_AUDIO)
	{
		pkt.m_nChannel = RTMP_AUDIO_CHANNEL;
	}
	else if (msg_id == RTMP_PACKET_TYPE_VIDEO)
	{
		pkt.m_nChannel = RTMP_VIDEO_CHANNEL;
	}
	else
	{
		pkt.m_nChannel = RTMP_NETWORK_CHANNEL;
	}
	pkt.m_packetType = msg_id;
	pkt.m_headerType = RTMP_PACKET_SIZE_LARGE;
	pkt.m_nInfoField2 = rtmp_->m_stream_id;
	pkt.m_nTimeStamp = timestamp;
	pkt.m_nBodySize = size;
	memcpy(pkt.m_body, data, size);

	int ret = RTMP_SendPacket(rtmp_, &pkt, 0);
	if (ret != TRUE)
	{
		cout << "RTMP_SendPacket failed" << endl;
	}
	RTMPPacket_Free(&pkt);
	
	return ret;
}

bool RtmpPusher::GetAudioSpecificConfig(uint8_t* data, uint8_t profile, uint8_t channels, uint32_t sample_rate)
{
	uint16_t audio_spec;
	uint16_t _profile = (profile + 1) << 11;
	uint16_t _channels = 0;
	uint16_t _sample_rate = 0;
	switch (channels)
	{
		case  96000:
			_sample_rate = 0;
			break;
		case 88200:
			_sample_rate = 1;
			break;
		case 64000:
			_sample_rate = 2;
			break;
		case 48000:
			_sample_rate = 3;
			break;
		case 44100:
			_sample_rate = 4;
			break;
		case 32000:
			_sample_rate = 5;
			break;
		case 24000:
			_sample_rate = 6;
			break;
		case 22050:
			_sample_rate = 7;
			break;
		case 16000:
			_sample_rate = 8;
			break;
		case 12000:
			_sample_rate = 9;
			break;
		case 11025:
			_sample_rate = 10;
			break;
		case 8000:
			_sample_rate = 11;
			break;
		case 7350:
			_sample_rate = 12;
			break;
		default:
			_sample_rate = 4;
			return false;
			break;
	}
	_sample_rate <<= 7;
	_channels = channels;
	_channels <<= 3;

	audio_spec = _profile | _sample_rate | _channels;
	data[0] = (uint8_t)(audio_spec >> 8);
	data[1] = 0xff & audio_spec;
	return true;
}