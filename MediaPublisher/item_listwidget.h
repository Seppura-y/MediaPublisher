#pragma once
#include <QListWidget.h>
#include <QListWidgetItem>
#include <QPoint>

class ItemListWidget : public QListWidget
{
	Q_OBJECT

public:
	explicit ItemListWidget(int itemType,QWidget* parent = nullptr);

	explicit ItemListWidget(QWidget* parent = nullptr);

	void mousePressEvent(QMouseEvent* ev) override;

	void mouseMoveEvent(QMouseEvent* ev) override;
public:
	int item_type_ = -1;
private:
	QPoint drag_point_;
	QListWidgetItem* drag_item_ = nullptr;
};

