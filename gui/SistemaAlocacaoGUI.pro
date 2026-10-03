QT += core gui widgets
 
CONFIG += c++17
 
TARGET = SistemaAlocacaoGUI
TEMPLATE = app
 
# Reaproveita as classes que ja existem (sem alterar nenhuma delas)
INCLUDEPATH += ../include
 
SOURCES += \
    gui_main.cpp \
    ../src/sala.cpp
 
HEADERS += \
    ../include/salas.h
 