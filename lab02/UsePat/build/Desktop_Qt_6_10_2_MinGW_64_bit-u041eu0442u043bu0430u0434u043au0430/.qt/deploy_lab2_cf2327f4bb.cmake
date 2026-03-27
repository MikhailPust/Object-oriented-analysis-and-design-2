include("C:/OOP2/lab2/build/Desktop_Qt_6_10_2_MinGW_64_bit-u041eu0442u043bu0430u0434u043au0430/.qt/QtDeploySupport.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/lab2-plugins.cmake" OPTIONAL)
set(__QT_DEPLOY_I18N_CATALOGS "qtbase")

qt6_deploy_runtime_dependencies(
    EXECUTABLE "C:/OOP2/lab2/build/Desktop_Qt_6_10_2_MinGW_64_bit-u041eu0442u043bu0430u0434u043au0430/lab2.exe"
    GENERATE_QT_CONF
)
