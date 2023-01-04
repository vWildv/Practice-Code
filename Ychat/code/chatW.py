# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'ui/chatW.ui'
#
# Created by: PyQt5 UI code generator 5.13.0
#
# WARNING! All changes made in this file will be lost!


from PyQt5 import QtCore, QtGui, QtWidgets
import socket
import sql
import time
import threading
import queue
from vchat import Video_Server, Video_Client
from achat import Audio_Server, Audio_Client


class Ui_chatW(object):
    toport=None
    myuid=None
    seport=0
    toip="127.0.0.1"
    recv_q=recv_q=queue.Queue(30)
    def setupUi(self, chatW, name,recv):
        chatW.setObjectName("chatW")
        chatW.resize(593, 446)
        self.centralwidget = QtWidgets.QWidget(chatW)
        self.centralwidget.setObjectName("centralwidget")
        self.plainTextEdit = QtWidgets.QPlainTextEdit(self.centralwidget)
        self.plainTextEdit.setGeometry(QtCore.QRect(20, 290, 551, 81))
        self.plainTextEdit.setObjectName("plainTextEdit")
        self.textBrowser = QtWidgets.QTextBrowser(self.centralwidget)
        self.textBrowser.setGeometry(QtCore.QRect(20, 20, 551, 251))
        self.textBrowser.setObjectName("textBrowser")
        self.sendBu = QtWidgets.QPushButton(self.centralwidget)
        self.sendBu.setGeometry(QtCore.QRect(490, 380, 81, 31))
        self.sendBu.setObjectName("sendBu")
        self.vedioB = QtWidgets.QCommandLinkButton(self.centralwidget)
        self.vedioB.setGeometry(QtCore.QRect(20, 380, 141, 41))
        self.vedioB.setObjectName("vedioB")
        #chatW.setCentralWidget(self.centralwidget)
        self.statusbar = QtWidgets.QStatusBar(chatW)
        self.statusbar.setObjectName("statusbar")
        #chatW.setStatusBar(self.statusbar)

        self.retranslateUi(chatW,name)
        self.name=name
        QtCore.QMetaObject.connectSlotsByName(chatW)
        self.recv_q=recv

        self.sendBu.clicked.connect(self.__send)
        self.vedioB.clicked.connect(self.__vchat)

    def __recv_thread(self,uid,seuid):
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
        
    def __show_thread(self):
        while 1:
            msg=self.recv_q.get(True)
            fw=open("c:/Work/python/Ychat/logs/"+self.name+".log",'a+')
            fw.write(msg)
            self.textBrowser.append(msg)
            self.textBrowser.moveCursor(self.textBrowser.textCursor().End)
            time.sleep(0.2)

    def connect(self,uid,seuid):
        mySql = sql.getMySql().connect()
        cur = mySql.cursor()
        cur.execute("select uid,port from user where uid='%s'"%(seuid))
        result=cur.fetchone()
        cur.close()
        mySql.close()
        self.seport=result[1]
        self.myuid=seuid
        #th=threading.Thread(target=self.__recv_thread,args=(uid,seuid,))
        #th.start()
        th=threading.Thread(target=self.__show_thread)
        th.start()
        vserver=Video_Server((self.seport+1),4)
        aserver=Audio_Server((self.seport+2),4)
        vserver.start()
        aserver.start()
        mySql = sql.getMySql().connect()
        cur = mySql.cursor()
        cur.execute("select uid,port from user where uid='%s'"%(uid))
        result=cur.fetchone()
        cur.close()
        mySql.close()
        self.toport=result[1]
        print(self.seport,self.toport)
    
    def __send(self):
        s=socket.socket(socket.AF_INET,socket.SOCK_DGRAM)
        data=self.plainTextEdit.toPlainText()
        self.plainTextEdit.setPlainText("")
        data=self.myuid+'  '+time.strftime('%Y-%m-%d %H:%M:%S',time.\
                localtime(time.time()))+'\n'+data+'\n'
        s.sendto(data.encode(),(self.toip,self.toport))
        self.recv_q.put(data)

    def __vchat(self):
        print(self.toip,self.toport,self.seport)
        VERSION=4
        SHOWME=1
        LEVEL=1
        vclient = Video_Client(self.toip,self.toport+1, SHOWME, LEVEL, VERSION)
        aclient = Audio_Client(self.toip,self.toport+2, VERSION)
        vclient.start()
        aclient.start()

    def retranslateUi(self, chatW, name):
        _translate = QtCore.QCoreApplication.translate
        chatW.setWindowTitle(_translate("chatW", name))
        self.sendBu.setText(_translate("chatW", "发送"))
        self.vedioB.setText(_translate("chatW", "发起视频通话"))
