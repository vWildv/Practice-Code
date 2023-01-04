import loginW
import pymysql
import sql
import sys        
from PyQt5 import QtCore, QtGui, QtWidgets;
from PyQt5.QtWidgets import QApplication, QMainWindow

if __name__ == '__main__':
    app = QApplication(sys.argv)
    mMainWindow = QMainWindow ()
    ui=loginW.Ui_LoginW() 
    ui.setupUi(mMainWindow)
    mySql = sql.getMySql().connect()
    if(mySql):
        mySql.close()
    else:
        QtWidgets.QMessageBox.information(None,"提示:","数据库连接失败，请确定服务器是否开启;"); 
    mMainWindow.show(); 
    sys.exit(app.exec_())