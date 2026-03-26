#include "userrepository.h"
#include "core/database/databasemanager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QJsonDocument>
#include <QDateTime>
#include <QCryptographicHash>

UserRepository* UserRepository::instance = nullptr;

UserRepository::UserRepository(QObject* parent) : QObject(parent)
{
    dbManager = QSharedPointer<DatabaseManager>(DatabaseManager::getInstance());

    if (!dbManager->isConnected()) {
        dbManager->connect();
    }

    initDatabase();
}

UserRepository::~UserRepository()
{
}

UserRepository* UserRepository::getInstance()
{
    if (instance == nullptr) {
        instance = new UserRepository();
    }
    return instance;
}

bool UserRepository::initDatabase()
{
    if (!dbManager || !dbManager->isConnected()) {
        qWarning() << "数据库未连接，无法初始化 users 表";
        return false;
    }

    QSqlDatabase db = dbManager->getDatabase();
    QSqlQuery query(db);

    QString createSql = R"(
        CREATE TABLE IF NOT EXISTS users (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            username TEXT NOT NULL UNIQUE,
            password_hash TEXT NOT NULL,
            salt TEXT,
            display_name TEXT,
            email TEXT,
            role INTEGER NOT NULL DEFAULT 1,
            created_at TEXT NOT NULL,
            updated_at TEXT,
            extra TEXT
        )
    )";

    if (!query.exec(createSql)) {
        qWarning() << "创建 users 表失败：" << query.lastError().text();
        return false;
    }

    const QStringList migrationSqlList = {
        "ALTER TABLE users ADD COLUMN career_direction INTEGER NOT NULL DEFAULT 0",
        "ALTER TABLE users ADD COLUMN tech_stack_id TEXT",
        "ALTER TABLE users ADD COLUMN first_time INTEGER NOT NULL DEFAULT 1"
    };

    for (const QString& migrationSql : migrationSqlList) {
        if (!query.exec(migrationSql)) {
            const QString errorText = query.lastError().text().toLower();
            if (!errorText.contains("duplicate") && !errorText.contains("exists")) {
                qWarning() << "users 表迁移失败：" << query.lastError().text();
            }
        }
    }

    // 若表中无用户，插入一个默认管理员
    QSqlQuery countQ(db);
    if (countQ.exec("SELECT COUNT(*) FROM users") && countQ.next()) {
        int cnt = countQ.value(0).toInt();
        if (cnt == 0) {
            // 默认管理员密码为 admin123（将以简单哈希存储）
            QString salt = QString::fromUtf8(QDateTime::currentDateTimeUtc().toString(Qt::ISODate).toUtf8().toHex());
            QByteArray hash = QCryptographicHash::hash((salt + "admin123").toUtf8(), QCryptographicHash::Sha256).toHex();
            QSqlQuery ins(db);
            ins.prepare("INSERT INTO users (username, password_hash, salt, display_name, email, role, created_at) VALUES (?, ?, ?, ?, ?, ?, ?)");
            ins.addBindValue("admin");
            ins.addBindValue(QString(hash));
            ins.addBindValue(salt);
            ins.addBindValue("Administrator");
            ins.addBindValue("admin@local");
            ins.addBindValue(1);
            ins.addBindValue(QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
            if (!ins.exec()) {
                qWarning() << "插入默认管理员失败：" << ins.lastError().text();
            } else {
                qDebug() << "已创建默认管理员 (admin)";
            }
        }
    }

    qDebug() << "users 表初始化完成";
    return true;
}

// 新增：更新用户职业方向
bool UserRepository::updateUserCareer(int userId, CareerDirection direction) {
    return updateUserCareerSelection(userId, direction, QString());
}

bool UserRepository::updateUserCareerSelection(int userId, CareerDirection direction, const QString& techStackId) {
    if (!dbManager || !dbManager->isConnected() || userId <= 0) {
        return false;
    }

    QSqlDatabase db = dbManager->getDatabase();
    QSqlQuery query(db);
    query.prepare("UPDATE users SET career_direction = ?, tech_stack_id = ?, updated_at = ?, first_time = 0 WHERE id = ?");
    query.addBindValue(static_cast<int>(direction));
    query.addBindValue(techStackId);
    query.addBindValue(QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
    query.addBindValue(userId);

    if (!query.exec()) {
        qWarning() << "更新用户职业方向失败：" << query.lastError().text();
        return false;
    }

    qDebug() << "用户职业选择已更新，用户ID:" << userId
             << "方向:" << static_cast<int>(direction)
             << "技术栈:" << techStackId;
    return true;
}

// 新增：更新首次使用标志
bool UserRepository::updateFirstTimeFlag(int userId, bool firstTime) {
    if (!dbManager || !dbManager->isConnected() || userId <= 0) {
        return false;
    }

    QSqlDatabase db = dbManager->getDatabase();
    QSqlQuery query(db);
    query.prepare("UPDATE users SET first_time = ? WHERE id = ?");
    query.addBindValue(firstTime ? 1 : 0);
    query.addBindValue(userId);

    if (!query.exec()) {
        qWarning() << "更新首次使用标志失败：" << query.lastError().text();
        return false;
    }

    return true;
}

bool UserRepository::saveUser(const UserProfile& user)
{
    if (!dbManager || !dbManager->isConnected()) return false;
    QSqlDatabase db = dbManager->getDatabase();
    QSqlQuery query(db);

    if (user.getId() > 0) {
        query.prepare("UPDATE users SET username = :username, password_hash = :password_hash, salt = :salt, display_name = :display_name, email = :email, role = :role, updated_at = :updated_at, extra = :extra WHERE id = :id");
        query.bindValue(":id", user.getId());
    } else {
        query.prepare("INSERT INTO users (username, password_hash, salt, display_name, email, role, created_at, extra) VALUES (:username, :password_hash, :salt, :display_name, :email, :role, :created_at, :extra)");
        query.bindValue(":created_at", QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
    }

    // 修改查询语句，包含新字段
    if (user.getId() > 0) {
        query.prepare("UPDATE users SET username = :username, password_hash = :password_hash, "
                      "salt = :salt, display_name = :display_name, email = :email, "
                      "role = :role, career_direction = :career_direction, tech_stack_id = :tech_stack_id, "
                      "first_time = :first_time, updated_at = :updated_at, extra = :extra "
                      "WHERE id = :id");
        query.bindValue(":id", user.getId());
        query.bindValue(":career_direction", static_cast<int>(user.getCareerDirection()));
        query.bindValue(":tech_stack_id", user.getTechStackId());
        query.bindValue(":first_time", user.isFirstTime() ? 1 : 0);
    } else {
        query.prepare("INSERT INTO users (username, password_hash, salt, display_name, "
                      "email, role, career_direction, tech_stack_id, first_time, created_at, extra) "
                      "VALUES (:username, :password_hash, :salt, :display_name, "
                      ":email, :role, :career_direction, :tech_stack_id, :first_time, :created_at, :extra)");
        query.bindValue(":career_direction", static_cast<int>(user.getCareerDirection()));
        query.bindValue(":tech_stack_id", user.getTechStackId());
        query.bindValue(":first_time", user.isFirstTime() ? 1 : 0);
        query.bindValue(":created_at", QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
    }

    query.bindValue(":username", user.getUsername());
    query.bindValue(":password_hash", user.getPasswordHash());
    query.bindValue(":salt", user.getSalt());
    query.bindValue(":display_name", user.getDisplayName());
    query.bindValue(":email", user.getEmail());
    query.bindValue(":role", static_cast<int>(user.getRole()));
    query.bindValue(":updated_at", QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
    query.bindValue(":extra", user.getExtraJson());

    if (!query.exec()) {
        qWarning() << "保存用户失败：" << query.lastError().text();
        return false;
    }

    return true;
}

UserProfile UserRepository::findById(int id)
{
    UserProfile u;
    if (!dbManager || !dbManager->isConnected()) return u;
    if (id <= 0) return u;

    QSqlDatabase db = dbManager->getDatabase();
    QSqlQuery query(db);
    query.prepare("SELECT * FROM users WHERE id = ?");
    query.addBindValue(id);
    if (!query.exec()) return u;
    if (query.next()) {
        return UserProfile::fromSqlRecord(query.record());
    }
    return u;
}

UserProfile UserRepository::findByUsername(const QString& username)
{
    UserProfile u;
    if (!dbManager || !dbManager->isConnected()) return u;
    if (username.isEmpty()) return u;

    QSqlDatabase db = dbManager->getDatabase();
    QSqlQuery query(db);
    query.prepare("SELECT * FROM users WHERE username = ?");
    query.addBindValue(username);
    if (!query.exec()) return u;
    if (query.next()) {
        return UserProfile::fromSqlRecord(query.record());
    }
    return u;
}

bool UserRepository::updateUser(const UserProfile& user)
{
    return saveUser(user);
}

bool UserRepository::deleteUser(int id)
{
    if (!dbManager || !dbManager->isConnected()) return false;
    if (id <= 0) return false;
    QSqlDatabase db = dbManager->getDatabase();
    QSqlQuery query(db);
    query.prepare("DELETE FROM users WHERE id = ?");
    query.addBindValue(id);
    if (!query.exec()) return false;
    return true;
}
