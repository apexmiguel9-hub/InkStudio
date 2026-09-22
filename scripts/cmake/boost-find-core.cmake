# boost-find-core.cmake — lógica ÚNICA de localización de Boost para el port.
#
# Se carga desde dos entry points (los dos modos de find_package son
# excluyentes, nunca compiten):
#   - BoostConfig.cmake  → modo CONFIG (lo que inkscape/CMakeLists.txt:4-5
#                          fuerza con cmake_policy(SET CMP0167 NEW); primario)
#   - FindBoost.cmake    → modo MÓDULO (respaldo para refs de inkscape sin
#                          esa política, o CMake < 3.30)
#
# Por qué existe (run #3 del port): find_package(Boost 1.19.0 REQUIRED)
# murió con "By not providing FindBoost.cmake ... asked CMake to find a
# package configuration file provided by Boost". Causa doble: la política
# NEW exige BoostConfig.cmake y el boost precompilado de Blender
# (boost/include + lib/libboost_stacktrace_basic.a) NO lo incluye. Mismo
# patrón que scripts/gen-pc.sh: dar la capa que el consumidor exige sin
# reconstruir nada.
#
# Los call-sites reales (rama UNIX/Android de DefineDependsandFlags.cmake):
#   371: find_package(Boost 1.19.0 REQUIRED)                         # headers
#   373: find_package(Boost 1.19.0 COMPONENTS stacktrace_backtrace)  # opcional
#        if (BOOST_FOUND) -> -lbacktrace / BOOST_STACKTRACE_USE_BACKTRACE
#   378: find_package(Boost 1.19.0 REQUIRED COMPONENTS stacktrace_basic)
#   390: list(APPEND INKSCAPE_LIBS Boost::headers)
#
# => Exportar BOOST_FOUND (MAYÚSCULAS): la lee el if(BOOST_FOUND) de la 377 y
#    ni el modo config ni el módulo la rellenan por su cuenta con esta
#    semántica. Tras el 373 debe quedar FALSE (no tenemos backtrace) para
#    que Inkscape caiga al fallback stacktrace_basic que sí tenemos.
# => Semántica del módulo clásico: cualquier componente listado sin su lib
#    deja Boost_FOUND=FALSE; REQUIRED convierte eso en FATAL_ERROR.
#
# Búsqueda: BOOST_ROOT/-DBOOST_ROOT (con CMP0144 OLD find_package ignora la
# mayúscula, por eso el HINTS explícito — verificado en el test local) +
# CMAKE_PREFIX_PATH. FIND_ROOT_PATH_MODE_*=BOTH en el toolchain garantiza
# que el prefijo host absoluto se busque tal cual (clase sys_root de meson).

# --- cabeceras + versión -------------------------------------------------
find_path(Boost_INCLUDE_DIR
  NAMES boost/version.hpp
  HINTS ${BOOST_ROOT} $ENV{BOOST_ROOT} ${Boost_ROOT}
  PATH_SUFFIXES include
)

set(Boost_VERSION "")
set(Boost_VERSION_STRING "")
if(Boost_INCLUDE_DIR AND EXISTS "${Boost_INCLUDE_DIR}/boost/version.hpp")
  file(STRINGS "${Boost_INCLUDE_DIR}/boost/version.hpp" _boost_ver_line
       REGEX "^#define[ \t]+BOOST_VERSION[ \t]+[0-9]+")
  string(REGEX REPLACE ".*BOOST_VERSION[ \t]+([0-9]+).*" "\\1"
         _boost_ver_num "${_boost_ver_line}")
  # esquema boost: major*100000 + minor*100 + patch (108700 => 1.87.0)
  math(EXPR _boost_ver_major "${_boost_ver_num} / 100000")
  math(EXPR _boost_ver_minor "(${_boost_ver_num} / 100) % 100")
  math(EXPR _boost_ver_patch "${_boost_ver_num} % 100")
  set(Boost_VERSION "${_boost_ver_num}")
  set(Boost_VERSION_STRING
      "${_boost_ver_major}.${_boost_ver_minor}.${_boost_ver_patch}")
endif()

set(_boost_ver_ok TRUE)
if(Boost_FIND_VERSION)
  if(NOT Boost_VERSION_STRING OR
     Boost_VERSION_STRING VERSION_LESS Boost_FIND_VERSION)
    set(_boost_ver_ok FALSE)
  endif()
endif()

# --- componentes (boost_<nombre>) ---------------------------------------
set(_boost_missing_comps "")
foreach(_boost_comp IN LISTS Boost_FIND_COMPONENTS)
  find_library(Boost_${_boost_comp}_LIBRARY
    NAMES "boost_${_boost_comp}"
    HINTS ${BOOST_ROOT} $ENV{BOOST_ROOT} ${Boost_ROOT} "${Boost_INCLUDE_DIR}/.."
    PATH_SUFFIXES lib lib64 stage/lib
  )
  if(Boost_${_boost_comp}_LIBRARY)
    set(Boost_${_boost_comp}_FOUND TRUE)
  else()
    set(Boost_${_boost_comp}_FOUND FALSE)
    list(APPEND _boost_missing_comps "${_boost_comp}")
  endif()
endforeach()

# --- variables de salida (interfaz del modulo clasico) -------------------
set(Boost_INCLUDE_DIRS "${Boost_INCLUDE_DIR}")
set(Boost_LIBRARIES "")
foreach(_boost_comp IN LISTS Boost_FIND_COMPONENTS)
  if(Boost_${_boost_comp}_LIBRARY)
    list(APPEND Boost_LIBRARIES "${Boost_${_boost_comp}_LIBRARY}")
  endif()
endforeach()

set(Boost_FOUND FALSE)
if(Boost_INCLUDE_DIR AND _boost_ver_ok AND NOT _boost_missing_comps)
  set(Boost_FOUND TRUE)
endif()
set(BOOST_FOUND ${Boost_FOUND})   # LEGADO: Inkscape:377 lee la mayúscula

# --- targets importados ---------------------------------------------------
if(Boost_INCLUDE_DIR AND NOT TARGET Boost::headers)
  add_library(Boost::headers INTERFACE IMPORTED)
  set_target_properties(Boost::headers PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${Boost_INCLUDE_DIR}")
endif()
foreach(_boost_comp IN LISTS Boost_FIND_COMPONENTS)
  if(Boost_${_boost_comp}_LIBRARY AND NOT TARGET Boost::${_boost_comp})
    add_library(Boost::${_boost_comp} UNKNOWN IMPORTED)
    set_target_properties(Boost::${_boost_comp} PROPERTIES
      IMPORTED_LOCATION "${Boost_${_boost_comp}_LIBRARY}")
  endif()
endforeach()

# --- mensajes / errores ----------------------------------------------------
if(Boost_FOUND)
  if(NOT Boost_FIND_QUIETLY)
    message(STATUS "Found Boost: ${Boost_INCLUDE_DIR} (version ${Boost_VERSION_STRING})")
  endif()
else()
  set(_boost_why "")
  if(NOT Boost_INCLUDE_DIR)
    string(APPEND _boost_why " headers")
  elseif(NOT _boost_ver_ok)
    string(APPEND _boost_why " version(${Boost_VERSION_STRING} < ${Boost_FIND_VERSION})")
  endif()
  if(_boost_missing_comps)
    string(APPEND _boost_why " component:${_boost_missing_comps}")
  endif()
  if(Boost_FIND_REQUIRED)
    message(FATAL_ERROR
      "Could NOT find Boost (missing:${_boost_why}) "
      "requested version ${Boost_FIND_VERSION}. "
      "Busca en BOOST_ROOT y CMAKE_PREFIX_PATH; en este port debe existir "
      "<boost>/include/boost/version.hpp (Blender trae Boost 1.87.0).")
  elseif(NOT Boost_FIND_QUIETLY)
    message(STATUS "Could NOT find Boost (missing:${_boost_why})")
  endif()
endif()
