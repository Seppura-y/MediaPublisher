#ifndef RTMPBASE_H
#define RTMPBASE_H

#include <iostream>
#include "rtmp.h"

enum class RtmpBaseType
{
	RTMP_BASE_TYPE_PUSH = 0,
	RTMP_BASE_TYPE_PULL
};
class RtmpBase
{
public:
	RtmpBase() {};
	RtmpBase(RtmpBaseType type,std::string url);
	virtual ~RtmpBase();

	bool Connect();
	void DisConnect();
	bool IsConnected();
	void SetUrl(std::string url);
	bool SetReceiveAudio(bool is_recv);
	bool SetReceiveVideo(bool is_recv);
protected:
	RTMP* rtmp_ = nullptr;
	std::string url_;
	bool enable_audio_ = false;
	bool enable_video_ = false;
private:
	bool InitRtmp();
	RtmpBaseType rtmp_base_type_;
};

#endif