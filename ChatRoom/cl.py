import socket
import threading
from cmd import Cmd
import json

class c(Cmd):
    prompt = ''
    intro = 'Welcome easy chatroom ' + '(input help to get help)\n'
    def __init__(self):
        super().__init__()
        self.__socket=socket.socket(socket.AF_INET,socket.SOCK_STREAM)
        self.__id=None
        self.__nickname=None
    def __recv_msg_thread(self):
        while 1:
            try:
                buff=self.__socket.recv(1024).decode()
                obj=json.loads(buff)
                print('[%s (%s)] : %s'%(str(obj['sender_nickname']),\
                        str(obj['sender_id']),obj['message']))
            except Exception:
                print('can not receive message')
    def __send_msg_thread(self,message):
        self.__socket.send(json.dumps({
            'type':'broadcast',
            'sender_id':self.__id,
            'message':message
        }).encode())
    def start(self):
        self.__socket.connect(('127.0.0.1',1213))
        self.cmdloop()
    def do_login(self,args):
        nickname=args.split(' ')[0]
        self.__socket.send(json.dumps({
            'type':'login',
            'nickname':nickname
        }).encode())
        try:
            buff=self.__socket.recv(1024).decode()
            obj=json.loads(buff)
            if obj['id']:
                self.__nickname=nickname
                self.__id=obj['id']
                print('enter chatroom successfully')

                th=threading.Thread(target=self.__recv_msg_thread)
                th.setDaemon(True)
                th.start()
            else:
                print('can not enter chartoom')
        except Exception:
            print('can not receive data')
    def do_send(self,args):
        message=args
        print('[%s (%s)]: %s'%(str(self.__nickname),str(self.__id),message))
        th=threading.Thread(target=self.__send_msg_thread,args=(message,))
        th.setDaemon(True)
        th.start()
    def do_help(self,args):
        command = args.split(' ')[0]
        if command == '':
            print('[Help] login nickname - enter chatroom with your nickname')
            print('[Help] send message - send message to everyone')
        elif command == 'login':
            print('[Help] login nickname - enter chatroom with your nickname')
        elif command == 'send':
            print('[Help] send message - send message to everyone')
        else:
            print('[Help] no help infomation')

client=c()
client.start()
