QT += testlib
QT -= gui

CONFIG += qt console warn_on depend_includepath testcase
CONFIG -= app_bundle

TEMPLATE = app

HEADERS +=  \
    ../../src/douglaspeuker.h

SOURCES +=  \
    testdouglaspeucker.cpp  \
    ../../src/douglaspeucker.cpp
