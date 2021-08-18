#ifndef IAV_BASE_HANDLER_H
#define IAV_BASE_HANDLER_H

#include <thread>
#include <mutex>
#include <functional>

#include "av_data_tools.h"

struct AVPacket;

enum class AVHandlerPackageType
{
	AVHANDLER_PACKAGE_TYPE_PACKET,
	AVHANDLER_PACKAGE_TYPE_FRAME
};

enum class AVHandlerPackageAVType
{
	AVHANDLER_PACKAGE_AV_TYPE_AUDIO,
	AVHANDLER_PACKAGE_AV_TYPE_VIDEO
};

typedef struct AVHandlerPackage
{
	AVHandlerPackageAVType av_type_;
	AVHandlerPackageType type_;
	union AVHandlerPackagePayload
	{
		AVPacket* packet_;
		AVFrame* frame_;
	} payload_;
} AVHandlerPackage;


class IAVBaseHandler
{
public:
	IAVBaseHandler() {}
	virtual ~IAVBaseHandler() {}

	virtual void Start();
	virtual void Stop();
	virtual void Handle(AVHandlerPackage* package) = 0;

	void SetNextHandler(IAVBaseHandler* node);
	void SetPushCallbackFunction(std::function<void(AVPacket*)> fun);
	void SetCallbackEnable(bool status);

	bool IsExit() { return is_exit_; };
protected:
	virtual void Loop() = 0;
	IAVBaseHandler* GetNextHandler();


protected:
	std::mutex mtx_;
	std::thread worker_;
	std::function<void(AVPacket*)> callable_object_ = nullptr;
	bool is_callback_enable_ = false;
	IAVBaseHandler* next_ = nullptr;

	bool is_exit_ = true;
	int thread_index_ = 0;

private:
};

#endif