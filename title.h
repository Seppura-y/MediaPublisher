#pragma once

#include <QWidget>
namespace Ui { class Title; };

class Title : public QWidget
{
	Q_OBJECT

public:
	explicit Title(QWidget *parent = Q_NULLPTR);
	~Title();

	int InitUi();

signals:
	void SigCloseBtClicked();
	void SigMinBtClicked();
public slots:

private:
	Ui::Title *ui;
};
