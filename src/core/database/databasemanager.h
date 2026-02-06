#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QString>

class DatabaseManager : public QObject
{
    Q_OBJECT

public:
    // 单例模式
    static DatabaseManager* getInstance();

    // 基础功能
    bool connect(const QString& dbPath = "se_kg.db");
    bool disconnect();
    bool isConnected() const;

    QSqlDatabase getDatabase() const;

    ~DatabaseManager();

private:
    explicit DatabaseManager(QObject *parent = nullptr);

    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    static DatabaseManager* instance;
    QSqlDatabase database;
};

#endif // DATABASEMANAGER_H
