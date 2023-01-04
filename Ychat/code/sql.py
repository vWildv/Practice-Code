import pymysql

mysql=None
class SQLdata():
    sql=None
    def __init__(self):
        self.sql=None
    def connect(self):
        try:
            self.sql=pymysql.connect(host='localhost', user='root',password='1213',database='pythondb',charset='utf8')
        except:
            print("error:连接db失败")
            return None
        return self.sql
    def close(self):
        self.sql.close()
        self.sql=None

mysql=SQLdata()
def getMySql():
    return mysql
