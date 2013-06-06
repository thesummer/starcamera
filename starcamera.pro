TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    starcamera.cpp

LIBS += -lopencv_core \
        -lopencv_highgui \
        -lopencv_imgproc \
        -lopencv_features2d \
        -lopencv_calib3d

HEADERS += \
    starcamera.h

