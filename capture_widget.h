#pragma once

#include <QWidget>
#include <QVBoxLayout>

#include "av_screen_cap_handler.h"
#include "av_encode_handler.h"
#include "av_mux_handler.h"
#include "sdl_view.h"

struct CaptureWidgetParameters
{
	int output_width_;
	int output_height_;
	QString url_;
};

class CaptureWidget : public QWidget
{
	Q_OBJECT

public:
	CaptureWidget(QWidget *parent);
	~CaptureWidget();

public slots:
	void OnSignalPush(CaptureWidgetParameters);
	void OnParametersSet(CaptureWidgetParameters);
	void OnSignalStop();
protected:
	void paintEvent(QPaintEvent* ev) override;
	void timerEvent(QTimerEvent* ev)override;
	void resizeEvent(QResizeEvent* ev)override;

	void DrawFrame();
private:
	int output_width_ = -1;
	int output_height_ = -1;
	QString url_;

	IVideoView* view_ = nullptr;
	AVMuxHandler* mux_handler_ = nullptr;
	AVEncodeHandler* encode_handler_ = nullptr;
	AVScreenCapHandler* capture_handler_ = nullptr;

};
