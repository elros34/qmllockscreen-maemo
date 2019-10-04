#ifndef WINDOWITEM_H
#define WINDOWITEM_H

#include <QDeclarativeItem>
#include <QtGui>
#include "x11utils.h"
#include <QPainter>
#include "hildonutils.h"

class WindowItem : public QDeclarativeItem
{
    Q_OBJECT

    Q_PROPERTY(QString source READ source WRITE setSource NOTIFY sourceChanged)
public:
    explicit WindowItem(QDeclarativeItem *parent = 0);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *);

    void setSource(const QString &source);
    QString source() const;

private:
    QString _source;
    QImage image;
    bool isPortrait;

    HildonUtils hildonUtils;
    
signals:
    void sourceChanged();

public slots:
    void setWIdImage();

private slots:
    void onOrientationChanged();
    
};

#endif // WINDOWITEM_H
