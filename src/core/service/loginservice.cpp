#include "loginservice.h"

#include <QDebug>
#include <QCryptographicHash>
#include <QUuid>

LoginService* LoginService::instance = nullptr;

LoginService::LoginService(QObject* parent) : QObject(parent)
{
    userRepo = UserRepository::getInstance();
}

LoginService* LoginService::getInstance() {
    if (!instance) {
        instance = new LoginService();
    }
    return instance;
}

static QString hashWithSalt(const QString& salt, const QString& password)
{
    QByteArray h = QCryptographicHash::hash((salt + password).toUtf8(), QCryptographicHash::Sha256).toHex();
    return QString(h);
}

bool LoginService::login(const QString& username, const QString& password) {

    if (username.isEmpty() || password.isEmpty()) {
        qDebug() << "用户名或密码为空";
        return false;
    }

    // 从仓库查找用户
    UserProfile u = userRepo->findByUsername(username);
    if (u.getId() <= 0) {
        qDebug() << "登录失败: 用户不存在";
        return false;
    }

    // QString expected = u.getPasswordHash();
    // QString salt = u.getSalt();
    // QString actual = hashWithSalt(salt, password);

    // if (expected == actual) {
    //     currentUser = u;
    //     loggedIn = true;
    //     qDebug() << "登录成功:" << username;
    //     return true;
    // }

    // ===== 临时方案：直接比较明文密码 =====
    // 注意：这只是临时测试用，不要用于生产环境！

    // 你可以在这里预设一些测试账号的密码
    QMap<QString, QString> testPasswords;
    testPasswords["admin"] = "admin123";
    testPasswords["zhangsan"] = "zhangsan123";
    testPasswords["lisi"] = "lisi123";
    testPasswords["wangwu"] = "wangwu123";
    testPasswords["zhaoliu"] = "zhaoliu123";
    testPasswords["tianqi"] = "tianqi123";

    if (testPasswords.contains(username) && testPasswords[username] == password) {
        currentUser = u;
        loggedIn = true;
        qDebug() << "登录成功(明文验证):" << username;
        return true;
    }

    qDebug() << "登录失败: 用户名或密码错误";
    return false;
}

UserProfile LoginService::loginAsGuest() {
    currentUser = UserProfile("Guest", UserProfile::Role::GUEST);
    loggedIn = true;
    qDebug() << "游客登录成功";
    return currentUser;
}

bool LoginService::isLoggedIn() const { return loggedIn; }
UserProfile LoginService::getCurrentUser() const { return currentUser; }

void LoginService::logout() {
    loggedIn = false;
    currentUser = UserProfile();
    qDebug() << "已退出登录";
}
