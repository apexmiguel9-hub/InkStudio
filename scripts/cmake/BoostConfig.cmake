# BoostConfig.cmake — entry point en modo CONFIG (el primario del port).
#
# inkscape/CMakeLists.txt:4-5 hace explícitamente:
#   if(POLICY CMP0167)
#       cmake_policy(SET CMP0167 NEW)  # use Boost config de boost 1.70+
# => en CMake >= 3.30 find_package(Boost ...) va DIRECTO a config mode y NUNCA
#    carga un FindBoost.cmake (por eso la run #3 falló aunque el módulo
#    existiera: la política lo deshabilita). Este archivo se localiza vía
#    -DBoost_DIR (vía rápida) o instalándolo como el propio boost lo haría en
#    <boost>/lib/cmake/boost-<ver>/ (lo hace el workflow; sobrevive a cualquier
#    reset de Boost_DIR entre llamadas, porque también está en la búsqueda por
#    CMAKE_PREFIX_PATH / BOOST_ROOT).
#
# Toda la lógica vive en boost-find-core.cmake (compartida con FindBoost.cmake
# del modo módulo, que el workflow instala igualmente como respaldo para refs
# de inkscape sin esa política o CMake < 3.30).

include("${CMAKE_CURRENT_LIST_DIR}/boost-find-core.cmake")
