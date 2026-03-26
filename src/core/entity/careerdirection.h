#ifndef CAREERDIRECTION_H
#define CAREERDIRECTION_H

#include <QString>

enum class CareerDirection {
    DEFAULT_SE_COURSE = 0,   // 软件工程必修课（默认）
    FRONTEND_ENGINEER,       // 前端工程师
    BACKEND_ENGINEER,        // 后端工程师
    FULLSTACK_ENGINEER,      // 全栈工程师
    MOBILE_DEVELOPER,        // 移动开发
    DEVOPS_ENGINEER,         // DevOps工程师
    TEST_ENGINEER,           // 测试工程师
    PROJECT_MANAGER,         // 项目经理

    // 可以根据需要继续扩展
    DATA_SCIENTIST,          // 数据科学家
    AI_ENGINEER,             // AI工程师
    GAME_DEVELOPER,          // 游戏开发
    CYBERSECURITY_ENGINEER   // 网络安全
};

// 辅助函数：枚举转字符串
QString careerDirectionToString(CareerDirection direction);
CareerDirection stringToCareerDirection(const QString& str);

#endif // CAREERDIRECTION_H
