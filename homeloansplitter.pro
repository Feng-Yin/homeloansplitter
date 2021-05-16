QT += charts widgets

HEADERS += \
    homeloansplitter.h \
    loan.h

SOURCES += \
    homeloansplitter.cpp \
    loan.cpp \
    main.cpp

target.path = ./build/
INSTALLS += target

FORMS += \
    homeloansplitter.ui

DISTFILES += \
    android/AndroidManifest.xml \
    android/build.gradle \
    android/gradle.properties \
    android/gradle/wrapper/gradle-wrapper.jar \
    android/gradle/wrapper/gradle-wrapper.properties \
    android/gradlew \
    android/gradlew.bat \
    android/res/values/libs.xml

contains(ANDROID_TARGET_ARCH,arm64-v8a) {
    ANDROID_PACKAGE_SOURCE_DIR = \
        $$PWD/android
}
