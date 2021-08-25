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
private:
	AVParamWarpper();
};

class AVPacketDataList
{
public:

	void Push(AVPacket* packet);
	AVPacket* Pop();
	void Clear();
private:
	std::list<AVPacket*> pkt_list_;
	int max_list_ = 100;
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
	int max_list_ = 100;
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
	int max_list_ = 100;
	std::mutex mtx_;
};
#endif
