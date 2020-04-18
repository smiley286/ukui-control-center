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
#ifndef PASSDIALOG_H
#define PASSDIALOG_H

#include <QObject>
#include <QWidget>
#include "area_code_lineedit.h"
#include <QLineEdit>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>

class PassDialog : public QWidget
{
    Q_OBJECT
public:
    explicit PassDialog(QWidget *parent = nullptr);
    QPushButton* get_send_msg_btn();
    QString     get_user_name();
    QString     get_user_newpass();
    QString     get_user_confirm();
    QString     get_user_mcode();
    QLineEdit*  get_reg_phone();
    QLineEdit*  get_reg_pass();
    QLineEdit*  get_reg_pass_confirm();
    QLineEdit*  get_valid_code();
    void        set_code(QString codenum);
    void        set_clear();
    QLabel*     get_tips();
public slots:
    void change_uppercase();
    void setstyleline();

signals:
    void code_changed();
private:
    QLineEdit *reg_phone;
    QLineEdit *reg_pass;
    QLineEdit *reg_pass_confirm;
    QLineEdit *valid_code;
    QPushButton *send_msg_btn;
    QVBoxLayout *vlayout;
    QHBoxLayout *hlayout;
    QLabel *tips;
    QString     code;
};

#endif // PASSDIALOG_H
