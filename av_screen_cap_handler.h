#ifndef AV_SCREEN_CAP_HANDLER_H
#define AV_SCREEN_CAP_HANDLER_H

#include "iav_base_handler.h"
#include "av_screen_capturer.h"
class AVScreenCapHandler : public IAVBaseHandler
{
public:
	virtual void Handle(AVHandlerPackage* pkg) override;
	bool Init();
	bool InitScale(int oWidth, int oHeight);

	AVFrame* GetFrame();
	int getInputWidth();
	int getInputHeight();
	void setOutputSize(int width, int height);
protected:
	virtual void Loop() override;

private:
	bool is_need_play_ = false;
	AVScreenCapturer capturer_;
	std::mutex mtx_;

	AVFrame* frame_ = nullptr;

	int64_t start_time_ = 0;
};

#endif