QT += core
QT += network
QT -= gui

TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
#CONFIG -= qt

SOURCES += main.cpp \
    command_processors.cpp

HEADERS += \
    st_basic.h \
    ht_bucket.h \
    my_stack.h \
    my_sort.h \
    ht_items.h \
    my_fifo.h

DEFINES += QT_DEPRECATED_WARNINGS
