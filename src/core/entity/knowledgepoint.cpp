#include "knowledgepoint.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QDebug>

// 构造函数
KnowledgePoint::KnowledgePoint()
    : id(-1)  // -1表示无效ID
    , type(KnowledgeType::CONCEPT)
    , difficulty(DifficultyLevel::BEGINNER)
    , estimatedTime(60)  // 默认60分钟
    , parentId(-1)  // -1表示没有父节点
    , isLearned(false)
    , masteryLevel(0)
{
    lastStudied = QDateTime();  // 默认构造为无效时间
}

KnowledgePoint::KnowledgePoint(int id, const QString& title, const QString& description)
    : id(id)
    , title(title)
    , description(description)
    , type(KnowledgeType::CONCEPT)
    , difficulty(DifficultyLevel::BEGINNER)
    , estimatedTime(60)
    , parentId(-1)
    , isLearned(false)
    , masteryLevel(0)
{
    lastStudied = QDateTime();
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
    , parentId(other.parentId)
    , prerequisiteIds(other.prerequisiteIds)
    , childIds(other.childIds)
    , isLearned(other.isLearned)
    , masteryLevel(other.masteryLevel)
    , lastStudied(other.lastStudied)
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
        parentId = other.parentId;
        prerequisiteIds = other.prerequisiteIds;
        childIds = other.childIds;
        isLearned = other.isLearned;
        masteryLevel = other.masteryLevel;
        lastStudied = other.lastStudied;
    }
    return *this;
}

// ============ Getter方法实现 ============
int KnowledgePoint::getId() const {
    return id;
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
int KnowledgePoint::getParentId() const {
    return parentId;
}
QList<int> KnowledgePoint::getPrerequisiteIds() const {
    return prerequisiteIds;
}
QList<int> KnowledgePoint::getChildIds() const {
    return childIds;
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

// ============ Setter方法实现 ============
void KnowledgePoint::setId(int id) {
    this->id = id;
}
void KnowledgePoint::setTitle(const QString& title) {
    this->title = title;
}
void KnowledgePoint::setDescription(const QString& description) {
    this->description = description;
}
void KnowledgePoint::setContent(const QString& content) {
    this->content = content;
}
void KnowledgePoint::setType(KnowledgeType type) {
    this->type = type;
}
void KnowledgePoint::setDifficulty(DifficultyLevel difficulty) {
    this->difficulty = difficulty;
}
void KnowledgePoint::setEstimatedTime(int minutes) {
    if (minutes > 0) {
        this->estimatedTime = minutes;
    }
}
void KnowledgePoint::setParentId(int parentId) {
    this->parentId = parentId;
}
void KnowledgePoint::setPrerequisiteIds(const QList<int>& ids) {
    this->prerequisiteIds = ids;
}
void KnowledgePoint::setChildIds(const QList<int>& ids) {
    this->childIds = ids;
}
void KnowledgePoint::setIsLearned(bool learned) {
    this->isLearned = learned;
    if (learned && masteryLevel < 1) {
        masteryLevel = 1;  // 至少设置为1
    }
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
}
void KnowledgePoint::setLastStudied(const QDateTime& time) {
    this->lastStudied = time;
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
    return QString("KnowledgePoint[ID:%1, 标题:%2, 类型:%3, 难度:%4]")
        .arg(id)
        .arg(title)
        .arg(getTypeString())
        .arg(getDifficultyString());
}

// ============ 状态更新方法 ============
void KnowledgePoint::markAsLearned(int masteryLevel) {
    isLearned = true;
    setMasteryLevel(masteryLevel);
    lastStudied = QDateTime::currentDateTime();
}
void KnowledgePoint::markAsUnlearned() {
    isLearned = false;
    masteryLevel = 0;
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
}

// ============ 关系检查方法 ============
bool KnowledgePoint::hasParent() const {
    return parentId > 0;  // 大于0表示有父节点
}
bool KnowledgePoint::hasChildren() const {
    return !childIds.isEmpty();
}
bool KnowledgePoint::hasPrerequisites() const {
    return !prerequisiteIds.isEmpty();
}
bool KnowledgePoint::isPrerequisiteFor(int knowledgeId) const {
    // 检查当前知识点是否是指定知识点的先修
    // 注意：这个方法的完整实现需要外部提供知识点的依赖关系
    // 这里只是占位实现
    Q_UNUSED(knowledgeId);
    return false;  // 实际实现中需要查询关系数据库
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
    json["parentId"] = parentId;

    // 将列表转换为JSON数组
    QJsonArray prereqArray;
    for (int prereqId : prerequisiteIds) {
        prereqArray.append(prereqId);
    }
    json["prerequisiteIds"] = prereqArray;

    QJsonArray childArray;
    for (int childId : childIds) {
        childArray.append(childId);
    }
    json["childIds"] = childArray;

    // 学习状态
    json["isLearned"] = isLearned;
    json["masteryLevel"] = masteryLevel;
    json["lastStudied"] = lastStudied.toString(Qt::ISODate);

    return json;
}
KnowledgePoint KnowledgePoint::fromJson(const QJsonObject& json) {
    KnowledgePoint kp;

    kp.id = json["id"].toInt();
    kp.title = json["title"].toString();
    kp.description = json["description"].toString();
    kp.content = json["content"].toString();
    kp.type = static_cast<KnowledgeType>(json["type"].toInt());
    kp.difficulty = static_cast<DifficultyLevel>(json["difficulty"].toInt());
    kp.estimatedTime = json["estimatedTime"].toInt();
    kp.parentId = json["parentId"].toInt();

    // 解析数组
    QJsonArray prereqArray = json["prerequisiteIds"].toArray();
    for (const QJsonValue& value : prereqArray) {
        kp.prerequisiteIds.append(value.toInt());
    }

    QJsonArray childArray = json["childIds"].toArray();
    for (const QJsonValue& value : childArray) {
        kp.childIds.append(value.toInt());
    }

    // 学习状态
    kp.isLearned = json["isLearned"].toBool();
    kp.masteryLevel = json["masteryLevel"].toInt();
    QString lastStudiedStr = json["lastStudied"].toString();
    if (!lastStudiedStr.isEmpty()) {
        kp.lastStudied = QDateTime::fromString(lastStudiedStr, Qt::ISODate);
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
void KnowledgePoint::addPrerequisite(int knowledgeId) {
    if (knowledgeId > 0 && !hasPrerequisite(knowledgeId)) {
        prerequisiteIds.append(knowledgeId);
    }
}
void KnowledgePoint::removePrerequisite(int knowledgeId) {
    prerequisiteIds.removeAll(knowledgeId);
}
bool KnowledgePoint::hasPrerequisite(int knowledgeId) const {
    return prerequisiteIds.contains(knowledgeId);
}
void KnowledgePoint::clearPrerequisites() {
    prerequisiteIds.clear();
}
void KnowledgePoint::addChild(int knowledgeId) {
    if (knowledgeId > 0 && !hasChild(knowledgeId)) {
        childIds.append(knowledgeId);
    }
}
void KnowledgePoint::removeChild(int knowledgeId) {
    childIds.removeAll(knowledgeId);
}
bool KnowledgePoint::hasChild(int knowledgeId) const {
    return childIds.contains(knowledgeId);
}
void KnowledgePoint::clearChildren() {
    childIds.clear();
}
