#include "element_widget.h"
#include "item_listwidget.h"
#include "camera_menu.h"

#include <QPainter>
#include <QStyleOption>
#include <QAction>
#include <QString>
#include <QDebug>
#include <QListWidget>
#include <QTabWidget>
#include <QMimeData>
#include <QJsonObject>
#include <QJsonDocument>
#include <QPixMap>

#define ITEM_LIST_CONFIG "./media_publisher/conf/configuration.json"
#define GRID_CONFIG "./config/grid_conf.json"

ElementWidget::ElementWidget(int row, int colum, QWidget* parent) : QWidget(parent)
{
    colum_ = colum;
    row_ = row;

    this->setAcceptDrops(true);
}

ElementWidget::ElementWidget(QWidget* parent)
{

}

ElementWidget::~ElementWidget()
{
    ResetAllHandler();
}

void ElementWidget::paintEvent(QPaintEvent* ev)
{
    QStyleOption opt;
    opt.init(this);

    QPainter painter(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);
}

void ElementWidget::mousePressEvent(QMouseEvent* ev)
{
    return QWidget::mousePressEvent(ev);
}
void ElementWidget::dragEnterEvent(QDragEnterEvent* ev)
{
    ev->acceptProposedAction();
}

void ElementWidget::dropEvent(QDropEvent* ev)
{
    int itemIndex = ((ItemListWidget*)ev->source())->currentIndex().row();
    CameraMenu::ItemListType itemType = (CameraMenu::ItemListType)((ItemListWidget*)ev->source())->item_type_;

    QByteArray arr = ev->mimeData()->data("application/json");
    qDebug() << arr;
    QJsonParseError err;
    QJsonDocument doc(QJsonDocument::fromJson(arr, &err));
    QJsonObject obj = doc.object();

    this->name_ = obj.find("name").value().toString();
    this->url_ = obj.find("url").value().toString();
    this->sub_url_ = obj.find("sub_url").value().toString();
    this->item_type_ = itemType;
    switch (itemType)
    {
    case CameraMenu::ItemListType::ITEM_LIST_TYPE_CAMERA:
    {
        qDebug() << "drop camera";
        break;
    }
    case CameraMenu::ItemListType::ITEM_LIST_TYPE_LOCAL_FILE:
    {
        qDebug() << "drop local file";
        break;
    }
    default:
    {
        qDebug() << "drop default";
        break;
    }
    }
    ev->setDropAction(Qt::MoveAction);
    ev->accept();
}

void ElementWidget::ResetAllHandler()
{
    if (demux_handler_)
    {
        demux_handler_->Stop();
        demux_handler_ = nullptr;
    }
    if (decode_handler_)
    {
        decode_handler_->Stop();
        decode_handler_ = nullptr;
    }
    if (encode_handler_)
    {
        encode_handler_->Stop();
        encode_handler_ = nullptr;
    }
    if (mux_handler_)
    {
        mux_handler_->Stop();
        mux_handler_ = nullptr;
    }
    if (devide_handler_)
    {
        devide_handler_->Stop();
        devide_handler_ = nullptr;
    }
}

void ElementWidget::OnSignalOpen(QString url)
{
    qDebug() << "signal push received" << url;
    //if (OpenMedia(url) != 0)
    //{
    //    qDebug() << "open url failed";
    //}
}

int ElementWidget::OpenMedia(QString url)
{
    return 0;
}

int ElementWidget::DrawMediaFrame()
{
    return 0;
}