# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'ui/loginW.ui'
#
# Created by: PyQt5 UI code generator 5.13.0
#
# WARNING! All changes made in this file will be lost!


from PyQt5 import QtCore, QtGui, QtWidgets
import sql
import listW
import sys
import time
from PyQt5.QtWidgets import QApplication, QMainWindow,QLineEdit


class Ui_LoginW(object):
    def setupUi(self, LoginW):
        LoginW.setObjectName("LoginW")
        LoginW.resize(323, 235)
        LoginW.setMouseTracking(False)
        self.centralwidget = QtWidgets.QWidget(LoginW)
        self.centralwidget.setObjectName("centralwidget")
        self.loginBu = QtWidgets.QPushButton(self.centralwidget)
        self.loginBu.setGeometry(QtCore.QRect(70, 180, 81, 31))
        self.loginBu.setObjectName("loginBu")
        self.label_1 = QtWidgets.QLabel(self.centralwidget)
        self.label_1.setGeometry(QtCore.QRect(23, 52, 31, 41))
        self.label_1.setObjectName("label_1")
        self.label_2 = QtWidgets.QLabel(self.centralwidget)
        self.label_2.setGeometry(QtCore.QRect(20, 119, 31, 41))
        self.label_2.setObjectName("label_2")
        self.line_user = QtWidgets.QLineEdit(self.centralwidget)
        self.line_user.setGeometry(QtCore.QRect(60, 50, 221, 41))
        self.line_user.setObjectName("line_user")
        self.line_passwd = QtWidgets.QLineEdit(self.centralwidget)
        self.line_passwd.setGeometry(QtCore.QRect(60, 120, 221, 41))
        self.line_passwd.setObjectName("line_passwd")
        self.line_passwd.setEchoMode(QLineEdit.Password)
        self.signBu = QtWidgets.QPushButton(self.centralwidget)
        self.signBu.setGeometry(QtCore.QRect(170, 180, 81, 31))
        self.signBu.setObjectName("signBu")
        self.logintitle = QtWidgets.QLabel(self.centralwidget)
        self.logintitle.setGeometry(QtCore.QRect(73, 12, 181, 20))
        self.logintitle.setObjectName("logintitle")
        LoginW.setCentralWidget(self.centralwidget)
        self.statusbar = QtWidgets.QStatusBar(LoginW)
        self.statusbar.setObjectName("statusbar")
        LoginW.setStatusBar(self.statusbar)

        self.retranslateUi(LoginW)
        QtCore.QMetaObject.connectSlotsByName(LoginW)

        self.loginBu.clicked.connect(self.__login)
        self.signBu.clicked.connect(self.__sign)


    def __login(self):
        try:
            user = self.line_user.text()
            passwd = self.line_passwd.text()
            mySql = sql.getMySql().connect()
            if mySql:
                cur = mySql.cursor()
                try:
                    cur.execute("select uid,upasswd from user where uid='%s'"%(user))
                    result = cur.fetchone()
                    if passwd==result[1]:
                        dialog = QtWidgets.QDialog()
                        ui = listW.Ui_listW()
                        ui.setupUi(dialog)
                        ui.init(user)
                        dialog.show()
                        dialog.exec_() 
                        fw=open("c:/Work/python/Ychat/logs/login.log",'a+')
                        fw.write(time.strftime('%Y-%m-%d %H:%M:%S',time.localtime(time.time()))+\
                            " "+user+" login\n")
                    else:
                        QtWidgets.QMessageBox.information(None, "提示:", "操作失败，请检查密码输入是否正确;")
                    mySql.commit()
                except:
                    QtWidgets.QMessageBox.information(None, "提示:", "操作失败，请检查账户输入是否正确;")
                cur.close()
                mySql.close()
        except:
            QtWidgets.QMessageBox.information(None, "提示:", "操作失败;")

    def __sign(self):
        try:
            user = self.line_user.text()
            passwd = self.line_passwd.text()
            mySql = sql.getMySql().connect()
            if mySql:
                cur = mySql.cursor()
                cur.execute("select count(*) from user")
                cnt=cur.fetchone()[0]
                port=1213+cnt*3
                print(port,cnt)
                try:
                    cur.execute("insert into user values('%s','%s',%d)"%(user,passwd,port))
                    mySql.commit()
                    QtWidgets.QMessageBox.information(None, "提示:", "注册成功,请登录使用;")
                except:
                    QtWidgets.QMessageBox.information(None, "提示:", "操作失败,账号可能已经存在")
                cur.close()
                mySql.close()
        except:
            QtWidgets.QMessageBox.information(None, "提示:", "操作失败;")


    def retranslateUi(self, LoginW):
        _translate = QtCore.QCoreApplication.translate
        LoginW.setWindowTitle(_translate("LoginW", "MainWindow"))
        self.loginBu.setText(_translate("LoginW", "登录"))
        self.label_1.setText(_translate("LoginW", "<html><head/><body><p><span style=\" font-size:10pt;\">账号</span></p></body></html>"))
        self.label_2.setText(_translate("LoginW", "<html><head/><body><p align=\"center\"><span style=\" font-size:10pt;\">密码</span></p></body></html>"))
        self.signBu.setText(_translate("LoginW", "注册"))
        self.logintitle.setText(_translate("LoginW", "<html><head/><body><p align=\"center\"><span style=\" font-size:16pt;\">Ychat</span></p></body></html>"))
