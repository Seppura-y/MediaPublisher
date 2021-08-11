#include "capture_widget.h"

#include <QStyleOption>
#include <QPainter>
#include <QDebug>

#include <thread>

extern"C"
{
#include <libavcodec/avcodec.h>
}

#ifdef _WIN32
#pragma comment(lib,"avcodec.lib")
#endif

using namespace std;

CaptureWidget::CaptureWidget(QWidget *parent)
	: QWidget(parent)
{
	this->setStyleSheet("background-color: rgb(55,55,55)");
	view_ = IVideoView::CreateView(RenderType::RENDER_TYPE_SDL);
	view_->InitView(this->width(), this->height(), PixFormat::PIX_FORMAT_YUV420P, (void*)this->winId());

	bool ret = true;
	capture_handler_ = new AVScreenCapHandler();
	ret = capture_handler_->Init();
	if (!ret)
	{
		qDebug() << "capture_th_->Init() failed";
		return;
	}

	output_width_ = capture_handler_->getInputWidth();
	output_height_ = capture_handler_->getInputHeight();
	url_ = QString("rtmp://107.172.153.24/live/livestream");

	ret = capture_handler_->InitScale(output_width_, output_height_);
	if (!ret)
	{
		qDebug() << "capture_th_->InitScale(inWidth, inHeight) failed";
		return;
	}

	encode_handler_ = new AVEncodeHandler();
	ret = encode_handler_->EncoderInit(output_width_, output_height_);
	if (ret != 0)
	{
		qDebug() << "encode_th_->Open(inWidth, inHeight) failed";
		return;
	}
	auto param = encode_handler_->CopyCodecParameters();

	mux_handler_ = new AVMuxHandler();

	encode_handler_->SetNextHandler(mux_handler_);
	capture_handler_->SetNextHandler(encode_handler_);

	encode_handler_->Start();
	this_thread::sleep_for(50ms);

	capture_handler_->Start();
	startTimer(1);
}

CaptureWidget::~CaptureWidget()
{

}

void CaptureWidget::paintEvent(QPaintEvent* ev)
{
	QStyleOption opt;
	opt.init(this);
	QPainter painter(this);
	style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);
}

void CaptureWidget::timerEvent(QTimerEvent* ev)
{
	this->DrawFrame();
}

void CaptureWidget::resizeEvent(QResizeEvent* ev)
{
	if (view_)
	{
		view_->ResetView();
		delete view_;
		view_ = nullptr;
	}
	view_ = IVideoView::CreateView(RenderType::RENDER_TYPE_SDL);
	view_->InitView(output_width_, output_height_, PixFormat::PIX_FORMAT_YUV420P, (void*)this->winId());
}

void CaptureWidget::DrawFrame()
{
	if (!view_ || !capture_handler_)
	{
		return;
	}
	AVFrame* frame = capture_handler_->GetFrame();
	if (!frame)
	{
		return;
	}
	view_->DrawFrame(frame);
	av_frame_unref(frame);
	av_frame_free(&frame); //If the frame is reference counted, it will be unreferenced first.
}

void CaptureWidget::OnParametersSet(CaptureWidgetParameters param)
{
	output_width_ = param.output_width_;
	output_height_ = param.output_height_;
	url_ = param.url_;
}

void CaptureWidget::OnSignalPush(CaptureWidgetParameters param)
{
	output_width_ = param.output_width_;
	output_height_ = param.output_height_;
	url_ = param.url_;
	if (!encode_handler_ || !mux_handler_)
	{
		qDebug() << "start push faield : (!encode_handler_ || !mux_handler_)";
		return;
	}
	encode_handler_->SetEncodePause(false);
	auto codec_param = encode_handler_->CopyCodecParameters();
	int extra_data_size = 0;
	uint8_t extra_data[4096] = { 0 };
	int ret = encode_handler_->CopyCodecExtraData(extra_data,extra_data_size);

	ret = mux_handler_->Open(url_.toStdString(), codec_param->para, codec_param->time_base, nullptr, nullptr, extra_data,extra_data_size);
	if (ret != 0)
	{
		qDebug() << "mux_handler_ open failed";
		return;
	}

	mux_handler_->Start();
}

void CaptureWidget::OnSignalStop()
{
	if (!encode_handler_ || !mux_handler_)
	{
		return;
	}
	encode_handler_->SetEncodePause(true);
	mux_handler_->Stop();
}