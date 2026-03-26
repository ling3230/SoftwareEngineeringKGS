// service/CareerService.h
#ifndef CAREERSERVICE_H
#define CAREERSERVICE_H

#include <QObject>
#include <QList>
#include <QMap>
#include "../entity/CareerPath.h"
#include "../entity/userprofile.h"

class UserRepository;

class CareerService : public QObject
{
    Q_OBJECT

public:
    // 单例模式
    static CareerService* getInstance();

    // 初始化
    void initialize();

    // 获取所有可选的职业路径
    QList<CareerPath> getAllCareerPaths() const;

    // 获取特定职业路径
    CareerPath getCareerPath(CareerDirection direction) const;
    QList<TechStack> getTechStacks(CareerDirection direction) const;
    TechStack getTechStack(CareerDirection direction, const QString& techStackId) const;

    // 用户职业方向管理
    bool setUserCareer(int userId, CareerDirection direction);
    bool setUserCareerSelection(int userId, CareerDirection direction, const QString& techStackId);
    CareerDirection getUserCareer(int userId);
    QString getUserTechStackId(int userId);
    CareerPath getCurrentUserCareer(int userId);
    TechStack getCurrentUserTechStack(int userId);

    // 检查是否需要选择职业
    bool shouldSelectCareer(int userId);

    // 获取推荐职业（给首次使用的用户）
    QList<CareerPath> getRecommendedCareers() const;

    // 加载/保存职业配置
    bool loadCareerConfig(const QString& configPath = "");
    bool saveCareerConfig(const QString& configPath = "");

private:
    CareerService(QObject *parent = nullptr);

    static CareerService* instance;

    QMap<CareerDirection, CareerPath> careerMap;
    UserRepository* userRepo = nullptr;

    void createDefaultCareers();
};

#endif // CAREERSERVICE_H
