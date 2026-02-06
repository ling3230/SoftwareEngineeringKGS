#include "databasemanager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QCoreApplication>

// 初始化静态成员
DatabaseManager* DatabaseManager::instance = nullptr;

DatabaseManager::DatabaseManager(QObject *parent)
    : QObject(parent)
{
}

DatabaseManager::~DatabaseManager()
{
    disconnect();
}

DatabaseManager* DatabaseManager::getInstance()
{
    if (instance == nullptr) {
        instance = new DatabaseManager();
    }
    return instance;
}

bool DatabaseManager::connect(const QString& dbPath)
{
    if (database.isOpen()) {
        database.close();
    }

    // 创建连接
    database = QSqlDatabase::addDatabase("QSQLITE");
    QString absPath = QCoreApplication::applicationDirPath() + "/" + dbPath;
    database.setDatabaseName(absPath);

    qDebug() << "数据库路径：" << absPath;

    if (!database.open()) {
        qWarning() << "数据库连接失败：" << database.lastError().text();
        return false;
    }

    qDebug() << "数据库连接成功";
    return true;
}

bool DatabaseManager::disconnect()
{
    if (database.isOpen()) {
        database.close();
        qDebug() << "数据库已关闭";
        return true;
    }
    return false;
}

bool DatabaseManager::isConnected() const
{
    return database.isOpen();
}

QSqlDatabase DatabaseManager::getDatabase() const
{
    return database;
}
