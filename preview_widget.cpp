#include "preview_widget.h"
#include <QDebug>
#include <QRadioButton>

PreviewWidget::PreviewWidget(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	InitUI();
	ConnectSigAndSlots();
}

PreviewWidget::~PreviewWidget()
{

}

void PreviewWidget::InitUI()
{
	pb_push_ = ui.pb_push;
	pb_stop_ = ui.pb_stop;
	pb_reset_ = ui.pb_reset;
	le_width_ = ui.le_width;
	le_height_ = ui.le_height;
	le_url_ = ui.le_url;
	capture_widget_ = ui.el_wid_show;

	pb_push_->setEnabled(true);
	pb_stop_->setEnabled(false);
	le_width_->setText("1280");
	le_height_->setText("720");
	//le_url_->setText("rtmp://107.172.153.24/live/livestream");
	le_url_->setText("rtmp://192.168.1.152/live/livestream1");

	width_ = le_width_->text().toInt();
	height_ = le_width_->text().toInt();
	url_ = le_url_->text();

	capture_widget_->setAcceptDrops(false);

	ui.rb_ffmpeg->setChecked(true);
}

void PreviewWidget::ConnectSigAndSlots()
{
	QObject::connect(pb_push_, &QPushButton::clicked, this, &PreviewWidget::OnPbPushClicked);
	QObject::connect(pb_stop_, &QPushButton::clicked, this, &PreviewWidget::OnPbStopClicked);
	QObject::connect(pb_reset_, &QPushButton::clicked, this, &PreviewWidget::OnPbResetClicked);

	QObject::connect(this, &PreviewWidget::SigStartPush, capture_widget_, &CaptureWidget::OnStartPush);
	QObject::connect(this, &PreviewWidget::SigStopPush, capture_widget_, &CaptureWidget::OnStopPush);
	QObject::connect(this, &PreviewWidget::SigResetParam, capture_widget_, &CaptureWidget::OnResetParam);

	QObject::connect(ui.rb_ffmpeg, &QRadioButton::clicked, this, &PreviewWidget::OnRbFfmpegClicked);
	QObject::connect(ui.rb_librtmp, &QRadioButton::clicked, this, &PreviewWidget::OnRbLibrtmpClicked);
	QObject::connect(this, &PreviewWidget::SigSetIsLibRtmpMethod, capture_widget_, &CaptureWidget::OnSetIsLibrtmpMethod);
}

void PreviewWidget::OnPbPushClicked()
{
	//struct CaptureWidgetParameters param;
	//param.output_width_ = le_width_->text().toInt();
	//param.output_height_ = le_height_->text().toInt();
	//param.url_ = le_url_->text();
	//emit SigStartPush(param);
	pb_push_->setEnabled(false);
	pb_stop_->setEnabled(true);

	emit SigStartPush();
}

void PreviewWidget::OnPbStopClicked()
{
	emit SigStopPush();
	pb_push_->setEnabled(false);
	pb_stop_->setEnabled(false);
}

void PreviewWidget::OnPbResetClicked()
{
	qDebug() << "on pb reset clicked";
	struct CaptureWidgetParameters param;
	param.output_width_ = le_width_->text().toInt();
	param.output_height_ = le_height_->text().toInt();
	param.url_ = le_url_->text();
	pb_push_->setEnabled(true);
	emit SigResetParam(param);
}

void PreviewWidget::OnRbFfmpegClicked()
{
	emit SigSetIsLibRtmpMethod(false);
}

void PreviewWidget::OnRbLibrtmpClicked()
{
	emit SigSetIsLibRtmpMethod(true);
}