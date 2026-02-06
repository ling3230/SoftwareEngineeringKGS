#include "knowledgerepository.h"
#include "core/database/databasemanager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDebug>

// 初始化静态成员
KnowledgeRepository* KnowledgeRepository::instance = nullptr;

KnowledgeRepository::KnowledgeRepository(QObject *parent)
    : QObject(parent)
{

    dbManager = QSharedPointer<DatabaseManager>(DatabaseManager::getInstance());

    // 确保数据库连接
    if (!dbManager->isConnected()) {
        dbManager->connect();
    }

    // 初始化数据库表
    initDatabase();
}

KnowledgeRepository::~KnowledgeRepository()
{
    // 清理资源
}

KnowledgeRepository* KnowledgeRepository::getInstance()
{
    if (instance == nullptr) {
        instance = new KnowledgeRepository();
    }
    return instance;
}

bool KnowledgeRepository::initDatabase()
{
    if (!dbManager || !dbManager->isConnected()) {
        qWarning() << "数据库未连接，无法初始化";
        return false;
    }

    QSqlDatabase db = dbManager->getDatabase();
    QSqlQuery query(db);

    // 创建知识点表
    QString createTableSql = R"(
        CREATE TABLE IF NOT EXISTS knowledge_points (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            title TEXT NOT NULL,
            description TEXT,
            content TEXT,
            type INTEGER DEFAULT 0,
            difficulty INTEGER DEFAULT 1,
            estimated_time INTEGER DEFAULT 60,
            parent_id INTEGER DEFAULT -1,
            prerequisite_ids TEXT DEFAULT '[]',
            child_ids TEXT DEFAULT '[]',
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            updated_at DATETIME DEFAULT CURRENT_TIMESTAMP
        )
    )";

    if (!query.exec(createTableSql)) {
        qWarning() << "创建knowledge_points表失败：" << query.lastError().text();
        return false;
    }

    qDebug() << "数据库表初始化完成";
    return true;
}

//======核心CRUD======
bool KnowledgeRepository::saveKnowledge(const KnowledgePoint& knowledge)
{
    if (!dbManager || !dbManager->isConnected()) {
        qWarning() << "数据库未连接，无法保存知识点";
        return false;
    }

    QSqlDatabase db = dbManager->getDatabase();
    QSqlQuery query(db);

    QVariantMap data = knowledgeToMap(knowledge);//QVariantMap = QMap<QString, QVariant>

    if (knowledge.getId() > 0) {
        // 更新现有记录
        query.prepare("UPDATE knowledge_points SET "
                      "title = :title, description = :description, "
                      "content = :content, type = :type, "
                      "difficulty = :difficulty, estimated_time = :estimated_time, "
                      "parent_id = :parent_id, prerequisite_ids = :prerequisite_ids, "
                      "child_ids = :child_ids, updated_at = CURRENT_TIMESTAMP "
                      "WHERE id = :id");
    } else {
        // 插入新记录
        query.prepare("INSERT INTO knowledge_points "
                      "(title, description, content, type, difficulty, "
                      "estimated_time, parent_id, prerequisite_ids, child_ids) "
                      "VALUES (:title, :description, :content, :type, "
                      ":difficulty, :estimated_time, :parent_id, "
                      ":prerequisite_ids, :child_ids)");
    }

    // 绑定所有参数
    for (auto it = data.constBegin(); it != data.constEnd(); ++it) {
        query.bindValue(":" + it.key(), it.value());
    }

    if (!query.exec()) {
        qWarning() << "保存知识点失败：" << query.lastError().text();
        qWarning() << "SQL:" << query.lastQuery();
        return false;
    }

    // 如果是新插入的记录，获取生成的ID
    if (knowledge.getId() <= 0) {
        int newId = query.lastInsertId().toInt();
        const_cast<KnowledgePoint&>(knowledge).setId(newId);
    }

    qDebug() << "知识点保存成功，ID:" << knowledge.getId();
    return true;
}
KnowledgePoint KnowledgeRepository::findById(int id)
{
    if (!dbManager || !dbManager->isConnected()) {
        qWarning() << "数据库未连接";
        return KnowledgePoint();
    }

    if (id <= 0) {
        qWarning() << "无效的知识点ID";
        return KnowledgePoint();
    }

    QSqlDatabase db = dbManager->getDatabase();
    QSqlQuery query(db);
    query.prepare("SELECT * FROM knowledge_points WHERE id = ?");
    query.addBindValue(id);

    if (!query.exec()) {
        qWarning() << "查询知识点失败：" << query.lastError().text();
        return KnowledgePoint();
    }

    if (query.next()) {
        return mapToKnowledge(query.record());
    }

    qDebug() << "未找到知识点，ID:" << id;
    return KnowledgePoint();
}
QList<KnowledgePoint> KnowledgeRepository::findAll()
{
    QList<KnowledgePoint> result;

    if (!dbManager || !dbManager->isConnected()) {
        qWarning() << "数据库未连接";
        return result;
    }

    QSqlDatabase db = dbManager->getDatabase();
    QSqlQuery query("SELECT * FROM knowledge_points ORDER BY parent_id, id", db);

    if (!query.exec()) {
        qWarning() << "查询所有知识点失败：" << query.lastError().text();
        return result;
    }

    while (query.next()) {
        result.append(mapToKnowledge(query.record()));
    }

    qDebug() << "找到" << result.size() << "个知识点";
    return result;
}
bool KnowledgeRepository::deleteKnowledge(int id)
{
    if (!dbManager || !dbManager->isConnected()) {
        qWarning() << "数据库未连接";
        return false;
    }

    if (id <= 0) {
        qWarning() << "无效的知识点ID";
        return false;
    }

    QSqlDatabase db = dbManager->getDatabase();
    QSqlQuery query(db);
    query.prepare("DELETE FROM knowledge_points WHERE id = ?");
    query.addBindValue(id);

    if (!query.exec()) {
        qWarning() << "删除知识点失败：" << query.lastError().text();
        return false;
    }

    qDebug() << "知识点删除成功，ID:" << id;
    return true;
}
bool KnowledgeRepository::updateKnowledge(const KnowledgePoint& knowledge)
{
    return saveKnowledge(knowledge);
}

int KnowledgeRepository::countAll()
{
    if (!dbManager || !dbManager->isConnected()) {
        return 0;
    }

    QSqlDatabase db = dbManager->getDatabase();
    QSqlQuery query("SELECT COUNT(*) FROM knowledge_points", db);

    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }

    return 0;
}

//======数据转换======(****在这匹配)
QList<int> KnowledgeRepository::parseIdList(const QString& jsonStr)
{
    QList<int> ids;

    if (jsonStr.isEmpty() || jsonStr == "[]") {
        return ids;
    }

    QJsonDocument doc = QJsonDocument::fromJson(jsonStr.toUtf8());
    if (doc.isArray()) {
        QJsonArray array = doc.array();
        for (const QJsonValue& value : array) {
            ids.append(value.toInt());
        }
    }

    return ids;
}
QString KnowledgeRepository::idListToJson(const QList<int>& ids)
{
    QJsonArray array;
    for (int id : ids) {
        array.append(id);
    }

    QJsonDocument doc(array);
    return doc.toJson(QJsonDocument::Compact);
}
KnowledgePoint KnowledgeRepository::mapToKnowledge(const QSqlRecord& record)
{
    KnowledgePoint kp;

    kp.setId(record.value("id").toInt());
    kp.setTitle(record.value("title").toString());
    kp.setDescription(record.value("description").toString());
    kp.setContent(record.value("content").toString());

    // 枚举类型转换
    int typeValue = record.value("type").toInt();
    kp.setType(static_cast<KnowledgeType>(typeValue));

    int difficultyValue = record.value("difficulty").toInt();
    kp.setDifficulty(static_cast<DifficultyLevel>(difficultyValue));

    kp.setEstimatedTime(record.value("estimated_time").toInt());
    kp.setParentId(record.value("parent_id").toInt());

    // 解析JSON数组
    QString prereqJson = record.value("prerequisite_ids").toString();
    kp.setPrerequisiteIds(parseIdList(prereqJson));

    QString childJson = record.value("child_ids").toString();
    kp.setChildIds(parseIdList(childJson));

    return kp;
}
QVariantMap KnowledgeRepository::knowledgeToMap(const KnowledgePoint& knowledge)
{
    QVariantMap data;

    if (knowledge.getId() > 0) {
        data["id"] = knowledge.getId();
    }

    data["title"] = knowledge.getTitle();
    data["description"] = knowledge.getDescription();
    data["content"] = knowledge.getContent();
    data["type"] = static_cast<int>(knowledge.getType());
    data["difficulty"] = static_cast<int>(knowledge.getDifficulty());
    data["estimated_time"] = knowledge.getEstimatedTime();
    data["parent_id"] = knowledge.getParentId();
    data["prerequisite_ids"] = idListToJson(knowledge.getPrerequisiteIds());
    data["child_ids"] = idListToJson(knowledge.getChildIds());

    return data;
}




