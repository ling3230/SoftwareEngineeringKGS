#ifndef KNOWLEDGEREPOSITORY_H
#define KNOWLEDGEREPOSITORY_H

#include <QObject>
#include <QList>
#include <QSharedPointer>
#include <QSqlRecord>
#include <QVariantMap>
#include <QSqlQuery>

#include "../entity/knowledgepoint.h"

// 前向声明（保持不变）
class DatabaseManager;

class KnowledgeRepository : public QObject
{
    Q_OBJECT

public:
    explicit KnowledgeRepository(QObject *parent = nullptr);
    ~KnowledgeRepository();

    // 单例模式
    static KnowledgeRepository* getInstance();

    // ============ 核心CRUD操作 ============
    bool saveKnowledge(const KnowledgePoint& knowledge);
    KnowledgePoint findById(int id);
    QList<KnowledgePoint> findAll();
    bool deleteKnowledge(int id);
    bool updateKnowledge(const KnowledgePoint& knowledge);

    // ============ 批量操作 ============
    bool saveAll(const QList<KnowledgePoint>& knowledgeList);
    int countAll();

    // ============ 条件查询 ============
    QList<KnowledgePoint> findByTitle(const QString& keyword);
    QList<KnowledgePoint> findByType(KnowledgeType type);
    QList<KnowledgePoint> findByDifficulty(DifficultyLevel minLevel, DifficultyLevel maxLevel);
    QList<KnowledgePoint> findByEstimatedTime(int maxMinutes);

    // ============ 结构查询 ============
    QList<KnowledgePoint> findRoots();
    QList<KnowledgePoint> findChildren(int parentId);
    QList<KnowledgePoint> findSiblings(int knowledgeId);
    QList<KnowledgePoint> findPrerequisites(int knowledgeId);
    QList<KnowledgePoint> findDependents(int knowledgeId);

    // ============ 学习状态查询 ============
    QList<KnowledgePoint> findLearned();
    QList<KnowledgePoint> findUnlearned();
    QList<KnowledgePoint> findLearnable();
    QList<KnowledgePoint> findInProgress();
    QList<KnowledgePoint> findMastered();

    // ============ 业务方法 ============
    QList<KnowledgePoint> getLearningPath(int startId, int endId);
    QList<KnowledgePoint> getRecommended(int userId, int limit = 5);
    bool checkPrerequisitesCompleted(int knowledgeId, int userId);

    // ============ 关系管理 ============
    bool addPrerequisite(int knowledgeId, int prerequisiteId);
    bool removePrerequisite(int knowledgeId, int prerequisiteId);
    bool addChild(int parentId, int childId);
    bool removeChild(int parentId, int childId);
    bool updateParent(int knowledgeId, int newParentId);

    // ============ 学习状态更新 ============
    bool updateLearningStatus(int knowledgeId, int userId,bool isLearned, int masteryLevel);
    bool recordStudyTime(int knowledgeId, int userId, int minutes);

    // ============ 导入导出 ============
    bool importFromJson(const QString& filePath);
    bool exportToJson(const QString& filePath);
    bool importFromJsonString(const QString& jsonStr);
    QString exportToJsonString();

private:
    // 数据库连接
    QSharedPointer<DatabaseManager> dbManager;

    // 单例实例
    static KnowledgeRepository* instance;

    // 私有方法
    KnowledgePoint mapToKnowledge(const QSqlRecord& record);//数据库查询结果映射为业务类型
    QVariantMap knowledgeToMap(const KnowledgePoint& knowledge);//业务类型映射为数据库类型
    QList<int> parseIdList(const QString& jsonStr);//json类型转换knowledge列表
    QString idListToJson(const QList<int>& ids);//knowledge列表转换json类型

    // 初始化数据库表
    bool initDatabase();
};

#endif // KNOWLEDGEREPOSITORY_H
