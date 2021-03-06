/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "theme.h"
#include "ui_theme.h"

#include <QGSettings/QGSettings>

#include "SwitchButton/switchbutton.h"

#include "themewidget.h"
#include "widgetgroup.h"
#include "cursor/xcursortheme.h"

#include "../../../shell/customstyle.h"

#include <QDebug>
#include <QtDBus/QDBusConnection>
/**
 * GTK主题
 */
#define THEME_GTK_SCHEMA "org.mate.interface"
#define MODE_GTK_KEY "gtk-theme"
/* GTK图标主题 */
#define ICON_GTK_KEY "icon-theme"

/**
 * QT主题
 */
#define THEME_QT_SCHEMA "org.ukui.style"
#define MODE_QT_KEY "style-name"
/* QT图标主题 */
#define ICON_QT_KEY "icon-theme-name"


/**
 * 窗口管理器Marco主题
 */
#define MARCO_SCHEMA "org.gnome.desktop.wm.preferences"
#define MARCO_THEME_KEY "theme"

#define ICONTHEMEPATH "/usr/share/icons/"
#define SYSTHEMEPATH "/usr/share/themes/"
#define CURSORS_THEMES_PATH "/usr/share/icons/"

#define CURSOR_THEME_SCHEMA "org.ukui.peripherals-mouse"
#define CURSOR_THEME_KEY "cursor-theme"

#define ICONWIDGETHEIGH 74


namespace {

    // Preview cursors
    const char * const cursor_names[] =
    {
        "left_ptr",
        "left_ptr_watch",
        "wait",
        "pointing_hand",
        "whats_this",
        "ibeam",
        "size_all",
        "size_fdiag",
        "cross",
        "split_h",
        "size_ver",
        "size_hor",
        "size_bdiag",
        "split_v",
    };

    const int numCursors      = 9;     // The number of cursors from the above list to be previewed
}

Theme::Theme()
{
    ui = new Ui::Theme;
    pluginWidget = new QWidget;
    pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(pluginWidget);

    pluginName = tr("Theme");
    pluginType = PERSONALIZED;

    ui->titleLabel->setStyleSheet("QLabel{font-size: 18px; color: palette(windowText);}");

    settingsCreate = false;

    const QByteArray id(THEME_GTK_SCHEMA);
    const QByteArray idd(THEME_QT_SCHEMA);
    const QByteArray iid(CURSOR_THEME_SCHEMA);

    //设置样式
    setupStylesheet();
    //设置组件
    setupComponent();

    // init kwin settings
    setupSettings();

    if (QGSettings::isSchemaInstalled(id) && QGSettings::isSchemaInstalled(idd) && QGSettings::isSchemaInstalled(iid)){
        gtkSettings = new QGSettings(id);
        qtSettings = new QGSettings(idd);
        curSettings = new QGSettings(iid);

        settingsCreate = true;

        initThemeMode();
        initIconTheme();
        initCursorTheme();
        initEffectSettings();
        initConnection();
    } else {
        qCritical() << THEME_GTK_SCHEMA << "or" << THEME_QT_SCHEMA << "or" << CURSOR_THEME_SCHEMA << "not installed\n";
    }

}

Theme::~Theme()
{
    delete ui;
    if (settingsCreate){
        delete gtkSettings;
        delete qtSettings;
        delete curSettings;
    }

}

QString Theme::get_plugin_name(){
    return pluginName;
}

int Theme::get_plugin_type(){
    return pluginType;
}

QWidget *Theme::get_plugin_ui(){
    return pluginWidget;
}

void Theme::plugin_delay_control(){

}

void Theme::setupStylesheet(){
//    pluginWidget->setStyleSheet("background: #ffffff;");

//    ui->controlWidget->setStyleSheet("QWidget#controlWidget{background: #F4F4F4; border-radius: 6px;}");

//    ui->effectWidget->setStyleSheet("QWidget{background: #F4F4F4; border-radius: 6px;}");
//    ui->line->setStyleSheet("QFrame{border-top: 1px solid #CCCCCC; border-left: none; border-right: none; border-bottom: none;}");

//    ui->resetBtn->setStyleSheet("QPushButton#resetBtn{background: #F4F4F4; border: none; border-radius: 4px;}"
//                                "QPushButton:hover:!pressed#resetBtn{border: none; background: #3D6BE5; border-radius: 2px;}"
//                                "QPushButton:hover:pressed#resetBtn{border: none; background: #2C5AD6; border-radius: 2px;}");

//    ui->transparencySlider->setStyleSheet("QSlider{height: 20px;}"
//                                          "QSlider::groove:horizontal{border: none;}"
//                                          "QSlider::add-page:horizontal{background: #808080; border-radius: 2px; margin-top: 8px; margin-bottom: 9px;}"
//                                          "QSlider::sub-page:horizontal{background: #3D6BE5; border-radius: 2px; margin-top: 8px; margin-bottom: 9px;}"
//                                          "QSlider::handle:horizontal{width: 20px; height: 20px; border-image: url(:/img/plugins/fonts/bigRoller.png);}"
//                                          "");
}


void Theme::setupSettings() {
    QString filename = QDir::homePath() + "/.config/ukui-kwinrc";
    kwinSettings = new QSettings(filename, QSettings::IniFormat);

    kwinSettings->beginGroup("Plugins");

    bool kwin = kwinSettings->value("blurEnabled", kwin).toBool();

    kwinSettings->endGroup();

    effectSwitchBtn->setChecked(!kwin);

}

void Theme::setupComponent(){

    ui->lightButton->hide();
    //隐藏现阶段不支持功能
    ui->controlLabel->hide();
    ui->controlWidget->hide();

    ui->defaultButton->setProperty("value", "ukui-white");
    ui->lightButton->setProperty("value", "ukui-default");
    ui->darkButton->setProperty("value", "ukui-black");

    buildThemeModeBtn(ui->defaultButton, tr("Default"), "default");
    buildThemeModeBtn(ui->lightButton, tr("Light"), "light");
    buildThemeModeBtn(ui->darkButton, tr("Dark"), "dark");

    setupControlTheme();

//    ui->effectLabel->hide();
//    ui->effectWidget->hide();
    ui->transparencySlider->hide();
    ui->label_9->hide();
    ui->label_11->hide();
    ui->line->hide();


    //构建并填充特效开关按钮
    effectSwitchBtn = new SwitchButton(pluginWidget);
    ui->effectHorLayout->addWidget(effectSwitchBtn);


//    kwinGsettings = new QDBusInterface("org.freedesktop.timedate1",
//                                       "/org/freedesktop/timedate1",
//                                       "org.freedesktop.timedate1",
//                                       QDBusConnection::systemBus());
//    QDBusConnection::sessionBus().unregisterService("com.ukui.KWin");
//    QDBusConnection::sessionBus().registerService("com.ukui.KWin");
//    QDBusConnection::sessionBus().registerObject("/KWin", this,QDBusConnection :: ExportAllSlots | QDBusConnection :: ExportAllSignals);

}

void Theme::buildThemeModeBtn(QPushButton *button, QString name, QString icon){
    //设置默认按钮
//    button->setStyleSheet("QPushButton{background: #ffffff; border: none;}");

    QVBoxLayout * baseVerLayout = new QVBoxLayout(button);
    baseVerLayout->setSpacing(8);
    baseVerLayout->setMargin(0);
    QLabel * iconLabel = new QLabel(button);
    iconLabel->setFixedSize(QSize(176, 105));
    iconLabel->setScaledContents(true);
    QString fullicon = QString("://img/plugins/theme/%1.png").arg(icon);
    iconLabel->setPixmap(QPixmap(fullicon));

    QHBoxLayout * bottomHorLayout = new QHBoxLayout;
    bottomHorLayout->setSpacing(8);
    bottomHorLayout->setMargin(0);
    QLabel * statusLabel = new QLabel(button);
    statusLabel->setFixedSize(QSize(16, 16));
    statusLabel->setScaledContents(true);
#if QT_VERSION <= QT_VERSION_CHECK(5, 12, 0)
    connect(ui->themeModeBtnGroup, static_cast<void (QButtonGroup::*)(QAbstractButton *)>(&QButtonGroup::buttonClicked), [=](QAbstractButton * eBtn){
#else
    connect(ui->themeModeBtnGroup, QOverload<QAbstractButton *>::of(&QButtonGroup::buttonClicked), [=](QAbstractButton * eBtn){
#endif
        if (eBtn == button)
            statusLabel->setPixmap(QPixmap("://img/plugins/theme/selected.png"));
        else
            statusLabel->clear();
    });
    QLabel * nameLabel = new QLabel(button);
    QSizePolicy nameSizePolicy = nameLabel->sizePolicy();
    nameSizePolicy.setVerticalPolicy(QSizePolicy::Fixed);
    nameSizePolicy.setHorizontalPolicy(QSizePolicy::Fixed);
    nameLabel->setSizePolicy(nameSizePolicy);
    nameLabel->setText(name);

    bottomHorLayout->addStretch();
    bottomHorLayout->addWidget(statusLabel);
    bottomHorLayout->addWidget(nameLabel);
    bottomHorLayout->addStretch();

    baseVerLayout->addWidget(iconLabel);
    baseVerLayout->addLayout(bottomHorLayout);

    button->setLayout(baseVerLayout);
}

void Theme::initThemeMode(){
    //监听主题改变
    connect(qtSettings, &QGSettings::changed, this, [=](const QString &key){
        if (key == "styleName") {
            auto style = qtSettings->get(key).toString();
            qApp->setStyle(new InternalStyle(style));
        }

        //获取当前主题
        QString currentThemeMode = qtSettings->get(MODE_QT_KEY).toString();
        qApp->setStyle(new InternalStyle(currentThemeMode));

        for (QAbstractButton * button : ui->themeModeBtnGroup->buttons()){
            QVariant valueVariant = button->property("value");
            if (valueVariant.isValid() && valueVariant.toString() == currentThemeMode)
                button->click();
        }
    });

    //获取当前主题
    QString currentThemeMode = qtSettings->get(MODE_QT_KEY).toString();
    qApp->setStyle(new InternalStyle(currentThemeMode));
    //设置界面
    for (QAbstractButton * button : ui->themeModeBtnGroup->buttons()){
        QVariant valueVariant = button->property("value");
        if (valueVariant.isValid() && valueVariant.toString() == currentThemeMode)
            button->click();
//            button->setChecked(true);
    }

#if QT_VERSION <= QT_VERSION_CHECK(5, 12, 0)
    connect(ui->themeModeBtnGroup, static_cast<void (QButtonGroup::*)(QAbstractButton *)>(&QButtonGroup::buttonClicked), [=](QAbstractButton * button){
#else
    connect(ui->themeModeBtnGroup, QOverload<QAbstractButton *>::of(&QButtonGroup::buttonClicked), this, [=](QAbstractButton * button){
#endif
        //设置主题
        QString themeMode = button->property("value").toString();
        QString currentThemeMode = qtSettings->get(MODE_QT_KEY).toString();
        if (QString::compare(currentThemeMode, themeMode)){
            qApp->setStyle(new InternalStyle(themeMode));
            qtSettings->set(MODE_QT_KEY, themeMode);
            gtkSettings->set(MODE_GTK_KEY, themeMode);

            writeKwinSettings(!effectSwitchBtn->isChecked(), themeMode);
        }
    });
}

void Theme::initIconTheme(){
    //获取当前图标主题(以QT为准，后续可以对比GTK两个值)
    QString currentIconTheme = qtSettings->get(ICON_QT_KEY).toString();

    //构建图标主题Widget Group，方便更新选中/非选中状态
    WidgetGroup * iconThemeWidgetGroup = new WidgetGroup;
    connect(iconThemeWidgetGroup, &WidgetGroup::widgetChanged, [=](ThemeWidget * preWidget, ThemeWidget * curWidget){
        if (preWidget)
            preWidget->setSelectedStatus(false);
        curWidget->setSelectedStatus(true);

        QString value = curWidget->getValue();
        //设置图标主题
        qtSettings->set(ICON_QT_KEY, value);
        gtkSettings->set(ICON_GTK_KEY, value);
    });

    //构建图标主题QDir
    QDir themesDir = QDir(ICONTHEMEPATH);

    foreach (QString themedir, themesDir.entryList(QDir::Dirs)) {
        if (themedir.startsWith("ukui-icon-theme-")){
            QDir appsDir = QDir(ICONTHEMEPATH + themedir + "/48x48/apps/");
            appsDir.setFilter(QDir::Files | QDir::NoSymLinks);
            QStringList appIconsList = appsDir.entryList();

            QStringList showIconsList;
            for (int i = 0; i < appIconsList.size(); i++){
                if (i%64 == 0 && i < 6 * 64){
                    showIconsList.append(appsDir.path() + "/" + appIconsList.at(i));
                }
            }

            ThemeWidget * widget = new ThemeWidget(QSize(48, 48), dullTranslation(themedir.section("-", -1, -1, QString::SectionSkipEmpty)), showIconsList);
//            widget->setFrameShape(QFrame::Shape::Box);
            widget->setValue(themedir);
            //加入Layout
            ui->iconThemeVerLayout->addWidget(widget);

            //加入WidgetGround实现获取点击前Widget
            iconThemeWidgetGroup->addWidget(widget);

            if (themedir == currentIconTheme){
                iconThemeWidgetGroup->setCurrentWidget(widget);
                widget->setSelectedStatus(true);
            } else {
                widget->setSelectedStatus(false);
            }
        }
    }
}

void Theme::setupControlTheme(){
    QStringList colorStringList;
    colorStringList << QString("#3D6BE5");
    colorStringList << QString("#FA6C63");
    colorStringList << QString("#6cd472");
    colorStringList << QString("#f9a959");
    colorStringList << QString("#BA7Bd8");
    colorStringList << QString("#F8D15D");
    colorStringList << QString("#E7BBB0");
    colorStringList << QString("#176F57");

    QButtonGroup * colorBtnGroup = new QButtonGroup();

    for (QString color : colorStringList){

        QPushButton * button = new QPushButton(ui->controlWidget);
        button->setFixedSize(QSize(48, 48));
        button->setCheckable(true);
        QString btnStyle = QString("QPushButton{background: %1; border-radius: 4px;}").arg(color);
//        button->setStyleSheet(btnStyle);
        colorBtnGroup->addButton(button, colorStringList.indexOf(color));



        QVBoxLayout * colorVerLayout = new QVBoxLayout();
        colorVerLayout->setSpacing(0);
        colorVerLayout->setMargin(0);

        QHBoxLayout * colorHorLayout = new QHBoxLayout();
        colorHorLayout->setSpacing(0);
        colorHorLayout->setMargin(0);

        QLabel * selectedColorLabel = new QLabel(button);
        QSizePolicy scSizePolicy = selectedColorLabel->sizePolicy();
        scSizePolicy.setHorizontalPolicy(QSizePolicy::Fixed);
        scSizePolicy.setVerticalPolicy(QSizePolicy::Fixed);
        selectedColorLabel->setSizePolicy(scSizePolicy);
        selectedColorLabel->setScaledContents(true);
        selectedColorLabel->setPixmap(QPixmap("://img/plugins/theme/selected.png"));
        //初始化选中图标状态
        selectedColorLabel->setVisible(button->isChecked());
//        connect(colorBtnGroup, QOverload<QAbstractButton *>::of(&QButtonGroup::buttonClicked), this,[=]{
//            selectedColorLabel->setVisible(button->isChecked());
//            //设置控件主题
//        });

        colorHorLayout->addStretch();
        colorHorLayout->addWidget(selectedColorLabel);
        colorVerLayout->addLayout(colorHorLayout);
        colorVerLayout->addStretch();

        button->setLayout(colorVerLayout);

        ui->controlHorLayout->addWidget(button);
    }
}

void Theme::initCursorTheme(){

    QStringList cursorThemes = _getSystemCursorThemes();
//    qDebug() << cursorThemes;

    //获取当前指针主题
    QString currentCursorTheme;
    currentCursorTheme = curSettings->get(CURSOR_THEME_KEY).toString();

    WidgetGroup * cursorThemeWidgetGroup = new WidgetGroup;
    connect(cursorThemeWidgetGroup, &WidgetGroup::widgetChanged, [=](ThemeWidget * preWidget, ThemeWidget * curWidget){
        if (preWidget)
            preWidget->setSelectedStatus(false);
        curWidget->setSelectedStatus(true);

        QString value = curWidget->getValue();
        //设置光标主题
        curSettings->set(CURSOR_THEME_KEY, value);
    });

    for (QString cursor : cursorThemes){

        QList<QPixmap> cursorVec;
        QString path = CURSORS_THEMES_PATH + cursor;
        XCursorTheme *cursorTheme = new XCursorTheme(path);

        for(int i = 0; i < numCursors; i++){
            QImage image = cursorTheme->loadImage(cursor_names[i],20);
            cursorVec.append(QPixmap::fromImage(image));
        }

        ThemeWidget * widget  = new ThemeWidget(QSize(24, 24), cursor, cursorVec);
//        widget->setFrameShape(QFrame::Shape::Box);
        widget->setValue(cursor);

        //加入Layout
        ui->cursorVerLayout->addWidget(widget);

        //加入WidgetGround实现获取点击前Widget
        cursorThemeWidgetGroup->addWidget(widget);

        //初始化指针主题选中界面
        if (currentCursorTheme == cursor){
            cursorThemeWidgetGroup->setCurrentWidget(widget);
            widget->setSelectedStatus(true);
        } else {
            widget->setSelectedStatus(false);
        }

    }
}

void Theme::initEffectSettings(){
//    ui->effectLabel->hide();
//    ui->effectWidget->hide();
}

void Theme::initConnection() {

    connect(ui->resetBtn, &QPushButton::clicked, this, &Theme::resetBtnClickSlot);

    connect(effectSwitchBtn, &SwitchButton::checkedChanged, [this](bool checked) {
        QString currentThemeMode = qtSettings->get(MODE_QT_KEY).toString();
        writeKwinSettings(!checked, currentThemeMode);
    });
}

QStringList Theme::_getSystemCursorThemes(){
    QStringList themes;
    QDir themesDir(CURSORS_THEMES_PATH);

    if (themesDir.exists()){
        foreach (QString dirname, themesDir.entryList(QDir::Dirs)){
            if (dirname == "." || dirname == "..")
                continue;
//            QString fullpath(CURSORS_THEMES_PATH + dirname);
            QDir themeDir(CURSORS_THEMES_PATH + dirname + "/cursors/");
            if (themeDir.exists())
                themes.append(dirname);
        }
    }
    return themes;
}

QString Theme::dullTranslation(QString str){
    if (!QString::compare(str, "basic")){
        return QObject::tr("basic");
    } else if (!QString::compare(str, "classical")){
        return QObject::tr("classical");
    } else if (!QString::compare(str, "default")){
        return QObject::tr("default");
    } else
        return QObject::tr("Unknown");
}

// reset all of themes, include cursor, icon,and etc...
void Theme::resetBtnClickSlot() {

    // reset theme(because MODE_QT_KEY's default is null, use "SET" to reset default key )
    QString theme = "ukui-white";
    qtSettings->set(MODE_QT_KEY, theme);
    gtkSettings->set(MODE_GTK_KEY, theme);

    // reset cursor default theme    
    QString cursorTheme = "breeze_cursors";
    curSettings->set(CURSOR_THEME_KEY,cursorTheme);


    //reset icon default theme
    qtSettings->reset(ICON_QT_KEY);
    gtkSettings->reset(ICON_GTK_KEY);



    clearLayout(ui->iconThemeVerLayout->layout(), true);
    clearLayout(ui->cursorVerLayout->layout(), true);

    initThemeMode();
    initIconTheme();
    initCursorTheme();
}

void Theme::writeKwinSettings(bool change, QString theme) {
    QString th;
    if ("ukui-white" == theme) {
        th = "__aurorate__svg__Ukui-classic";
    } else {
        th = "__aurorate__svg__Ukui-classic_dark";
    }
    kwinSettings->beginGroup("Plugins");
    kwinSettings->setValue("blurEnabled",change);
    kwinSettings->endGroup();

    kwinSettings->beginGroup("org.kde.kdecoration2");
    kwinSettings->setValue("theme", th);
    kwinSettings->endGroup();

    kwinSettings->sync();

#if QT_VERSION <= QT_VERSION_CHECK(5,12,0)

#else
    QDBusMessage message = QDBusMessage::createSignal("/KWin", "org.ukui.KWin", "reloadConfig");
    QDBusConnection::sessionBus().send(message);
#endif
}

void Theme::clearLayout(QLayout* mlayout, bool deleteWidgets)
{
    if ( mlayout->layout() != NULL )
    {
        QLayoutItem* item;
        while ( ( item = mlayout->layout()->takeAt( 0 ) ) != NULL )
        {
            delete item->widget();
            delete item;
        }
    }
}
