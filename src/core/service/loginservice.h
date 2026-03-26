#ifndef LOGINSERVICE_H
#define LOGINSERVICE_H

#include "../entity/userprofile.h"
#include "../repository/userrepository.h"
#include <QObject>
#include <QMap>

class LoginService : public QObject{
    Q_OBJECT

public:
    static LoginService* getInstance();

    // 登录方法
    bool login(const QString& username, const QString& password);//登入，1成功，0失败
    UserProfile loginAsGuest();
    bool isLoggedIn() const;
    UserProfile getCurrentUser() const;
    void logout();

private:
    LoginService(QObject* parent = nullptr);
    static LoginService* instance;

    UserProfile currentUser;
    bool loggedIn = false;

    // 持久化用户仓库
    UserRepository* userRepo = nullptr;
};

#endif // LOGINSERVICE_H
