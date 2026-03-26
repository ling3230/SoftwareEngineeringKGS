#ifndef IKNOWLEDGESTORAGE_H
#define IKNOWLEDGESTORAGE_H

#include <QList>
#include <QString>

#include "src/core/entity/knowledgepoint.h"

class IKnowledgeStorage
{
public:
    virtual ~IKnowledgeStorage() = default;

    virtual bool initialize() = 0;
    virtual QList<KnowledgePoint> loadAll() const = 0;
    virtual bool saveAll(const QList<KnowledgePoint>& points) = 0;
    virtual bool upsert(const KnowledgePoint& point) = 0;
    virtual bool removeById(const QString& id) = 0;
    virtual KnowledgePoint findById(const QString& id) const = 0;
};

#endif // IKNOWLEDGESTORAGE_H
