#ifndef NETWORK_ACCOUNT_H
#define NETWORK_ACCOUNT_H

#include <QObject>
#include <QtPlugin>

#include "config_list_widget.h"
#include "dialog_login_reg.h"
#include <libkylinssoclient/libkylinssoclient.h>

#include "shell/interface.h"

class network_account : public QObject, CommonInterface
{
    Q_OBJECT
public:
    explicit network_account(QObject *parent = nullptr);

    QString get_plugin_name() Q_DECL_OVERRIDE;
    int get_plugin_type() Q_DECL_OVERRIDE;
    QWidget * get_plugin_ui() Q_DECL_OVERRIDE;
    void plugin_delay_control() Q_DECL_OVERRIDE;


//signals:
private:
    config_list_widget *widget;
    Dialog_login_reg *dialog;
    libkylinssoclient *client;

private:
    QString pluginName;
    int pluginType;
    QWidget * pluginWidget;
};

#endif // NETWORK_ACCOUNT_H
