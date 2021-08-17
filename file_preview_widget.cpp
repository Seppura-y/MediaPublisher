#include "file_preview_widget.h"
#include <QDebug>

FilePreviewWidget::FilePreviewWidget(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	central_widget_ = ui.central_widget;
	cb_widget_set_ = ui.cb_widget_set;

	grid_widgets_layout_ = new QGridLayout();
	grid_widgets_layout_->setSpacing(1);
	grid_widgets_layout_->setContentsMargins(0,0,0,0);
	central_widget_->setLayout(grid_widgets_layout_);

	element_widgets_list_.resize(25);
	
	InitUi();
}

FilePreviewWidget::~FilePreviewWidget()
{

}

void FilePreviewWidget::InitUi()
{
	cb_widget_set_->addItem("1");
	cb_widget_set_->addItem("4");
	cb_widget_set_->addItem("9");
	cb_widget_set_->addItem("16");
	cb_widget_set_->addItem("25");
	
	//QObject::connect(cb_widget_set_, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &FilePreviewWidget::OnSetWidgets);
	QObject::connect(cb_widget_set_, &QComboBox::currentTextChanged, this, &FilePreviewWidget::OnSetWidgets);

	SetWidgets(cb_widget_set_->currentText().toInt());
}

void FilePreviewWidget::contextMenuEvent(QContextMenuEvent* ev)
{

}

void FilePreviewWidget::OnSetWidgets(QString index)
{
	qDebug() << "on set widget";
	int n = index.toInt();
	SetWidgets(n);
}

void FilePreviewWidget::SetWidgets(int count)
{
	int row = sqrt(count);
	int remain = element_widgets_list_.size();

	for (int i = 0; i < count; i++)
	{
		if (!element_widgets_list_[i])
		{
			element_widgets_list_[i] = new ElementWidget(i);
			//connect slots and init
			
			element_widgets_list_[i]->setStyleSheet("background-color:rgb(62,62,62)");
		}
		grid_widgets_layout_->addWidget(element_widgets_list_[i], i / row, i % row);
	}

	for (int i = count; i < count; i++)
	{
		if (element_widgets_list_[i])
		{
			//disconnect slots
			delete element_widgets_list_[i];
			element_widgets_list_[i] = nullptr;
		}
	}
}