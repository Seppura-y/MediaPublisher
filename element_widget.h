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
#include "rtmp_pusher.h"
class ElementWidget : public QWidget
{
	Q_OBJECT

public:
    explicit ElementWidget(QWidget* parent = nullptr);
    explicit ElementWidget(int index, QWidget* parent = nullptr);
    ~ElementWidget();
protected:
    void paintEvent(QPaintEvent* ev) override;
    void mousePressEvent(QMouseEvent* ev) override;
    void dragEnterEvent(QDragEnterEvent* ev) override;
    void dropEvent(QDropEvent* ev) override;
    void contextMenuEvent(QContextMenuEvent* ev) override;

    void InitUi();
    int OpenMedia(QString url);
    int DrawMediaFrame();
    void ResetAllHandler();

    bool IsVideoSeqHeaderNeeded();
    bool IsAudioSeqHeaderNeeded();
    void SetVideoSeqHeaderNeeded(bool status);
    void SetAudioSeqHeaderNeeded(bool status);

signals:
    //void selected(int x, int y);
public slots:
    void OnSignalOpen(QString url);
    void OnSigalSet();
protected:

private:
    QString url_;
    QMenu menu_;
    QString name_;
    QString sub_url_;
    CameraMenu::ItemListType item_type_ = CameraMenu::ItemListType::ITEM_LIST_TYPE_NONE;

    IVideoView* view_ = nullptr;
    AVDemuxHandler* demux_handler_ = nullptr;
    AVMuxHandler* mux_handler_ = nullptr;
    AVDecodeHandler* decode_handler_ = nullptr;
    AVEncodeHandler* encode_handler_ = nullptr;
    AVDevideHandler* devide_handler_ = nullptr;
    RtmpPusher* rtmp_pusher_ = nullptr;

    int widget_index_ = -1;
    int output_width_ = -1;
    int output_height_ = -1;

    bool is_librtmp_method_ = false;
    bool is_video_seq_header_needed_ = true;
    bool is_audio_seq_header_needed_ = true;

};

#endif

