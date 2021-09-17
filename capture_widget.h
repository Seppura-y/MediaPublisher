#pragma once

#include <QWidget>
#include <QVBoxLayout>

#include "av_screen_cap_handler.h"
#include "av_encode_handler.h"
#include "av_mux_handler.h"
#include "sdl_view.h"
#include "rtmp_pusher.h"

struct CaptureWidgetParameters
{
	int output_width_;
	int output_height_;
	QString url_;
};
struct AVPacket;

class CaptureWidget : public QWidget
{
	Q_OBJECT

public:
	CaptureWidget(QWidget *parent);
	~CaptureWidget();

public slots:
	void OnStartPush();
	void OnResetParam(CaptureWidgetParameters);
	void OnStopPush();

	void OnSetIsLibrtmpMethod(bool);


protected:
	void paintEvent(QPaintEvent* ev) override;
	void timerEvent(QTimerEvent* ev)override;
	void resizeEvent(QResizeEvent* ev)override;

	void DrawFrame();

	void VideoEncodeCallback(AVPacket* v_pkt);
	void AudioEncodeCallback(AVPacket* a_pkt);

	bool IsVideoSeqHeaderNeeded();
	bool IsAudioSeqHeaderNeeded();
	void SetVideoSeqHeaderNeeded(bool status);
	void SetAudioSeqHeaderNeeded(bool status);
private:
	bool is_librtmp_method_ = false;
	bool is_need_draw_ = false;
	int output_width_ = -1;
	int output_height_ = -1;
	QString url_;

	IVideoView* view_ = nullptr;
	AVMuxHandler* mux_handler_ = nullptr;
	AVEncodeHandler* encode_handler_ = nullptr;
	AVScreenCapHandler* capture_handler_ = nullptr;

	bool is_video_seq_header_needed_ = true;
	bool is_audio_seq_header_needed_ = true;
	RtmpPusher* rtmp_pusher_ = nullptr;

	FlvOnMetaData flv_on_metadata_;
};
