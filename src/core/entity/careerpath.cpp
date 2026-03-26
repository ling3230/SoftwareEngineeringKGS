// entity/CareerPath.cpp
#include "CareerPath.h"
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDebug>

TechStack::TechStack(const QString& id, const QString& name,
                     const QString& description, const QString& icon)
    : id(id)
    , name(name)
    , description(description)
    , icon(icon)
{
}

QJsonObject TechStack::toJson() const {
    QJsonObject json;
    json["id"] = id;
    json["name"] = name;
    json["description"] = description;
    json["icon"] = icon;
    json["skillTreeFile"] = skillTreeFile;
    json["recommended"] = recommended;
    return json;
}

TechStack TechStack::fromJson(const QJsonObject& json) {
    TechStack stack;
    stack.id = json.value("id").toString();
    stack.name = json.value("name").toString();
    stack.description = json.value("description").toString();
    stack.icon = json.value("icon").toString();
    stack.skillTreeFile = json.value("skillTreeFile").toString();
    stack.recommended = json.value("recommended").toBool(false);
    return stack;
}

CareerPath::CareerPath()
    : direction(CareerDirection::DEFAULT_SE_COURSE)
    , name("软件工程必修课")
    , description("软件工程核心课程体系")
    , icon("📚")
    , recommended(true) {}

CareerPath::CareerPath(CareerDirection direction, const QString& name,
                       const QString& description, const QString& icon)
    : direction(direction)
    , name(name)
    , description(description)
    , icon(icon)
    , recommended(false) {}

QJsonObject CareerPath::toJson() const {
    QJsonObject json;
    json["direction"] = static_cast<int>(direction);
    json["name"] = name;
    json["description"] = description;
    json["icon"] = icon;
    json["skillTreeFile"] = skillTreeFile;
    json["recommended"] = recommended;

    QJsonArray techStackArray;
    for (const TechStack& stack : techStacks) {
        techStackArray.append(stack.toJson());
    }
    json["techStacks"] = techStackArray;

    return json;
}

CareerPath CareerPath::fromJson(const QJsonObject& json) {
    CareerPath path;

    if (json.contains("direction")) {
        path.direction = static_cast<CareerDirection>(json["direction"].toInt());
    }

    if (json.contains("name")) {
        path.name = json["name"].toString();
    }

    if (json.contains("description")) {
        path.description = json["description"].toString();
    }

    if (json.contains("icon")) {
        path.icon = json["icon"].toString();
    }

    if (json.contains("skillTreeFile")) {
        path.skillTreeFile = json["skillTreeFile"].toString();
    }

    if (json.contains("recommended")) {
        path.recommended = json["recommended"].toBool();
    }

    if (json.contains("techStacks") && json["techStacks"].isArray()) {
        const QJsonArray techStackArray = json["techStacks"].toArray();
        for (const QJsonValue& value : techStackArray) {
            if (value.isObject()) {
                path.techStacks.append(TechStack::fromJson(value.toObject()));
            }
        }
    }

    return path;
}

TechStack CareerPath::getTechStackById(const QString& techStackId) const
{
    for (const TechStack& stack : techStacks) {
        if (stack.getId() == techStackId) {
            return stack;
        }
    }

    return TechStack();
}
