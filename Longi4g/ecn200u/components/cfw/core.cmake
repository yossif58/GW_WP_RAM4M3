# Copyright (C) 2018 RDA Technologies Limited and/or its affiliates("RDA").
# All rights reserved.
#
# This software is supplied "AS IS" without any warranties.
# RDA assumes no responsibility or liability for the use of the software,
# conveys no license or title under any patent, copyright, or mask work
# right to the product. RDA reserves the right to make changes in the
# software without notification.  RDA also make no representation or
# warranty that such application will be suitable for the specified use
# without further testing or modification.

set(import_lib ${out_lib_dir}/libcfw.a)
configure_file(${ql_dm_dir}/libcfw.a ${import_lib} COPYONLY)
add_app_libraries(${import_lib})
add_library(cfw STATIC IMPORTED GLOBAL)
set_target_properties(cfw PROPERTIES IMPORTED_LOCATION ${import_lib} INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(import_lib ${out_lib_dir}/libcfw_core.a)
configure_file(core/${CONFIG_SOC}/libcfw_core.a ${import_lib} COPYONLY)
add_app_libraries(${import_lib})
add_library(cfw_core STATIC IMPORTED GLOBAL)
set_target_properties(cfw_core PROPERTIES IMPORTED_LOCATION ${import_lib})
