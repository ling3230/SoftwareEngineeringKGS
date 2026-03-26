#ifndef USERPROFILE_H
#define USERPROFILE_H

#include <QString>
#include <QDateTime>
#include <QSqlRecord>
#include "careerdirection.h"

class UserProfile
{
public:
    enum class Role { GUEST = 0, USER = 1 };

    UserProfile();
    UserProfile(const QString& username, Role role = Role::USER);

    // 基础属性
    int getId() const;
    QString getUsername() const;
    Role getRole() const;
    bool isGuest() const;

    // 持久化字段
    QString getPasswordHash() const;
    QString getSalt() const;
    QString getDisplayName() const;
    QString getEmail() const;
    QDateTime getCreatedAt() const;
    QDateTime getUpdatedAt() const;
    QString getExtraJson() const;

    // 设置方法
    void setId(int id);
    void setUsername(const QString& username);
    void setRole(Role role);
    void setPasswordHash(const QString& hash);
    void setSalt(const QString& salt);
    void setDisplayName(const QString& name);
    void setEmail(const QString& email);
    void setCreatedAt(const QDateTime& dt);
    void setUpdatedAt(const QDateTime& dt);
    void setExtraJson(const QString& json);

    // 数据库映射
    static UserProfile fromSqlRecord(const QSqlRecord& record);

    // 新增：职业方向相关
    CareerDirection getCareerDirection() const { return careerDirection; }
    QString getCareerDirectionString() const;
    void setCareerDirection(CareerDirection direction);
    QString getTechStackId() const { return techStackId; }
    void setTechStackId(const QString& id) { techStackId = id; }

    // 新增：首次使用标志
    bool isFirstTime() const { return firstTime; }
    void setFirstTime(bool first) { firstTime = first; }

private:
    int id = -1;
    QString username;
    Role role = Role::GUEST;

    // 新增字段
    CareerDirection careerDirection = CareerDirection::DEFAULT_SE_COURSE;
    QString techStackId;
    bool firstTime = true;  // 是否首次使用

    // 持久化字段
    QString passwordHash;
    QString salt;
    QString displayName;
    QString email;
    QDateTime createdAt;
    QDateTime updatedAt;
    QString extraJson;
};

#endif // USERPROFILE_H
