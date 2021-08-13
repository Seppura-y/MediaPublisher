#include "base_handler.h"
#include <iostream>

using namespace std;
BaseHandler::BaseHandler(int max_queue_size)
{
	unique_lock<mutex> lock(mtx_);
	max_queue_size_ = max_queue_size;
	msg_avaliable_ = new Semaphore();
	worker_ = new std::thread(&BaseHandler::Trampoline,this);
	is_exit_ = false;
}

BaseHandler::~BaseHandler()
{
	cout << "Handler Deconstruct" << endl;
	Stop();
}

void BaseHandler::Trampoline(void* p)
{
	((BaseHandler*)p)->Loop();
}

void BaseHandler::Loop()
{
	cout << "BaseHandler::Loop()" << endl;
	MessagePayload* msg;
	while (1)
	{
		mtx_.lock();
		if (message_queue_.size() > 0)
		{
			msg = message_queue_.front();
			message_queue_.pop_front();
			mtx_.unlock();

			if (msg->is_exit)
			{
				break;
			}

			Handle(msg->what, msg->message);
		}
		else
		{
			mtx_.unlock();
			msg_avaliable_->Wait();
			this_thread::sleep_for(5ms);
		}
	}
}

void BaseHandler::Post(MessagePayloadType what,void* data,bool flush)
{
	MessagePayload* msg = new MessagePayload();
	msg->what = what;
	msg->message = (MessageBase*)data;
	msg->is_exit = false;
	AddMsg(msg, flush);
}

void BaseHandler::Stop()
{
	if (!is_exit_)
	{
		MessagePayload* msg = new MessagePayload();
		msg->what = MessagePayloadType::MESSAGE_PAYLOAD_TYPE_UNDEFINE;
		msg->message = nullptr;
		msg->is_exit = true;
		AddMsg(msg, true);
		if (worker_)
		{
			if (worker_->joinable())
			{
				worker_->join();
				delete worker_;
				worker_ = nullptr;
			}
		}
		is_exit_ = true;
	}
}

void BaseHandler::AddMsg(MessagePayload* obj, bool flush)
{
	MessagePayload* msg;
	mtx_.lock();
	if (flush || message_queue_.size() > max_queue_size_)
	{
		cout << "BaseHandler::AddMsg flush queue" << endl;
		while (message_queue_.size() > 0)
		{
			msg = message_queue_.front();
			message_queue_.pop_front();
			delete msg->message;
			delete msg;
		}
	}
	message_queue_.push_back(obj);
	mtx_.unlock();

	if (MessagePayloadType::MESSAGE_PAYLOAD_TYPE_METADATA == obj->what)
		cout << "metadata" << endl;
	msg_avaliable_->Post(1);

}

void BaseHandler::Handle(MessagePayloadType what,MessageBase* obj)
{
	cout << "base handle" << endl;
}