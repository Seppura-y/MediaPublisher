#include "iav_base_handler.h"

#include <iostream>
#include <sstream>
using namespace std;

void IAVBaseHandler::Start()
{
	static int i = 0;
	unique_lock<mutex> lock(mtx_);

	this->thread_index_ = i++;
	is_exit_ = false;

	worker_ = thread(&IAVBaseHandler::Loop, this);
	cout << "thread %d : start" << endl;
}


void IAVBaseHandler::Stop()
{
	//unique_lock<mutex> lock(mtx_);
	cout << "thread %d : request stop" << endl;
	is_exit_ = true;
	if (worker_.joinable())
	{
		worker_.join();
	}
	cout << "thread %d : stop" << endl;
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