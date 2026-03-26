#ifndef CAREERPATH_H
#define CAREERPATH_H

#include "CareerDirection.h"
#include <QString>
#include <QList>
#include <QJsonObject>

class TechStack
{
public:
    TechStack() = default;
    TechStack(const QString& id, const QString& name,
              const QString& description, const QString& icon = "");

    QString getId() const { return id; }
    QString getName() const { return name; }
    QString getDescription() const { return description; }
    QString getIcon() const { return icon; }
    QString getSkillTreeFile() const { return skillTreeFile; }
    bool isRecommended() const { return recommended; }

    void setId(const QString& value) { id = value; }
    void setName(const QString& value) { name = value; }
    void setDescription(const QString& value) { description = value; }
    void setIcon(const QString& value) { icon = value; }
    void setSkillTreeFile(const QString& value) { skillTreeFile = value; }
    void setRecommended(bool value) { recommended = value; }

    QJsonObject toJson() const;
    static TechStack fromJson(const QJsonObject& json);

private:
    QString id;
    QString name;
    QString description;
    QString icon;
    QString skillTreeFile;
    bool recommended = false;
};

class CareerPath {
public:
    // 构造函数
    CareerPath();
    CareerPath(CareerDirection direction, const QString& name,
               const QString& description, const QString& icon = "");

    // 获取方法
    CareerDirection getDirection() const { return direction; }
    QString getName() const { return name; }
    QString getDescription() const { return description; }
    QString getIcon() const { return icon; }
    QString getSkillTreeFile() const { return skillTreeFile; }
    bool isRecommended() const { return recommended; }
    QList<TechStack> getTechStacks() const { return techStacks; }
    bool hasTechStacks() const { return !techStacks.isEmpty(); }
    TechStack getTechStackById(const QString& techStackId) const;

    // 设置方法
    void setDirection(CareerDirection dir) { direction = dir; }
    void setName(const QString& n) { name = n; }
    void setDescription(const QString& desc) { description = desc; }
    void setIcon(const QString& i) { icon = i; }
    void setSkillTreeFile(const QString& file) { skillTreeFile = file; }
    void setRecommended(bool rec) { recommended = rec; }
    void setTechStacks(const QList<TechStack>& stacks) { techStacks = stacks; }
    void addTechStack(const TechStack& stack) { techStacks.append(stack); }

    // 序列化方法
    QJsonObject toJson() const;
    static CareerPath fromJson(const QJsonObject& json);

private:
    CareerDirection direction;
    QString name;           // 职业名称
    QString description;    // 职业描述
    QString icon;          // 图标路径或Unicode图标
    QString skillTreeFile; // 对应的技能树JSON文件路径
    bool recommended = false; // 是否推荐给新手
    QList<TechStack> techStacks; // 该主职业下可选的技术栈
};

#endif // CAREERPATH_H
