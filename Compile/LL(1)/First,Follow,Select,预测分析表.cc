#include <bits/stdc++.h>
using namespace std;

char VN[]={'Z','S','A','B','G','E','H','T','F',};
char VT[]={'a','b','=','d','`','(',')','e','f','g','#'};
vector<string> g[15];
map<char,int> V;
int vis[9];
vector<int>fg[15],lg[15];
vector<char>f[15],l[15],s[15][15];
int flag[15];
string mp[305][305];

void init(){
    V['Z']=0; V['S']=1; V['A']=2;
    V['B']=3; V['G']=4; V['E']=5;
    V['H']=6; V['T']=7; V['F']=8;
    g[0].push_back("S");
    g[1].push_back("A");
    g[1].push_back("B");
    g[1].push_back("G");
    g[2].push_back("ab");
    g[3].push_back("b=E");
    g[4].push_back("g(F)S");
    g[5].push_back("TH");
    g[6].push_back("dTH");
    g[6].push_back("`");
    g[7].push_back("(E)");
    g[7].push_back("b");
    g[7].push_back("e");
    g[8].push_back("EfE");
    l[0].push_back('#');
}

void initvis(){
    for(int i=0;i<9;i++){
        if(vis[i]!=0) continue;
        for(int j=0;j<g[i].size();j++){
            string tmp=g[i][j];
            if(tmp=="`") vis[i]=1;
            else vis[i]=-1;
        }
    }
    /*for(int i=0;i<9;i++){
        printf("%d ",vis[i]);
    }
    printf("\n");
    */
}

void dfs(int u,int pre){
    if(fg[u].size()==0){
        flag[u]=1;
        if(vis[u]==1) f[u].push_back('`');
        if(pre==-1) return;
        for(int i=0;i<f[u].size();i++){
            f[pre].push_back(f[u][i]);
        }
        return;
    }
    for(int i=0;i<fg[u].size();i++){
        if(flag[u]==0) dfs(fg[u][i],u);
    }
    flag[u]=1;
    if(vis[u]==1) f[u].push_back('`');
    if(pre==-1) return;
    for(int i=0;i<f[u].size();i++){
        f[pre].push_back(f[u][i]);
    }
}

bool em(string x){
    for(int i=0;i<x.length();i++){
        if(x[i]=='`') continue;
        if(x[i]<'A'||x[i]>'Z') return 0;
        if(vis[V[x[i]]]==-1) return 0;
    }
    return 1;
}

void FIRST(){
    for(int i=0;i<9;i++){
        for(int j=0;j<g[i].size();j++){
            string tmp=g[i][j];
            for(int k=0;k<tmp.length();k++){
                if((tmp[k]<'A'||tmp[k]>'Z')&&tmp[k]!='`'){
                    f[i].push_back(tmp[k]);
                    break;
                }
                else{
                    if(tmp[k]=='`') break;
                    if(k==0){
                        fg[i].push_back(V[tmp[k]]);
                        if(vis[V[tmp[k]]]==-1) break;
                    }
                    else if(vis[V[tmp[k-1]]]==1){
                        fg[i].push_back(V[tmp[k]]);
                    }
                    else break;
                }
            }
        }
    }
    for(int i=0;i<9;i++) if(flag[i]==0) dfs(i,-1);
}

void bfs(int s){
    queue<int> q;
    q.push(s);
    while(!q.empty()){
        int x=q.front();
        q.pop();
        for(int i=0;i<9;i++){
            for(int j=0;j<lg[i].size();j++){
                if(lg[i][j]==x&&flag[i]==0){
                    q.push(i);
                    for(int k=0;k<l[x].size();k++){
                        int tf=0;
                        for(int m=0;m<l[i].size();m++){
                            if(l[i][m]==l[x][k]){
                                tf=1;
                                break;
                            }
                        }
                        if(tf==0) l[i].push_back(l[x][k]);
                    }
                    flag[i]=1;
                }
            }
        }
    }
}

void FOLLOW(){
    for(int i=0;i<9;i++){
        for(int j=0;j<g[i].size();j++){
            string tmp=g[i][j];
            for(int k=0;k<tmp.length();k++){
                if(tmp[k]>='A'&&tmp[k]<='Z'){
                    if(k==tmp.length()-1||em(tmp.substr(k+1,tmp.length()-k-1))){
                        lg[V[tmp[k]]].push_back(i);
                    }
                    else{
                        if(tmp[k+1]<'A'||tmp[k+1]>'Z') l[V[tmp[k]]].push_back(tmp[k+1]);
                        else{
                            for(int m=0;m<f[V[tmp[k+1]]].size();m++){
                                if(f[V[tmp[k+1]]][m]=='`') continue;
                                l[V[tmp[k]]].push_back(f[V[tmp[k+1]]][m]);
                            }
                        }
                    }
                }
            }
        }
    }
    memset(flag,0,sizeof(flag));
    for(int i=0;i<9;i++) if(lg[i].size()==0){
        memset(flag,0,sizeof(flag));
        bfs(i);
    }
}

void SELECT(){
    for(int i=0;i<9;i++){
        for(int j=0;j<g[i].size();j++){
            string tmp=g[i][j];
            for(int k=0;k<tmp.size();k++){
                if((tmp[k]<'A'||tmp[k]>'Z')){
                    if(tmp[k]!='`')s[i][j].push_back(tmp[k]);
                    break;
                }
                else if(vis[V[tmp[k]]]==-1){
                    //printf("~~ %c\n",tmp[k]);
                    for(int m=0;m<f[V[tmp[k]]].size();m++){
                        int tf=0;
                        for(int o=0;o<s[i][j].size();o++){
                            if(s[i][j][o]==f[V[tmp[k]]][m]){
                                tf=1;
                                break;
                            }
                        }
                        if(tf==0) s[i][j].push_back(f[V[tmp[k]]][m]);
                    }
                    break;
                }
            }
            if(em(tmp)){
                //cout<<tmp+" "<<em(tmp)<<VN[i]<<endl;
                for(int k=0;k<l[i].size();k++){
                    int tf=0;
                    for(int m=0;m<s[i][j].size();m++){
                        if(l[i][k]==s[i][j][m]){
                            tf=1;
                            break;
                        }
                    }
                    if(tf==0) s[i][j].push_back(l[i][k]);
                }
            }
        }
    }
}

int main(){
    init();
    initvis();
    FIRST();
    FOLLOW();
    SELECT();
    printf("========FIRST========\n");
    for(int i=0;i<9;i++){
        printf("%c ",VN[i]);
        for(int j=0;j<f[i].size();j++){
            printf("%c ",f[i][j]);
        }
        printf("\n");
    }
    printf("========FOLLOW========\n");
    for(int i=0;i<9;i++){
        printf("%c ",VN[i]);
        for(int j=0;j<l[i].size();j++){
            printf("%c ",l[i][j]);
        }
        printf("\n");
    }
    printf("========SELECT========\n");
    for(int i=0;i<9;i++){
        for(int j=0;j<g[i].size();j++){
            printf("%c->",VN[i]);
            cout<<g[i][j]<<endl;
            for(int k=0;k<s[i][j].size();k++){
                printf("%c ",s[i][j][k]);
                mp[VN[i]][s[i][j][k]]=g[i][j];
            }
            printf("\n");
        }
    }
    printf("========PREDICT========\n");
    for(int i=0;i<11;i++){
        printf("%5c",VT[i]);
    }
    for(int i=0;i<9;i++){
        printf("\n%c",VN[i]);
        for(int j=0;j<11;j++){
            for(int k=0;k<5-mp[VN[i]][VT[j]].length();k++) printf(" ");
            for(int k=0;k<mp[VN[i]][VT[j]].length();k++) printf("%c",mp[VN[i]][VT[j]][k]);
        }
    }
}
