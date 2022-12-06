#include "translationhelper.h"

TranslationHelper::TranslationHelper() : QWidget() {
}

void TranslationHelper::useEnglishTranslator() {
    QCoreApplication::removeTranslator(&translator);

    QString dir = QString(QCoreApplication::applicationDirPath());
    if (translator.load(QString("Translation_en_HK.qm"), dir)) {
        QCoreApplication::installTranslator(&translator);
    }
}

void TranslationHelper::useSimplifiedChineseTranslator()
{
    QCoreApplication::removeTranslator(&translator);

    QString dir = QString(QCoreApplication::applicationDirPath());
    if (translator.load(QString("Translation_zh_CN.qm"), dir)) {
        QCoreApplication::installTranslator(&translator);
    }
}
