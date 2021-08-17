#pragma once

#include <QWidget>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QMenu>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>

#include <vector>

#include "ui_preview_widget.h"
#include "capture_widget.h"
#include "element_widget.h"

class PreviewWidget : public QWidget
{
	Q_OBJECT

public:
	PreviewWidget(QWidget *parent = Q_NULLPTR);
	~PreviewWidget();


protected:
	void InitUI();
	void ConnectSigAndSlots();
signals:
	void SigStartPush(CaptureWidgetParameters);
	void SigStopPush();
protected slots:

	void OnPbPushClicked();
	void OnPbStopClicked();
	void OnPbResetClicked();
private:
	Ui::PreviewWidget ui;

	int width_ = -1;
	int height_ = -1;
	QString url_;

	QPushButton* pb_push_ = nullptr;
	QPushButton* pb_stop_ = nullptr;
	QPushButton* pb_reset_ = nullptr;
	QLineEdit* le_width_ = nullptr;
	QLineEdit* le_height_ = nullptr;
	QLineEdit* le_url_ = nullptr;
	QGridLayout* grid_layout_ = nullptr;
	CaptureWidget* capture_widget_ = nullptr;
};
