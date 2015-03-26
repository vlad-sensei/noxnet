TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

QMAKE_CXXFLAGS += -std=c++14

LIBS += -lboost_system
LIBS += -lboost_thread
LIBS += -lpthread
LIBS += -lsqlite3
LIBS += -lcryptopp
LIBS += -lreadline

SOURCES += main.cpp \
    message_base.cpp \
    message.cpp \
    core.cpp \
    peer.cpp \
    ui.cpp \
    library.cpp \
    sqlite3_base.cpp \
    database.cpp \
    common.cpp \
    inventory.cpp \
    network_initiator_base.cpp \
    ui_client.cpp

HEADERS += \
    glob.h \
    message_base.h \
    message.h \
    core.h \
    peer.h \
    ui.h \
    library.h \
    sqlite3_base.h \
    database.h \
    common.h \
    inventory.h \
    network_base.h \
    network_initiator_base.h \
    ui_client.h

