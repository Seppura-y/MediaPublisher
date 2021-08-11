#include "rtmp_base.h"
#include "rtmp_sys.h"
#include "log.h"

#include <stdint.h>
#ifdef _WIN32
#pragma comment(lib,"ws2_32.lib")
#endif
using namespace std;

RtmpBase::RtmpBase(RtmpBaseType type, std::string url)
{
	rtmp_base_type_ = type;
	url_ = url;
	InitRtmp();
}

RtmpBase::~RtmpBase()
{
	if (IsConnected())
	{
		DisConnect();
	}
	RTMP_Free(rtmp_);
	rtmp_ = nullptr;
#ifdef _WIN32
	WSACleanup();
#endif
}

bool RtmpBase::InitRtmp()
{
	bool ret = true;
#ifdef _WIN32
	WORD version;
	WSADATA wsa_data;
	version = MAKEWORD(1, 1);
	ret = (WSAStartup(version, &wsa_data) == 0) ? true : false;
#endif
	rtmp_ = RTMP_Alloc();
	RTMP_Init(rtmp_);
	return ret;
}

bool RtmpBase::Connect()
{
	RTMP_Free(rtmp_);
	rtmp_ = RTMP_Alloc();
	RTMP_Init(rtmp_);

	rtmp_->Link.timeout = 10;
	if (RTMP_SetupURL(rtmp_, (char*)url_.c_str()) < 0)
	{
		return false;
	}
	rtmp_->Link.lFlags |= RTMP_LF_LIVE;
	RTMP_SetBufferMS(rtmp_, (60 * 60 * 1000));

	if (rtmp_base_type_ == RTMP_BASE_TYPE_PUSH)
	{
		RTMP_EnableWrite(rtmp_);
	}

	if (!RTMP_Connect(rtmp_, NULL))
	{
		return false;
	}

	if (!RTMP_ConnectStream(rtmp_, 0))
	{
		return false;
	}

	if (rtmp_base_type_ == RTMP_BASE_TYPE_PUSH)
	{
		if (!enable_audio_)
		{
			RTMP_SendReceiveAudio(rtmp_, enable_audio_);
		}
		if (!enable_video_)
		{
			RTMP_SendReceiveVideo(rtmp_, enable_video_);
		}
	}


}

bool RtmpBase::SetReceiveAudio(bool is_recv)
{
	if (enable_audio_ == is_recv)
	{
		return true;
	}
	if (IsConnected())
	{
		if (RTMP_SendReceiveAudio(rtmp_, is_recv))
		{
			return true;
		}
	}
	return false;
}

bool RtmpBase::SetReceiveVideo(bool is_recv)
{
	if (enable_video_ == is_recv)
	{
		return true;
	}
	if (IsConnected())
	{
		if (RTMP_SendReceiveVideo(rtmp_, is_recv))
		{
			return true;
		}
	}
	return false;
}

void RtmpBase::DisConnect()
{
	RTMP_Close(rtmp_);
}

bool RtmpBase::IsConnected()
{
	return RTMP_IsConnected(rtmp_);
}

void RtmpBase::SetUrl(std::string url)
{
	url_ = url;
}