#include "filemodel.h"
#include <QSettings>

FileModel::FileModel(QObject *parent) :
    QAbstractListModel(parent)
{
    QHash<int, QByteArray> roles;
    roles[PathRole] = "path";
    roles[NameRole] = "name";
    roles[IsDirRole] = "isDir";
    setRoleNames(roles);

}

FileModel::~FileModel()
{
    qDeleteAll(appList);
    appList.clear();
}

QVariant FileModel::data(const QModelIndex &index, int role) const
{
    if(index.row() < 0 || index.row() > appList.count())
        return QVariant();
    else if(role == PathRole)
    {
        return appList.at(index.row())->path();
    }
    else if(role == NameRole)
    {
        return appList.at(index.row())->name();
    }
    else if(role == IsDirRole)
    {
        return appList.at(index.row())->isDir();
    }
    else return QVariant();
}

int FileModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return appList.count();
}

void FileModel::append(AppItem *item)
{
    if(!appList.contains(item))
    {
        beginInsertRows(QModelIndex(), rowCount(QModelIndex()), rowCount(QModelIndex()));
        appList.append(item);
        endInsertRows();
    }
}

void FileModel::clear()
{
    if(!appList.isEmpty())
    {
        beginResetModel();
        appList.clear();
        endResetModel();
    }
}

void FileModel::sortElements()
{
    beginResetModel();
    qSort(appList.begin(), appList.end(), compareAppItem);
    endResetModel();
}

void FileModel::setDir(QString directory)
{

    clear();
    QDir dir(directory);
    QFileInfoList fileInfoList = dir.entryInfoList(_filters,
                 QDir::AllDirs | QDir::NoSymLinks | QDir::NoDot | QDir::Files);
    for (int i = 0; i < fileInfoList.count(); ++i)
    {
        append(new AppItem(fileInfoList.at(i).fileName(), fileInfoList.at(i).absoluteFilePath(), fileInfoList.at(i).isDir()));
    }
}
void FileModel::setFilters(const QStringList &filters)
{
    _filters = filters;
}

QStringList FileModel::getDesktopFileInfo(const QString &path)
{
    QSettings *settings = new QSettings(path, QSettings::IniFormat, this);
    settings->beginGroup("Desktop Entry");
    QString appIcon = settings->value("Icon").toString();
    settings->endGroup();

    if(!QFile::exists(appIcon))
        appIcon = findIcon(appIcon);

    QStringList result;
    result.append(QFileInfo(path).fileName().remove(".desktop", Qt::CaseSensitive));
    result.append(appIcon);
    settings->deleteLater();

    return result;
}

QString FileModel::findIcon(QString appIcon)
{

    QStringList iconsSize = QDir("/usr/share/icons/hicolor/64x64/").entryList(QStringList(),
                            QDir::AllDirs | QDir::NoDotAndDotDot);
    bool emptyExt = true;
    if(appIcon.endsWith(".png") || appIcon.endsWith(".svg"))
        emptyExt = false;

    QString ext;


    foreach(QString iconSize, iconsSize)
    {
        if(emptyExt)
            ext = ".png";
        else ext = "";

        if(QFile::exists("/usr/share/icons/hicolor/64x64/"+iconSize+QString("/")+appIcon+ext))
        {
            appIcon = QString("/usr/share/icons/hicolor/64x64/"+iconSize+QString("/")+appIcon+ext);
            return appIcon;
        }
        else
        {
            if(emptyExt)
                ext = ".svg";

            if(QFile::exists("/usr/share/icons/hicolor/64x64/"+iconSize+QString("/")+appIcon+ext))
            {
                appIcon = QString("/usr/share/icons/hicolor/64x64/"+iconSize+QString("/")+appIcon+ext);
                return appIcon;
            }
        }
    }

    iconsSize = QDir("/usr/share/icons/hicolor/80x80/").entryList(QStringList(),
                QDir::AllDirs | QDir::NoDotAndDotDot);
    foreach(QString iconSize, iconsSize)
    {
        if(emptyExt)
            ext = ".png";

        if(QFile::exists("/usr/share/icons/hicolor/80x80/"+iconSize+QString("/")+appIcon+ext))
        {
            appIcon = QString("/usr/share/icons/hicolor/80x80/"+iconSize+QString("/")+appIcon+ext);
            return appIcon;
        }
        else
        {
            if(emptyExt)
                ext = ".svg";

            if(QFile::exists("/usr/share/icons/hicolor/80x80/"+iconSize+QString("/")+appIcon+ext))
            {
                appIcon = QString("/usr/share/icons/hicolor/80x80/"+iconSize+QString("/")+appIcon+ext);
                return appIcon;
            }
        }
    }

    iconsSize = QDir("/usr/share/icons/hicolor/48x48/").entryList(QStringList(),
                QDir::AllDirs | QDir::NoDotAndDotDot);
    foreach(QString iconSize, iconsSize)
    {
        if(emptyExt)
            ext = ".png";

        if(QFile::exists("/usr/share/icons/hicolor/48x48/"+iconSize+QString("/")+appIcon+ext))
        {
            appIcon = QString("/usr/share/icons/hicolor/48x48/"+iconSize+QString("/")+appIcon+ext);
            return appIcon;
        }
        else
        {
            if(emptyExt)
                ext = ".svg";

            if(QFile::exists("/usr/share/icons/hicolor/48x48/"+iconSize+QString("/")+appIcon+ext))
            {
                appIcon = QString("/usr/share/icons/hicolor/48x48/"+iconSize+QString("/")+appIcon+ext);
                return appIcon;
            }
        }
    }



    iconsSize = QDir("/usr/share/icons/hicolor/scalable/").entryList(QStringList(),
                QDir::AllDirs | QDir::NoDotAndDotDot);



    foreach(QString iconSize, iconsSize)
    {
        if(emptyExt)
            ext = ".png";

        if(QFile::exists("/usr/share/icons/hicolor/scalable/"+iconSize+QString("/")+appIcon+ext))
        {
            appIcon = QString("/usr/share/icons/hicolor/scalable/"+iconSize+QString("/")+appIcon+ext);
            return appIcon;
        }
        else
        {

            if(emptyExt)
                ext = ".svg";

            if(QFile::exists("/usr/share/icons/hicolor/scalable/"+iconSize+QString("/")+appIcon+ext))
            {
                appIcon = QString("/usr/share/icons/hicolor/scalable/"+iconSize+QString("/")+appIcon+ext);
                return appIcon;
            }
        }
    }

    iconsSize = QDir("/usr/share/icons/hicolor/scaleable/").entryList(QStringList(), //gnuplot
                QDir::AllDirs | QDir::NoDotAndDotDot);



    foreach(QString iconSize, iconsSize)
    {
        if(emptyExt)
            ext = ".png";

        if(QFile::exists("/usr/share/icons/hicolor/scaleable/"+iconSize+QString("/")+appIcon+ext))
        {
            appIcon = QString("/usr/share/icons/hicolor/scaleable/"+iconSize+QString("/")+appIcon+ext);
            return appIcon;
        }
        else
        {

            if(emptyExt)
                ext = ".svg";

            if(QFile::exists("/usr/share/icons/hicolor/scaleable/"+iconSize+QString("/")+appIcon+ext))
            {
                appIcon = QString("/usr/share/icons/hicolor/scaleable/"+iconSize+QString("/")+appIcon+ext);
                return appIcon;
            }
        }
    }

    if(emptyExt)
        ext = ".png";

        if(QFile::exists("/usr/share/pixmaps/"+appIcon+ext))
        {
            appIcon = QString("/usr/share/pixmaps/"+appIcon+ext);
            return appIcon;
        }

        return QString();
}


