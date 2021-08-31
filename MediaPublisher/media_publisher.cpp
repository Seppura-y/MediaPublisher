#include "media_publisher.h"
#include <QMouseEvent>
#include <QDebug>
#include <QGridLayout>

#include "configuration_tools.h"

MediaPublisher::MediaPublisher(QWidget *parent)
 : QMainWindow(parent)
{
    ui.setupUi(this);
    this->setWindowFlag(Qt::FramelessWindowHint);
    this->showMaximized();
    this->InitUi();

    conf_tools_ = ConfigurationTools::GetInstance();

    cw_title_.InitUi();

    //QGridLayout* layout = (QGridLayout*)ui.centralWidget->layout();
    //layout->setContentsMargins(0, 0, 0, 0);
    //layout->setSpacing(0);



    QObject::connect(&cw_title_, &Title::SigCloseBtClicked, this, &MediaPublisher::OnTitleCloseButtonClicked);
    QObject::connect(&cw_title_, &Title::SigMinBtClicked, this, &MediaPublisher::OnTitleMinButtonClicked);

    //QObject::connect(&cw_cmr_menu_, &CameraMenu::SigAddButtonClicked, this, &MediaPublisher::OnCmrMenuAddButtonClicked);
    //QObject::connect(&cw_cmr_menu_, &CameraMenu::SigSetButtonClicked, this, &MediaPublisher::OnCmrMenuSetButtonClicked);
    //QObject::connect(&cw_cmr_menu_, &CameraMenu::SigDelButtonClicked, this, &MediaPublisher::OnCmrMenuDelButtonClicked);
}

MediaPublisher::~MediaPublisher()
{

}

void MediaPublisher::InitUi()
{
    setStyleSheet(ConfigurationTools::GetQssString(":/res/css/media_publisher.css"));

    ui.dw_title->setTitleBarWidget(&wid_empty_title_bar_);
    ui.dw_title->setWidget(&cw_title_);

    ui.dw_playlist->setTitleBarWidget(&wid_empty_menu_bar_);
    ui.dw_playlist->setWidget(&cw_cmr_menu_);
}

void MediaPublisher::OnTitleCloseButtonClicked()
{
    this->close();
}

void MediaPublisher::OnTitleMinButtonClicked()
{
    this->showMinimized();
}

void MediaPublisher::mousePressEvent(QMouseEvent* ev)
{
    if (ev->button() == Qt::LeftButton)
    {
        is_left_mouse_button_pressed_ = true;
        mouse_drag_point_ = ev->pos();
    }
    else
    {
        return QMainWindow::mousePressEvent(ev);
    }
}

void MediaPublisher::mouseMoveEvent(QMouseEvent* ev)
{
    if (ui.dw_title->geometry().contains(mouse_drag_point_) && is_left_mouse_button_pressed_)
    {
        this->move(ev->globalPos() - mouse_drag_point_);
    }
    else
    {
        return QMainWindow::mouseMoveEvent(ev);
    }
}

void MediaPublisher::mouseReleaseEvent(QMouseEvent* ev)
{
    is_left_mouse_button_pressed_ = false;
}

//void MediaPublisher::OnCmrMenuAddButtonClicked()
//{
//    qDebug() << "OnCmrMenuAddButtonClicked";
//}
//
//void MediaPublisher::OnCmrMenuSetButtonClicked()
//{
//    qDebug() << "OnCmrMenuSetButtonClicked";
//}
//
//void MediaPublisher::OnCmrMenuDelButtonClicked()
//{
//    qDebug() << "OnCmrMenuDelButtonClicked";
//}

