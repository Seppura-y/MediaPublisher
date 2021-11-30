#include "av_data_tools.h"

extern"C"
{
#include <libavcodec/avcodec.h>
}
#ifdef _WIN32
#include <Windows.h>
#include <sysinfoapi.h>

#pragma comment(lib,"avcodec.lib")
#pragma comment(lib,"avutil.lib")

#endif
using namespace std;
using namespace chrono;

int64_t GetCurrentTimeMsec()
{
#ifdef _WIN32
	return (int64_t)GetTickCount();

#else
	return 0;
#endif
}

void SleepForMsec(int ms)
{
	int64_t begin = (int64_t)GetTickCount();
	while (GetTickCount() - begin < ms)
	{
		this_thread::sleep_for(1ms);
	}
}

void FreeFrame(AVFrame** frame)
{
	if (!frame)return;
	av_frame_free(frame);
}

AVParamWarpper* AVParamWarpper::Create()
{
	AVParamWarpper* para = new AVParamWarpper();
	return para;
}
AVParamWarpper::~AVParamWarpper()
{
	if (para)
	{
		avcodec_parameters_free(&para);
	}
	if (time_base)
	{
		delete time_base;
		time_base = nullptr;
	}
}

AVParamWarpper::AVParamWarpper()
{
	para = avcodec_parameters_alloc();
	time_base = new AVRational();
}

AVParamWarpper::AVParamWarpper(AVParamWarpper&& p)
{
	time_base = p.time_base;
	para = p.para;

	p.time_base = nullptr;
	p.para = nullptr;
}


void AVPacketDataList::Push(AVPacket* packet)
{
	unique_lock<mutex> lock(mtx_);
	AVPacket* pkt = av_packet_alloc();
	av_packet_ref(pkt, packet);
	pkt_list_.push_back(pkt);
	if (pkt_list_.size() > max_list_)
	{
		if (pkt_list_.front()->flags & AV_PKT_FLAG_KEY)
		{
			AVPacket* tmp = pkt_list_.front();
			av_packet_unref(tmp);
			av_packet_free(&tmp);
			pkt_list_.pop_front();
			//return;
		}
		while (!pkt_list_.empty())
		{
			if (pkt_list_.front()->flags & AV_PKT_FLAG_KEY)
			{
				return;
			}
			AVPacket* tmp = pkt_list_.front();
			av_packet_unref(tmp);
			av_packet_free(&tmp);
			pkt_list_.pop_front();
		}
	}
}

AVPacket* AVPacketDataList::Pop()
{
	unique_lock<mutex> lock(mtx_);
	if (pkt_list_.empty())
	{
		return nullptr;
	}
	AVPacket* pkt = pkt_list_.front();
	pkt_list_.pop_front();
	return pkt;
}

void AVPacketDataList::Clear()
{
	unique_lock<mutex> lock(mtx_);
	while (!pkt_list_.empty())
	{
		AVPacket* tmp = pkt_list_.front();
		av_packet_unref(tmp);
		av_packet_free(&tmp);
		pkt_list_.pop_front();
	}
	return;
}



void AVFrameDataList::Push(AVFrame* frm)
{
	unique_lock<mutex> lock(mtx_);
	if (frm_list_.size() > max_list_)
	{
		AVFrame* tmp = frm_list_.front();
		av_frame_unref(tmp);
		av_frame_free(&tmp);
		frm_list_.pop_front();
	}
	AVFrame* frame = av_frame_alloc();
	av_frame_ref(frame, frm);
	frm_list_.push_back(frame);
}

AVFrame* AVFrameDataList::Pop()
{
	unique_lock<mutex> lock(mtx_);
	if (frm_list_.empty())
	{
		return nullptr;
	}
	AVFrame* frame = frm_list_.front();
	frm_list_.pop_front();
	return frame;
}

void AVFrameDataList::Clear()
{
	unique_lock<mutex> lock(mtx_);
	if (!frm_list_.empty())
	{
		AVFrame* tmp = frm_list_.front();
		av_frame_unref(tmp);
		av_frame_free(&tmp);
		frm_list_.pop_front();
	}
}


void AVCachedPacketDataList::Push(AVPacket* packet)
{
	unique_lock<mutex> lock(mtx_);
	AVPacket* pkt = av_packet_alloc();
	av_packet_ref(pkt, packet);
	pkt_list_.push_back(pkt);
	if (pkt_list_.size() > max_list_)
	{
		if (pkt_list_.front()->flags & AV_PKT_FLAG_KEY)
		{
			AVPacket* tmp = pkt_list_.front();
			av_packet_unref(tmp);
			av_packet_free(&tmp);
			pkt_list_.pop_front();
			//return;
		}
		while (!pkt_list_.empty())
		{
			if (pkt_list_.front()->flags & AV_PKT_FLAG_KEY)
			{
				return;
			}
			AVPacket* tmp = pkt_list_.front();
			av_packet_unref(tmp);
			av_packet_free(&tmp);
			pkt_list_.pop_front();
		}
	}
}

AVPacket* AVCachedPacketDataList::Pop()
{
	unique_lock<mutex> lock(mtx_);
	if (pkt_list_.empty())
	{
		return nullptr;
	}
	AVPacket* pkt = pkt_list_.front();
	pkt_list_.pop_front();
	return pkt;
}

void AVCachedPacketDataList::Clear()
{
	unique_lock<mutex> lock(mtx_);
	while (!pkt_list_.empty())
	{
		AVPacket* tmp = pkt_list_.front();
		av_packet_unref(tmp);
		av_packet_free(&tmp);
		pkt_list_.pop_front();
	}
	return;
}

int AVCachedPacketDataList::Size()
{
	unique_lock<mutex> lock(mtx_);
	return pkt_list_.size();
}

void AVSharedPacketList::Push(std::shared_ptr<AVPacket> packet)
{

}

std::shared_ptr<AVPacket> AVSharedPacketList::Pop()
{
	shared_ptr<AVPacket> pkt = make_shared<AVPacket>();
	return pkt;
}

void AVSharedPacketList::Clear()
{
	
}

int AVSharedPacketList::Size()
{
	return 0;
}

AVParametersWarpper::AVParametersWarpper()
{
	para = avcodec_parameters_alloc();
	src_time_base = new AVRational();
	dst_time_base = new AVRational();
	src_framerate = new AVRational();
}

AVParametersWarpper::~AVParametersWarpper()
{
	if (para)
	{
		avcodec_parameters_free(&para);
	}
	if (src_time_base)
	{
		delete src_time_base;
	}
	if (dst_time_base)
	{
		delete dst_time_base;
	}
	if (src_framerate)
	{
		delete src_framerate;
	}
}