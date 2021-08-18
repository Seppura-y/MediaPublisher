#pragma once

#include <QDialog>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>


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

	void SetUrl(QString url);
	void SetSubUrl(QString url);
	void SetServerUrl(QString url);
	void SetName(QString name);
protected slots:
	void GetLocalFilename();
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

	QString filename_;
};
