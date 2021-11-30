#ifndef ELEMENT_WIDGET_H
#define ELEMENT_WIDGET_H

#include <QObject>
#include <QWidget>
#include <QMenu>
#include <QContextMenuEvent>
#include <QPaintEvent>

#include <mutex>

#include "camera_menu.h"
#include "configuration_tools.h"
#include "av_decode_handler.h"
#include "av_encode_handler.h"
#include "av_audio_encode_handler.h"
#include "av_demux_handler.h"
#include "av_mux_handler.h"
#include "av_devide_handler.h"
#include "sdl_view.h"
#include "rtmp_pusher.h"
#include "av_ffmpeg.h"

enum class WidgetType
{
    WID_TYPE_SOURCE = 0,
    WID_TYPE_SCALED = 1
};
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
    void timerEvent(QTimerEvent* ev) override;

    void InitUi();

    bool IsVideoSeqHeaderNeeded();
    bool IsAudioSeqHeaderNeeded();
    void SetVideoSeqHeaderNeeded(bool status);
    void SetAudioSeqHeaderNeeded(bool status);

    int StartHandle();
    int ConfigHandlers();
    void DestroyAllHandler();

signals:
    //void selected(int x, int y);
    void SigConfigAndStartHandler();
    void SigWidgetDestroyed(int index);
public slots:
    void OnSignalSetCycling();
    void OnSignalStopPublishing();
    void OnConfigAndStartHandler();
protected:
    void DemuxVideoCallback(AVPacket* pkt);
    void DemuxAudioCallback(AVPacket* pkt);
    void VideoEncodeCallback(AVPacket* pkt);
    void VideoInfoCallback(AVRational frame_rate);
    void AudioInfoCallback(int sample_rate, int nb_sample);
private:
    std::mutex mtx_;

    QMenu menu_;

    QString url_;
    QString name_;
    QString sub_url_;
    QString server_url_;
    QString str_width_;
    QString str_height_;
    CameraMenu::ItemListType item_type_ = CameraMenu::ItemListType::ITEM_LIST_TYPE_NONE;

    AVDemuxHandler* demux_handler_ = nullptr;
    AVDecodeHandler* v_decode_handler_ = nullptr;
    AVEncodeHandler* v_encode_handler_ = nullptr;
    AVDecodeHandler* a_decode_handler_ = nullptr;
    AVAudioEncodeHandler* a_encode_handler_ = nullptr;
    AVMuxHandler* mux_handler_ = nullptr;
    RtmpPusher* rtmp_pusher_ = nullptr;
    IVideoView* view_ = nullptr;

    std::vector<std::shared_ptr<InputStream>>  input_streams_;

    int widget_index_ = -1;
    bool has_video_ = false;
    bool has_audio_ = false;
    int input_width_ = -1;
    int input_height_ = -1;
    int output_width_ = -1;
    int output_height_ = -1;

    bool is_librtmp_method_ = false;
    bool is_video_seq_header_needed_ = true;
    bool is_audio_seq_header_needed_ = true;

    WidgetType widget_type_ = WidgetType::WID_TYPE_SOURCE;

    FlvOnMetaData flv_on_metadata_;
};

#endif

