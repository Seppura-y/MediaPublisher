#include "item_set_dialog.h"
#include <QFileDialog>

ItemSetDialog::ItemSetDialog(QWidget *parent) : QDialog(parent)
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
	le_name_ = ui.le_name;
	le_url_ = ui.le_url;
	le_server_ = ui.le_server;
	le_sub_url_ = ui.le_sub_url;
	le_width_ = ui.le_width;
	le_height_ = ui.le_height;
	cb_default_ = ui.cb_default;

	if (type == 1)
	{
		le_sub_url_->hide();
		ui.label_2->hide();
	}
	else
	{
		pb_open_->hide();
	}

	QObject::connect(pb_ok_, &QPushButton::clicked, this, &ItemSetDialog::OnDialogAccepted);
	QObject::connect(pb_cancel_, &QPushButton::clicked, this, &ItemSetDialog::reject);
	QObject::connect(pb_open_, &QPushButton::clicked, this, &ItemSetDialog::OnGetLocalFilename);
	QObject::connect(cb_default_, &QCheckBox::stateChanged, this, &ItemSetDialog::OnCheckBoxStateChanged);
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

QString ItemSetDialog::GetName()
{
	return le_name_->text();
}

QString ItemSetDialog::GetWidth()
{
	return le_width_->text();
}

QString ItemSetDialog::GetHeight()
{
	return le_height_->text();
}

void ItemSetDialog::OnDialogAccepted()
{
	this->accept();
}

void ItemSetDialog::OnGetLocalFilename()
{
     filename_ = QFileDialog::getOpenFileName(this, QString::fromLocal8Bit("select file"), QString("./"), QString("Files (*.mp4 *.flv)"));
	 ui.le_url->setText(filename_);
}

void ItemSetDialog::OnCheckBoxStateChanged()
{
	if (cb_default_->isChecked())
	{
		le_width_->setEnabled(false);
		le_height_->setEnabled(false);
		le_width_->setText("Source width");
		le_height_->setText("Source height");
	}
	else
	{
		le_width_->setEnabled(true);
		le_height_->setEnabled(true);
		le_width_->clear();
		le_height_->clear();
	}
}

void ItemSetDialog::SetUrl(QString url)
{
	le_url_->setText(url);
}

void ItemSetDialog::SetSubUrl(QString url)
{
	le_sub_url_->setText(url);
}

void ItemSetDialog::SetServerUrl(QString url)
{
	le_server_->setText(url);
}

void ItemSetDialog::SetName(QString name)
{
	le_name_->setText(name);
}

void ItemSetDialog::SetWidth(QString width)
{
	if (width == "Source width")
	{
		cb_default_->setChecked(true);
	}
	le_width_->setText(width);
}

void ItemSetDialog::SetHeight(QString height)
{
	if (height == "Source height")
	{
		cb_default_->setChecked(true);
	}
	le_height_->setText(height);
}