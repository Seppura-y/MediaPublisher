#include "configuration_tools.h"
#include <QFile>
#include <QJsonArray>
#include <QJsonParseError>
#include <QDebug>
#include <QDir>

static const QString config_file_path = "./media_publisher/conf/configuration.json";

ConfigurationTools::ConfigurationTools()
{
	QDir dir;
	QString directory(config_file_path.left(config_file_path.lastIndexOf("/")));
	if (!dir.exists(directory))
	{
		dir.mkpath(directory);
	}
	QFile file(config_file_path);
	if (!file.exists())
	{
		file.open(QIODevice::WriteOnly);
		file.close();
	}
}

ConfigurationTools* ConfigurationTools::GetInstance()
{
	static ConfigurationTools conf_tools;
	return &conf_tools;
}

QString ConfigurationTools::GetQssString(QString path)
{
	QString QssString;
	QFile QssFile(path);
	if (QssFile.open(QIODevice::ReadOnly))
	{
		QssString = QssFile.readAll();
		QssFile.close();
	}
	else
	{
		qDebug() << "open qss file : " << path << "failed";
	}
	return QssString;
}

void ConfigurationTools::SetButtonIcon(QPushButton* pb, int size, QChar icon)
{
	QFont font;
	font.setFamily("FontAwesome");
	font.setPointSize(size);

	pb->setFont(font);
	pb->setText(icon);
}

void ConfigurationTools::SetItemIcon(QWidget* item, int size, QChar icon)
{
	QFont font;
	font.setFamily("FontAwesome");
	font.setPointSize(size);

	item->setFont(font);
	item->setWindowIconText(icon);
}


QJsonObject* ConfigurationTools::GetObject(JsonObjType type)
{
    QJsonObject temp;
    QString key;
    switch (type)
    {
		case JsonObjType::JSONOBJ_TYPE_CAMERA:
		{
			return &json_camera_src_;
		}
		case JsonObjType::JSONOBJ_TYPE_LOCAL_FILE:
		{
			return &json_local_src_;
		}
		default:
		{
			return nullptr;
		}
    }
}

void ConfigurationTools::InitJson()
{
	LoadJson(config_file_path);
}

int ConfigurationTools::LoadJson(const QString filePath)
{
	QFile load_file(filePath);

	if (!load_file.open(QIODevice::ReadOnly))
	{
		qDebug() << "LoadJson : " << filePath << "failed";
		is_init_ = false;
		return -1;
	}

	QByteArray src_data = load_file.readAll();
	load_file.close();

	QJsonParseError json_error;
	QJsonDocument json_src_doc(QJsonDocument::fromJson(src_data, &json_error));

	if (json_error.error != QJsonParseError::NoError)
	{
		qDebug() << "json parse error occurred : " << filePath;
		is_init_ = false;
		return -1;
	}

	json_src_ = json_src_doc.object();

	if (json_src_.contains("Camera"))
	{
		json_camera_src_ = json_src_.value("Camera").toObject();
	}

	if (json_src_.contains("LocalFile"))
	{
		json_local_src_ = json_src_.value("LocalFile").toObject();
	}
	is_init_ = true;
	return -1;
}

int ConfigurationTools::SaveJson(const QString filepath)
{
	QFile save_file(filepath);
	QJsonObject save_object;
	QJsonDocument save_doc;
	save_object.insert("Camera", json_camera_src_);
	save_object.insert("LocalFile", json_local_src_);
	save_doc.setObject(save_object);

	if (!save_file.open(QIODevice::WriteOnly))
	{
		qDebug() << "save json " << filepath << "open file failed";
		return -1;
	}

	save_file.write(save_doc.toJson());
	save_file.close();

	return 0;
}

int ConfigurationTools::DeleteObject(const QString key, JsonObjType type)
{
	QJsonObject* temp = nullptr;
	switch (type)
	{
		case JsonObjType::JSONOBJ_TYPE_CAMERA:
		{
			temp = &json_camera_src_;
			break;
		}
		case JsonObjType::JSONOBJ_TYPE_UNDEFINE:
		{
			temp = &json_local_src_;
			break;
		}
		default:
		{
			temp = nullptr;
		}
	}
	if (!temp)
	{
		return false;
	}
	else
	{
		if ((*temp).contains(key) && (*temp).find(key).value().isObject())
		{
			(*temp).erase((*temp).find(key));
		}
	}

	return true;
}

int ConfigurationTools::WriteJson(const QString key, QJsonObject object, enum JsonObjType type)
{

	QJsonObject* temp = nullptr;
	switch (type)
	{
		case JsonObjType::JSONOBJ_TYPE_CAMERA:
		{
			temp = &json_camera_src_;
			break;
		}
		case JsonObjType::JSONOBJ_TYPE_LOCAL_FILE:
		{
			temp = &json_local_src_;
			break;
		}
		default:
		{
			temp = nullptr;
		}
	}
	if (!temp)
	{
		return -1;
	}
	else
	{
		(*temp).insert(key, object);
	}
	return 0;
}