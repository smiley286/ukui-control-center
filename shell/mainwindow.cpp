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
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QLabel>
#include <QPushButton>
#include <QButtonGroup>
#include <QHBoxLayout>
#include <QPluginLoader>
#include <QPainter>
#include <QProcess>
#include <libmatemixer/matemixer.h>
#include "utils/keyvalueconverter.h"
#include "utils/functionselect.h"

//#include <KWindowSystem>

#include <QDebug>
#include <QMessageBox>

/* qt会将glib里的signals成员识别为宏，所以取消该宏
 * 后面如果用到signals时，使用Q_SIGNALS代替即可
 **/
#ifdef signals
#undef signals
#endif

extern "C" {
#include <glib.h>
#include <gio/gio.h>
}

extern void qt_blurImage(QImage &blurImage, qreal radius, bool quality, int transposed);

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //初始化mixer
    mate_mixer_init();
    //设置初始大小
    resize(QSize(820, 600));
    //设置窗体无边框
    setWindowFlags(Qt::FramelessWindowHint | Qt::Widget);

    //该设置去掉了窗体透明后的黑色背景
    setAttribute(Qt::WA_TranslucentBackground, true);
    //将最外层窗体设置为透明

//    setStyleSheet("QMainWindow#MainWindow{background-color: transparent;}");


    //设置panel图标
    QIcon panelicon;
    if (QIcon::hasThemeIcon("ukui-control-center"))
        panelicon = QIcon::fromTheme("ukui-control-center");
//    else
//        panelicon = QIcon("://applications-system.svg");
    this->setWindowIcon(panelicon);
    this->setWindowTitle(tr("ukcc"));

    ui->searchLineEdit->setVisible(false);
    //中部内容区域
    ui->stackedWidget->setStyleSheet("QStackedWidget#stackedWidget{background: palette(base); border-bottom-left-radius: 6px; border-bottom-right-radius: 6px;}");
    //标题栏widget
    ui->titlebarWidget->setStyleSheet("QWidget#titlebarWidget{background: palette(base); border-top-left-radius: 6px; border-top-right-radius: 6px;}");
////    //左上角文字
////    ui->mainLabel->setStyleSheet("QLabel#mainLabel{font-size: 18px; color: #40000000;}");

    //左上角返回按钮
    ui->backBtn->setProperty("useIconHighlightEffect", true);
    ui->backBtn->setProperty("iconHighlightEffectMode", 1);
    ui->backBtn->setFlat(true);

//    ui->backBtn->setStyleSheet("QPushButton#backBtn{background: #ffffff; border: none;}");
//    //顶部搜索框
//    ui->searchLineEdit->setStyleSheet("QLineEdit#searchLineEdit{background: #FFEDEDED; border: none; border-radius: 6px;}");
    //右上角按钮stylesheet
    ui->minBtn->setProperty("useIconHighlightEffect", true);
    ui->minBtn->setProperty("iconHighlightEffectMode", 1);
    ui->minBtn->setFlat(true);
    ui->maxBtn->setProperty("useIconHighlightEffect", true);
    ui->maxBtn->setProperty("iconHighlightEffectMode", 1);
    ui->maxBtn->setFlat(true);
    ui->closeBtn->setProperty("useIconHighlightEffect", true);
    ui->closeBtn->setProperty("iconHighlightEffectMode", 1);
    ui->closeBtn->setFlat(true);

//    ui->minBtn->setStyleSheet("QPushButton#minBtn{background: #ffffff; border: none;}"
//                              "QPushButton:hover:!pressed#minBtn{background: #FF3D6BE5; border-radius: 2px;}"
//                              "QPushButton:hover:pressed#minBtn{background: #415FC4; border-radius: 2px;}");
//    ui->maxBtn->setStyleSheet("QPushButton#maxBtn{background: #ffffff; border: none;}"
//                              "QPushButton:hover:!pressed#maxBtn{background: #FF3D6BE5; border-radius: 2px;}"
//                              "QPushButton:hover:pressed#maxBtn{background: #415FC4; border-radius: 2px;}");
    ui->closeBtn->setStyleSheet("QPushButton:hover:!pressed#closeBtn{background: #FA6056; border-radius: 4px;}"
                                "QPushButton:hover:pressed#closeBtn{background: #E54A50; border-radius: 4px;}");

    //左侧一级菜单
//    ui->leftsidebarWidget->setStyleSheet("QWidget#leftsidebarWidget{background: #cccccc; border: none; border-top-left-radius: 6px; border-bottom-left-radius: 6px;}");
    ui->leftsidebarWidget->setStyleSheet("QWidget#leftsidebarWidget{background-color: palette(button);border: none; border-top-left-radius: 6px; border-bottom-left-radius: 6px;}");

    //设置左上角按钮图标
    ui->backBtn->setIcon(QIcon("://img/titlebar/back.png"));

    //设置右上角按钮图标
    ui->minBtn->setIcon(QIcon(":/img/titlebar/min.png"));
    ui->maxBtn->setIcon(QIcon("://img/titlebar/max.png"));
    ui->closeBtn->setIcon(QIcon("://img/titlebar/close.png"));

    //

    //初始化功能列表数据
    FunctionSelect::initValue();

    //构建枚举键值转换对象
    kvConverter = new KeyValueConverter(); //继承QObject，No Delete

    //加载插件
    loadPlugins();

    connect(ui->minBtn, SIGNAL(clicked()), this, SLOT(showMinimized()));
//    connect(ui->minBtn, &QPushButton::clicked, [=]{
//        KWindowSystem::minimizeWindow(this->winId());
//    });
    connect(ui->maxBtn, &QPushButton::clicked, this, [=]{
        if (isMaximized()){
            showNormal();
            ui->maxBtn->setIcon(QIcon("://img/titlebar/max.png"));
        } else {
            showMaximized();
            ui->maxBtn->setIcon(QIcon("://img/titlebar/revert.png"));
        }
    });
    connect(ui->closeBtn, &QPushButton::clicked, this, [=]{
        close();
//        qApp->quit();
    });
//    connect(ui->backBtn, &QPushButton::clicked, this, [=]{
//        if (ui->stackedWidget->currentIndex())
//            ui->stackedWidget->setCurrentIndex(0);
//        else
//            ui->stackedWidget->setCurrentIndex(1);
//    });


//    ui->leftsidebarWidget->setVisible(ui->stackedWidget->currentIndex());
    connect(ui->stackedWidget, &QStackedWidget::currentChanged, this, [=](int index){
        //左侧边栏显示/不显示
        ui->leftsidebarWidget->setVisible(index);
        //左上角显示字符/返回按钮
        ui->backBtn->setVisible(index);
        ui->titleLabel->setHidden(index);

        if (index){ //首页部分组件样式
            //中部内容区域
            ui->stackedWidget->setStyleSheet("QStackedWidget#stackedWidget{background: palette(base); border-bottom-right-radius: 6px;}");
            //标题栏widget
            ui->titlebarWidget->setStyleSheet("QWidget#titlebarWidget{background:  palette(base); border-top-right-radius: 6px;}");
        } else { //次页部分组件样式
            //中部内容区域
            ui->stackedWidget->setStyleSheet("QStackedWidget#stackedWidget{background:  palette(base); border-bottom-left-radius: 6px; border-bottom-right-radius: 6px;}");
            //标题栏widget
            ui->titlebarWidget->setStyleSheet("QWidget#titlebarWidget{background:  palette(base); border-top-left-radius: 6px; border-top-right-radius: 6px;}");
        }
    });

    //加载左侧边栏一级菜单
    initLeftsideBar();

    //加载首页Widget
    homepageWidget = new HomePageWidget(this);
    ui->stackedWidget->addWidget(homepageWidget);

    //加载功能页Widget
    modulepageWidget = new ModulePageWidget(this);
    ui->stackedWidget->addWidget(modulepageWidget);

    //top left return button
    connect(ui->backBtn, &QPushButton::clicked, this, [=]{
        FunctionSelect::popRecordValue();

        //if recordFuncStack is empty, it means there is no history record. So return to homepage
        if (FunctionSelect::recordFuncStack.length() < 1) {
            ui->stackedWidget->setCurrentIndex(0);
        } else {
            QMap<QString, QObject *> pluginsObjMap = modulesList.at(FunctionSelect::recordFuncStack.last().type);
            modulepageWidget->switchPage(pluginsObjMap.value(FunctionSelect::recordFuncStack.last().namei18nString), false);
        }

    });

    //快捷参数
    if (QApplication::arguments().length() > 1){

        bootOptionsFilter(QApplication::arguments().at(1));
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::bootOptionsFilter(QString opt){
    if (opt == "-m"){
        //显示器
        bootOptionsSwitch(SYSTEM, DISPLAY);

    } else if (opt == "-b"){
        //背景
        bootOptionsSwitch(PERSONALIZED, BACKGROUND);

    } else if (opt == "-d"){
        //桌面
        bootOptionsSwitch(PERSONALIZED, DESKTOP);

    } else if (opt == "-u"){
        //账户
        bootOptionsSwitch(ACCOUNT, USERINFO);

    } else if (opt == "-a"){
        //关于
        bootOptionsSwitch(NOTICEANDTASKS, ABOUT);

    } else if (opt == "-p"){
        //电源
        bootOptionsSwitch(SYSTEM, POWER);
    } else if (opt == "-t") {
        // Datetime moudle
        bootOptionsSwitch(DATETIME, DAT);
    } else if (opt == "-s") {
        // Audio module
        bootOptionsSwitch(DEVICES, AUDIO);
    } else if (opt == "-n") {
        // notice module
        bootOptionsSwitch(NOTICEANDTASKS, NOTICE);
    }
}

void MainWindow::bootOptionsSwitch(int moduleNum, int funcNum){

    QList<FuncInfo> pFuncStructList = FunctionSelect::funcinfoList[moduleNum];
    QString funcStr = pFuncStructList.at(funcNum).namei18nString;

    QMap<QString, QObject *> pluginsObjMap = modulesList.at(moduleNum);

    if (pluginsObjMap.keys().contains(funcStr)){
        //开始跳转
        ui->stackedWidget->setCurrentIndex(1);
        modulepageWidget->switchPage(pluginsObjMap.value(funcStr));
    }
}

void MainWindow::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    QPainterPath rectPath;
    rectPath.addRoundedRect(this->rect().adjusted(1, 1, -1, -1), 6, 6);

    // 画一个黑底
    QPixmap pixmap(this->rect().size());
    pixmap.fill(Qt::transparent);
    QPainter pixmapPainter(&pixmap);
    pixmapPainter.setRenderHint(QPainter::Antialiasing);
    pixmapPainter.setPen(Qt::transparent);
    pixmapPainter.setBrush(Qt::black);
    pixmapPainter.drawPath(rectPath);
    pixmapPainter.end();

    // 模糊这个黑底
    QImage img = pixmap.toImage();
    qt_blurImage(img, 5, false, false);

    // 挖掉中心
    pixmap = QPixmap::fromImage(img);
    QPainter pixmapPainter2(&pixmap);
    pixmapPainter2.setRenderHint(QPainter::Antialiasing);
    pixmapPainter2.setCompositionMode(QPainter::CompositionMode_Clear);
    pixmapPainter2.setPen(Qt::transparent);
    pixmapPainter2.setBrush(Qt::transparent);
    pixmapPainter2.drawPath(rectPath);

    // 绘制阴影
    p.drawPixmap(this->rect(), pixmap, pixmap.rect());

    // 绘制一个背景
    p.save();
    p.fillPath(rectPath,palette().color(QPalette::Base));
//    p.fillPath(rectPath,QColor(0,0,0));
    p.restore();
}

void MainWindow::setBtnLayout(QPushButton * &pBtn){
    QLabel * imgLabel = new QLabel(pBtn);
    QSizePolicy imgLabelPolicy = imgLabel->sizePolicy();
    imgLabelPolicy.setHorizontalPolicy(QSizePolicy::Fixed);
    imgLabelPolicy.setVerticalPolicy(QSizePolicy::Fixed);
    imgLabel->setSizePolicy(imgLabelPolicy);
    imgLabel->setScaledContents(true);

    QVBoxLayout * baseVerLayout = new QVBoxLayout(pBtn);

    QHBoxLayout * contentHorLayout = new QHBoxLayout();
    contentHorLayout->addStretch();
    contentHorLayout->addWidget(imgLabel);
    contentHorLayout->addStretch();

    baseVerLayout->addStretch();
    baseVerLayout->addLayout(contentHorLayout);
    baseVerLayout->addStretch();

    pBtn->setLayout(baseVerLayout);
}

void MainWindow::loadPlugins(){
    for (int index = 0; index < TOTALMODULES; index++){
        QMap<QString, QObject *> pluginsMaps;
        modulesList.append(pluginsMaps);
    }

    static bool installed = (QCoreApplication::applicationDirPath() == QDir(("/usr/bin")).canonicalPath());

    if (installed)
        pluginsDir = QDir("/usr/lib/control-center/pluginlibs/");
    else {
        pluginsDir = QDir(qApp->applicationDirPath() + "/pluginlibs/");
    }

    bool isExistCloud  = isExitsCloudAccount();    
    foreach (QString fileName, pluginsDir.entryList(QDir::Files)){
//        if (fileName == "libdesktop.so")
//            continue;
//        if (fileName == "libnotice.so")
//            continue;
        if (fileName == "libexperienceplan.so")
            continue;
        if ("libnetworkaccount.so" == fileName && !isExistCloud) {
            continue;
        }

        qDebug() << "Scan Plugin: " << fileName;
        //gsettings-desktop-schemas
//        const char * proxyFile = "/usr/share/glib-2.0/schemas/org.gnome.system.proxy.gschema.xml";
//        const char * gnomedesktopFile = "/usr/share/glib-2.0/schemas/org.gnome.desktop.wm.preferences.gschema.xml";
        //mate-desktop-common
//        const char * interfaceFile = "/usr/share/glib-2.0/schemas/org.mate.interface.gschema.xml";
//        const char * bgFile = "/usr/share/glib-2.0/schemas/org.mate.background.gschema.xml";
        //peony-common
//        const char * peonyFile = "/usr/share/glib-2.0/schemas/org.ukui.peony.gschema.xml";
        //libmatekbd-common
//        const char * kbdFile = "/usr/share/glib-2.0/schemas/org.mate.peripherals-keyboard-xkb.gschema.xml";
        //ukui-power-manager-common
//        const char * powerFile = "/usr/share/glib-2.0/schemas/org.ukui.power-manager.gschema.xml";
        //ukui-session-manager
        const char * sessionFile = "/usr/share/glib-2.0/schemas/org.ukui.session.gschema.xml";
        //ukui-screensaver
        const char * screensaverFile = "/usr/share/glib-2.0/schemas/org.ukui.screensaver.gschema.xml";
        //ukui-settings-daemon-common
//        const char * usdFile = "/usr/share/glib-2.0/schemas/org.ukui.font-rendering.gschema.xml";

        //代理功能依赖gsettings-desktop-schemas
//        if (!g_file_test(proxyFile, G_FILE_TEST_EXISTS) && fileName == "libproxy.so")
//            continue;
        //字体功能依赖gsettings-desktop-schemas,mate-desktop-common,peony-common,ukui-settings-daemon-common
//        if ((!g_file_test(interfaceFile, G_FILE_TEST_EXISTS) ||
//                !g_file_test(gnomedesktopFile, G_FILE_TEST_EXISTS) ||
//                !g_file_test(peonyFile, G_FILE_TEST_EXISTS) ||
//                !g_file_test(usdFile, G_FILE_TEST_EXISTS)) && fileName == "libfonts.so")
//            continue;
        //键盘功能的键盘布局依赖libmatekbd-common
//        if (!g_file_test(kbdFile, G_FILE_TEST_EXISTS) && fileName == "libkeyboard.so")
//            continue;
        //电源功能依赖ukui-power-manager-common
//        if (!g_file_test(powerFile, G_FILE_TEST_EXISTS) && fileName == "libpower.so")
//            continue;
        //屏保功能依赖ukui-session-manager
        if ((!g_file_test(screensaverFile, G_FILE_TEST_EXISTS) ||
                !g_file_test(sessionFile, G_FILE_TEST_EXISTS)) &&
                (fileName == "libscreensaver.so" || fileName == "libscreenlock.so"))
            continue;
//        //桌面功能依赖peony-common
//        if (!g_file_test(peonyFile, G_FILE_TEST_EXISTS) && fileName == "libdesktop.so")
//            continue;
        //wallpaper mate-desktop-common
//        if (!g_file_test(bgFile, G_FILE_TEST_EXISTS) && fileName == "libwallpaper.so")
//            continue;
        //主题功能依赖gsettings-desktop-schemas,mate-desktop-common
//        if ((!g_file_test(interfaceFile, G_FILE_TEST_EXISTS) ||
//                !g_file_test(gnomedesktopFile, G_FILE_TEST_EXISTS)) && fileName == "libtheme.so")
//            continue;

        QPluginLoader loader(pluginsDir.absoluteFilePath(fileName));
        QObject * plugin = loader.instance();
        if (plugin){
            CommonInterface * pluginInstance = qobject_cast<CommonInterface *>(plugin);
            modulesList[pluginInstance->get_plugin_type()].insert(pluginInstance->get_plugin_name(), plugin);

            qDebug() << "Load Plugin :" << kvConverter->keycodeTokeyi18nstring(pluginInstance->get_plugin_type()) << "->" << pluginInstance->get_plugin_name() ;

            int moduletypeInt = pluginInstance->get_plugin_type();
            if (!moduleIndexList.contains(moduletypeInt))
                moduleIndexList.append(moduletypeInt);
        }
        else {
            //如果加载错误且文件后缀为so，输出错误
            if (fileName.endsWith(".so"))
                qDebug() << fileName << "Load Failed: " << loader.errorString() << "\n";
        }
    }
}

void MainWindow::initLeftsideBar(){

    leftBtnGroup = new QButtonGroup();
    leftMicBtnGroup = new QButtonGroup();

    //构建左侧边栏返回首页按钮
    QPushButton * hBtn = buildLeftsideBtn("homepage");
    hBtn->setObjectName("homepage");
    connect(hBtn, &QPushButton::clicked, this, [=]{
        ui->stackedWidget->setCurrentIndex(0);
    });
    hBtn->setStyleSheet("QPushButton#homepage{background: palette(button); border: none;}");
//    hBtn->setStyleSheet("QPushButton#homepage{background: palette(base);}");

    ui->leftsidebarVerLayout->addWidget(hBtn);

    for(int type = 0; type < TOTALMODULES; type++){
        //循环构建左侧边栏一级菜单按钮
        if (moduleIndexList.contains(type)){
            QString mnameString = kvConverter->keycodeTokeystring(type);
            QString mnamei18nString  = kvConverter->keycodeTokeyi18nstring(type); //设置TEXT
            QPushButton * button = buildLeftsideBtn(mnameString);
            button->setCheckable(true);
            leftBtnGroup->addButton(button, type);
            //设置样式
//            button->setStyleSheet("QPushButton::checked{background: palette(button); border: none; border-image: url('://img/primaryleftmenu/checked.png');}"
//                                  "QPushButton::!checked{background: palette(button);border: none;}");
            button->setStyleSheet("QPushButton::checked{background: palette(base); border-top-left-radius: 6px;border-bottom-left-radius: 6px;}"
                                  "QPushButton::!checked{background: palette(button);border: none;}");

            connect(button, &QPushButton::clicked, this, [=]{
                QPushButton * btn = dynamic_cast<QPushButton *>(QObject::sender());

                int selectedInt = leftBtnGroup->id(btn);

                //获取一级菜单列表的第一项
                QList<FuncInfo> tmpList = FunctionSelect::funcinfoList[selectedInt];
                QMap<QString, QObject *> currentFuncMap = modulesList[selectedInt];

                for (FuncInfo tmpStruct : tmpList){
                    if (currentFuncMap.keys().contains(tmpStruct.namei18nString)){
                        modulepageWidget->switchPage(currentFuncMap.value(tmpStruct.namei18nString));
                        break;
                    }
                }
            });

            ui->leftsidebarVerLayout->addWidget(button);
        }
    }

    ui->leftsidebarVerLayout->addStretch();
}

QPushButton * MainWindow::buildLeftsideBtn(QString bname){
    QString iname = bname.toLower();
    int itype = kvConverter->keystringTokeycode(bname);

    QPushButton * leftsidebarBtn = new QPushButton();
    leftsidebarBtn->setAttribute(Qt::WA_DeleteOnClose);
    leftsidebarBtn->setCheckable(true);
    leftsidebarBtn->setFixedSize(QSize(60, 56)); //Widget Width 60

    QPushButton * iconBtn = new QPushButton(leftsidebarBtn);
    iconBtn->setCheckable(true);
    iconBtn->setFixedSize(QSize(24, 24));
    iconBtn->setFocusPolicy(Qt::NoFocus);


    QString iconHomePageBtnQss = QString("QPushButton{background: palette(button); border: none; border-image: url('://img/primaryleftmenu/%1.png');}").arg(iname);
    QString iconBtnQss = QString("QPushButton:checked{background: palette(base); border: none; border-image: url('://img/primaryleftmenu/%1Checked.png');}"
                                 "QPushButton:!checked{background: palette(button); border: none; border-image: url('://img/primaryleftmenu/%2.png');}").arg(iname).arg(iname);
    //单独设置HomePage按钮样式
    if (iname == "homepage")
        iconBtn->setStyleSheet(iconHomePageBtnQss);
    else
        iconBtn->setStyleSheet(iconBtnQss);

    leftMicBtnGroup->addButton(iconBtn, itype);

    connect(iconBtn, &QPushButton::clicked, leftsidebarBtn, &QPushButton::click);

    connect(leftsidebarBtn, &QPushButton::clicked, this, [=](bool checked){
        iconBtn->setChecked(checked);
    });

    QLabel * textLabel = new QLabel(leftsidebarBtn);
    QSizePolicy textLabelPolicy = textLabel->sizePolicy();
    textLabelPolicy.setHorizontalPolicy(QSizePolicy::Fixed);
    textLabelPolicy.setVerticalPolicy(QSizePolicy::Fixed);
    textLabel->setSizePolicy(textLabelPolicy);
    textLabel->setScaledContents(true);

    QHBoxLayout * btnHorLayout = new QHBoxLayout();
    btnHorLayout->addWidget(iconBtn, Qt::AlignCenter);
    btnHorLayout->addWidget(textLabel);
    btnHorLayout->addStretch();
    btnHorLayout->setSpacing(10);

    leftsidebarBtn->setLayout(btnHorLayout);

    return leftsidebarBtn;
}

bool MainWindow::isExitsCloudAccount() {
    QProcess *wifiPro = new QProcess();
    QString shellOutput = "";
    wifiPro->start("dpkg -l  | grep kylin-sso-client");
    wifiPro->waitForFinished();
    QString output = wifiPro->readAll();
    shellOutput += output;
    QStringList slist = shellOutput.split("\n");

    for (QString res : slist) {
        if (res.contains("kylin-sso-client")) {
            return true;
        }
    }
    return false;
}

void MainWindow::setModuleBtnHightLight(int id){
    leftBtnGroup->button(id)->setChecked(true);
    leftMicBtnGroup->button(id)->setChecked(true);
}

QMap<QString, QObject *> MainWindow::exportModule(int type){
    QMap<QString, QObject *> emptyMaps;
    if (type < modulesList.length())
        return modulesList[type];
    else
        return emptyMaps;
}

void MainWindow::functionBtnClicked(QObject *plugin){
    ui->stackedWidget->setCurrentIndex(1);
    modulepageWidget->switchPage(plugin);
}

void MainWindow::sltMessageReceived(const QString &msg) {

    bootOptionsFilter(msg);

    Qt::WindowFlags flags = windowFlags();
    flags |= Qt::WindowStaysOnTopHint;
    setWindowFlags(flags);
    show();
    flags &= ~Qt::WindowStaysOnTopHint;
    setWindowFlags(flags);
    showNormal();
}

