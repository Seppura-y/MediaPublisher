#include "av_demux_handler.h"

#include <iostream>

extern"C"
{
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
}

#ifdef _WIN32
#pragma comment (lib,"avformat.lib")
#pragma comment (lib,"avutil.lib")

#endif

using namespace std;
bool AVDemuxHandler::OpenAVSource(const char* url,int timeout)
{
	demuxer_.CloseContext();
	AVFormatContext* fmt_ctx = nullptr;

	fmt_ctx = demuxer_.OpenContext(url);
	demuxer_.SetFormatContext(fmt_ctx);
	url_ = url;
	if (strstr(url, "mp4"))
	{
		return true;
	}
	demuxer_.SetTimeout(timeout);
	return true;
}


void AVDemuxHandler::Loop()
{
	AVPacket* pkt = av_packet_alloc();
	AVHandlerPackage* payload = new AVHandlerPackage();
	while (!is_exit_)
	{
		if (demuxer_.Read(pkt) >= 0)
		{
			payload->type_ = AVHandlerPackageType::AVHANDLER_PACKAGE_TYPE_PACKET;
			payload->payload_.packet_ = av_packet_alloc();
			av_packet_ref(payload->payload_.packet_,pkt);
			GetNextHandler()->Handle(payload);
			av_packet_unref(pkt);
		}
		if (!demuxer_.is_network_connected())
		{
			demuxer_.OpenContext(url_.c_str());
		}
	}
	av_packet_free(&pkt);
	delete payload;
}


void AVDemuxHandler::Handle(AVHandlerPackage* pkt)
{
	//first node : nothing to do
	return;
}

void AVDemuxHandler::Stop()
{
	IAVBaseHandler::Stop();
	demuxer_.CloseContext();
}


std::shared_ptr<AVParamWarpper> AVDemuxHandler::CopyVideoParameters()
{
	return demuxer_.CopyVideoParameters();
}

std::shared_ptr<AVParamWarpper> AVDemuxHandler::CopyAudioParameters()
{
	return demuxer_.CopyAudioParameters();
}