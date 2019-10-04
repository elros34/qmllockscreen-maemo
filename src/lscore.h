#ifndef LSCORE_H
#define LSCORE_H

#include <QApplication>
#include "mainlock.h"
#include "x11utils.h"

class LSCore : public QApplication
{
    Q_OBJECT
public:
    explicit LSCore(int &argc, char **argv);

    bool x11EventFilter(XEvent *event);

private:
    MainLock *mainLock;
    
signals:
    
public slots:
    
};

#endif // LSCORE_H
