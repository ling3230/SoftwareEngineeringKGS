#include "knowledgerepository.h"

#include "storage/iknowledgestorage.h"
#include "storage/jsonknowledgestorage.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QQueue>
#include <QSet>
#include <QDebug>

namespace {

QString normalizeText(const QString& text)
{
    return text.trimmed().toLower();
}

bool listContainsIgnoreCase(const QList<QString>& values, const QString& expected)
{
    const QString target = normalizeText(expected);
    for (const QString& value : values) {
        if (normalizeText(value) == target) {
            return true;
        }
    }
    return false;
}

}

KnowledgeRepository* KnowledgeRepository::instance = nullptr;

KnowledgeRepository::KnowledgeRepository(QObject *parent)
    : QObject(parent)
{
    const QString defaultRoot = QDir(QCoreApplication::applicationDirPath()).filePath("data/knowledge");
    setJsonStorageRoot(defaultRoot);
}

KnowledgeRepository::~KnowledgeRepository()
{
}

KnowledgeRepository* KnowledgeRepository::getInstance()
{
    if (instance == nullptr) {
        instance = new KnowledgeRepository();
    }
    return instance;
}

void KnowledgeRepository::setStorage(const QSharedPointer<IKnowledgeStorage>& customStorage)
{
    storage = customStorage;
    if (storage && !storage->initialize()) {
        qWarning() << "初始化自定义知识存储失败";
    }
}

bool KnowledgeRepository::setJsonStorageRoot(const QString& rootPath)
{
    QSharedPointer<IKnowledgeStorage> jsonStorage(new JsonKnowledgeStorage(rootPath));
    if (!jsonStorage->initialize()) {
        qWarning() << "初始化JSON知识存储失败:" << rootPath;
        return false;
    }

    storage = jsonStorage;
    return true;
}

bool KnowledgeRepository::saveKnowledge(const KnowledgePoint& knowledge)
{
    if (!storage) {
        return false;
    }
    return storage->upsert(knowledge);
}

KnowledgePoint KnowledgeRepository::findById(int id)
{
    if (!storage || id <= 0) {
        return KnowledgePoint();
    }
    return storage->findById(QString::number(id));
}

QList<KnowledgePoint> KnowledgeRepository::findAll()
{
    if (!storage) {
        return QList<KnowledgePoint>();
    }
    return storage->loadAll();
}

bool KnowledgeRepository::deleteKnowledge(int id)
{
    if (!storage || id <= 0) {
        return false;
    }
    return storage->removeById(QString::number(id));
}

bool KnowledgeRepository::updateKnowledge(const KnowledgePoint& knowledge)
{
    return saveKnowledge(knowledge);
}

bool KnowledgeRepository::saveAll(const QList<KnowledgePoint>& knowledgeList)
{
    if (!storage) {
        return false;
    }
    return storage->saveAll(knowledgeList);
}

int KnowledgeRepository::countAll()
{
    return findAll().size();
}

QList<KnowledgePoint> KnowledgeRepository::findByTitle(const QString& keyword)
{
    QList<KnowledgePoint> result;
    const QString key = keyword.trimmed();
    if (key.isEmpty()) {
        return result;
    }

    for (const KnowledgePoint& kp : findAll()) {
        if (kp.getTitle().contains(key, Qt::CaseInsensitive)
            || kp.getDescription().contains(key, Qt::CaseInsensitive)) {
            result.append(kp);
        }
    }

    return result;
}

QList<KnowledgePoint> KnowledgeRepository::findByType(KnowledgeType type)
{
    QList<KnowledgePoint> result;
    for (const KnowledgePoint& kp : findAll()) {
        if (kp.getType() == type) {
            result.append(kp);
        }
    }
    return result;
}

QList<KnowledgePoint> KnowledgeRepository::findByDifficulty(DifficultyLevel minLevel, DifficultyLevel maxLevel)
{
    QList<KnowledgePoint> result;
    const int minValue = static_cast<int>(minLevel);
    const int maxValue = static_cast<int>(maxLevel);

    for (const KnowledgePoint& kp : findAll()) {
        const int level = static_cast<int>(kp.getDifficulty());
        if (level >= minValue && level <= maxValue) {
            result.append(kp);
        }
    }

    return result;
}

QList<KnowledgePoint> KnowledgeRepository::findByEstimatedTime(int maxMinutes)
{
    QList<KnowledgePoint> result;
    if (maxMinutes <= 0) {
        return result;
    }

    for (const KnowledgePoint& kp : findAll()) {
        if (kp.getEstimatedTime() <= maxMinutes) {
            result.append(kp);
        }
    }

    return result;
}

QList<KnowledgePoint> KnowledgeRepository::findRoots()
{
    QList<KnowledgePoint> result;
    for (const KnowledgePoint& kp : findAll()) {
        if (kp.getPrerequisiteIds().isEmpty()) {
            result.append(kp);
        }
    }
    return result;
}

QList<KnowledgePoint> KnowledgeRepository::findChildren(int parentId)
{
    QList<KnowledgePoint> result;
    if (parentId <= 0) {
        return result;
    }

    const QString parentKey = QString::number(parentId);
    for (const KnowledgePoint& kp : findAll()) {
        if (kp.getPrerequisiteIds().contains(parentKey)) {
            result.append(kp);
        }
    }

    return result;
}

QList<KnowledgePoint> KnowledgeRepository::findSiblings(int knowledgeId)
{
    QList<KnowledgePoint> result;
    if (knowledgeId <= 0) {
        return result;
    }

    const QString idKey = QString::number(knowledgeId);
    const QList<KnowledgePoint> all = findAll();

    QString parent;
    for (const KnowledgePoint& kp : all) {
        if (kp.getId() == idKey) {
            parent = kp.getPrimaryParentId();
            break;
        }
    }

    if (parent.isEmpty()) {
        return result;
    }

    for (const KnowledgePoint& kp : all) {
        if (kp.getId() != idKey && kp.getPrimaryParentId() == parent) {
            result.append(kp);
        }
    }

    return result;
}

QList<KnowledgePoint> KnowledgeRepository::findPrerequisites(int knowledgeId)
{
    QList<KnowledgePoint> result;
    if (knowledgeId <= 0) {
        return result;
    }

    const QString targetId = QString::number(knowledgeId);
    QStringList prerequisiteIds;

    const QList<KnowledgePoint> all = findAll();
    for (const KnowledgePoint& kp : all) {
        if (kp.getId() == targetId) {
            for (const QString& id : kp.getPrerequisiteIds()) {
                prerequisiteIds.append(id);
            }
            break;
        }
    }

    for (const KnowledgePoint& kp : all) {
        if (prerequisiteIds.contains(kp.getId())) {
            result.append(kp);
        }
    }

    return result;
}

QList<KnowledgePoint> KnowledgeRepository::findDependents(int knowledgeId)
{
    QList<KnowledgePoint> result;
    if (knowledgeId <= 0) {
        return result;
    }

    const QString idKey = QString::number(knowledgeId);
    for (const KnowledgePoint& kp : findAll()) {
        if (kp.getPrerequisiteIds().contains(idKey)) {
            result.append(kp);
        }
    }

    return result;
}

QList<KnowledgePoint> KnowledgeRepository::findLearned()
{
    QList<KnowledgePoint> result;
    for (const KnowledgePoint& kp : findAll()) {
        if (kp.getIsLearned()) {
            result.append(kp);
        }
    }
    return result;
}

QList<KnowledgePoint> KnowledgeRepository::findUnlearned()
{
    QList<KnowledgePoint> result;
    for (const KnowledgePoint& kp : findAll()) {
        if (!kp.getIsLearned()) {
            result.append(kp);
        }
    }
    return result;
}

QList<KnowledgePoint> KnowledgeRepository::findLearnable()
{
    QList<KnowledgePoint> result;
    const QList<KnowledgePoint> all = findAll();

    QSet<QString> learned;
    for (const KnowledgePoint& kp : all) {
        if (kp.getIsLearned()) {
            learned.insert(kp.getId());
        }
    }

    for (const KnowledgePoint& kp : all) {
        if (kp.getIsLearned()) {
            continue;
        }

        bool canLearnNow = true;
        for (const QString& preId : kp.getPrerequisiteIds()) {
            if (!learned.contains(preId)) {
                canLearnNow = false;
                break;
            }
        }

        if (canLearnNow) {
            result.append(kp);
        }
    }

    return result;
}

QList<KnowledgePoint> KnowledgeRepository::findInProgress()
{
    QList<KnowledgePoint> result;
    for (const KnowledgePoint& kp : findAll()) {
        if (kp.getMasteryLevel() > 0 && kp.getMasteryLevel() < 80) {
            result.append(kp);
        }
    }
    return result;
}

QList<KnowledgePoint> KnowledgeRepository::findMastered()
{
    QList<KnowledgePoint> result;
    for (const KnowledgePoint& kp : findAll()) {
        if (kp.getMasteryLevel() >= 80) {
            result.append(kp);
        }
    }
    return result;
}

QList<KnowledgePoint> KnowledgeRepository::getLearningPath(int startId, int endId)
{
    QList<KnowledgePoint> path;
    if (startId <= 0 || endId <= 0) {
        return path;
    }

    const QString startKey = QString::number(startId);
    const QString endKey = QString::number(endId);
    const QList<KnowledgePoint> all = findAll();

    QHash<QString, KnowledgePoint> nodeMap;
    for (const KnowledgePoint& kp : all) {
        nodeMap.insert(kp.getId(), kp);
    }

    if (!nodeMap.contains(startKey) || !nodeMap.contains(endKey)) {
        return path;
    }

    QQueue<QString> queue;
    QHash<QString, QString> parent;
    QSet<QString> visited;

    queue.enqueue(startKey);
    visited.insert(startKey);

    bool found = false;
    while (!queue.isEmpty()) {
        const QString current = queue.dequeue();
        if (current == endKey) {
            found = true;
            break;
        }

        const KnowledgePoint currentNode = nodeMap.value(current);
        for (const QString& next : currentNode.getPostrequisiteIds()) {
            if (!nodeMap.contains(next) || visited.contains(next)) {
                continue;
            }
            visited.insert(next);
            parent.insert(next, current);
            queue.enqueue(next);
        }
    }

    if (!found) {
        return path;
    }

    QString cursor = endKey;
    while (true) {
        path.prepend(nodeMap.value(cursor));
        if (cursor == startKey) {
            break;
        }
        cursor = parent.value(cursor);
    }

    return path;
}

QList<KnowledgePoint> KnowledgeRepository::getRecommended(int userId, int limit)
{
    Q_UNUSED(userId);

    QList<KnowledgePoint> candidates = findLearnable();
    std::sort(candidates.begin(), candidates.end(), [](const KnowledgePoint& a, const KnowledgePoint& b) {
        return a.getImportance() > b.getImportance();
    });

    if (limit <= 0 || candidates.size() <= limit) {
        return candidates;
    }

    return candidates.mid(0, limit);
}

bool KnowledgeRepository::checkPrerequisitesCompleted(int knowledgeId, int userId)
{
    Q_UNUSED(userId);

    if (knowledgeId <= 0) {
        return false;
    }

    const QString key = QString::number(knowledgeId);
    const QList<KnowledgePoint> all = findAll();

    QSet<QString> learned;
    for (const KnowledgePoint& kp : all) {
        if (kp.getIsLearned()) {
            learned.insert(kp.getId());
        }
    }

    for (const KnowledgePoint& kp : all) {
        if (kp.getId() == key) {
            for (const QString& preId : kp.getPrerequisiteIds()) {
                if (!learned.contains(preId)) {
                    return false;
                }
            }
            return true;
        }
    }

    return false;
}

bool KnowledgeRepository::addPrerequisite(int knowledgeId, int prerequisiteId)
{
    if (knowledgeId <= 0 || prerequisiteId <= 0) {
        return false;
    }

    KnowledgePoint target = findById(knowledgeId);
    if (target.getId().isEmpty()) {
        return false;
    }

    target.addPrerequisite(prerequisiteId);
    return saveKnowledge(target);
}

bool KnowledgeRepository::removePrerequisite(int knowledgeId, int prerequisiteId)
{
    if (knowledgeId <= 0 || prerequisiteId <= 0) {
        return false;
    }

    KnowledgePoint target = findById(knowledgeId);
    if (target.getId().isEmpty()) {
        return false;
    }

    target.removePrerequisite(prerequisiteId);
    return saveKnowledge(target);
}

bool KnowledgeRepository::addChild(int parentId, int childId)
{
    if (parentId <= 0 || childId <= 0) {
        return false;
    }

    KnowledgePoint parent = findById(parentId);
    KnowledgePoint child = findById(childId);
    if (parent.getId().isEmpty() || child.getId().isEmpty()) {
        return false;
    }

    parent.addChild(childId);
    child.addPrerequisite(parentId);

    return saveKnowledge(parent) && saveKnowledge(child);
}

bool KnowledgeRepository::removeChild(int parentId, int childId)
{
    if (parentId <= 0 || childId <= 0) {
        return false;
    }

    KnowledgePoint parent = findById(parentId);
    KnowledgePoint child = findById(childId);
    if (parent.getId().isEmpty() || child.getId().isEmpty()) {
        return false;
    }

    parent.removeChild(childId);
    child.removePrerequisite(parentId);

    return saveKnowledge(parent) && saveKnowledge(child);
}

bool KnowledgeRepository::updateParent(int knowledgeId, int newParentId)
{
    if (knowledgeId <= 0) {
        return false;
    }

    KnowledgePoint target = findById(knowledgeId);
    if (target.getId().isEmpty()) {
        return false;
    }

    target.setParentId(newParentId);
    return saveKnowledge(target);
}

bool KnowledgeRepository::updateLearningStatus(int knowledgeId, int userId, bool isLearned, int masteryLevel)
{
    Q_UNUSED(userId);

    if (knowledgeId <= 0) {
        return false;
    }

    KnowledgePoint target = findById(knowledgeId);
    if (target.getId().isEmpty()) {
        return false;
    }

    target.setIsLearned(isLearned);
    target.setMasteryLevel(masteryLevel);
    if (isLearned) {
        target.setLastStudied(QDateTime::currentDateTime());
    }

    return saveKnowledge(target);
}

bool KnowledgeRepository::recordStudyTime(int knowledgeId, int userId, int minutes)
{
    Q_UNUSED(userId);

    if (knowledgeId <= 0 || minutes <= 0) {
        return false;
    }

    KnowledgePoint target = findById(knowledgeId);
    if (target.getId().isEmpty()) {
        return false;
    }

    const int delta = qMax(1, minutes / 5);
    target.updateMastery(delta);
    target.setLastStudied(QDateTime::currentDateTime());
    return saveKnowledge(target);
}

bool KnowledgeRepository::importFromJson(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "导入JSON失败，无法打开文件:" << filePath;
        return false;
    }

    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    if (!doc.isArray()) {
        qWarning() << "导入JSON失败，根节点必须是数组:" << filePath;
        return false;
    }

    QList<KnowledgePoint> imported;
    for (const QJsonValue& value : doc.array()) {
        if (value.isObject()) {
            imported.append(KnowledgePoint::fromJson(value.toObject()));
        }
    }

    for (const KnowledgePoint& kp : imported) {
        if (!saveKnowledge(kp)) {
            return false;
        }
    }

    return true;
}

bool KnowledgeRepository::exportToJson(const QString& filePath)
{
    QJsonArray array;
    for (const KnowledgePoint& kp : findAll()) {
        array.append(kp.toJson());
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qWarning() << "导出JSON失败，无法写入文件:" << filePath;
        return false;
    }

    file.write(QJsonDocument(array).toJson(QJsonDocument::Indented));
    file.close();
    return true;
}

bool KnowledgeRepository::importFromJsonString(const QString& jsonStr)
{
    const QJsonDocument doc = QJsonDocument::fromJson(jsonStr.toUtf8());
    if (!doc.isArray()) {
        return false;
    }

    for (const QJsonValue& value : doc.array()) {
        if (value.isObject()) {
            if (!saveKnowledge(KnowledgePoint::fromJson(value.toObject()))) {
                return false;
            }
        }
    }

    return true;
}

QString KnowledgeRepository::exportToJsonString()
{
    QJsonArray array;
    for (const KnowledgePoint& kp : findAll()) {
        array.append(kp.toJson());
    }

    return QJsonDocument(array).toJson(QJsonDocument::Indented);
}

QList<KnowledgePoint> KnowledgeRepository::findByCategory(const QString& category)
{
    QList<KnowledgePoint> result;
    const QString expected = normalizeText(category);
    if (expected.isEmpty()) {
        return result;
    }

    for (const KnowledgePoint& kp : findAll()) {
        if (normalizeText(kp.getCategory()) == expected) {
            result.append(kp);
        }
    }

    return result;
}

QList<KnowledgePoint> KnowledgeRepository::findByLanguage(const QString& language)
{
    QList<KnowledgePoint> result;
    const QString expected = normalizeText(language);
    if (expected.isEmpty()) {
        return result;
    }

    for (const KnowledgePoint& kp : findAll()) {
        if (listContainsIgnoreCase(kp.getTags(), expected)
            || kp.getTitle().contains(language, Qt::CaseInsensitive)
            || kp.getDescription().contains(language, Qt::CaseInsensitive)) {
            result.append(kp);
        }
    }

    return result;
}

KnowledgePoint KnowledgeRepository::mapToKnowledge(const QSqlRecord& record)
{
    Q_UNUSED(record);
    return KnowledgePoint();
}

QVariantMap KnowledgeRepository::knowledgeToMap(const KnowledgePoint& knowledge)
{
    Q_UNUSED(knowledge);
    return QVariantMap();
}

QList<QString> KnowledgeRepository::parseIdList(const QString& jsonStr)
{
    QList<QString> ids;
    const QJsonDocument doc = QJsonDocument::fromJson(jsonStr.toUtf8());
    if (!doc.isArray()) {
        return ids;
    }

    for (const QJsonValue& value : doc.array()) {
        if (value.isString()) {
            ids.append(value.toString());
        } else if (value.isDouble()) {
            ids.append(QString::number(value.toInt()));
        }
    }

    return ids;
}

QString KnowledgeRepository::idListToJson(const QList<QString>& ids)
{
    QJsonArray array;
    for (const QString& id : ids) {
        array.append(id);
    }
    return QJsonDocument(array).toJson(QJsonDocument::Compact);
}

bool KnowledgeRepository::initDatabase()
{
    return true;
}
