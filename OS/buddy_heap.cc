#include <bits/stdc++.h>
using namespace std;

#define SIZE 10
#define PAGE_SIZE 1
#define inf 0x3f3f3f3f

struct Node{
    int sz,pid,flag;
    Node *l;
    Node *r;
    int addr_s,addr_t;
    /**
    sz,节点代表的内存块大小
    pid,占用当前节点的进程id
    flag,节点占用情况,0为为占用,1为占用,-1为节点不存在
    *l,*r,左右孩子指针
    addr_s,内存块起始物理地址
    addr_t,内存块结束物理地址
    */
};

struct p_node{
    int id,sz,addr_s,addr_t;
    bool operator < (const p_node &rhs) const{
        return id<rhs.id;
    }
    /**
    id,进程id
    sz,进程申请内存大小
    addr_s,进程在内存中占用空间的物理起始地址
    addr_t,进程在内存中占用空间的物理结束地址
    */
};

struct Buddy_Heap{
    Node *root;
    set<p_node> ps;
    vector<p_node> tmps;

    void create(Node *rt,int sz,int s,int t){
        rt->sz=sz;
        rt->pid=-1;
        rt->flag=0;
        rt->addr_s=s;
        rt->addr_t=t;
        rt->l=NULL;
        rt->r=NULL;
    }

    bool alloc(Node *rt,int id,int sz){
        int s_sz=(rt->sz)>>1;
        ///计算子节点大小
        if(s_sz<sz){
        ///子节点内存块过小,则考虑当前节点释放空闲
            if(rt->sz>=sz&&rt->flag==0){
            ///rt->flag为0,内存块空闲,可以分配
                rt->flag=1;
                rt->pid=id;
                ps.insert((p_node) {id,sz,rt->addr_s,rt->addr_t});
                ///在进程信息集合中插入进程信息
                return 1;
            }
            else return 0;    ///分配失败
        }
        if(rt->flag==1) return 0;
        if(rt->l==NULL){
            ///若其子节点不存在,则创建子节点,当前节点flag为-1
            rt->l=(Node*) malloc(sizeof(Node));
            create(rt->l,s_sz,rt->addr_s,(rt->addr_t+rt->addr_s)>>1);
            rt->r=(Node*) malloc(sizeof(Node));
            create(rt->r,s_sz,(rt->addr_t+rt->addr_s+1)>>1,rt->addr_t);
            rt->flag=-1;
        }
        bool s=alloc(rt->l,id,sz);
        ///向左儿子分配
        if(!s) s=alloc(rt->r,id,sz);
        ///左儿子分配失败时,向右儿子分配
        return s;
    }

    void relea(Node *rt,int id){
        if(rt->pid==id){
            ///找到需要释放的进程
            ps.erase(ps.lower_bound((p_node){id,0,0,0}));
            ///在集合中二分查找进程信息,并删除进程信息,进程所在节点为空闲
            rt->flag=0;
            rt->pid=-1;
            return;
        }
        if(rt->flag==-1){
            ///若存在左右儿子
            relea(rt->l,id);
            relea(rt->r,id);
            ///向左右儿子寻找需要释放的节点
            if(rt->l->flag==0&&rt->r->flag==0){
            ///左右儿子释放完成时,向父节点回溯,若儿子节点都为空闲,则合并
                free(rt->l); rt->l=NULL;
                free(rt->r); rt->r=NULL;
                ///在内存中释放左右儿子节点,当前节点状态从不存在转到空闲
                rt->flag=0;
            }
        }
    }

    void outT(Node *rt,bool l,const string s){
        if(rt->r) outT(rt->r,0,s+(l?"|      ":"       "));
        cout<<s<<(l?'\\':'/')<<"------";
        if(rt->pid==-1) printf("(x,");
        else printf("(%d,",rt->pid);
        printf("%d)\n",rt->flag!=-1?rt->sz:-1);
        if(rt->l) outT(rt->l,1,s+(l?"       ":"|      "));
    }
    void printT(Node *rt){
        if(rt==NULL) return;
        if(rt->r) outT(rt->r,0,"");
        ///打印左儿子
        if(rt->pid==-1) printf("(x,");
        else printf("(%d,",rt->pid);
        printf("%d)\n",rt->flag!=-1?rt->sz:-1);
        ///打印根节点信息
        if(rt->l) outT(rt->l,1,"");
        ///打印右儿子
        set<p_node>::iterator it;
        for(it=ps.begin();it!=ps.end();it++){
            int tsz=it->sz;
            int tas=it->addr_s;
            int tat=it->addr_t;
            printf("pid:%d sz:%d add_s:%d add_t:%d\n",it->id,tsz,tas,tat);
        }
        ///打印进程信息表
    }

    void eraseT(Node *rt){
        if(rt->l->l==NULL&&rt->l->r==NULL){
        ///若左儿子为叶子节点,释放左儿子
            free(rt->l);
            rt->l=NULL;
        }
        if(rt->r->l==NULL&&rt->r->r==NULL){
        ///若右儿子为叶子节点,释放右儿子
            free(rt->r);
            rt->r=NULL;
        }
        if(rt->l) eraseT(rt->l);
        if(rt->r) eraseT(rt->r);
        rt->flag=0;
        rt->l=NULL; rt->r=NULL;
        ///向下递归,清空左右儿子
    }
    void anti_fgm(){
        eraseT(root);
        ///清空伙伴堆
        set<p_node>::iterator it;
        for(it=ps.begin();it!=ps.end();it++){
            tmps.push_back((p_node){it->id,it->sz,it->addr_s,it->addr_t});
        }
        ps.clear();
        for(int i=0;i<tmps.size();i++){
            int tid=tmps[i].id,tsz=tmps[i].sz;
            alloc(root,tid,tsz);
        }
        tmps.clear();
        ///根据表中节点信息重新分配
    }

    /**
    *root,根节点
    set<p_node>,进程信息哈希表
    vector<p_node>,进程信息暂存容器
    create,创建节点信息
    alloc,分配函数
    relea,释放函数
    outT,printT,打印堆的树形图与进程信息
    eraseT,anti_fgm,清空堆,反碎片函数
    */
}BH;

void init(){
    BH.root=(Node*) malloc(sizeof(Node));
    BH.create(BH.root,1<<SIZE,0,(1<<SIZE)*PAGE_SIZE-1);
}

void solve(){
    int op,psz,pid;
    printf("please choose the function that you want to try ");
    printf("(1.allocate 2.release 3.print 4.anti-fragment)\n");
    while(~scanf("%d",&op)&&op){
        if(op==1){
            printf("input the process id and the size: ");
            scanf("%d%d",&pid,&psz);
            printf("allocate %s\n",BH.alloc(BH.root,pid,psz)?"sucess":"failed");
        }
        else if(op==2){
            printf("input the id of the process that you want to release: ");
            scanf("%d",&pid);
            BH.relea(BH.root,pid);
        }
        else if(op==3) BH.printT(BH.root);
        else{
            BH.anti_fgm();
            BH.printT(BH.root);
        }
    }
}

int main(){
    init();
    solve();
}

