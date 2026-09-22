# FindBoost.cmake — entry point en modo MÓDULO (respaldo del port).
#
# Sólo se carga si el modo módulo está activo: política CMP0167 OLD/ausente
# (CMake < 3.30, o refs de inkscape anteriores a CMakeLists.txt:4-5). Con el
# ref actual (CMP0167 NEW explícito) manda BoostConfig.cmake; este archivo
# evita romper si se cambia el ref del workflow a un tag viejo.
#
# El workflow lo copia a inkscape/CMakeScripts/Modules/ (el dir que
# CMakeLists.txt:23 mete en CMAKE_MODULE_PATH — ahí viven FindPotrace y
# FindJeMalloc), así que se encuentra ANTES que cualquier módulo nativo.
# Toda la lógica vive en boost-find-core.cmake.

include("${CMAKE_CURRENT_LIST_DIR}/boost-find-core.cmake")
