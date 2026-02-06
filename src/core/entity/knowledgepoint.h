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
    KnowledgePoint(int id, const QString& title, const QString& description);

    // 拷贝构造函数和赋值运算符（如果需要）
    KnowledgePoint(const KnowledgePoint& other);
    KnowledgePoint& operator=(const KnowledgePoint& other);

    // ============ Getter方法 ============
    int getId() const;
    QString getTitle() const;
    QString getDescription() const;
    QString getContent() const;
    KnowledgeType getType() const;
    DifficultyLevel getDifficulty() const;
    int getEstimatedTime() const;
    int getParentId() const;
    QList<int> getPrerequisiteIds() const;
    QList<int> getChildIds() const;
    bool getIsLearned() const;
    int getMasteryLevel() const;
    QDateTime getLastStudied() const;

    // ============ Setter方法 ============
    void setId(int id);
    void setTitle(const QString& title);
    void setDescription(const QString& description);
    void setContent(const QString& content);
    void setType(KnowledgeType type);
    void setDifficulty(DifficultyLevel difficulty);
    void setEstimatedTime(int minutes);
    void setParentId(int parentId);
    void setPrerequisiteIds(const QList<int>& ids);
    void setChildIds(const QList<int>& ids);
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
    bool isPrerequisiteFor(int knowledgeId) const;  // 是否是指定知识点的先修

    // ============ 序列化/反序列化 ============
    QJsonObject toJson() const;
    static KnowledgePoint fromJson(const QJsonObject& json);
    QString toJsonString() const;
    static KnowledgePoint fromJsonString(const QString& jsonStr);

    // ============ 关系操作 ============
    void addPrerequisite(int knowledgeId);
    void removePrerequisite(int knowledgeId);
    bool hasPrerequisite(int knowledgeId) const;
    void clearPrerequisites();

    void addChild(int knowledgeId);
    void removeChild(int knowledgeId);
    bool hasChild(int knowledgeId) const;
    void clearChildren();

private:
    // ============ 核心属性 ============
    int id;                       // 唯一标识
    QString title;                // 标题
    QString description;          // 简短描述
    QString content;              // 详细内容（支持富文本）

    // ============ 分类属性 ============
    KnowledgeType type;           // 知识点类型
    DifficultyLevel difficulty;   // 难度等级
    int estimatedTime;            // 预计学习时间（分钟）

    // ============ 结构属性 ============
    int parentId;                 // 父知识点ID（用于树形结构）
    QList<int> prerequisiteIds;   // 先修知识点ID列表
    QList<int> childIds;          // 子知识点ID列表

    // ============ 学习状态 ============
    bool isLearned;               // 是否已学习
    int masteryLevel;             // 掌握程度 0-100
    QDateTime lastStudied;        // 最后学习时间
};

#endif // KNOWLEDGEPOINT_H
