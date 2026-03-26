#ifndef KNOWLEDGEPOINT_H
#define KNOWLEDGEPOINT_H

#include <QString>
#include <QJsonObject>
#include <QList>
#include <QDateTime>

// 知识点类型：概念/实践/原理/工具
enum class KnowledgeType {
    CONCEPT,     // 概念性知识
    PRACTICE,    // 实践操作
    THEORY,      // 原理理论
    TOOL         // 工具使用
};

// 难度等级：1-5级
enum class DifficultyLevel {
    BEGINNER = 1,     // 入门
    EASY = 2,         // 简单
    INTERMEDIATE = 3, // 中等
    ADVANCED = 4,     // 进阶
    EXPERT = 5        // 专家
};

class KnowledgePoint
{
public:
    // 构造函数
    KnowledgePoint();
    KnowledgePoint(const QString& id, const QString& title, const QString& description);
    KnowledgePoint(int id, const QString& title, const QString& description);

    // 拷贝构造函数和赋值运算符（如果需要）
    KnowledgePoint(const KnowledgePoint& other);
    KnowledgePoint& operator=(const KnowledgePoint& other);

    // ============ Getter方法 ============
    QString getId() const;
    int getIdAsInt() const;
    QString getTitle() const;
    QString getDescription() const;
    QString getContent() const;
    KnowledgeType getType() const;
    DifficultyLevel getDifficulty() const;
    int getEstimatedTime() const;
    QList<QString> getPrerequisiteIds() const;
    QList<QString> getPostrequisiteIds() const;
    QList<QString> getChildIds() const;
    QString getPrimaryParentId() const;
    int getParentId() const;
    QString getCategory() const;
    QList<QString> getTags() const;
    int getImportance() const;
    bool getIsCore() const;
    bool getIsLearned() const;
    int getMasteryLevel() const;
    QDateTime getLastStudied() const;
    QDateTime getCreatedAt() const;
    QDateTime getUpdatedAt() const;

    // ============ Setter方法 ============
    void setId(const QString& id);
    void setId(int id);
    void setTitle(const QString& title);
    void setDescription(const QString& description);
    void setContent(const QString& content);
    void setType(KnowledgeType type);
    void setDifficulty(DifficultyLevel difficulty);
    void setEstimatedTime(int minutes);
    void setPrerequisiteIds(const QList<QString>& ids);
    void setPostrequisiteIds(const QList<QString>& ids);
    void setChildIds(const QList<QString>& ids);
    void setPrimaryParentId(const QString& parentId);
    void setParentId(int parentId);
    void setCategory(const QString& category);
    void setTags(const QList<QString>& tags);
    void setImportance(int importance);
    void setIsCore(bool isCore);
    void setIsLearned(bool learned);
    void setMasteryLevel(int level);  // 0-100
    void setLastStudied(const QDateTime& time);

    // ============ 业务方法 ============
    bool canLearn() const;  // 是否可以开始学习（检查先修条件）
    QString getTypeString() const;  // 获取类型字符串
    QString getDifficultyString() const;  // 获取难度字符串
    QString toString() const;  // 转换为字符串表示

    // 状态更新方法
    void markAsLearned(int masteryLevel = 80);
    void markAsUnlearned();
    void updateMastery(int delta);  // 更新掌握度（正负值）

    // 关系检查方法
    bool hasParent() const;
    bool hasChildren() const;
    bool hasPrerequisites() const;
    bool isPrerequisiteFor(const QString& knowledgeId) const;  // 是否是指定知识点的先修

    // ============ 序列化/反序列化 ============
    QJsonObject toJson() const;
    static KnowledgePoint fromJson(const QJsonObject& json);
    QString toJsonString() const;
    static KnowledgePoint fromJsonString(const QString& jsonStr);

    // ============ 关系操作 ============
    void addPrerequisite(const QString& knowledgeId);
    void addPrerequisite(int knowledgeId);
    void removePrerequisite(const QString& knowledgeId);
    void removePrerequisite(int knowledgeId);
    bool hasPrerequisite(const QString& knowledgeId) const;
    bool hasPrerequisite(int knowledgeId) const;
    void clearPrerequisites();

    void addPostrequisite(const QString& knowledgeId);
    void removePostrequisite(const QString& knowledgeId);
    bool hasPostrequisite(const QString& knowledgeId) const;
    void clearPostrequisites();

    void addChild(const QString& knowledgeId);
    void addChild(int knowledgeId);
    void removeChild(const QString& knowledgeId);
    void removeChild(int knowledgeId);
    bool hasChild(const QString& knowledgeId) const;
    bool hasChild(int knowledgeId) const;
    void clearChildren();

private:
    // ============ 核心属性 ============
    QString id;                   // 唯一标识（如 java_basic）
    QString title;                // 标题
    QString description;          // 简短描述
    QString content;              // 详细内容（支持富文本）

    // ============ 分类属性 ============
    KnowledgeType type;           // 知识点类型
    DifficultyLevel difficulty;   // 难度等级
    int estimatedTime;            // 预计学习时间（分钟）

    // ============ 图关系属性 ============
    QList<QString> prerequisiteIds;   // 前置知识点ID列表
    QList<QString> postrequisiteIds;  // 后置知识点ID列表

    // ============ 基础属性 ============
    QString category;             // 所属分类
    QList<QString> tags;          // 标签
    int importance;               // 重要性 1-10
    bool isCore;                  // 是否核心知识点
    QDateTime createdAt;          // 创建时间
    QDateTime updatedAt;          // 更新时间

    // ============ 学习状态 ============
    bool isLearned;               // 是否已学习
    int masteryLevel;             // 掌握程度 0-100
    QDateTime lastStudied;        // 最后学习时间
};

#endif // KNOWLEDGEPOINT_H
