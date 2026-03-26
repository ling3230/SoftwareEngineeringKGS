#include "careerdirection.h"
#include <QMap>

QString careerDirectionToString(CareerDirection direction) {
    static QMap<CareerDirection, QString> directionMap = {
        {CareerDirection::DEFAULT_SE_COURSE, "软件工程必修课"},
        {CareerDirection::FRONTEND_ENGINEER, "前端工程师"},
        {CareerDirection::BACKEND_ENGINEER, "后端工程师"},
        {CareerDirection::FULLSTACK_ENGINEER, "全栈工程师"},
        {CareerDirection::MOBILE_DEVELOPER, "移动开发工程师"},
        {CareerDirection::DEVOPS_ENGINEER, "DevOps工程师"},
        {CareerDirection::TEST_ENGINEER, "测试工程师"},
        {CareerDirection::PROJECT_MANAGER, "项目经理"},
        {CareerDirection::DATA_SCIENTIST, "数据科学家"},
        {CareerDirection::AI_ENGINEER, "AI工程师"},
        {CareerDirection::GAME_DEVELOPER, "游戏开发工程师"},
        {CareerDirection::CYBERSECURITY_ENGINEER, "网络安全工程师"}
    };

    return directionMap.value(direction, "未知方向");
}

CareerDirection stringToCareerDirection(const QString& str) {
    static QMap<QString, CareerDirection> stringMap = {
        {"软件工程必修课", CareerDirection::DEFAULT_SE_COURSE},
        {"前端工程师", CareerDirection::FRONTEND_ENGINEER},
        {"后端工程师", CareerDirection::BACKEND_ENGINEER},
        {"全栈工程师", CareerDirection::FULLSTACK_ENGINEER},
        {"移动开发工程师", CareerDirection::MOBILE_DEVELOPER},
        {"DevOps工程师", CareerDirection::DEVOPS_ENGINEER},
        {"测试工程师", CareerDirection::TEST_ENGINEER},
        {"项目经理", CareerDirection::PROJECT_MANAGER},
        {"数据科学家", CareerDirection::DATA_SCIENTIST},
        {"AI工程师", CareerDirection::AI_ENGINEER},
        {"游戏开发工程师", CareerDirection::GAME_DEVELOPER},
        {"网络安全工程师", CareerDirection::CYBERSECURITY_ENGINEER}
    };

    return stringMap.value(str, CareerDirection::DEFAULT_SE_COURSE);
}
