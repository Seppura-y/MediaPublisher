#ifndef SHOW_WIDGET_H
#define SHOW_WIDGET_H

#include <QWidget>

#include "ui_show_widget.h"
#include "preview_widget.h"

class ShowWidget : public QWidget
{
	Q_OBJECT

public:
	ShowWidget(QWidget *parent = Q_NULLPTR);
	~ShowWidget();

protected:
	//void contextMenuEvent(QContextMenuEvent* ev) override;
	int InitUi();
private:
	Ui::ShowWidget ui;

	PreviewWidget* wid_preview_ = nullptr;
};

#endif
