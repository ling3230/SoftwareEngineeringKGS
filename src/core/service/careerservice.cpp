// service/CareerService.cpp
#include "CareerService.h"
#include "../repository/UserRepository.h"
#include <QDebug>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>

namespace {

TechStack buildFallbackTechStack(const CareerPath& career)
{
    TechStack stack(
        QString("default_%1").arg(static_cast<int>(career.getDirection())),
        QString("%1默认技术栈").arg(career.getName()),
        QString("%1的默认学习技术栈").arg(career.getName()),
        career.getIcon().isEmpty() ? QString("🧩") : career.getIcon());
    stack.setRecommended(true);
    stack.setSkillTreeFile(career.getSkillTreeFile());
    return stack;
}

QList<TechStack> buildDefaultBackendTechStacks()
{
    TechStack javaStack("backend_java", "Java 技术栈",
                        "以 Java、Spring Boot、MyBatis 和微服务为核心的后端路线", "☕");
    javaStack.setRecommended(true);
    javaStack.setSkillTreeFile(":/skilltrees/backend.json");

    TechStack pythonStack("backend_python", "Python 技术栈",
                          "以 Python、Django/FastAPI 为核心的后端路线", "🐍");
    pythonStack.setSkillTreeFile(":/skilltrees/backend_python.json");

    TechStack goStack("backend_go", "Go 技术栈",
                      "以 Go、Gin 和高并发服务开发为核心的后端路线", "🐹");
    goStack.setSkillTreeFile(":/skilltrees/backend_go.json");

    TechStack cppStack("backend_cpp", "C++ 技术栈",
                       "以 C++、网络编程、高性能服务与系统开发为核心的后端路线", "⚙️");
    cppStack.setSkillTreeFile(":/skilltrees/backend_cpp.json");

    TechStack nodeStack("backend_nodejs", "Node.js 技术栈",
                        "以 Node.js、NestJS/Express 和事件驱动架构为核心的后端路线", "🟩");
    nodeStack.setSkillTreeFile(":/skilltrees/backend_nodejs.json");

    TechStack rustStack("backend_rust", "Rust 技术栈",
                        "以 Rust、异步运行时和高可靠服务开发为核心的后端路线", "🦀");
    rustStack.setSkillTreeFile(":/skilltrees/backend_rust.json");

    return {javaStack, pythonStack, goStack, cppStack, nodeStack, rustStack};
}

bool ensureBackendTechStacks(CareerPath& careerPath)
{
    if (careerPath.getDirection() != CareerDirection::BACKEND_ENGINEER) {
        return false;
    }

    bool changed = false;
    QList<TechStack> stacks = careerPath.getTechStacks();
    const QList<TechStack> defaultStacks = buildDefaultBackendTechStacks();

    for (const TechStack& builtin : defaultStacks) {
        bool exists = false;
        for (const TechStack& existing : stacks) {
            if (existing.getId().compare(builtin.getId(), Qt::CaseInsensitive) == 0) {
                exists = true;
                break;
            }
        }

        if (!exists) {
            stacks.append(builtin);
            changed = true;
        }
    }

    if (changed) {
        careerPath.setTechStacks(stacks);
    }

    return changed;
}

}

CareerService* CareerService::instance = nullptr;

CareerService::CareerService(QObject *parent) : QObject(parent)
{
    userRepo = UserRepository::getInstance();
}

CareerService* CareerService::getInstance() {
    if (!instance) {
        instance = new CareerService();
        instance->initialize();
    }
    return instance;
}

void CareerService::initialize() {
    // 加载职业配置
    loadCareerConfig();

    // 如果没有加载到任何配置，创建默认配置
    if (careerMap.isEmpty()) {
        createDefaultCareers();
        saveCareerConfig();
    }

    qDebug() << "CareerService 初始化完成，共加载" << careerMap.size() << "个职业路径";
}

void CareerService::createDefaultCareers() {
    // 软件工程必修课（默认推荐）
    CareerPath defaultPath(CareerDirection::DEFAULT_SE_COURSE,
                           "软件工程必修课",
                           "软件工程核心课程体系，适合所有软件开发学习者",
                           "📚");
    defaultPath.setRecommended(false);
    defaultPath.setSkillTreeFile(":/skilltrees/se_course.json");
    TechStack generalStack("se_core", "软件工程核心栈",
                           "面向课程学习的基础知识体系", "📘");
    generalStack.setRecommended(false);
    generalStack.setSkillTreeFile(":/skilltrees/se_course.json");
    defaultPath.addTechStack(generalStack);
    careerMap[CareerDirection::DEFAULT_SE_COURSE] = defaultPath;

    // 前端工程师
    CareerPath frontend(CareerDirection::FRONTEND_ENGINEER,
                        "前端工程师",
                        "专注于Web界面和用户体验开发，掌握HTML/CSS/JavaScript等核心技术",
                        "🌐");
    frontend.setSkillTreeFile(":/skilltrees/frontend.json");
    TechStack vueStack("frontend_vue", "Vue 技术栈",
                       "以 Vue、TypeScript 和工程化体系为核心的前端路线", "🟢");
    vueStack.setRecommended(true);
    vueStack.setSkillTreeFile(":/skilltrees/frontend.json");
    TechStack reactStack("frontend_react", "React 技术栈",
                         "以 React、TypeScript 和现代状态管理为核心的前端路线", "⚛️");
    reactStack.setSkillTreeFile(":/skilltrees/frontend_react.json");
    frontend.setTechStacks({vueStack, reactStack});
    careerMap[CareerDirection::FRONTEND_ENGINEER] = frontend;

    // 后端工程师
    CareerPath backend(CareerDirection::BACKEND_ENGINEER,
                       "后端工程师",
                       "专注于服务器端和数据库开发，掌握Java/Python/Go等后端技术",
                       "🗄️");
    backend.setRecommended(true);
    backend.setSkillTreeFile(":/skilltrees/backend.json");
    backend.setTechStacks(buildDefaultBackendTechStacks());
    careerMap[CareerDirection::BACKEND_ENGINEER] = backend;

    // 全栈工程师
    CareerPath fullstack(CareerDirection::FULLSTACK_ENGINEER,
                         "全栈工程师",
                         "掌握前后端全链路开发，能够独立完成完整项目",
                         "🔄");
    fullstack.setSkillTreeFile(":/skilltrees/fullstack.json");
    TechStack fullstackWeb("fullstack_web", "Web 全栈栈",
                           "以前端框架 + Java/Node 后端为主的全链路开发路线", "🌍");
    fullstackWeb.setRecommended(true);
    fullstackWeb.setSkillTreeFile(":/skilltrees/fullstack.json");
    fullstack.addTechStack(fullstackWeb);
    careerMap[CareerDirection::FULLSTACK_ENGINEER] = fullstack;

    // 移动开发工程师
    CareerPath mobile(CareerDirection::MOBILE_DEVELOPER,
                      "移动开发工程师",
                      "专注于iOS/Android移动应用开发，掌握React Native/Flutter等跨平台技术",
                      "📱");
    mobile.setSkillTreeFile(":/skilltrees/mobile.json");
    TechStack flutterStack("mobile_flutter", "Flutter 技术栈",
                           "以 Flutter 和 Dart 为核心的跨平台移动开发路线", "🦋");
    flutterStack.setRecommended(true);
    flutterStack.setSkillTreeFile(":/skilltrees/mobile.json");
    TechStack androidStack("mobile_android", "Android 原生栈",
                           "以 Kotlin、Jetpack 为核心的 Android 开发路线", "🤖");
    androidStack.setSkillTreeFile(":/skilltrees/mobile_android.json");
    mobile.setTechStacks({flutterStack, androidStack});
    careerMap[CareerDirection::MOBILE_DEVELOPER] = mobile;

    // DevOps工程师
    CareerPath devops(CareerDirection::DEVOPS_ENGINEER,
                      "DevOps工程师",
                      "专注于开发运维一体化，掌握CI/CD、容器化、云计算等技术",
                      "⚙️");
    devops.setSkillTreeFile(":/skilltrees/devops.json");
    TechStack cloudNative("devops_cloud_native", "云原生栈",
                          "以 Docker、Kubernetes、CI/CD 和云平台为核心", "☁️");
    cloudNative.setRecommended(true);
    cloudNative.setSkillTreeFile(":/skilltrees/devops.json");
    devops.addTechStack(cloudNative);
    careerMap[CareerDirection::DEVOPS_ENGINEER] = devops;

    // 测试工程师
    CareerPath tester(CareerDirection::TEST_ENGINEER,
                      "测试工程师",
                      "专注于软件质量保证，掌握自动化测试、性能测试等技术",
                      "🧪");
    tester.setSkillTreeFile(":/skilltrees/test.json");
    TechStack automationStack("test_automation", "自动化测试栈",
                              "以接口测试、UI 自动化和持续测试为核心", "✅");
    automationStack.setRecommended(true);
    automationStack.setSkillTreeFile(":/skilltrees/test.json");
    tester.addTechStack(automationStack);
    careerMap[CareerDirection::TEST_ENGINEER] = tester;

    // 项目经理
    CareerPath pm(CareerDirection::PROJECT_MANAGER,
                  "项目经理",
                  "专注于项目管理和团队协作，掌握敏捷开发、项目管理等技能",
                  "👔");
    pm.setSkillTreeFile(":/skilltrees/pm.json");
    TechStack agileStack("pm_agile", "敏捷管理栈",
                         "以 Scrum、需求管理和团队协作为核心", "📋");
    agileStack.setRecommended(true);
    agileStack.setSkillTreeFile(":/skilltrees/pm.json");
    pm.addTechStack(agileStack);
    careerMap[CareerDirection::PROJECT_MANAGER] = pm;
}

bool CareerService::loadCareerConfig(const QString& configPath) {
    QString path = configPath.isEmpty() ? "career_paths.json" : configPath;

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "无法打开职业配置文件：" << path;
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull() || !doc.isArray()) {
        qWarning() << "职业配置文件格式错误：" << path;
        return false;
    }

    careerMap.clear();
    bool migratedLegacyFormat = false;
    QJsonArray array = doc.array();
    for (const QJsonValue& value : array) {
        CareerPath careerPath = CareerPath::fromJson(value.toObject());

        // 兼容旧版配置：若缺少 techStacks，自动补一个默认技术栈，避免直接跳过第二步。
        if (!careerPath.hasTechStacks()) {
            careerPath.addTechStack(buildFallbackTechStack(careerPath));
            migratedLegacyFormat = true;
        }

        if (ensureBackendTechStacks(careerPath)) {
            migratedLegacyFormat = true;
        }

        careerMap[careerPath.getDirection()] = careerPath;
    }

    if (migratedLegacyFormat) {
        qDebug() << "检测到旧版职业配置，已自动补齐默认技术栈并回写文件:" << path;
        saveCareerConfig(path);
    }

    qDebug() << "从" << path << "加载了" << careerMap.size() << "个职业路径";
    return true;
}

bool CareerService::saveCareerConfig(const QString& configPath) {
    QString path = configPath.isEmpty() ? "career_paths.json" : configPath;

    QJsonArray array;
    for (const CareerPath& path : careerMap.values()) {
        array.append(path.toJson());
    }

    QJsonDocument doc(array);

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "无法写入职业配置文件：" << path;
        return false;
    }

    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();

    qDebug() << "职业配置已保存到：" << path;
    return true;
}

QList<CareerPath> CareerService::getAllCareerPaths() const {
    return careerMap.values();
}

CareerPath CareerService::getCareerPath(CareerDirection direction) const {
    return careerMap.value(direction);
}

QList<TechStack> CareerService::getTechStacks(CareerDirection direction) const {
    return getCareerPath(direction).getTechStacks();
}

TechStack CareerService::getTechStack(CareerDirection direction, const QString& techStackId) const {
    return getCareerPath(direction).getTechStackById(techStackId);
}

bool CareerService::setUserCareer(int userId, CareerDirection direction) {
    return setUserCareerSelection(userId, direction, QString());
}

bool CareerService::setUserCareerSelection(int userId, CareerDirection direction, const QString& techStackId) {
    if (!userRepo) {
        qWarning() << "UserRepository未初始化";
        return false;
    }

    bool success = userRepo->updateUserCareerSelection(userId, direction, techStackId);
    if (success) {
        // 标记用户不再需要首次选择
        userRepo->updateFirstTimeFlag(userId, false);
        qDebug() << "用户" << userId << "职业方向设置为:" << careerDirectionToString(direction)
                 << "技术栈:" << techStackId;
    }

    return success;
}

CareerDirection CareerService::getUserCareer(int userId) {
    if (!userRepo) {
        return CareerDirection::DEFAULT_SE_COURSE;
    }

    UserProfile user = userRepo->findById(userId);
    return user.getCareerDirection();
}

QString CareerService::getUserTechStackId(int userId) {
    if (!userRepo) {
        return QString();
    }

    UserProfile user = userRepo->findById(userId);
    return user.getTechStackId();
}

CareerPath CareerService::getCurrentUserCareer(int userId) {
    const CareerDirection direction = getUserCareer(userId);
    CareerPath career = getCareerPath(direction);
    const TechStack stack = getCurrentUserTechStack(userId);

    if (!stack.getId().isEmpty()) {
        if (!stack.getSkillTreeFile().isEmpty()) {
            career.setSkillTreeFile(stack.getSkillTreeFile());
        }
        career.setDescription(QString("%1\n当前技术栈：%2").arg(career.getDescription(), stack.getName()));
    }

    return career;
}

TechStack CareerService::getCurrentUserTechStack(int userId) {
    const CareerDirection direction = getUserCareer(userId);
    const QString techStackId = getUserTechStackId(userId);

    TechStack stack = getTechStack(direction, techStackId);
    if (!stack.getId().isEmpty()) {
        return stack;
    }

    const QList<TechStack> stacks = getTechStacks(direction);
    for (const TechStack& candidate : stacks) {
        if (candidate.isRecommended()) {
            return candidate;
        }
    }

    return stacks.isEmpty() ? TechStack() : stacks.first();
}

bool CareerService::shouldSelectCareer(int userId) {
    if (!userRepo) {
        return true; // 默认需要选择
    }

    UserProfile user = userRepo->findById(userId);
    if (user.isFirstTime()) {
        return true;
    }

    const CareerPath currentCareer = getCareerPath(user.getCareerDirection());
    if (currentCareer.hasTechStacks() && user.getTechStackId().isEmpty()) {
        return true;
    }

    return user.getCareerDirection() == CareerDirection::DEFAULT_SE_COURSE;
}

QList<CareerPath> CareerService::getRecommendedCareers() const {
    QList<CareerPath> recommended;
    for (const CareerPath& path : careerMap.values()) {
        if (path.isRecommended()) {
            recommended.append(path);
        }
    }
    return recommended;
}
