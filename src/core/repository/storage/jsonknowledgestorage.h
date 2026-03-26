#ifndef JSONKNOWLEDGESTORAGE_H
#define JSONKNOWLEDGESTORAGE_H

#include <QHash>
#include <QList>
#include <QString>

#include "iknowledgestorage.h"

class JsonKnowledgeStorage : public IKnowledgeStorage
{
public:
    explicit JsonKnowledgeStorage(const QString& rootDirPath);

    bool initialize() override;
    QList<KnowledgePoint> loadAll() const override;
    bool saveAll(const QList<KnowledgePoint>& points) override;
    bool upsert(const KnowledgePoint& point) override;
    bool removeById(const QString& id) override;
    KnowledgePoint findById(const QString& id) const override;

private:
    QString rootDir;
    QString indexPath;
    QHash<QString, QString> categoryToFile;

    bool loadIndex();
    bool saveIndex() const;
    QString normalizeCategory(const QString& category) const;
    QString categoryFileName(const QString& category) const;
    QString ensureCategoryFile(const QString& category);

    QList<KnowledgePoint> readCategoryPoints(const QString& relativePath) const;
    bool writeCategoryPoints(const QString& relativePath, const QList<KnowledgePoint>& points) const;
};

#endif // JSONKNOWLEDGESTORAGE_H
