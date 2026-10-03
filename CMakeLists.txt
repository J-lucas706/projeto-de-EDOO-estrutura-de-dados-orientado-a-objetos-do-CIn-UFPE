cmake_minimum_required(VERSION 3.16)

project(SistemaAlocacao
        VERSION 1.0
        DESCRIPTION "Sistema de alocacao de salas - EDOO, CIn UFPE"
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Sem escolher tipo de build, usa Release (so vale para Makefile/Ninja)
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Tipo de build" FORCE)
endif()

# Desligue com -DBUILD_GUI=OFF para compilar so a versao de terminal
option(BUILD_GUI "Compilar a interface grafica Qt (precisa do Qt 6)" ON)

# ------------------------------------------------------------
# Biblioteca com as classes do projeto (Reserva, Sala, SalaTeorica,
# Laboratorio e SistemaAlocacao). Usada pelo terminal e pela GUI.
# ------------------------------------------------------------
add_library(salas STATIC
    src/sala.cpp
    include/salas.h
)
target_include_directories(salas PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

if(MSVC)
    target_compile_options(salas PRIVATE /W4)
else()
    target_compile_options(salas PRIVATE -Wall -Wextra)
endif()

# ------------------------------------------------------------
# Versao de terminal
# ------------------------------------------------------------
add_executable(sistema_salas main.cpp)
target_link_libraries(sistema_salas PRIVATE salas)

# O programa le salas.csv e reservas.csv da pasta atual, entao o alvo
# "run" executa sempre a partir da raiz do projeto (onde ficam os CSVs).
add_custom_target(run
    COMMAND $<TARGET_FILE:sistema_salas>
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS sistema_salas
    USES_TERMINAL
    COMMENT "Executando sistema_salas"
)

# ------------------------------------------------------------
# Interface grafica (Qt 6) - opcional
# ------------------------------------------------------------
if(BUILD_GUI)
    find_package(Qt6 COMPONENTS Widgets QUIET)

    if(Qt6_FOUND)
        add_executable(SistemaAlocacaoGUI gui/gui_main.cpp)
        target_link_libraries(SistemaAlocacaoGUI PRIVATE salas Qt6::Widgets)
        # No Windows, abre sem janela de console preta
        set_target_properties(SistemaAlocacaoGUI PROPERTIES WIN32_EXECUTABLE ON)

        # A GUI procura salas.csv subindo a partir da pasta do executavel
        # (ate 8 niveis), entao funciona com build/ dentro do projeto.
        add_custom_target(run_gui
            COMMAND $<TARGET_FILE:SistemaAlocacaoGUI>
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            DEPENDS SistemaAlocacaoGUI
            USES_TERMINAL
            COMMENT "Executando SistemaAlocacaoGUI"
        )
    else()
        message(WARNING
            "Qt 6 nao encontrado: compilando apenas a versao de terminal.\n"
            "Para a GUI, instale o Qt 6 (veja o passo a passo) e, se preciso, "
            "passe -DCMAKE_PREFIX_PATH=<pasta do Qt>. "
            "Para silenciar este aviso use -DBUILD_GUI=OFF.")
    endif()
endif()