#include "preview_widget.h"
#include <QDebug>

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
	le_width_ = ui.le_width;
	le_height_ = ui.le_height;
	le_url_ = ui.le_url;
	capture_widget_ = ui.el_wid_show;

	pb_push_->setEnabled(true);
	pb_stop_->setEnabled(false);
	le_width_->setText("1920");
	le_height_->setText("1080");
	le_url_->setText("rtmp://107.172.153.24/live/livestream");
	

	width_ = le_width_->text().toInt();
	height_ = le_width_->text().toInt();
	url_ = le_url_->text();

	capture_widget_->setAcceptDrops(false);
}

void PreviewWidget::ConnectSigAndSlots()
{
	QObject::connect(pb_push_, &QPushButton::clicked, this, &PreviewWidget::OnPbPushClicked);
	QObject::connect(pb_stop_, &QPushButton::clicked, this, &PreviewWidget::OnPbStopClicked);

	QObject::connect(this, &PreviewWidget::SigStartPush, capture_widget_, &CaptureWidget::OnSignalPush);
	QObject::connect(this, &PreviewWidget::SigStopPush, capture_widget_, &CaptureWidget::OnSignalStop);
}

void PreviewWidget::OnPbPushClicked()
{
	qDebug() << "OnPbPushClicked";
	struct CaptureWidgetParameters param;
	param.output_width_ = le_width_->text().toInt();
	param.output_height_ = le_height_->text().toInt();
	param.url_ = le_url_->text();
	emit SigStartPush(param);
	pb_push_->setEnabled(false);
	pb_stop_->setEnabled(true);

}

void PreviewWidget::OnPbStopClicked()
{
	emit SigStopPush();
	pb_push_->setEnabled(true);
	pb_stop_->setEnabled(false);
}

void PreviewWidget::OnPbResetClicked()
{

}