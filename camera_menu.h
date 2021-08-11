#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include "ui_camera_menu.h"
#include "item_listwidget.h"
#include "configuration_tools.h"

class CameraMenu : public QWidget
{
	Q_OBJECT

public:
	CameraMenu(QWidget *parent = Q_NULLPTR);
	~CameraMenu();

	int GetCurrentItemIndex();

public:
	enum class ItemListType
	{
		ITEM_LIST_TYPE_NONE = -1,
		ITEM_LIST_TYPE_CAMERA = 0,
		ITEM_LIST_TYPE_LOCAL_FILE = 1
	};
signals:
	void SigAddButtonClicked();
	void SigSetButtonClicked();
	void SigDelButtonClicked();

protected slots:
	void OnCmrMenuAddButtonClicked();
	void OnCmrMenuSetButtonClicked();
	void OnCmrMenuDelButtonClicked();
protected:
	void InitUi();
	
	void InitItemList();
	void AddListItem();
	void SetListItem();
	void SetListItem(ItemListType item_type, int item_index);
	void SetLocalListItem(ItemListType item_type, int item_index);

	void DeleteItem();
	void DeleteItem(ItemListType item_type, int item_index);

private:
	Ui::CameraMenu ui;

	ItemListWidget* lw_camera_ = nullptr;
	ItemListWidget* lw_local_file_ = nullptr;

	QVBoxLayout* vb_camera_ = nullptr;
	QVBoxLayout* vb_local_file_ = nullptr;

	ConfigurationTools* config_tools_ = nullptr;
};
