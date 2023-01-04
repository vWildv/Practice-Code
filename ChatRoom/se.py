import socket
import threading
from cmd import Cmd
import json

class s():
    def __init__(self):
        self.__socket=socket.socket(socket.AF_INET,socket.SOCK_STREAM)
        self.__conn=list()
        self.__nickname=list()
        self.lock=threading.RLock()
    
    def start(self):
        self.__socket.bind(('127.0.0.1',1213))
        self.__socket.listen(10)
        print('Server is working')
        
        self.__conn.clear()
        self.__nickname.clear()
        self.__conn.append(None)
        self.__nickname.append('System')

        while 1:
            #self.cmdloop()
            conn,addr=self.__socket.accept()
            self.lock.acquire()
            print('Server accept a connection',conn.getsockname(),conn.fileno())
            
            try:
                buff=conn.recv(1024).decode()
                print(buff)
                obj=json.loads(buff)
                if obj['type'] == 'login':
                    self.__conn.append(conn)
                    self.__nickname.append(obj['nickname'])
                    conn.send(json.dumps({'id':len(self.__conn)-1}).encode())

                    th=threading.Thread(target=self.__user_thread,args=(len(self.__conn)-1,))
                    th.setDaemon(True)
                    th.start()
                else:
                    print('can not decode json data:', conn.getsockname(), conn.fileno())
            except Exception:
                print('can not accpet data:', conn.getsockname(), conn.fileno())
            self.lock.release()
            
    def __user_thread(self,uid):
        conn=self.__conn[uid]
        nickname=self.__nickname[uid]
        print('user %s enter chatroom'%(nickname))
        msg='user '+str(nickname)+'('+str(uid)+') '+'enter'
        self.__broadcast(message=msg)

        while 1:
            try:
                buff=conn.recv(1024).decode()
                obj=json.loads(buff)
                if obj['type'] == 'broadcast':
                    self.__broadcast(obj['sender_id'],obj['message'])
                else:
                    print('can not decode json data:', conn.getsockname(), conn.fileno())
            except Exception:
                print('can not accpet connection:', conn.getsockname(), conn.fileno())
                self.__conn[uid].close()
                self.__conn[uid]=None
                self.__conn[uid]=None
    def __broadcast(self,uid=0,message=''):
        for i in range(1,len(self.__conn)):
            if uid!=i:
                self.__conn[i].send(json.dumps({
                    'sender_id':uid,
                    'sender_nickname':self.__nickname[uid],
                    'message':message
                }).encode()) 

server=s()
server.start()
