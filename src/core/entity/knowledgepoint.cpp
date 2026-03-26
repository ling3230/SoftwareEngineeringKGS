#include "knowledgepoint.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QDebug>

namespace {

QJsonArray toJsonArray(const QList<QString>& ids)
{
    QJsonArray array;
    for (const QString& id : ids) {
        array.append(id);
    }
    return array;
}

QList<QString> fromJsonArray(const QJsonValue& value)
{
    QList<QString> ids;
    const QJsonArray array = value.toArray();
    for (const QJsonValue& item : array) {
        if (item.isString()) {
            ids.append(item.toString());
        } else if (item.isDouble()) {
            ids.append(QString::number(item.toInt()));
        }
    }
    return ids;
}

QList<QString> deduplicateIds(const QList<QString>& input)
{
    QList<QString> result;
    for (const QString& id : input) {
        const QString trimmed = id.trimmed();
        if (!trimmed.isEmpty() && !result.contains(trimmed)) {
            result.append(trimmed);
        }
    }
    return result;
}

}

// 构造函数
KnowledgePoint::KnowledgePoint()
    : id("")
    , type(KnowledgeType::CONCEPT)
    , difficulty(DifficultyLevel::BEGINNER)
    , estimatedTime(60)  // 默认60分钟
    , importance(5)
    , isCore(false)
    , isLearned(false)
    , masteryLevel(0)
{
    const QDateTime now = QDateTime::currentDateTime();
    createdAt = now;
    updatedAt = now;
    lastStudied = QDateTime();
}

KnowledgePoint::KnowledgePoint(const QString& id, const QString& title, const QString& description)
    : id(id)
    , title(title)
    , description(description)
    , type(KnowledgeType::CONCEPT)
    , difficulty(DifficultyLevel::BEGINNER)
    , estimatedTime(60)
    , importance(5)
    , isCore(false)
    , isLearned(false)
    , masteryLevel(0)
{
    const QDateTime now = QDateTime::currentDateTime();
    createdAt = now;
    updatedAt = now;
    lastStudied = QDateTime();
}

KnowledgePoint::KnowledgePoint(int id, const QString& title, const QString& description)
    : KnowledgePoint(QString::number(id), title, description)
{
}

// 在构造函数后面添加：
KnowledgePoint::KnowledgePoint(const KnowledgePoint& other)
    : id(other.id)
    , title(other.title)
    , description(other.description)
    , content(other.content)
    , type(other.type)
    , difficulty(other.difficulty)
    , estimatedTime(other.estimatedTime)
    , prerequisiteIds(other.prerequisiteIds)
    , postrequisiteIds(other.postrequisiteIds)
    , category(other.category)
    , tags(other.tags)
    , importance(other.importance)
    , isCore(other.isCore)
    , isLearned(other.isLearned)
    , masteryLevel(other.masteryLevel)
    , lastStudied(other.lastStudied)
    , createdAt(other.createdAt)
    , updatedAt(other.updatedAt)
{
}

KnowledgePoint& KnowledgePoint::operator=(const KnowledgePoint& other)
{
    if (this != &other) {
        id = other.id;
        title = other.title;
        description = other.description;
        content = other.content;
        type = other.type;
        difficulty = other.difficulty;
        estimatedTime = other.estimatedTime;
        prerequisiteIds = other.prerequisiteIds;
        postrequisiteIds = other.postrequisiteIds;
        category = other.category;
        tags = other.tags;
        importance = other.importance;
        isCore = other.isCore;
        isLearned = other.isLearned;
        masteryLevel = other.masteryLevel;
        lastStudied = other.lastStudied;
        createdAt = other.createdAt;
        updatedAt = other.updatedAt;
    }
    return *this;
}

// ============ Getter方法实现 ============
QString KnowledgePoint::getId() const {
    return id;
}
int KnowledgePoint::getIdAsInt() const {
    return id.toInt();
}
QString KnowledgePoint::getTitle() const {
    return title;
}
QString KnowledgePoint::getDescription() const {
    return description;
}
QString KnowledgePoint::getContent() const {
    return content;
}
KnowledgeType KnowledgePoint::getType() const {
    return type;
}
DifficultyLevel KnowledgePoint::getDifficulty() const {
    return difficulty;
}
int KnowledgePoint::getEstimatedTime() const {
    return estimatedTime;
}
QList<QString> KnowledgePoint::getPrerequisiteIds() const {
    return prerequisiteIds;
}
QList<QString> KnowledgePoint::getPostrequisiteIds() const {
    return postrequisiteIds;
}
QList<QString> KnowledgePoint::getChildIds() const {
    return postrequisiteIds;
}
QString KnowledgePoint::getPrimaryParentId() const {
    if (prerequisiteIds.isEmpty()) {
        return QString();
    }
    return prerequisiteIds.first();
}
int KnowledgePoint::getParentId() const {
    return getPrimaryParentId().toInt();
}
QString KnowledgePoint::getCategory() const {
    return category;
}
QList<QString> KnowledgePoint::getTags() const {
    return tags;
}
int KnowledgePoint::getImportance() const {
    return importance;
}
bool KnowledgePoint::getIsCore() const {
    return isCore;
}
bool KnowledgePoint::getIsLearned() const {
    return isLearned;
}
int KnowledgePoint::getMasteryLevel() const {
    return masteryLevel;
}
QDateTime KnowledgePoint::getLastStudied() const {
    return lastStudied;
}
QDateTime KnowledgePoint::getCreatedAt() const {
    return createdAt;
}
QDateTime KnowledgePoint::getUpdatedAt() const {
    return updatedAt;
}

// ============ Setter方法实现 ============
void KnowledgePoint::setId(const QString& id) {
    this->id = id.trimmed();
    updatedAt = QDateTime::currentDateTime();
}
void KnowledgePoint::setId(int id) {
    this->id = QString::number(id);
    updatedAt = QDateTime::currentDateTime();
}
void KnowledgePoint::setTitle(const QString& title) {
    this->title = title;
    updatedAt = QDateTime::currentDateTime();
}
void KnowledgePoint::setDescription(const QString& description) {
    this->description = description;
    updatedAt = QDateTime::currentDateTime();
}
void KnowledgePoint::setContent(const QString& content) {
    this->content = content;
    updatedAt = QDateTime::currentDateTime();
}
void KnowledgePoint::setType(KnowledgeType type) {
    this->type = type;
    updatedAt = QDateTime::currentDateTime();
}
void KnowledgePoint::setDifficulty(DifficultyLevel difficulty) {
    this->difficulty = difficulty;
    updatedAt = QDateTime::currentDateTime();
}
void KnowledgePoint::setEstimatedTime(int minutes) {
    if (minutes > 0) {
        this->estimatedTime = minutes;
        updatedAt = QDateTime::currentDateTime();
    }
}
void KnowledgePoint::setPrerequisiteIds(const QList<QString>& ids) {
    prerequisiteIds = deduplicateIds(ids);
    updatedAt = QDateTime::currentDateTime();
}
void KnowledgePoint::setPostrequisiteIds(const QList<QString>& ids) {
    postrequisiteIds = deduplicateIds(ids);
    updatedAt = QDateTime::currentDateTime();
}
void KnowledgePoint::setChildIds(const QList<QString>& ids) {
    setPostrequisiteIds(ids);
}
void KnowledgePoint::setPrimaryParentId(const QString& parentId) {
    const QString trimmed = parentId.trimmed();
    if (trimmed.isEmpty()) {
        if (!prerequisiteIds.isEmpty()) {
            prerequisiteIds.removeFirst();
        }
    } else if (prerequisiteIds.isEmpty()) {
        prerequisiteIds.append(trimmed);
    } else {
        prerequisiteIds[0] = trimmed;
    }
    prerequisiteIds = deduplicateIds(prerequisiteIds);
    updatedAt = QDateTime::currentDateTime();
}
void KnowledgePoint::setParentId(int parentId) {
    if (parentId > 0) {
        setPrimaryParentId(QString::number(parentId));
    } else {
        setPrimaryParentId(QString());
    }
}
void KnowledgePoint::setCategory(const QString& category) {
    this->category = category;
    updatedAt = QDateTime::currentDateTime();
}
void KnowledgePoint::setTags(const QList<QString>& tags) {
    this->tags = deduplicateIds(tags);
    updatedAt = QDateTime::currentDateTime();
}
void KnowledgePoint::setImportance(int importance) {
    if (importance < 1) {
        this->importance = 1;
    } else if (importance > 10) {
        this->importance = 10;
    } else {
        this->importance = importance;
    }
    updatedAt = QDateTime::currentDateTime();
}
void KnowledgePoint::setIsCore(bool isCore) {
    this->isCore = isCore;
    updatedAt = QDateTime::currentDateTime();
}
void KnowledgePoint::setIsLearned(bool learned) {
    this->isLearned = learned;
    if (learned && masteryLevel < 1) {
        masteryLevel = 1;  // 至少设置为1
    }
    updatedAt = QDateTime::currentDateTime();
}
void KnowledgePoint::setMasteryLevel(int level) {
    // 限制在0-100之间
    if (level < 0) masteryLevel = 0;
    else if (level > 100) masteryLevel = 100;
    else masteryLevel = level;

    // 如果掌握度>0，自动标记为已学习
    if (masteryLevel > 0 && !isLearned) {
        isLearned = true;
    }

    updatedAt = QDateTime::currentDateTime();
}
void KnowledgePoint::setLastStudied(const QDateTime& time) {
    this->lastStudied = time;
    updatedAt = QDateTime::currentDateTime();
}

// ============ 业务方法实现 ============
bool KnowledgePoint::canLearn() const {
    // 如果可以学习返回true（没有先修要求或已满足先修条件）
    // 注意：实际实现中需要检查先修知识点是否已掌握
    // 这里简化处理：只要没有先修要求就可以学习
    return prerequisiteIds.isEmpty();
}
QString KnowledgePoint::getTypeString() const {
    switch (type) {
    case KnowledgeType::CONCEPT:
        return "概念";
    case KnowledgeType::PRACTICE:
        return "实践";
    case KnowledgeType::THEORY:
        return "原理";
    case KnowledgeType::TOOL:
        return "工具";
    default:
        return "未知";
    }
}
QString KnowledgePoint::getDifficultyString() const {
    switch (difficulty) {
    case DifficultyLevel::BEGINNER:
        return "入门";
    case DifficultyLevel::EASY:
        return "简单";
    case DifficultyLevel::INTERMEDIATE:
        return "中等";
    case DifficultyLevel::ADVANCED:
        return "进阶";
    case DifficultyLevel::EXPERT:
        return "专家";
    default:
        return "未知";
    }
}
QString KnowledgePoint::toString() const {
    return QString("KnowledgePoint[ID:%1, 标题:%2, 类型:%3, 难度:%4, 前置:%5, 后置:%6]")
        .arg(id)
        .arg(title)
        .arg(getTypeString())
        .arg(getDifficultyString())
        .arg(prerequisiteIds.size())
        .arg(postrequisiteIds.size());
}

// ============ 状态更新方法 ============
void KnowledgePoint::markAsLearned(int masteryLevel) {
    isLearned = true;
    setMasteryLevel(masteryLevel);
    lastStudied = QDateTime::currentDateTime();
    updatedAt = QDateTime::currentDateTime();
}
void KnowledgePoint::markAsUnlearned() {
    isLearned = false;
    masteryLevel = 0;
    updatedAt = QDateTime::currentDateTime();
}
void KnowledgePoint::updateMastery(int delta) {
    int newLevel = masteryLevel + delta;
    setMasteryLevel(newLevel);

    // 更新最后学习时间
    if (delta > 0) {
        lastStudied = QDateTime::currentDateTime();
        if (!isLearned && newLevel > 0) {
            isLearned = true;
        }
    }

    updatedAt = QDateTime::currentDateTime();
}

// ============ 关系检查方法 ============
bool KnowledgePoint::hasParent() const {
    return !prerequisiteIds.isEmpty();
}
bool KnowledgePoint::hasChildren() const {
    return !postrequisiteIds.isEmpty();
}
bool KnowledgePoint::hasPrerequisites() const {
    return !prerequisiteIds.isEmpty();
}
bool KnowledgePoint::isPrerequisiteFor(const QString& knowledgeId) const {
    // 当前节点是否直接指向目标后置节点
    return postrequisiteIds.contains(knowledgeId.trimmed());
}

// ============ 序列化/反序列化方法 ============
QJsonObject KnowledgePoint::toJson() const {
    QJsonObject json;
    json["id"] = id;
    json["title"] = title;
    json["description"] = description;
    json["content"] = content;
    json["type"] = static_cast<int>(type);
    json["difficulty"] = static_cast<int>(difficulty);
    json["estimatedTime"] = estimatedTime;
    json["prerequisiteIds"] = toJsonArray(prerequisiteIds);
    json["postrequisiteIds"] = toJsonArray(postrequisiteIds);
    // 兼容旧字段名
    json["childIds"] = toJsonArray(postrequisiteIds);

    json["category"] = category;
    json["tags"] = toJsonArray(tags);
    json["importance"] = importance;
    json["isCore"] = isCore;

    // 学习状态
    json["isLearned"] = isLearned;
    json["masteryLevel"] = masteryLevel;
    json["lastStudied"] = lastStudied.toString(Qt::ISODate);
    json["createdAt"] = createdAt.toString(Qt::ISODate);
    json["updatedAt"] = updatedAt.toString(Qt::ISODate);

    return json;
}
KnowledgePoint KnowledgePoint::fromJson(const QJsonObject& json) {
    KnowledgePoint kp;

    if (json["id"].isString()) {
        kp.id = json["id"].toString();
    } else if (json["id"].isDouble()) {
        kp.id = QString::number(json["id"].toInt());
    }
    kp.title = json["title"].toString();
    kp.description = json["description"].toString();
    kp.content = json["content"].toString();
    kp.type = static_cast<KnowledgeType>(json["type"].toInt());
    kp.difficulty = static_cast<DifficultyLevel>(json["difficulty"].toInt());
    kp.estimatedTime = json["estimatedTime"].toInt();

    kp.prerequisiteIds = deduplicateIds(fromJsonArray(json["prerequisiteIds"]));
    if (kp.prerequisiteIds.isEmpty() && json.contains("parentId") && json["parentId"].toInt() > 0) {
        kp.prerequisiteIds.append(QString::number(json["parentId"].toInt()));
    }

    kp.postrequisiteIds = deduplicateIds(fromJsonArray(json["postrequisiteIds"]));
    if (kp.postrequisiteIds.isEmpty()) {
        kp.postrequisiteIds = deduplicateIds(fromJsonArray(json["childIds"]));
    }

    kp.category = json["category"].toString();
    kp.tags = deduplicateIds(fromJsonArray(json["tags"]));
    kp.importance = json.contains("importance") ? json["importance"].toInt() : 5;
    kp.isCore = json["isCore"].toBool(false);

    // 学习状态
    kp.isLearned = json["isLearned"].toBool();
    kp.masteryLevel = json["masteryLevel"].toInt();
    QString lastStudiedStr = json["lastStudied"].toString();
    if (!lastStudiedStr.isEmpty()) {
        kp.lastStudied = QDateTime::fromString(lastStudiedStr, Qt::ISODate);
    }

    QString createdAtStr = json["createdAt"].toString();
    if (!createdAtStr.isEmpty()) {
        kp.createdAt = QDateTime::fromString(createdAtStr, Qt::ISODate);
    }

    QString updatedAtStr = json["updatedAt"].toString();
    if (!updatedAtStr.isEmpty()) {
        kp.updatedAt = QDateTime::fromString(updatedAtStr, Qt::ISODate);
    }

    if (!kp.createdAt.isValid()) {
        kp.createdAt = QDateTime::currentDateTime();
    }
    if (!kp.updatedAt.isValid()) {
        kp.updatedAt = kp.createdAt;
    }

    return kp;
}
QString KnowledgePoint::toJsonString() const {
    QJsonDocument doc(toJson());
    return doc.toJson(QJsonDocument::Indented);
}
KnowledgePoint KnowledgePoint::fromJsonString(const QString& jsonStr) {
    QJsonDocument doc = QJsonDocument::fromJson(jsonStr.toUtf8());
    if (doc.isNull()) {
        qWarning() << "JSON字符串解析失败：" << jsonStr;
        return KnowledgePoint();
    }
    return fromJson(doc.object());
}

// ============ 关系操作方法 ============
void KnowledgePoint::addPrerequisite(const QString& knowledgeId) {
    const QString trimmed = knowledgeId.trimmed();
    if (!trimmed.isEmpty() && !hasPrerequisite(trimmed)) {
        prerequisiteIds.append(trimmed);
        updatedAt = QDateTime::currentDateTime();
    }
}
void KnowledgePoint::addPrerequisite(int knowledgeId) {
    if (knowledgeId > 0) {
        addPrerequisite(QString::number(knowledgeId));
    }
}
void KnowledgePoint::removePrerequisite(const QString& knowledgeId) {
    prerequisiteIds.removeAll(knowledgeId.trimmed());
    updatedAt = QDateTime::currentDateTime();
}
void KnowledgePoint::removePrerequisite(int knowledgeId) {
    removePrerequisite(QString::number(knowledgeId));
}
bool KnowledgePoint::hasPrerequisite(const QString& knowledgeId) const {
    return prerequisiteIds.contains(knowledgeId.trimmed());
}
bool KnowledgePoint::hasPrerequisite(int knowledgeId) const {
    return hasPrerequisite(QString::number(knowledgeId));
}
void KnowledgePoint::clearPrerequisites() {
    prerequisiteIds.clear();
    updatedAt = QDateTime::currentDateTime();
}

void KnowledgePoint::addPostrequisite(const QString& knowledgeId) {
    const QString trimmed = knowledgeId.trimmed();
    if (!trimmed.isEmpty() && !hasPostrequisite(trimmed)) {
        postrequisiteIds.append(trimmed);
        updatedAt = QDateTime::currentDateTime();
    }
}
void KnowledgePoint::removePostrequisite(const QString& knowledgeId) {
    postrequisiteIds.removeAll(knowledgeId.trimmed());
    updatedAt = QDateTime::currentDateTime();
}
bool KnowledgePoint::hasPostrequisite(const QString& knowledgeId) const {
    return postrequisiteIds.contains(knowledgeId.trimmed());
}
void KnowledgePoint::clearPostrequisites() {
    postrequisiteIds.clear();
    updatedAt = QDateTime::currentDateTime();
}

void KnowledgePoint::addChild(const QString& knowledgeId) {
    addPostrequisite(knowledgeId);
}
void KnowledgePoint::addChild(int knowledgeId) {
    if (knowledgeId > 0) {
        addPostrequisite(QString::number(knowledgeId));
    }
}
void KnowledgePoint::removeChild(const QString& knowledgeId) {
    removePostrequisite(knowledgeId);
}
void KnowledgePoint::removeChild(int knowledgeId) {
    removePostrequisite(QString::number(knowledgeId));
}
bool KnowledgePoint::hasChild(const QString& knowledgeId) const {
    return hasPostrequisite(knowledgeId);
}
bool KnowledgePoint::hasChild(int knowledgeId) const {
    return hasPostrequisite(QString::number(knowledgeId));
}
void KnowledgePoint::clearChildren() {
    clearPostrequisites();
}
