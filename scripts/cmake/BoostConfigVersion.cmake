# BoostConfigVersion.cmake — check de versión para el modo CONFIG del port.
#
# Sin este archivo, find_package(Boost 1.19.0) en config mode descarta el
# paquete (pidió versión y no hay version file) aunque BoostConfig.cmake
# exista. La versión declarada aquí es la del boost de Blender (108700 =>
# 1.87.0); sólo se compara contra lo que pide Inkscape (1.19.0), así que un
# cambio futuro de Blender no la rompe: el version REAL la sigue leyendo y
# reportando boost-find-core.cmake desde el version.hpp.

set(PACKAGE_VERSION "1.87.0")

if(PACKAGE_FIND_VERSION VERSION_GREATER PACKAGE_VERSION)
  # pedido > disponible => incompatible
  set(PACKAGE_VERSION_COMPATIBLE FALSE)
else()
  # disponible >= pedido => compatible (1.87.0 cumple >= 1.19.0)
  set(PACKAGE_VERSION_COMPATIBLE TRUE)
  if(PACKAGE_FIND_VERSION VERSION_EQUAL PACKAGE_VERSION)
    set(PACKAGE_VERSION_EXACT TRUE)
  endif()
endif()
