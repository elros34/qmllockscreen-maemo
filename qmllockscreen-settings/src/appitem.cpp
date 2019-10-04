#include "appitem.h"

AppItem::AppItem(QObject *parent) :
    QObject(parent)
{
}

AppItem::AppItem(const QString &name, const QString &path, const bool &isDir, QObject *parent)
    : QObject(parent), _name(name), _path(path), _isDir(isDir)
{
}

QString AppItem::name() const
{
    return _name;
}

void AppItem::setName(const QString &name)
{
    if(name != _name)
    {
        _name = name;
        emit nameChanged();
    }
}

QString AppItem::path() const
{
    return _path;
}

void AppItem::setPath(const QString &path)
{
    if(path != _path)
    {
        _path = path;
        emit pathChanged();
    }
}

bool AppItem::isDir() const
{
    return _isDir;
}

void AppItem::setIsDir(const bool &isDir)
{
    if(isDir != _isDir)
    {
        _isDir = isDir;
        emit isDirChanged();
    }
}

