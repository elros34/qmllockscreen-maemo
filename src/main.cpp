#include <QApplication>
#include "lscore.h"

Q_DECL_EXPORT int main(int argc, char *argv[])
{
    LSCore lsCore(argc, argv);

    return lsCore.exec();
}
