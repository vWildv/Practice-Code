import queue
import threading
import time
import copy
import random
import os

class MsgRouter:
    node_list = []
    def register_node(self, node):
        self.node_list.append(node)

    def _send_thread(self, msg):
        dest = msg.dest
        for n in self.node_list:
            if dest==n.id:
                if n.is_alive(): n.recv_q.put(msg)
                return
        raise "Node Not Found!!"

    def send(self, msg):
        th=threading.Thread(target=self._send_thread,args=(msg,))
        th.start()

    def broadcast(self, msg):
        for n in self.node_list:
            tmp = copy.deepcopy(msg) 
            tmp.dest = n.id
            #n.recv_q.put(tmp)
            th=threading.Thread(target=n.recv_q.put,args=(tmp,))
            th.start()

class Message:
    src = 0
    dest = 0

class PrepareReq(Message):
    proposal_num = 0
    def __str__(self):
        return " ".join(("PrepareReq[src:",str(self.src),"dest:"+str(self.dest),
                        "proposal_num:",str(self.proposal_num),"]"))
      
class PrepareRsp(Message):
    promise_num = 0
    accepted_num = 0
    accepted_val = 0
    def __str__(self):
        return " ".join(("PrepareRsp[src:",str(self.src),"dest:",str(self.dest),
                        "promise_num:",str(self.promise_num),
                        "accepted_num:",str(self.accepted_num),
                        "accepted_val:",str(self.accepted_val), "]"))

class AcceptReq(Message):
    proposal_num = 0
    proposal_val = None 
    def __str__(self):
        return " ".join(("AcceptReq[src:",str(self.src),"dest:"+str(self.dest),
                        "proposal_num:",str(self.proposal_num),
                        "proposal_val:",str(self.proposal_val), "]"))

class AcceptRsp(Message):
    proposal_num = 0
    def __str__(self):
        return " ".join(("AcceptRsp[src:",str(self.src),"dest:",str(self.dest),
                        "proposal_num:",str(self.proposal_num), "]"))

class MsgNode:
    id = 0 
    recv_q = None
    def __init__(self):
        self.recv_q = queue.Queue(30)
        self.id = 0

    def send(self, msg):
        g_msg_router.send(msg)

    def recv(self, timeout=None):
        return self.recv_q.get(True, timeout)

class Proposer(MsgNode, threading.Thread):
    val = None
    _last_proposal_num = MsgNode.id
    _last_proposal_val = None
    def __init__(self, id, val):
        threading.Thread.__init__(self)
        MsgNode.__init__(self)
        self.id = id
        self.val = val
        self._last_proposal_num = id
        self._last_proposal_val = None

    def propose(self):
        self._last_proposal_num = self._last_proposal_num + NUM_OF_PROPOSER
        print ("proposer", self.id, "start propose", self._last_proposal_num)
        req = PrepareReq()
        req.src = self.id
        req.proposal_num = self._last_proposal_num
        g_msg_router.broadcast(req)
    
    def accept(self, prep_rsp_list,last_pos):
        acc_req_list = []
        _accepted_val = None
        _accepted_num = 0
        for i in prep_rsp_list:
            if i.accepted_num > _accepted_num:
                _accepted_num = i.accepted_num
                _accepted_val = i.accepted_val
        for i in range(last_pos,len(prep_rsp_list)):
            acpt = AcceptReq()
            acpt.src = self.id
            acpt.dest = prep_rsp_list[i].src
            acpt.proposal_num = self._last_proposal_num
            if _accepted_num == 0:
                acpt.proposal_val = self.val
            else:
                acpt.proposal_val = _accepted_val
            self._last_proposal_val = acpt.proposal_val
            g_msg_router.send(acpt)

    def run(self):
        _prep_rsp_list = []
        _acc_rsp_list = []
        _prep_rsp_cnt = 0
        _last_rsp_cnt = 0
        while True:
            try:
                time.sleep(random.randint(0,3))
                msg = self.recv(10)
                if isinstance(msg, PrepareRsp):
                    print ("proposer", self.id, "got", msg)
                    if msg.promise_num==self._last_proposal_num:
                        _prep_rsp_list.append(msg)
                        _prep_rsp_cnt=_prep_rsp_cnt+1
                        if _prep_rsp_cnt >= QUOROM_OF_ACCEPTOR:
                            print ("proposer", self.id, "start accept", self._last_proposal_num)
                            self.accept(_prep_rsp_list,_last_rsp_cnt)
                            _last_rsp_cnt=_prep_rsp_cnt
                    else:
                        pass
                elif isinstance(msg, PrepareReq):
                    pass
                elif isinstance(msg, AcceptRsp):
                    if msg.proposal_num==self._last_proposal_num:
                        _acc_rsp_list.append(msg)
                        if len(_acc_rsp_list) >= QUOROM_OF_ACCEPTOR:
                            print ("proposer", self.id,"  ===============consensus reached!!",\
                                "===============  ", self._last_proposal_val)
                            break
                elif isinstance(msg, AcceptReq):
                    pass
            except queue.Empty:
                self.propose()
    
    def __str__(self):
        return " ".join(("P", self.id))

class Acceptor(MsgNode, threading.Thread):
    h_n = 0
    a_n = 0
    a_v = None
    def __init__(self, id):
        threading.Thread.__init__(self)
        MsgNode.__init__(self)
        self.id = id

    def run(self):
        while True:
            msg = self.recv()
            if isinstance(msg, PrepareRsp):
                pass
            elif isinstance(msg, PrepareReq):
                print ("acceptor", self.id, "got", msg)
                print ("acceptor", self.id, "status", self)
                if msg.proposal_num > self.h_n:
                    rsp = PrepareRsp()
                    rsp.src = msg.dest
                    rsp.dest = msg.src
                    rsp.promise_num = msg.proposal_num
                    self.h_n = msg.proposal_num
                    rsp.accepted_num = self.a_n
                    rsp.accepted_val = self.a_v
                    g_msg_router.send(rsp)
                    print ("acceptor", self.id, "send", rsp)
                    self.h_n = msg.proposal_num
            elif isinstance(msg, AcceptRsp):
                pass
            elif isinstance(msg, AcceptReq):
                print ("acceptor", self.id, "got", msg)
                print ("acceptor", self.id, "status", self)
                if msg.proposal_num < self.h_n:
                    pass
                else:
                    rsp = AcceptRsp()
                    rsp.src = msg.dest
                    rsp.dest = msg.src
                    rsp.proposal_num = msg.proposal_num
                    self.a_n = msg.proposal_num
                    self.a_v = msg.proposal_val
                    g_msg_router.send(rsp)
                    print ("acceptor", self.id, "send", rsp)
    
    def __str__(self):
        return " ".join(("h_n", str(self.h_n), "a_n", str(self.a_n),"a_v", str(self.a_v)))

g_p_list = []
NUM_OF_PROPOSER = 3 
g_a_list = []
NUM_OF_ACCEPTOR = 3
QUOROM_OF_ACCEPTOR = 2
g_msg_router = MsgRouter()

def paxos_consensus():
   for i in range(0, NUM_OF_ACCEPTOR):
      tmp = Acceptor(i)
      tmp.start()
      g_a_list.append(tmp)
      g_msg_router.register_node(tmp)

   for i in range(10, 10+NUM_OF_PROPOSER):
      tmp = Proposer(i, "hello consensus!!"+str(i))
      tmp.start()
      g_p_list.append(tmp)
      g_msg_router.register_node(tmp)

if __name__ == "__main__":
   paxos_consensus()
   while True:
      try:
         time.sleep(3)
      except:
         os._exit(0)