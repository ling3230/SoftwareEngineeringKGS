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

    // 新增：数据库初始化
    bool initializeDatabase();
    bool checkTableExists(const QString& tableName);

private:
    explicit DatabaseManager(QObject *parent = nullptr);

    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    static DatabaseManager* instance;
    QSqlDatabase database;

    // 创建各个表的方法
    bool createUsersTable();
    bool createKnowledgePointsTable();
    bool createKnowledgeRelationsTable();
    bool createLearningRecordsTable();

};

#endif // DATABASEMANAGER_H
