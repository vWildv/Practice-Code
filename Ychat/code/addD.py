# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'ui/addD.ui'
#
# Created by: PyQt5 UI code generator 5.13.0
#
# WARNING! All changes made in this file will be lost!


from PyQt5 import QtCore, QtGui, QtWidgets
import chatW
import sql
import queue

class Ui_Dialog(object):
    def setupUi(self, Dialog, name):
        self.seuid=name
        Dialog.setObjectName("Dialog")
        Dialog.resize(312, 179)
        self.lineEdit = QtWidgets.QLineEdit(Dialog)
        self.lineEdit.setGeometry(QtCore.QRect(20, 60, 271, 51))
        self.lineEdit.setObjectName("lineEdit")
        self.label = QtWidgets.QLabel(Dialog)
        self.label.setGeometry(QtCore.QRect(100, 30, 111, 31))
        self.label.setObjectName("label")
        self.pushButton = QtWidgets.QPushButton(Dialog)
        self.pushButton.setGeometry(QtCore.QRect(110, 130, 81, 31))
        self.pushButton.setObjectName("pushButton")

        self.retranslateUi(Dialog)
        QtCore.QMetaObject.connectSlotsByName(Dialog)
        self.pushButton.clicked.connect(self.__open)

    def __open(self):
        user=self.lineEdit.text()
        mySql = sql.getMySql().connect()
        cur = mySql.cursor()
        try:
            cur.execute("select uid,port from user where uid='%s'"%(user))
            result=cur.fetchone()
            cur.execute("select uid1,uid2 from friend where uid1='%s' and uid2='%s'"%(user,self.seuid))
            try:
                tmp=cur.fetchone()[1]
            except:
                cur.execute("select uid1,uid2 from friend where uid1='%s' and uid2='%s'"%(self.seuid,user))
                try:
                    tmp=cur.fetchone()[1]
                except: cur.execute("insert into friend values('%s','%s')"%(result[0],self.seuid))
            mySql.commit()
            dialog = QtWidgets.QDialog()
            ui = chatW.Ui_chatW()
            name=self.seuid+' to '+user
            ui.setupUi(dialog,name,queue.Queue(30))
            ui.connect(user,self.seuid)
            dialog.show()
            dialog.exec_()
        except:
            QtWidgets.QMessageBox.information(None, "提示:", "用户不存在;")
        cur.close()
        mySql.close()



    def retranslateUi(self, Dialog):
        _translate = QtCore.QCoreApplication.translate
        Dialog.setWindowTitle(_translate("Dialog", "Dialog"))
        self.label.setText(_translate("Dialog", "<html><head/><body><p align=\"center\"><span style=\" font-size:11pt;\">输入对方用户名</span></p></body></html>"))
        self.pushButton.setText(_translate("Dialog", "确定"))
