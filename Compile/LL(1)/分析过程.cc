#include<bits/stdc++.h>
using namespace std;

stack<char>St;
char VN[]={'Z','S','A','B','G','E','H','T','F',};
char VT[]={'a','b','=','d','`','(',')','e','f','g','#'};
string mp[125][125];
vector<string> vstr[105];
map<string,int> mV;

int Getcnt(){
    ifstream myfile1("token.txt");
    string str;
    string str1;
    int nMax = 0;
    while(getline(myfile1,str)){
        //cout<<str<<endl;
        int st = 0;
        int cnt = 0;
        bool falg = true;
        int num = 0;
        for(int i=0;i<str.size();i++){
            if(str[i]==' '){
                cnt++;
            }
            if(cnt==1&&falg){
                num = str[i+1]-'0';
                falg = false;
            }
            if(cnt==2){
                st = i+1;
                break;
            }
        }
        cnt = 0;
        str1= str.substr(st);
        mV[str1] = num;
        if(num==5){
            nMax++;
            continue;
        }
        vstr[nMax].push_back(str1);
    }
    return nMax;
}

string Cto(string str){
    if(mV[str]!=2&&str.size()==1){
        for(int i=0;i<9;i++){
            if(VT[i]==str[0]) return str;
        }
    }
    if(str=="if"||str=="while"){
        return "g";
    }
    if(mV[str]==1){
        return "a";
    }
    if(mV[str]==2){
        return "b";
    }
    if(mV[str]==3){
        return "e";
    }
    if(mV[str]==4){
        if(str[0]=='+'||str[0]=='/'||str[0]=='*'||str[0]=='-') return "d";
        return "f";
    }
}

void init(){
    mp['Z']['a'] = "S";
    mp['S']['a'] = "A";
    mp['A']['a'] = "ab";
    mp['Z']['b'] = "S";
    mp['S']['b'] = "B";
    mp['B']['b'] = "b=E";
    mp['E']['b'] = "TH";
    mp['T']['b'] = "b";
    mp['F']['b'] = "EfE";
    mp['H']['d'] = "dTH";
    mp['E']['('] = "TH";
    mp['E']['e'] = "TH";
    mp['T']['('] = "(E)";
    mp['T']['e'] = "e";
    mp['F']['('] = "EfE";
    mp['F']['e'] = "EfE";
    mp['H'][')'] = "`";
    mp['H']['f'] = "`";
    mp['H']['#'] = "`";
    mp['Z']['g'] = "S";
    mp['S']['g'] = "G";
    mp['G']['g'] = "g(F)S";
}

int isvn(char a){
    if(a>='A' && a<='Z' || a == '#'){
        return 1;
    }
    return 0;
}

int analysis(string a){
    St.push('#');
    St.push('Z');
    int cnt2 = 0;
    int cnt = 0;
    while(1){
        cout<<cnt2<<":";
        ++cnt2;
        char tmp = St.top();
        St.pop();
        if(!isvn(tmp)){
            cout<<tmp<<endl;
            if(a[cnt]==tmp){
                cout<<tmp<<"Æ¥Åä"<<endl;
                cnt++;
            }
            else{
                cout<<"ERROR1"<<endl;
                break;
            }
        }
        else{
            if(tmp == '#'){
                //cout<<"!!!!"<<cnt<<"!!!!"<<endl;
                if(a[cnt] == tmp){
                    cout<<"½ÓÊÕ"<<endl<<endl;

                    break;
                }
                else{
                    cout<<"ERROR2"<<endl;
                    break;
                }
            }
            else{
                if(mp[tmp][a[cnt]]==""){
                    cout<<"ERROR3"<<endl;
                    break;
                }
                else{
                    string str = mp[tmp][a[cnt]];
                    for(int i = str.size()-1; i >= 0 ; i--){
                        if(str[i]!='`')
                            St.push(str[i]);
                    }
                    cout<<tmp<<"-->"<<str<<endl;
                }
            }
        }
    }
}

int main(){
    init();
    int cnt=Getcnt();
    for(int i=0;i<cnt;i++){
        printf("=========================\n");
        while(!St.empty()) St.pop();
        string tmp="";
        for(int j=0;j<vstr[i].size();j++){
            tmp+=Cto(vstr[i][j]);
            //printf("%c",Cto(vstr[i][j]));
        }
        tmp+="#";
        cout<<tmp<<endl;
        analysis(tmp);
    }
}
