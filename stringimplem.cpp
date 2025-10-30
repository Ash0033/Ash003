#include <cstring>
#include <iostream>

struct S {
    char* p; size_t n, cap;
    S():p(nullptr),n(0),cap(0){}
    S(const char* s){ if(!s){p=nullptr;n=cap=0;return;} n=strlen(s); cap=n; p=new char[cap+1]; memcpy(p,s,n+1); }
    S(const S& o){ n=o.n; cap=o.cap; p= cap? new char[cap+1]:nullptr; if(p) memcpy(p,o.p,n+1); }
    S& operator=(const S& o){ if(this==&o) return *this; delete[] p; n=o.n; cap=o.cap; p=cap? new char[cap+1]:nullptr; if(p) memcpy(p,o.p,n+1); return *this; }
    ~S(){ delete[] p; }
    size_t size() const { return n; }
    const char* c_str() const { return p?p:""; }
    char& operator[](size_t i){ return p[i]; }

    void reserve(size_t newcap){ if(newcap<=cap) return; char* q=new char[newcap+1]; if(p){ memcpy(q,p,n+1); delete[] p; } p=q; cap=newcap; }
    void push_back(char c){ if(n+1>cap) reserve(cap? cap*2:8); p[n++]=c; p[n]=0; }
    void append(const char* s){ if(!s) return; size_t a=strlen(s); if(n+a>cap) reserve(std::max(cap*2, n+a)); memcpy(p+n,s,a+1); n+=a; }
    void clear(){ n=0; if(p) p[0]=0; }
};

int main(){
    S a("hello");
    a.append(", world");
    a.push_back('!');
    std::cout<<a.c_str()<<" size="<<a.size()<<"\n";
}
