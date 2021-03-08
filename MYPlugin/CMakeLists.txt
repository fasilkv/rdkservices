
set(PLUGIN_NAME MYPlugin)
set(MODULE_NAME ${NAMESPACE}${PLUGIN_NAME})

find_package(${NAMESPACE}Plugins REQUIRED)


#add depended library file here
add_library(${MODULE_NAME} SHARED
        MYPlugin.cpp
        Module.cpp
        ../helpers/utils.cpp)

set_target_properties(${MODULE_NAME} PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES)

target_include_directories(${MODULE_NAME} PRIVATE ../helpers)

#it will find device setting configuration and find other packages present in cmake/
find_package(DS)
if (DS_FOUND)
    find_package(IARMBus)
    add_definitions(-DDS_FOUND)
    target_include_directories(${MODULE_NAME} PRIVATE ${IARMBUS_INCLUDE_DIRS})
    target_include_directories(${MODULE_NAME} PRIVATE ${DS_INCLUDE_DIRS})
    target_link_libraries(${MODULE_NAME} PRIVATE ${NAMESPACE}Plugins::${NAMESPACE}Plugins ${IARMBUS_LIBRARIES} ${DS_LIBRARIES})
else (DS_FOUND)
    target_link_libraries(${MODULE_NAME} PRIVATE ${NAMESPACE}Plugins::${NAMESPACE}Plugins)
endif(DS_FOUND)


#installing
install(TARGETS ${MODULE_NAME}
        DESTINATION lib/${STORAGE_DIRECTORY}/plugins)

write_config(${PLUGIN_NAME})
