#include "windowitem.h"

WindowItem::WindowItem(QDeclarativeItem *parent) :
    QDeclarativeItem(parent)
{
    setFlag(QGraphicsItem::ItemHasNoContents, false);
    connect(this, SIGNAL(sourceChanged()), this, SLOT(setWIdImage()));

    if(QApplication::desktop()->width() > QApplication::desktop()->height())
        isPortrait = false;
    else
        isPortrait = true;

    connect(QApplication::desktop(), SIGNAL(resized(int)), this, SLOT(onOrientationChanged()));
}

void WindowItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->drawImage(QRect(0,0, width(), height()), image);
}

void WindowItem::setSource(const QString &source)
{
    _source = source;
    emit sourceChanged();
}


QString WindowItem::source() const
{
    return _source;
}

void WindowItem::setWIdImage()
{
    if(_source.startsWith("desktop"))
    {
        image = hildonUtils.getHildonHome(isPortrait);//
    }
    else if(_source.startsWith("grab"))
    {
        image = QPixmap::grabWindow(QApplication::desktop()->winId()).toImage();
    }
    else {
        QStringList slist = _source.split("/");
        if(slist.count() == 2)
        {
        try {
                image = X11Utils::getWindowPixmap(slist.at(0).toInt(),
                                                  slist.at(1).toInt()).toImage();
        }
        catch(std::bad_alloc){}
        }
    }
    update();
}

void WindowItem::onOrientationChanged()
{
    if(QApplication::desktop()->width() > QApplication::desktop()->height())
        isPortrait = false;
    else
        isPortrait = true;
}
