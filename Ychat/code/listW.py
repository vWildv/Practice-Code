# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'ui/listW.ui'
#
# Created by: PyQt5 UI code generator 5.13.0
#
# WARNING! All changes made in this file will be lost!


from PyQt5 import QtCore, QtGui, QtWidgets
import sql
import chatW
import addD
import socket
import threading
import queue


class Ui_listW(object):
    friends=[]
    curuser=""
    recv_q=recv_q=queue.Queue(30)
    def setupUi(self, listW):
        listW.setObjectName("listW")
        listW.resize(292, 590)
        self.centralwidget = QtWidgets.QWidget(listW)
        self.centralwidget.setObjectName("centralwidget")
        self.listView = QtWidgets.QListView(self.centralwidget)
        self.listView.setGeometry(QtCore.QRect(10, 10, 271, 491))
        self.listView.setObjectName("listView")
        self.commandLinkButton = QtWidgets.QCommandLinkButton(self.centralwidget)
        self.commandLinkButton.setGeometry(QtCore.QRect(60, 520, 171, 41))
        self.commandLinkButton.setObjectName("commandLinkButton")
        #listW.setCentralWidget(self.centralwidget)
        self.statusbar = QtWidgets.QStatusBar(listW)
        self.statusbar.setObjectName("statusbar")
        #listW.setStatusBar(self.statusbar)

        self.retranslateUi(listW)
        QtCore.QMetaObject.connectSlotsByName(listW)

        self.listView.doubleClicked.connect(self.__chat)
        self.commandLinkButton.clicked.connect(self.__addchat)
    
    def __recv_thread(self,seuid):
        s=socket.socket(socket.AF_INET,socket.SOCK_DGRAM)
        mySql = sql.getMySql().connect()
        cur = mySql.cursor()
        cur.execute("select uid,port from user where uid='%s'"%(seuid))
        result=cur.fetchone()
        cur.close()
        mySql.close()
        #self.seport=result[1]
        s.bind(('',result[1]))
        while 1:
            data,addr=s.recvfrom(1024)
            self.recv_q.put(data.decode())
            #time.sleep(0.2)

    def init(self,uid):
        th=threading.Thread(target=self.__recv_thread,args=(uid,))
        th.start()

        self.friends=[]
        self.curuser=uid
        #try:
        mySql = sql.getMySql().connect()
        if mySql:
            cur = mySql.cursor()
            #try:
            cur.execute("select uid1,uid2 from friend where uid1='%s' or uid2='%s'"%(uid,uid))
            result = cur.fetchall()
            for line in result:
                if line[0]==uid: self.friends.append(line[1])
                if line[1]==uid: self.friends.append(line[0])
            #print(friends)
            slm = QtCore.QStringListModel()
            slm.setStringList(self.friends)
            #print(slm)
            self.listView.setModel(slm)
            mySql.commit()
            #except:
            #    pass
            cur.close()
            mySql.close()
        #except:
        #    pass
    
    def __chat(self,index):
        dialog = QtWidgets.QDialog()
        ui = chatW.Ui_chatW()
        name=self.curuser+' to '+self.friends[index.row()]
        ui.setupUi(dialog,name,self.recv_q)
        ui.connect(self.friends[index.row()],self.curuser)
        dialog.show()
        dialog.exec_()

    def __addchat(self):
        dialog = QtWidgets.QDialog()
        ui = addD.Ui_Dialog()
        ui.setupUi(dialog,self.curuser)
        dialog.show()
        dialog.exec_()

    def retranslateUi(self, listW):
        _translate = QtCore.QCoreApplication.translate
        listW.setWindowTitle(_translate("listW", "MainWindow"))
        self.commandLinkButton.setText(_translate("listW", "向陌生人发起会话"))
