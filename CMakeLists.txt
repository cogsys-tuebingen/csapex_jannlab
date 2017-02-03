cmake_minimum_required(VERSION 2.8.3)
project(csapex_jannlab)

## Find catkin macros and libraries
## if COMPONENTS list like find_package(catkin REQUIRED COMPONENTS xyz)
## is used, also find other catkin packages
find_package(catkin REQUIRED COMPONENTS
  csapex
  csapex_core_plugins
  csapex_ml
  cslibs_jcppsocket
)

add_definitions(-DQT_NO_KEYWORDS)

catkin_package(
   CATKIN_DEPENDS csapex_core_plugins csapex_ml
)

include_directories(include
  ${catkin_INCLUDE_DIRS}
)

###########
## Build ##
###########

include_directories(
  ${catkin_INCLUDE_DIRS}
)

add_library(jannlab_plugin
    src/jann_time_series_export.cpp
    src/jann_format_export.cpp
    src/jann_remote_connection.cpp
)

target_link_libraries(jannlab_plugin
   ${catkin_LIBRARIES}
)

#############
## Install ##
#############

install(FILES plugins.xml
        DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION})

install(TARGETS jannlab_plugin
        ARCHIVE DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
        LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
        RUNTIME DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION})


