#ifndef IMAGEPROVIDER_H
#define IMAGEPROVIDER_H


#include <QDeclarativeImageProvider>
#include "x11helper.h"

#include <QtConcurrentRun>

class ImageProvider : public QObject, public QDeclarativeImageProvider
{
    Q_OBJECT
public:
    explicit ImageProvider();
    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize);

    X11Helper x11Helper;

    QImage image;
signals:
    void imageReady();

public slots:
    void registerImage(const uint &wId);

};

#endif // IMAGEPROVIDER_H
