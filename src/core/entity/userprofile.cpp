#include "userprofile.h"
#include <QVariant>

UserProfile::UserProfile() : id(-1), role(Role::GUEST) {}

UserProfile::UserProfile(const QString& username, Role role)
    : id(-1), username(username), role(role) {}

int UserProfile::getId() const { return id; }
QString UserProfile::getUsername() const { return username; }
UserProfile::Role UserProfile::getRole() const { return role; }
bool UserProfile::isGuest() const { return role == Role::GUEST; }

QString UserProfile::getPasswordHash() const { return passwordHash; }
QString UserProfile::getSalt() const { return salt; }
QString UserProfile::getDisplayName() const { return displayName; }
QString UserProfile::getEmail() const { return email; }
QDateTime UserProfile::getCreatedAt() const { return createdAt; }
QDateTime UserProfile::getUpdatedAt() const { return updatedAt; }
QString UserProfile::getExtraJson() const { return extraJson; }

void UserProfile::setId(int id) { this->id = id; }
void UserProfile::setUsername(const QString& username) { this->username = username; }
void UserProfile::setRole(Role role) { this->role = role; }
void UserProfile::setPasswordHash(const QString& hash) { this->passwordHash = hash; }
void UserProfile::setSalt(const QString& salt) { this->salt = salt; }
void UserProfile::setDisplayName(const QString& name) { this->displayName = name; }
void UserProfile::setEmail(const QString& email) { this->email = email; }
void UserProfile::setCreatedAt(const QDateTime& dt) { this->createdAt = dt; }
void UserProfile::setUpdatedAt(const QDateTime& dt) { this->updatedAt = dt; }
void UserProfile::setExtraJson(const QString& json) { this->extraJson = json; }

// 新增方法实现
QString UserProfile::getCareerDirectionString() const {
    return careerDirectionToString(careerDirection);
}

void UserProfile::setCareerDirection(CareerDirection direction) {
    this->careerDirection = direction;
}

UserProfile UserProfile::fromSqlRecord(const QSqlRecord& record)
{
    UserProfile u;
    if (record.contains("id")) u.id = record.value("id").toInt();
    if (record.contains("username")) u.username = record.value("username").toString();
    if (record.contains("role")) u.role = static_cast<Role>(record.value("role").toInt());
    // 新增：职业方向
    if (record.contains("career_direction")) {
        int careerValue = record.value("career_direction").toInt();
        u.careerDirection = static_cast<CareerDirection>(careerValue);
    }
    if (record.contains("tech_stack_id")) {
        u.techStackId = record.value("tech_stack_id").toString();
    }
     // 新增：首次使用标志
    if (record.contains("first_time")) {
        u.firstTime = record.value("first_time").toBool();
    }

    if (record.contains("password_hash")) u.passwordHash = record.value("password_hash").toString();
    if (record.contains("salt")) u.salt = record.value("salt").toString();
    if (record.contains("display_name")) u.displayName = record.value("display_name").toString();
    if (record.contains("email")) u.email = record.value("email").toString();
    if (record.contains("created_at")) u.createdAt = QDateTime::fromString(record.value("created_at").toString(), Qt::ISODate);
    if (record.contains("updated_at")) u.updatedAt = QDateTime::fromString(record.value("updated_at").toString(), Qt::ISODate);
    if (record.contains("extra")) u.extraJson = record.value("extra").toString();
    return u;
}
