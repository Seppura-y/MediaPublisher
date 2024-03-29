#ifndef MEDIA_BASE_H
#define MEDIA_BASE_H


typedef enum FlvTagType
{
	FLV_TAG_TYPE_ONMETADATA = 0,
	FLV_TAG_TYPE_AAC_SEQUENCE_HEADER,
	FLV_TAG_TYPE_AAC_RAW,
	FLV_TAG_TYPE_AVC_SEQUENCE_HEADER,
	FLV_TAG_TYPE_AVC_RAW
} FlvTagType;

typedef enum NALUType
{
	NALU_TYPE_UNDEFINE_1 = 0x00,
	NALU_TYPE_NOIDR = 0x01,
	NALU_TYPE_SLICE_A = 0x02,
	NALU_TYPE_SLICE_B = 0x03,
	NALU_TYPE_SLICE_C = 0x04,
	NALU_TYPE_IDR = 0x05,
	NALU_TYPE_SEI = 0x06,
	NALU_TYPE_SPS = 0x07,
	NALU_TYPE_PPS = 0x08,
	NALU_TYPE_ACCESS_UNIT_DELIMITER = 0x09,
	NALU_TYPE_END_OF_SEQ = 0x0A,
	NALU_TYPE_END_OF_STREAM = 0x0B,
	NALU_TYPE_FILLER_DATA = 0x0C,
	NALU_TYPE_SPS_EXTENSION = 0x0D,
	NALU_TYPE_RESERVED_1 = 0x0E,
	NALU_TYPE_RESERVED_2 = 0x0F,
	NALU_TYPE_RESERVED_3 = 0x10,
	NALU_TYPE_RESERVED_4 = 0x11,
	NALU_TYPE_RESERVED_5 = 0x12,
	NALU_TYPE_SLICE_WITHOUT_PARTITION = 0x13,
	NALU_TYPE_RESERVED_6 = 0x14,
	NALU_TYPE_RESERVED_7 = 0x15,
	NALU_TYPE_RESERVED_8 = 0x16,
	NALU_TYPE_RESERVED_9 = 0x17,
	NALU_TYPE_UNDEFINE_2 = 0x18,
	NALU_TYPE_UNDEFINE_3 = 0x19,
	NALU_TYPE_UNDEFINE_4 = 0x1A,
	NALU_TYPE_UNDEFINE_5 = 0x1B,
	NALU_TYPE_UNDEFINE_6 = 0x1C,
	NALU_TYPE_UNDEFINE_7 = 0x1D,
	NALU_TYPE_UNDEFINE_8 = 0x1E,
	NALU_TYPE_UNDEFINE_9 = 0x1F,
} NALUType;

typedef enum RtmpPacketChannel
{
	RTMP_NETWORK_CHANNEL = 2,   //< channel for network-related messages (bandwidth report, ping, etc)
	RTMP_SYSTEM_CHANNEL,			 //< channel for sending server control messages
	RTMP_AUDIO_CHANNEL,				 //< channel for audio data
	RTMP_VIDEO_CHANNEL = 6,		 //< channel for video data
	RTMP_SOURCE_CHANNEL = 8,		 //< channel for a/v invokes
}RtmpPacketChannel;

#endif