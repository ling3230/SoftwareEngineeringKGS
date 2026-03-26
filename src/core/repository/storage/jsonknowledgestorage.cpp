#include "jsonknowledgestorage.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSet>
#include <QDebug>

JsonKnowledgeStorage::JsonKnowledgeStorage(const QString& rootDirPath)
    : rootDir(rootDirPath)
    , indexPath(QDir(rootDirPath).filePath("index.json"))
{
}

bool JsonKnowledgeStorage::initialize()
{
    QDir dir(rootDir);
    if (!dir.exists() && !dir.mkpath(".")) {
        qWarning() << "创建知识点JSON目录失败:" << rootDir;
        return false;
    }

    if (!QFileInfo::exists(indexPath)) {
        categoryToFile.clear();
        if (!saveIndex()) {
            return false;
        }
    }

    return loadIndex();
}

QList<KnowledgePoint> JsonKnowledgeStorage::loadAll() const
{
    QList<KnowledgePoint> all;
    QSet<QString> loadedFiles;

    for (const QString& relativePath : categoryToFile) {
        if (loadedFiles.contains(relativePath)) {
            continue;
        }
        loadedFiles.insert(relativePath);
        all.append(readCategoryPoints(relativePath));
    }

    return all;
}

bool JsonKnowledgeStorage::saveAll(const QList<KnowledgePoint>& points)
{
    QHash<QString, QList<KnowledgePoint>> grouped;
    for (const KnowledgePoint& kp : points) {
        const QString category = normalizeCategory(kp.getCategory());
        grouped[category].append(kp);
    }

    categoryToFile.clear();
    for (auto it = grouped.begin(); it != grouped.end(); ++it) {
        const QString category = it.key();
        const QString fileName = ensureCategoryFile(category);
        if (!writeCategoryPoints(fileName, it.value())) {
            return false;
        }
    }

    return saveIndex();
}

bool JsonKnowledgeStorage::upsert(const KnowledgePoint& point)
{
    QString category = normalizeCategory(point.getCategory());
    QString fileName = ensureCategoryFile(category);

    QList<KnowledgePoint> points = readCategoryPoints(fileName);
    bool replaced = false;
    for (KnowledgePoint& existing : points) {
        if (existing.getId() == point.getId()) {
            existing = point;
            replaced = true;
            break;
        }
    }

    if (!replaced) {
        points.append(point);
    }

    if (!writeCategoryPoints(fileName, points)) {
        return false;
    }

    return saveIndex();
}

bool JsonKnowledgeStorage::removeById(const QString& id)
{
    bool removed = false;

    QSet<QString> scanned;
    for (const QString& relativePath : categoryToFile) {
        if (scanned.contains(relativePath)) {
            continue;
        }
        scanned.insert(relativePath);

        QList<KnowledgePoint> points = readCategoryPoints(relativePath);
        const int before = points.size();
        for (int i = points.size() - 1; i >= 0; --i) {
            if (points[i].getId() == id) {
                points.removeAt(i);
                removed = true;
            }
        }

        if (points.size() != before && !writeCategoryPoints(relativePath, points)) {
            return false;
        }
    }

    return removed;
}

KnowledgePoint JsonKnowledgeStorage::findById(const QString& id) const
{
    const QList<KnowledgePoint> all = loadAll();
    for (const KnowledgePoint& kp : all) {
        if (kp.getId() == id) {
            return kp;
        }
    }

    return KnowledgePoint();
}

bool JsonKnowledgeStorage::loadIndex()
{
    QFile file(indexPath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "读取index.json失败:" << indexPath;
        return false;
    }

    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    if (!doc.isObject()) {
        qWarning() << "index.json格式错误:" << indexPath;
        return false;
    }

    categoryToFile.clear();
    const QJsonArray files = doc.object().value("files").toArray();
    for (const QJsonValue& value : files) {
        const QJsonObject obj = value.toObject();
        const QString category = normalizeCategory(obj.value("category").toString());
        const QString path = obj.value("path").toString().trimmed();
        if (!category.isEmpty() && !path.isEmpty()) {
            categoryToFile.insert(category, path);
        }
    }

    return true;
}

bool JsonKnowledgeStorage::saveIndex() const
{
    QJsonArray files;
    for (auto it = categoryToFile.constBegin(); it != categoryToFile.constEnd(); ++it) {
        QJsonObject obj;
        obj["category"] = it.key();
        obj["path"] = it.value();
        files.append(obj);
    }

    QJsonObject indexObj;
    indexObj["version"] = 1;
    indexObj["files"] = files;

    QFile file(indexPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qWarning() << "写入index.json失败:" << indexPath;
        return false;
    }

    file.write(QJsonDocument(indexObj).toJson(QJsonDocument::Indented));
    file.close();
    return true;
}

QString JsonKnowledgeStorage::normalizeCategory(const QString& category) const
{
    const QString normalized = category.trimmed().toLower();
    return normalized.isEmpty() ? QString("uncategorized") : normalized;
}

QString JsonKnowledgeStorage::categoryFileName(const QString& category) const
{
    QString safe = category;
    safe.replace("/", "_");
    safe.replace("\\", "_");
    safe.replace(" ", "_");
    return safe + ".json";
}

QString JsonKnowledgeStorage::ensureCategoryFile(const QString& category)
{
    const QString normalized = normalizeCategory(category);
    if (categoryToFile.contains(normalized)) {
        return categoryToFile.value(normalized);
    }

    const QString fileName = categoryFileName(normalized);
    categoryToFile.insert(normalized, fileName);
    return fileName;
}

QList<KnowledgePoint> JsonKnowledgeStorage::readCategoryPoints(const QString& relativePath) const
{
    QList<KnowledgePoint> points;
    const QString fullPath = QDir(rootDir).filePath(relativePath);

    QFile file(fullPath);
    if (!file.exists()) {
        return points;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "读取知识点文件失败:" << fullPath;
        return points;
    }

    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    if (!doc.isArray()) {
        qWarning() << "知识点文件格式错误(应为数组):" << fullPath;
        return points;
    }

    const QJsonArray array = doc.array();
    for (const QJsonValue& value : array) {
        if (value.isObject()) {
            points.append(KnowledgePoint::fromJson(value.toObject()));
        }
    }

    return points;
}

bool JsonKnowledgeStorage::writeCategoryPoints(const QString& relativePath, const QList<KnowledgePoint>& points) const
{
    QJsonArray array;
    for (const KnowledgePoint& kp : points) {
        array.append(kp.toJson());
    }

    const QString fullPath = QDir(rootDir).filePath(relativePath);
    QFile file(fullPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qWarning() << "写入知识点文件失败:" << fullPath;
        return false;
    }

    file.write(QJsonDocument(array).toJson(QJsonDocument::Indented));
    file.close();
    return true;
}
