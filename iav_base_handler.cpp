#include "iav_base_handler.h"
#include <iostream>
#include <sstream>

extern"C"
{
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
}

#ifdef _WIN32
#pragma comment (lib,"avcodec.lib")
#pragma comment (lib,"avutil.lib")

#endif

using namespace std;

void IAVBaseHandler::Start()
{
	static int i = 0;
	unique_lock<mutex> lock(mtx_);

	this->thread_index_ = i++;
	is_exit_ = false;

	worker_ = thread(&IAVBaseHandler::Loop, this);
	cout << "thread "<< this_thread::get_id() <<" : start" << endl;
}


void IAVBaseHandler::Stop()
{
	//unique_lock<mutex> lock(mtx_);
	cout << "thread " << this_thread::get_id() << " : request stop" << endl;
	is_exit_ = true;
	if (worker_.joinable())
	{
		worker_.join();
	}
	cout << "thread " << this_thread::get_id() << " : stop" << endl;
}


void IAVBaseHandler::SetNextHandler(IAVBaseHandler* node)
{
	unique_lock<mutex> lock(mtx_);
	this->next_ = node;
}


IAVBaseHandler* IAVBaseHandler::GetNextHandler()
{
	unique_lock<mutex> lock(mtx_);
	return next_;
}

void IAVBaseHandler::SetPushCallbackFunction(std::function<void(AVPacket*)> fun)
{
	callable_object_ = fun;
}

void IAVBaseHandler::SetCallbackEnable(bool status)
{
	unique_lock<mutex> lock(mtx_);
	is_callback_enable_ = status;
}

int64_t IAVBaseHandler::ScaleToMsec(int64_t duration, AVRational src_timebase)
{
	AVRational dst = { 1,1000 };
	return av_rescale_q(duration, src_timebase, dst);
}