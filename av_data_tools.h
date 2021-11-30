#ifndef AV_DATA_TOOLS_H
#define AV_DATA_TOOLS_H


#include <list>
#include <iostream>

#include <mutex>

struct AVFrame;
struct AVPacket;
struct AVCodecParameters;
struct AVRational;

enum LogLevel
{
	Log_Info,
	Log_Debug,
	Log_Error,
	Log_Fatal
};
#define MIN_LOG_LEVEL Log_Info
#define LOG(s,level) if(level >= Log_Info)\
std::cout<<"level_" << level<<":"<<"file : "<<__FILE__<<" "<<"line : "<<__LINE__<<": "\
<<s<<"\n"<<std::endl;

#define LOGINFO(s)     LOG(s,Log_Info);
#define LOGDEBUG(s)  LOG(s,Log_Debug);
#define LOGERROR(s)   LOG(s,Log_Error);
#define LOGFATAL(s)	LOG(s,Log_Fatal);


int64_t GetCurrentTimeMsec();
void SleepForMsec(int ms);
void FreeFrame(AVFrame** frame);

class AVParamWarpper
{
public:
	AVCodecParameters* para = nullptr;
	AVRational* time_base = nullptr;
	static AVParamWarpper* Create();
	~AVParamWarpper();
//private:
	AVParamWarpper();

	AVParamWarpper(const AVParamWarpper&) = delete;
	AVParamWarpper operator=(const AVParamWarpper&) = delete;
	AVParamWarpper(AVParamWarpper&& p);
};

class AVPacketDataList
{
public:

	void Push(AVPacket* packet);
	AVPacket* Pop();
	void Clear();
private:
	std::list<AVPacket*> pkt_list_;
	int max_list_ = 500;
	std::mutex mtx_;
};

class AVFrameDataList
{
public:
	void Push(AVFrame* frame);
	AVFrame* Pop();
	void Clear();

private:
	std::list<AVFrame*> frm_list_;
	int max_list_ = 500;
	std::mutex mtx_;
};


class AVCachedPacketDataList
{
public:

	void Push(AVPacket* packet);
	AVPacket* Pop();
	void Clear();
	int Size();
private:
	std::list<AVPacket*> pkt_list_;
	int max_list_ = 500;
	std::mutex mtx_;
};


class AVParametersWarpper
{
public:
	AVParametersWarpper();
	~AVParametersWarpper();

	AVParametersWarpper(const AVParametersWarpper&) = delete;
	AVParametersWarpper operator=(const AVParametersWarpper&) = delete;
	//AVParametersWarpper(AVParametersWarpper&& p);

	int dst_sample_fmt_ = 0;
	int dst_nb_samples_ = 0;
	int dst_channel_layout_ = 0;
	int dst_sample_rate_ = 0;

	int pix_fmt_ = 0;
	int dst_width_ = 0;
	int dst_height_ = 0;

	AVCodecParameters* para = nullptr;
	AVRational* src_time_base = nullptr;
	AVRational* dst_time_base = nullptr;
	AVRational* src_framerate = nullptr;
};

class AVSharedPacketList
{
public:
	void Push(std::shared_ptr<AVPacket> packet);
	std::shared_ptr<AVPacket> Pop();
	void Clear();
	int Size();
private:
	std::list<std::shared_ptr<AVPacket>> shared_pkt_list_;
};
#endif
