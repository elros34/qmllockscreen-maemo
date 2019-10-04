#ifndef CORE_H
#define CORE_H

#include <QApplication>

#include "mainview.h"
#include <X11/extensions/Xdamage.h>

class Core : public QApplication
{
    Q_OBJECT
public:
    explicit Core(int &argc, char **argv);
    ~Core();

    static int myXErrorHandler( Display*, XErrorEvent*) {
        return 1;
    }

    bool x11EventFilter(XEvent *event);
    MainView *view;

    Damage _damage;
    int damage_event, damage_error;
    Window wId;

signals:
    void updateStatusArea();
public slots:

};

#endif // CORE_H
