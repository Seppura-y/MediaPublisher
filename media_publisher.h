#pragma once

#include <QtWidgets/QMainWindow>
#include <QPoint>
#include <QMenu>

#include "ui_media_publisher.h"
#include "camera_menu.h"
#include "title.h"

class MediaPublisher : public QMainWindow
{
    Q_OBJECT

public:
    MediaPublisher(QWidget *parent = Q_NULLPTR);
    ~MediaPublisher();

protected slots:
    void OnTitleCloseButtonClicked();
    void OnTitleMinButtonClicked();

    //void OnCmrMenuAddButtonClicked();
    //void OnCmrMenuSetButtonClicked();
    //void OnCmrMenuDelButtonClicked();

protected:
    void InitUi();


    void mousePressEvent(QMouseEvent* ev) override;
    void mouseMoveEvent(QMouseEvent* ev) override;
    void mouseReleaseEvent(QMouseEvent* ev) override;

private:
    Ui::MediaPublisherClass ui;

    bool is_left_mouse_button_pressed_ = false;
    QMenu menu_;
    QPoint mouse_drag_point_;
    QWidget wid_empty_title_bar_;
    QWidget wid_empty_menu_bar_;

    Title cw_title_;//custom_widget_title
    CameraMenu cw_cmr_menu_;//custom_widget_camera_menu
    ConfigurationTools* conf_tools_ = nullptr;
};
