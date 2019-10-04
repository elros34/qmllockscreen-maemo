#ifndef APPITEM_H
#define APPITEM_H

#include <QObject>

class AppItem : public QObject
{
    Q_OBJECT
public:
    explicit AppItem(QObject *parent = 0);
    explicit AppItem(const QString &name, const QString &path, const bool &isDir, QObject *parent=0);

    QString name() const;
    void setName(const QString &name);

    QString path() const;
    void setPath(const QString &path);

    bool isDir() const;
    void setIsDir(const bool &isDir);

signals:
    void nameChanged();
    void pathChanged();
    void isDirChanged();

private:
    QString _name;
    QString _path;
    bool _isDir;

public slots:

};

#endif // APPITEM_H
