#include "item_set_dialog.h"
#include <QFileDialog>

ItemSetDialog::ItemSetDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
}

ItemSetDialog::ItemSetDialog(int list_index, QWidget* parent) : QDialog(parent)
{
	ui.setupUi(this);
	InitUi(list_index);
}

ItemSetDialog::~ItemSetDialog()
{

}

void ItemSetDialog::InitUi(int type)
{
	pb_ok_ = ui.pb_ok;
	pb_cancel_ = ui.pb_cancel;
	pb_open_ = ui.pb_open;
	le_url_ = ui.le_url;
	le_server_ = ui.le_server;
	le_sub_url_ = ui.le_sub_url;
	if (type == 1)
	{
		le_sub_url_->hide();
		ui.label_2->hide();
	}
	else
	{
		pb_open_->hide();
	}

	QObject::connect(pb_ok_, &QPushButton::clicked, this, &ItemSetDialog::accept);
	QObject::connect(pb_cancel_, &QPushButton::clicked, this, &ItemSetDialog::reject);
	QObject::connect(pb_open_, &QPushButton::clicked, this, &ItemSetDialog::GetLocalFilename);
}

QString ItemSetDialog::GetUrl()
{
	return le_url_->text();
}

QString ItemSetDialog::GetServerUrl()
{
	return le_server_->text();
}

QString ItemSetDialog::GetSubUrl()
{
	return le_sub_url_->text();
}

void ItemSetDialog::GetLocalFilename()
{
     filename_ = QFileDialog::getOpenFileName(this, QString::fromLocal8Bit("select file"), QString("./"), QString("Files (*.mp4)"));
	 ui.le_url->setText(filename_);
}
