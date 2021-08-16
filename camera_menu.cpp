#include "camera_menu.h"
#include <QVBoxLayout>
#include <QDialog>
#include <QFormLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QFileDialog>


#define ITEM_LIST_CONFIG "./media_publisher/conf/configuration.json"
//#define GRID_CONFIG "./config/MediaPublisher/grid_conf.json"
//#define DIMENSION_CONFIG "./config/MediaPublisher/Manager_init.json"
#define ITEM_LIST_COUNT 2

CameraMenu::CameraMenu(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	InitUi();

    ui.tw_item->setTabText(0, "cam");
    ui.tw_item->setTabText(1, "mp4");
	//lw_camera_ = new ItemListWidget(this);
	lw_camera_ = (ItemListWidget*)ui.tw_item->widget(1);
	lw_camera_->setDragEnabled(true);

	config_tools_ = ConfigurationTools::GetInstance();
    config_tools_->InitJson();
    InitItemList();

	QObject::connect(ui.pb_add, &QPushButton::clicked, this, &CameraMenu::OnCmrMenuAddButtonClicked);
	QObject::connect(ui.pb_set, &QPushButton::clicked, this, &CameraMenu::OnCmrMenuSetButtonClicked);
	QObject::connect(ui.pb_del, &QPushButton::clicked, this, &CameraMenu::OnCmrMenuDelButtonClicked);
}

CameraMenu::~CameraMenu()
{
	delete lw_local_file_;
	//delete lw_camera_;
	delete vb_local_file_;
	//delete vb_camera_;
}

void CameraMenu::InitUi()
{
	setStyleSheet(ConfigurationTools::GetQssString(":/res/css/camera_menu.css"));

	ConfigurationTools::SetButtonIcon(ui.pb_add, 12, QChar(0xf07c));
	ConfigurationTools::SetButtonIcon(ui.pb_set, 12, QChar(0xf0ad));
	ConfigurationTools::SetButtonIcon(ui.pb_del, 12, QChar(0xf1f8));
	//ConfigurationTools::SetButtonIcon(ui.pb_del, 12, QChar(0xf008));

	lw_camera_ = new ItemListWidget(0, this);
	lw_camera_->setDragEnabled(true);
	vb_camera_ = new QVBoxLayout();
	vb_camera_->addWidget(lw_camera_);
	ui.tw_item->widget(0)->setLayout(vb_camera_);

	lw_local_file_ = new ItemListWidget(1, this);
	lw_local_file_->setDragEnabled(true);
	vb_local_file_ = new QVBoxLayout();
	vb_local_file_->addWidget(lw_local_file_);
	ui.tw_item->widget(1)->setLayout(vb_local_file_);

}

int CameraMenu::GetCurrentItemIndex()
{
	//lw_camera_->currentItem();
	return 0;
}

void CameraMenu::OnCmrMenuAddButtonClicked()
{
    AddListItem();
}

void CameraMenu::OnCmrMenuSetButtonClicked()
{
    SetListItem();
}

void CameraMenu::OnCmrMenuDelButtonClicked()
{
    DeleteItem();
}


void CameraMenu::AddListItem()
{
	ItemListType type = (ItemListType)ui.tw_item->currentIndex();
	switch (type)
	{
		case ItemListType::ITEM_LIST_TYPE_CAMERA:
		{
			SetListItem(type, -1);
			break;
		}
		case ItemListType::ITEM_LIST_TYPE_LOCAL_FILE:
		{
			SetLocalListItem(type, -1);
			break;
		}
		default:
		{
			qDebug() << "AddListItem ITEM_LIST_TYPE_NONE";
			break;
		}
	}
}

void CameraMenu::SetListItem()
{
	ItemListType type = (ItemListType)ui.tw_item->currentIndex();
	int index = ((ItemListWidget*)(ui.tw_item->widget((int)type)->layout()->itemAt(0)->widget()))->currentIndex().row();
	if (index < 0)
	{
		qDebug() << "SetListItem failed : please select a item";
		return;
	}
	switch (type)
	{
		case ItemListType::ITEM_LIST_TYPE_CAMERA:
		{
			SetListItem(type, index);
			break;
		}
		case ItemListType::ITEM_LIST_TYPE_LOCAL_FILE:
		{
			SetLocalListItem(type, index);
			break;
		}
		default:
		{
			qDebug() << "SetListItem ITEM_LIST_TYPE_NONE";
			return;
		}
	}
}

void CameraMenu::SetListItem(ItemListType item_type, int item_index)
{

    QDialog dia(this);
    dia.resize(600, 120);

    QFormLayout* formLayout = new QFormLayout();
    dia.setLayout(formLayout);

    QLineEdit leName;
    formLayout->addRow(QString::fromLocal8Bit("name : "), &leName);

    QLineEdit leUrl;
    formLayout->addRow(QString::fromLocal8Bit("url"), &leUrl);

    QLineEdit leSubUrl;
    if (item_type == ItemListType::ITEM_LIST_TYPE_CAMERA)
    {
        formLayout->addRow(QString::fromLocal8Bit("sub_url"), &leSubUrl);
    }

    QPushButton pbAccept;
    pbAccept.setText(QString::fromLocal8Bit("accept"));
    QObject::connect(&pbAccept, SIGNAL(clicked()), &dia, SLOT(accept()));
    formLayout->addRow(&pbAccept);

    QJsonObject obj;
    QString key;
    if (item_index >= 0)
    {
        ItemListWidget* lwCur = (ItemListWidget*)ui.tw_item->currentWidget()->layout()->itemAt(0)->widget();
        lwCur->item_type_ = (int)item_type;
        key = lwCur->currentItem()->text();
        obj = *config_tools_->GetObject((ConfigurationTools::JsonObjType)item_type);

        if (!obj.contains(key))
        {
            qDebug() << "set item : do not have this key";
            return;
        }
        QJsonObject::iterator it = obj.find(key);
        obj = it.value().toObject();
        leName.setText(obj.find("name").value().toString());
        leUrl.setText(obj.find("url").value().toString());
        if (item_type == ItemListType::ITEM_LIST_TYPE_CAMERA)
        {
            leSubUrl.setText(obj.find("SubUrl").value().toString());
        }
        DeleteItem(item_type, item_index);
    }

    QJsonObject info;

    for (;;)
    {
        if (dia.exec() == QDialog::Accepted)
        {
            if (leName.text().isEmpty())
            {
                QMessageBox::information(nullptr, "error", "please set a name");
                continue;
            }

            if (leUrl.text().isEmpty())
            {
                QMessageBox::information(nullptr, "error", "please set a name");
                continue;
            }

            if (item_type == ItemListType::ITEM_LIST_TYPE_CAMERA)
            {
                if (leSubUrl.text().isEmpty())
                {
                    QMessageBox::information(nullptr, "error", "please set a name");
                    continue;
                }
                info.insert("SubUrl", leSubUrl.text());
            }

            info.insert("name", leName.text());
            info.insert("url", leUrl.text());
            info.insert("item_type", (int)item_type);
            //info.insert("item_index", index);
            config_tools_->WriteJson(leName.text(), info, (ConfigurationTools::JsonObjType)item_type);
            config_tools_->SaveJson(ITEM_LIST_CONFIG);
            InitItemList();
            break;
        }
        else if (dia.close())
        {
            //config_tools_->WriteJson(key, obj, (ConfigurationTools::JsonObjType)item_type);
            //QJsonDocument doc;
            //doc.setObject(obj);
            //QByteArray arr = doc.toJson();
            //config_tools_->SaveJson(ITEM_LIST_CONFIG);
            //InitItemList();
            return;
        }
    }

}

void CameraMenu::SetLocalListItem(ItemListType item_type, int item_index)
{
    QString openPicUrl = QFileDialog::getOpenFileName(this, QString::fromLocal8Bit("Ñ¡ÔñÎÄ¼þ"), QString("./"), QString("Files (*.mp4)"));
    QString openPicName = openPicUrl.right(openPicUrl.size() - openPicUrl.lastIndexOf('/') - 1);
    if (openPicName.size())
    {
        if (item_index >= 0)
        {
            ItemListWidget* lwCur = (ItemListWidget*)ui.tw_item->currentWidget()->layout()->itemAt(0)->widget();
            //lwCur->itemType = itemType;
            lwCur->currentItem()->setText(openPicName);
            DeleteItem(item_type, item_index);
        }

        QJsonObject obj;
        obj.insert("name", openPicName);
        obj.insert("url", openPicUrl);
        obj.insert("item_type", (int)item_type);
        config_tools_->WriteJson(openPicName, obj, (ConfigurationTools::JsonObjType)item_type);
        config_tools_->SaveJson(ITEM_LIST_CONFIG);
        InitItemList();
    }

}

void CameraMenu::DeleteItem()
{
    int type = ui.tw_item->currentIndex();
    ItemListWidget* curList = (ItemListWidget*)ui.tw_item->currentWidget()->layout()->itemAt(0)->widget();
    if (curList->currentIndex().row() < 0)
    {
        QMessageBox::information(nullptr, "error", QString::fromLocal8Bit("please select a item"));
        return;
    }
    this->DeleteItem((ItemListType)type, curList->currentIndex().row());
}

void CameraMenu::DeleteItem(ItemListType itemType, int index)
{
    QString key = ((ItemListWidget*)ui.tw_item->currentWidget()->layout()->itemAt(0)->widget())->currentItem()->text();
    QJsonObject* obj = config_tools_->GetObject((ConfigurationTools::JsonObjType)itemType);
    if (obj->contains(key))
    {
        //QJsonObject::iterator it = obj.find(key);
        //obj.erase(it);
        obj->take(key);
        config_tools_->SaveJson(ITEM_LIST_CONFIG);
        InitItemList();
    }

}

void CameraMenu::InitItemList()
{
    if (!config_tools_->LoadJson(ITEM_LIST_CONFIG))
    {
        QMessageBox::information(nullptr, "error", QString("load %1 failed").arg(ITEM_LIST_CONFIG));
        return;
    }
    QJsonObject* src;
    QStringList src_key;
    for (int i = 0; i < ITEM_LIST_COUNT; i++)
    {
        ItemListWidget* list = (ItemListWidget*)ui.tw_item->widget(i)->layout()->itemAt(0)->widget();
        list->clear();
        src = config_tools_->GetObject((ConfigurationTools::JsonObjType)i);
        if (!src)
        {
            break;
        }
        src_key = src->keys();
        for (auto it = src_key.begin(); it != src_key.end(); it++)
        {
            QString key = *it;
            list->addItem(key);
        }
    }
}