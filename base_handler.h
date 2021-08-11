#ifndef BASE_HANDLER_H
#define BASE_HANDLER_H



#include <thread>
#include <deque>

#include "message_base.h"
#include "semaphore.h"

class BaseHandler
{
public:
	BaseHandler(int max_queue_size);
	virtual ~BaseHandler();
	void Post(int what,void* data,bool flush);
	void Stop();

protected:
	bool is_exit_ = true;
	int max_queue_size_ = 30;
	std::thread* worker_;
	std::mutex mtx_;
	std::deque<MessagePayload*> message_queue_;
	Semaphore* msg_avaliable_;

private:
	virtual void AddMsg(MessagePayload* obj, bool flush);
	virtual void Handle(int what, MessageBase* obj);
	static void Trampoline(void* p);
	void Loop();
};

#endif