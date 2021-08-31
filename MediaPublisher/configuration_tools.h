#pragma once

#include <QString>
#include <QJsonObject>
#include <QPushButton>
class ConfigurationTools
{
public:
	static enum class JsonObjType
	{
		JSONOBJ_TYPE_UNDEFINE = -1,
		JSONOBJ_TYPE_CAMERA = 0,
		JSONOBJ_TYPE_LOCAL_FILE = 1

	};


public:
	static ConfigurationTools* GetInstance();
	static QString GetQssString(QString path);
	static void SetButtonIcon(QPushButton* pb, int size, QChar icon);
	static void SetItemIcon(QWidget* item, int size, QChar icon);


	QJsonObject* GetObject(JsonObjType type);
	void InitJson();
	int LoadJson(const QString filePath);
	int SaveJson(const QString filepath);
	int DeleteObject(const QString key, JsonObjType type);
	int WriteJson(const QString key, QJsonObject object, enum JsonObjType type);

protected:

private:
	ConfigurationTools();

private:
	bool is_init_ = false;
	QString file_path_;
	QJsonObject json_src_;
	QJsonObject json_camera_src_;
	QJsonObject json_local_src_;
};

