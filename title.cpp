#include "title.h"
#include "ui_title.h"
#include "configuration_tools.h"

Title::Title(QWidget *parent) : 
	QWidget(parent)
{
	ui = new Ui::Title();
	ui->setupUi(this);

	QObject::connect(ui->pb_close, &QPushButton::clicked, this, &Title::SigCloseBtClicked);
	QObject::connect(ui->pb_mini, &QPushButton::clicked, this, &Title::SigMinBtClicked);
}

Title::~Title()
{
	delete ui;
}


int Title::InitUi()
{
	setStyleSheet(ConfigurationTools::GetQssString(":/res/css/title.css"));

	ConfigurationTools::SetButtonIcon(ui->pb_close, 12, QChar(0xf00d));
	ConfigurationTools::SetButtonIcon(ui->pb_mini, 9, QChar(0xf2d1));

	return 0;
}
