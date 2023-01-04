#include <bits/stdc++.h>
using namespace std;

#define LIST_SIZE 10
#define PAGE_SIZE 1
#define SIZE 111

struct page_list{
    int self_block,level;
    int block[LIST_SIZE];
    page_list *p[LIST_SIZE];
} *pl[SIZE];

struct quick_list{
    int pid,pnum,block;
};

struct ps_info{
    int level,block,sz,flag;
} ps[SIZE];

struct page_info{
    bool f1;
    int f2;
}bitmap[SIZE];

queue<quick_list> FIFO;
queue<quick_list> tmp;
queue<int> q;

void init(page_list *rt,int block,int le){
    rt->self_block=block;
    rt->level=le;
    for(int i=0;i<LIST_SIZE;i++){
        rt->block[i]=-1;
        rt->p[i]=NULL;
    }
}

int build(page_list *rt,int sz,int level){
    if(level==1){
        int cnt=0;
        for(int i=0;i<min(sz,LIST_SIZE);i++,cnt++){
            rt->block[i]=q.front();
            bitmap[q.front()].f2=1;
            //printf("~ %d\n",rt->block[i]);
            q.pop();
        }
        return cnt;
    }
    int sum=0;
    for(int i=0;i<LIST_SIZE&&sz>0;i++){
        rt->p[i]=(page_list*) malloc(sizeof(page_list));
        rt->block[i]=q.front(); q.pop();
        init(rt->p[i],rt->block[i],level-1);
        bitmap[rt->block[i]].f2=-1;
        //printf("- %d %d %d\n",rt->p[i]->level,rt->block[i],sz);
        int tmp=build(rt->p[i],sz,level-1);
        sum+=tmp; sz-=tmp;
    }
    return sum;
}

bool alloc(int pid,int sz){
    int level=0,base=1,tsz=sz;
    while(base<sz){
        base*=LIST_SIZE;
        level++;
        tsz+=sz%base==0?(sz/base):(sz/base+1);
    }
    //printf("-- %d %d\n",level,tsz);
    for(int i=0;i<SIZE;i++){
        if(!bitmap[i].f1){
            q.push(i);
            bitmap[i].f1=1;
            if(q.size()==tsz) break;
        }
    }
    if(q.size()==tsz){
        pl[pid]=(page_list*) malloc(sizeof(page_list));
        bitmap[q.front()].f2=-1;
        init(pl[pid],q.front(),level); q.pop();
        build(pl[pid],sz,level);
        ps[pid]=(ps_info) {level,pl[pid]->self_block,sz,1};
        return 1;
    }
    return 0;
}

int locat(page_list *rt,int pnum){
    if(rt->level==1){
        return rt->block[pnum-1];
    }
    int i,cnt=0,base=1;
    for(int k=0;k<rt->level-1;k++) base*=LIST_SIZE;
    for(i=0;i<pnum;i+=base,cnt++);
    //printf("---%d %d %d %d %d\n",cnt,i,pnum,pnum-i+LIST_SIZE*base,rt->level);
    return locat(rt->p[cnt-1],pnum-i+base);
}

int faddr(int pid,int pnum){
    int res,flag=0;
    while(!tmp.empty()) tmp.pop();
    while(!FIFO.empty()){
        quick_list x=FIFO.front();
        if(x.pid==pid&&x.pnum==pnum){
            printf("快表命中\n");
            flag=1; res=x.block;
        }
        tmp.push(FIFO.front());
        FIFO.pop();
    }
    if(flag){
        while(!tmp.empty()){
            FIFO.push(tmp.front());
            tmp.pop();
        }
        return res;
    }
    printf("快表未命中\n");
    res=locat(pl[pid],pnum);
    if(tmp.size()>=LIST_SIZE) tmp.pop();
    while(!tmp.empty()){
        FIFO.push(tmp.front());
        tmp.pop();
    }
    FIFO.push((quick_list){pid,pnum,res});
    return res;
}

void relea(page_list *rt){
    bitmap[rt->self_block].f1=0;
    bitmap[rt->self_block].f2=0;
    if(rt->level==1){
        for(int i=0;i<LIST_SIZE;i++){
            if(rt->block[i]!=-1){
                bitmap[rt->block[i]].f1=0;
                bitmap[rt->block[i]].f2=0;
            }
        }
        return;
    }
    for(int i=0;i<LIST_SIZE;i++){
        if(rt->p[i]!=NULL){
            relea(rt->p[i]);
            free(rt->p[i]);
            rt->p[i]=NULL;
        }
    }
    free(rt);
    rt=NULL;
}

int main(){
    int op;
    while(~scanf("%d",&op)&&op){
        if(op==1){
            int id,sz;
            scanf("%d%d",&id,&sz);
            if(alloc(id,sz)==0){
                printf("空间不足,分配失败\n");
                continue;
            }
            printf("分配成功,物理内存位图如下");
            printf("(括号中第一位代表该页被占用,第二位为1,则为进程占用,-1则为页表占用):\n");
            for(int i=1;i<=SIZE;i++){
                printf("(%d,%d)%s",bitmap[i-1].f1,bitmap[i-1].f2,i%6==0?"\n":" ");
            }
            printf("\n");
        }
        else if(op==2){
            int id,pnum;
            scanf("%d%d",&id,&pnum);
            printf("进程 %d 第 %d 页位于 %d 号页框\n",id,pnum,faddr(id,pnum));
        }
        else if(op==3){
            int id;
            scanf("%d",&id);
            if(ps[id].flag==0){
                printf("进程不位于内存中\n");
                continue;
            }
            relea(pl[id]);
            ps[id].flag=0;
            printf("释放完毕,物理内存位图如下:");
            printf("(括号中第一位代表该页是否被占用,第二位为1则为进程占用,-1则为页表占用):\n");
            for(int i=1;i<=SIZE;i++){
                printf("(%d,%d)%s",bitmap[i-1].f1,bitmap[i-1].f2,i%6==0?"\n":" ");
            }
            printf("\n");
        }
        else{
            int id;
            scanf("%d",&id);
            if(ps[id].flag==0){
                printf("进程不位于内存中\n");
                continue;
            }
            printf("进程 %d 大小为 %d 页,由 %d 级页表管理,最高级页表处于%d号页框\n"
            ,id,ps[id].sz,ps[id].level,ps[id].block);
        }
    }
}
