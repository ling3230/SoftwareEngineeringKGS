#ifndef USERREPOSITORY_H
#define USERREPOSITORY_H

#include <QObject>
#include <QSharedPointer>
#include <QSqlRecord>

#include "../entity/userprofile.h"
#include "../entity/careerdirection.h"

class DatabaseManager;

class UserRepository : public QObject
{
    Q_OBJECT
public:
    explicit UserRepository(QObject* parent = nullptr);
    ~UserRepository();

    static UserRepository* getInstance();

    bool initDatabase();

    bool saveUser(const UserProfile& user);
    UserProfile findById(int id);
    UserProfile findByUsername(const QString& username);
    bool updateUser(const UserProfile& user);
    bool deleteUser(int id);

    // 新增：职业方向相关方法
    bool updateUserCareer(int userId, CareerDirection direction);
    bool updateUserCareerSelection(int userId, CareerDirection direction, const QString& techStackId);
    bool updateFirstTimeFlag(int userId, bool firstTime);

private:
    QSharedPointer<DatabaseManager> dbManager;
    static UserRepository* instance;
};

#endif // USERREPOSITORY_H
