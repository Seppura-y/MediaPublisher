#include "item_listwidget.h"
#include "configuration_tools.h"

#include <QMouseEvent>
#include <QApplication>
#include <QDrag>
#include <QMimeData>
#include <QJsonDocument>
#include <QJsonObject>
#include <QByteArray>

ItemListWidget::ItemListWidget(int item_type,QWidget* parent) : QListWidget(parent)
{
	item_type_ = item_type;
}


ItemListWidget::ItemListWidget(QWidget* parent) : QListWidget(parent)
{

}

void ItemListWidget::mousePressEvent(QMouseEvent* ev)
{
    //Make sure to drag with the left button.
    if (ev->button() == Qt::LeftButton)
    {
        //Save the starting point of the drag first.
        drag_point_ = ev->pos();
        //Keep the dragged item.
        drag_item_ = this->itemAt(ev->pos());
    }
    //Retain the mouse click operation of the original QListWidget widget.
    QListWidget::mousePressEvent(ev);
}

void ItemListWidget::mouseMoveEvent(QMouseEvent* event)
{
    //Make sure to hold down the left button to move.
    if (event->buttons() & Qt::LeftButton)
    {
        QPoint temp = event->pos() - drag_point_;
        //Only this length is greater than the default distance, it will be considered by the system as a drag operation.
        if (temp.manhattanLength() > QApplication::startDragDistance())
        {
            QJsonObject send;
            ConfigurationTools* conf = ConfigurationTools::GetInstance();
            QJsonObject* obj = conf->GetObject((ConfigurationTools::JsonObjType)this->item_type_);
            if (drag_item_ == nullptr || obj == nullptr)
            {
                return;
            }
            QString name = drag_item_->text();
            if (obj->contains(name))
            {
                send = obj->find(name).value().toObject();
            }

            QJsonDocument doc(send);
            QByteArray arr = doc.toJson();

            QDrag* drag = new QDrag(this);
            QMimeData* mimeData = new QMimeData;
            mimeData->setData("application/json", arr);

            drag->setMimeData(mimeData);
            auto action = drag->exec(Qt::CopyAction | Qt::MoveAction);

            if (action == (Qt::CopyAction) || (action == Qt::MoveAction))
            {
                //After successfully dragging, delete the dragged item.
                //auto i = this->takeItem(this->row(m_dragItem));
                //delete i;
            }
        }
    }
    //QListWidget::mouseMoveEvent(event);
}
