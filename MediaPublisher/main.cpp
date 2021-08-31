#include "media_publisher.h"
#include <QtWidgets/QApplication>
#include <QFontDatabase>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QFontDatabase::addApplicationFont(":res/fontawesome-webfont.ttf");
    MediaPublisher w;
    w.show();
    return a.exec();
}
