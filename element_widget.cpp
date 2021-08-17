#include "element_widget.h"
#include "item_listwidget.h"
#include "camera_menu.h"
#include "item_set_dialog.h"

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

ElementWidget::ElementWidget(int index, QWidget* parent) : QWidget(parent)
{
    widget_index_ = index;
    this->setAcceptDrops(true);

    QAction* act = menu_.addAction(QString::fromLocal8Bit("set"));
    QObject::connect(act, &QAction::triggered, this, &ElementWidget::OnSigalSet);
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

void ElementWidget::contextMenuEvent(QContextMenuEvent* ev)
{
    menu_.exec(QCursor::pos());
    ev->accept();
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
        ItemSetDialog* dia = new ItemSetDialog((int)itemType);
        Qt::WindowFlags flag = dia->windowFlags();
        dia->setWindowFlags(flag | Qt::MSWindowsFixedSizeDialogHint);
        while (1)
        {
            if (dia->exec() == ItemSetDialog::Accepted)
            {
                QString url = dia->GetUrl();
                QString svr = dia->GetServerUrl();
                break;
            }
            else
            {
                break;
            }
        }
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

    }
    if (decode_handler_)
    {
        decode_handler_->Stop();

    }
    if (encode_handler_)
    {
        encode_handler_->Stop();

    }
    if (mux_handler_)
    {
        mux_handler_->Stop();

    }
    if (devide_handler_)
    {
        devide_handler_->Stop();

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

void ElementWidget::InitUi()
{

}

void ElementWidget::OnSigalSet()
{
    qDebug() << "on signal set";
}

bool ElementWidget::IsVideoSeqHeaderNeeded()
{
    return is_video_seq_header_needed_;
}

bool ElementWidget::IsAudioSeqHeaderNeeded()
{
    return is_audio_seq_header_needed_;
}

void ElementWidget::SetVideoSeqHeaderNeeded(bool status)
{
    is_video_seq_header_needed_ = status;
}

void ElementWidget::SetAudioSeqHeaderNeeded(bool status)
{
    is_audio_seq_header_needed_ = status;
}