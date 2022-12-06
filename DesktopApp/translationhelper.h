#ifndef TRANSLATIONHELPER_H
#define TRANSLATIONHELPER_H

#include <QtWidgets/QWidget>
#include "stdafx.h"
#include <qtranslator.h>

class TranslationHelper : public QWidget
{
    Q_OBJECT

public:
    static TranslationHelper& getInstance() {
        static TranslationHelper instance;
        return instance;
    }

    TranslationHelper(TranslationHelper const&) = delete;

    void operator=(TranslationHelper const&) = delete;

    QTranslator translator;

    void useEnglishTranslator();
    void useSimplifiedChineseTranslator();

private:
    TranslationHelper();
};

#endif
