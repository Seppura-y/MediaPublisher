#include "capture_widget.h"

#include <QStyleOption>
#include <QPainter>
#include <QDebug>
#include <thread>

extern"C"
{
#include <libavcodec/avcodec.h>
}

#include "avtimebase.h"
#include "message_base.h"

#ifdef _WIN32
#pragma comment(lib,"avcodec.lib")
#endif

using namespace std;

CaptureWidget::CaptureWidget(QWidget *parent) : QWidget(parent)
{
	//view_ = IVideoView::CreateView(RenderType::RENDER_TYPE_SDL);
	this->setStyleSheet("background-color: rgb(55,55,55)");
	//view_ = IVideoView::CreateView(RenderType::RENDER_TYPE_SDL);
	//view_->InitView(this->width(), this->height(), PixFormat::PIX_FORMAT_YUV420P, (void*)this->winId());

	//bool ret = true;
	//capture_handler_ = new AVScreenCapHandler();
	//ret = capture_handler_->Init();
	//if (!ret)
	//{
	//	qDebug() << "capture_th_->Init() failed";
	//	return;
	//}

	//output_width_ = capture_handler_->getInputWidth();
	//output_height_ = capture_handler_->getInputHeight();
	//url_ = QString("rtmp://107.172.153.24/live/livestream");

	//ret = capture_handler_->InitScale(output_width_, output_height_);
	//if (!ret)
	//{
	//	qDebug() << "capture_th_->InitScale(inWidth, inHeight) failed";
	//	return;
	//}

	//encode_handler_ = new AVEncodeHandler();
	//ret = encode_handler_->EncoderInit(output_width_, output_height_);
	//if (ret != 0)
	//{
	//	qDebug() << "encode_th_->Open(inWidth, inHeight) failed";
	//	return;
	//}
	//capture_handler_->SetNextHandler(encode_handler_);

	//encode_handler_->SetPushCallbackFunction(std::bind(&CaptureWidget::VideoEncodeCallback,this,std::placeholders::_1));
	//encode_handler_->SetCallbackEnable(true);
	//encode_handler_->Start();
	//this_thread::sleep_for(50ms);

	//capture_handler_->Start();
	startTimer(1);

	//encode_handler_->SetEncodePause(true);
	//mux_handler_ = new AVMuxHandler();

	//encode_handler_->SetNextHandler(mux_handler_);
	//capture_handler_->SetNextHandler(encode_handler_);

	//encode_handler_->Start();
	//this_thread::sleep_for(50ms);

	//capture_handler_->Start();
	//startTimer(1);
}

CaptureWidget::~CaptureWidget()
{
	if (rtmp_pusher_)
	{
		rtmp_pusher_->Stop();
	}
	if (mux_handler_)
	{
		mux_handler_->Stop();
	}
	if (encode_handler_)
	{
		encode_handler_->Stop();
	}
	if (capture_handler_)
	{
		capture_handler_->Stop();
	}
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

void CaptureWidget::OnResetParam(CaptureWidgetParameters param)
{
	qDebug() << "on reset param";
	output_width_ = param.output_width_;
	output_height_ = param.output_height_;
	url_ = param.url_;
	bool ret = false;
	////////////////////////////////////////////////////////////////////////////////////////////////////////
	//reset view according to output dimension
	if (view_)
	{
		view_->ResetView();
	}
	else if(!view_)
	{
		view_ = IVideoView::CreateView(RenderType::RENDER_TYPE_SDL);
	}
	view_->InitView(output_width_, output_height_, PixFormat::PIX_FORMAT_YUV420P, (void*)this->winId());
	if (output_width_ > this->width() || output_height_ > this->height())
	{
		view_->ScaleView(this->width(), this->height());
	}
	else
	{
		view_->ScaleView(output_width_, output_height_);
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////
	//reset caputure handler  according to output dimension
	if (!capture_handler_)
	{
		capture_handler_ = new AVScreenCapHandler();
		ret = capture_handler_->Init();
		if (!ret)
		{
			qDebug() << "capture_th_->Init() failed";
			return;
		}
	}
	else
	{
		capture_handler_->Stop();
	}
	ret = capture_handler_->InitScale(output_width_, output_height_);
	if (!ret)
	{
		qDebug() << "capture_th_->InitScale(inWidth, inHeight) failed";
		return;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////
	//reset encode handler according to output dimension
	if (!encode_handler_)
	{
		encode_handler_ = new AVEncodeHandler();
	}
	else
	{
		encode_handler_->Stop();
	}
	ret = encode_handler_->EncoderInit(output_width_, output_height_);
	if (ret != 0)
	{
		qDebug() << "encode_th_->Open(inWidth, inHeight) failed";
		return;
	}
	encode_handler_->SetEncodePause(true);
	capture_handler_->SetNextHandler(encode_handler_);

	////////////////////////////////////////////////////////////////////////////////////////////////////////
	//reset mux handler(ffmpeg) or rtmp_pusher(librtmp) according to encode parameters
	if (is_librtmp_method_)
	{
		if (!rtmp_pusher_)
		{
			rtmp_pusher_ = new RtmpPusher(RtmpBaseType::RTMP_BASE_TYPE_PUSH,url_.toStdString());
		}
		encode_handler_->SetPushCallbackFunction(std::bind(&CaptureWidget::VideoEncodeCallback, this, std::placeholders::_1));
		encode_handler_->SetCallbackEnable(true);
	}
	else
	{
		encode_handler_->SetCallbackEnable(false);
		if (!mux_handler_)
		{
			mux_handler_ = new AVMuxHandler();
		}
		else
		{
			mux_handler_->Stop();
		}
		auto codec_param = encode_handler_->CopyCodecParameters();
		int extra_data_size = 0;
		uint8_t extra_data[4096] = { 0 };
		ret = encode_handler_->CopyCodecExtraData(extra_data, extra_data_size);

		ret = mux_handler_->Open(url_.toStdString(), codec_param->para, codec_param->time_base, nullptr, nullptr, extra_data, extra_data_size);
		if (ret != 0)
		{
			qDebug() << "mux_handler_ open failed";
			return;
		}
	}
	//capture_handler_->Start();
}


void CaptureWidget::OnStartPush()
{
	if (capture_handler_)
	{
		if (capture_handler_->IsExit())
		{
			capture_handler_->Start();
		}
	}
	if (encode_handler_)
	{
		encode_handler_->SetEncodePause(false);
		if (encode_handler_->IsExit())
		{
			encode_handler_->Start();
		}
	}

	if (!is_librtmp_method_)
	{
		if (mux_handler_->IsExit())
		{
			mux_handler_->Start();
		}
	}
}

void CaptureWidget::OnStopPush()
{
	if (!encode_handler_ || !mux_handler_ || !rtmp_pusher_ || capture_handler_)
	{
		return;
	}
	encode_handler_->SetEncodePause(true);
	if (!encode_handler_->IsExit())
	{
		encode_handler_->Stop();
	}
	if (!mux_handler_->IsExit())
	{
		mux_handler_->Stop();
	}
}


void CaptureWidget::VideoEncodeCallback(AVPacket* v_pkt)
{
	if (IsVideoSeqHeaderNeeded())
	{
		VideoSequenceHeaderMessage* video_seq_header = new VideoSequenceHeaderMessage(
			encode_handler_->GetSpsData(),
			encode_handler_->GetSpsSize(),
			encode_handler_->GetPpsData(),
			encode_handler_->GetPpsSize()
		);
		//video_seq_header->width_ = output_width_;
		//video_seq_header->height_ = output_height_;

		rtmp_pusher_->Post(MessagePayloadType::MESSAGE_PAYLOAD_TYPE_VIDEO_SEQ,
			video_seq_header, false);

		SetVideoSeqHeaderNeeded(false);
	}

	NALUStruct* nalu = new NALUStruct(v_pkt->data, v_pkt->size);
	nalu->pts_ = AVPublishTime::GetInstance()->GetVideoPts();
	rtmp_pusher_->Post(MessagePayloadType::MESSAGE_PAYLOAD_TYPE_NALU,
		nalu, false);
}

void CaptureWidget::AudioEncodeCallback(AVPacket* a_pkt)
{

}

bool CaptureWidget::IsVideoSeqHeaderNeeded()
{
	return is_video_seq_header_needed_;
}

bool CaptureWidget::IsAudioSeqHeaderNeeded()
{
	return is_audio_seq_header_needed_;
}

void CaptureWidget::SetVideoSeqHeaderNeeded(bool status)
{
	is_video_seq_header_needed_ = status;
}

void CaptureWidget::SetAudioSeqHeaderNeeded(bool status)
{
	is_audio_seq_header_needed_ = status;
}

void CaptureWidget::OnSetIsLibrtmpMethod(bool status)
{
	is_librtmp_method_ = status;
}

//void CaptureWidget::OnStartPush(/*CaptureWidgetParameters param*/)
//{
//	//encode_handler_->Start();
//	//this_thread::sleep_for(50ms);
//
//	//capture_handler_->Start();
//	//startTimer(1);
//
//	//int ret = -1;
//	//output_width_ = param.output_width_;
//	//output_height_ = param.output_height_;
//	//url_ = param.url_;
//	//view_->InitView(output_width_, output_height_, PixFormat::PIX_FORMAT_YUV420P, (void*)this->winId());
//	//if (output_width_ > this->width() || output_height_ > this->height())
//	//{
//	//	view_->ScaleView(this->width(), this->height());
//	//}
//	//else
//	//{
//	//	view_->ScaleView(output_width_, output_height_);
//	//}
//
//	//if (!encode_handler_ || !mux_handler_)
//	//{
//	//	qDebug() << "start push failed : (!encode_handler_ || !mux_handler_)";
//	//	return;
//	//}
//
//	//ret = capture_handler_->InitScale(output_width_, output_height_);
//	//if (!ret)
//	//{
//	//	qDebug() << "capture_th_->InitScale(inWidth, inHeight) failed";
//	//	return;
//	//}
//
//	//encode_handler_->Stop();
//	//ret = encode_handler_->EncoderInit(output_width_, output_height_);
//	//if (ret != 0)
//	//{
//	//	qDebug() << "encode_handler_->EncoderInit() failed";
//	//	return;
//	//}
//	//encode_handler_->Start();
//
//	//encode_handler_->SetEncodePause(false);
//	//auto codec_param = encode_handler_->CopyCodecParameters();
//	//int extra_data_size = 0;
//	//uint8_t extra_data[4096] = { 0 };
//	//ret = encode_handler_->CopyCodecExtraData(extra_data, extra_data_size);
//
//	//ret = mux_handler_->Open(url_.toStdString(), codec_param->para, codec_param->time_base, nullptr, nullptr, extra_data, extra_data_size);
//	//if (ret != 0)
//	//{
//	//	qDebug() << "mux_handler_ open failed";
//	//	return;
//	//}
//
//	//mux_handler_->Start();
//}