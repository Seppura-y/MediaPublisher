#ifndef ELEMENT_WIDGET_H
#define ELEMENT_WIDGET_H

#include <QObject>
#include <QWidget>
#include <QMenu>
#include <QContextMenuEvent>
#include <QPaintEvent>

#include "camera_menu.h"

#include "configuration_tools.h"
#include "av_decode_handler.h"
#include "av_encode_handler.h"
#include "av_demux_handler.h"
#include "av_mux_handler.h"
#include "av_devide_handler.h"
#include "sdl_view.h"
class ElementWidget : public QWidget
{
	Q_OBJECT

public:
    explicit ElementWidget(QWidget* parent = nullptr);
    explicit ElementWidget(int row, int colum, QWidget* parent = nullptr);
    ~ElementWidget();
protected:
    void paintEvent(QPaintEvent* ev) override;
    void mousePressEvent(QMouseEvent* ev) override;
    void dragEnterEvent(QDragEnterEvent* ev) override;
    void dropEvent(QDropEvent* ev) override;

    int OpenMedia(QString url);
    int DrawMediaFrame();
    void ResetAllHandler();

signals:
    //void selected(int x, int y);
public slots:
    void OnSignalOpen(QString url);

protected:
    int colum_ = -1;
    int row_ = -1;

    CameraMenu::ItemListType item_type_ = CameraMenu::ItemListType::ITEM_LIST_TYPE_NONE;
    QString name_;
    QString url_;
    QString sub_url_;

private:
    QMenu menu_;
    AVDemuxHandler* demux_handler_ = nullptr;
    AVMuxHandler* mux_handler_ = nullptr;
    AVDecodeHandler* decode_handler_ = nullptr;
    AVEncodeHandler* encode_handler_ = nullptr;
    AVDevideHandler* devide_handler_ = nullptr;
};

#endif

