#pragma once

#include <QWidget>
#include <QMenu>
#include <QComboBox>
#include <QGridLayout>

#include <vector>

#include "element_widget.h"
#include "ui_file_preview_widget.h"

class FilePreviewWidget : public QWidget
{
	Q_OBJECT

public:
	FilePreviewWidget(QWidget *parent = Q_NULLPTR);
	~FilePreviewWidget();

protected slots:
	void OnSignalSetWidgets(QString index);
	void OnSignalWidgetDestroyed(int index);
protected:
	void InitUi();
	void SetWidgets(int count);

	void contextMenuEvent(QContextMenuEvent* ev) override;

private:
	Ui::FilePreviewWidget ui;

	QMenu menu_;
	QWidget* central_widget_ = nullptr;
	QComboBox* cb_widget_set_ = nullptr;
	QGridLayout* grid_widgets_layout_ = nullptr;

	int current_widgets_count_ = -1;

	std::vector<ElementWidget*> element_widgets_list_;
};
