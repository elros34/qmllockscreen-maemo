#include "imageprovider.h"
#include <QDebug>

ImageProvider::ImageProvider() :
    QObject(0),
    QDeclarativeImageProvider(QDeclarativeImageProvider::Image)
{
    image = QImage();
}

QImage ImageProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    Q_UNUSED(id);
    Q_UNUSED(size);
    Q_UNUSED(requestedSize);

    return image;
}

void ImageProvider::registerImage(const uint &wId)
{
    try {
        image = x11Helper.getWindowPixmap(wId).toImage();
        emit imageReady();
    }
    catch(std::bad_alloc){}
}
