#pragma once

#include <QDialog>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QCheckBox>


#include "ui_item_set_dialog.h"

class ItemSetDialog : public QDialog
{
	Q_OBJECT

public:
	ItemSetDialog(QWidget *parent = Q_NULLPTR);
	ItemSetDialog(int list_index, QWidget* parent = Q_NULLPTR);
	~ItemSetDialog();

	QString GetUrl();
	QString GetServerUrl();
	QString GetSubUrl();
	QString GetName();
	QString GetWidth();
	QString GetHeight();

	void SetUrl(QString url);
	void SetSubUrl(QString url);
	void SetServerUrl(QString url);
	void SetName(QString name);
	void SetWidth(QString width);
	void SetHeight(QString height);
protected slots:
	void OnGetLocalFilename();
	void OnCheckBoxStateChanged();
	void OnDialogAccepted();
protected:
	void InitUi(int type);
private:
	Ui::ItemSetDialog ui;
	QPushButton* pb_ok_ = nullptr;
	QPushButton* pb_cancel_ = nullptr;
	QPushButton* pb_open_ = nullptr;
	QLineEdit* le_url_ = nullptr;
	QLineEdit* le_sub_url_ = nullptr;
	QLineEdit* le_server_ = nullptr;
	QLineEdit* le_name_ = nullptr;

	QCheckBox* cb_default_ = nullptr;
	QLineEdit* le_width_ = nullptr;
	QLineEdit* le_height_ = nullptr;

	QString filename_;
};
