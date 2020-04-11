#include "network_account.h"

network_account::network_account(QObject *parent) : QObject(parent)
{
    pluginWidget = new config_list_widget();
    pluginName = tr("CloudAccount");
    pluginType = ACCOUNT;
}

QString network_account::get_plugin_name(){
    return pluginName;
}

int network_account::get_plugin_type(){
    return pluginType;
}

QWidget *network_account::get_plugin_ui(){
    return pluginWidget;
}

void network_account::plugin_delay_control(){

}
