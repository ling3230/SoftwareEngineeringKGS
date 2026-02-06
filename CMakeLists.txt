cmake_minimum_required(VERSION 3.16)

project(SE_KG VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Sql)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Sql)

# ============ 主要修改开始 ============

# 1. 设置头文件包含路径
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# 2. 设置源文件列表（按新目录结构组织）
set(PROJECT_SOURCES
    src/main.cpp                 # main.cpp移动到src/下
    src/ui/mainwindow.cpp       # UI文件放到src/ui/
    src/ui/mainwindow.h
    src/ui/mainwindow.ui
    src/core/database/databasemanager.cpp  # 数据库文件放到src/core/database/
    src/core/database/databasemanager.h
    src/core/entity/knowledgepoint.h
    src/core/entity/knowledgepoint.cpp
    src/core/repository/knowledgerepository.cpp
    src/core/repository/knowledgerepository.h
)

# 3. 设置资源文件（如果需要）
# set(PROJECT_RESOURCES
#     resources/database/se_kg.db
#     resources/icons/app_icon.png
# )

# ============ 主要修改结束 ============

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    find_package(Qt6 REQUIRED COMPONENTS Core)

    qt_add_executable(SE_KG
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        # 资源文件可以在这里添加
        # ${PROJECT_RESOURCES}
    )
else()
    if(ANDROID)
        add_library(SE_KG SHARED
            ${PROJECT_SOURCES}
        )
    else()
        add_executable(SE_KG
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(SE_KG PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Sql)
target_link_libraries(SE_KG PRIVATE Qt6::Core)

# 设置目标属性
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.SE_KG)
endif()
set_target_properties(SE_KG PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

# 安装配置
include(GNUInstallDirs)
install(TARGETS SE_KG
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(SE_KG)
endif()

# 可选：复制资源文件到构建目录
# add_custom_command(TARGET SE_KG POST_BUILD
#     COMMAND ${CMAKE_COMMAND} -E copy_directory
#     ${CMAKE_CURRENT_SOURCE_DIR}/resources
#     $<TARGET_FILE_DIR:SE_KG>/resources
# )
