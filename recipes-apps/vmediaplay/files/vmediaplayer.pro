QT += core gui widgets gui-private
requires(qtConfig(combobox))

INCLUDEPATH += ./recipe-sysroot/usr/include/glib-2.0
INCLUDEPATH += ./recipe-sysroot/usr/lib/glib-2.0/include
INCLUDEPATH += ./recipe-sysroot/usr/include/gstreamer-1.0
INCLUDEPATH += ./recipe-sysroot/usr/lib/gstreamer-1.0/include
INCLUDEPATH += ./recipe-sysroot/usr/include/gstreamer-1.0/include

LIBS += -lgstreamer-1.0 -lgobject-2.0 -lglib-2.0 -lgstvideo-1.0 -lwayland-client -lgstwayland-1.0 -lgstapp-1.0

SOURCES += mainwindow.cpp setting_dialog.cpp
HEADERS += mainwindow.h setting_dialog.h
