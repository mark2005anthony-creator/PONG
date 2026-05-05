// ============================================================================
// PONG  —  Native C++ Windows executable
// Compile: g++ pong.cpp -o pong.exe -lgdi32 -lwinmm -static -O2 -mwindows
// ============================================================================

#define WIN32_LEAN_AND_MEAN
#define _USE_MATH_DEFINES
#include <windows.h>
#include <windowsx.h>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <string>
#include <fstream>
#include <algorithm>

const int GW = 800, GH = 500, PW = 12, BS = 10;

COLORREF colors[] = {
    RGB(255,255,255), RGB(255,80,80), RGB(80,255,80), RGB(80,150,255),
    RGB(255,255,80), RGB(255,128,0), RGB(200,80,255), RGB(0,255,255)
};
const char* cN[] = {"WHITE","RED","GREEN","BLUE","YELLOW","ORANGE","PURPLE","CYAN"};
const int NC = 8;

struct Settings {
    std::string p1n = "PLAYER 1", p2n = "COMPUTER";
    int p1c = 0, p2c = 0, bc = 0, sc = 0;
    int bg = 0, sl = 7, tl = 0, bspd = 20, psz = 80, ai = 5, pspd = 8;
    bool snd = true;
    COLORREF p1C(){return colors[p1c];}
    COLORREF p2C(){return colors[p2c];}
    COLORREF bC(){return colors[bc];}
    COLORREF sC(){return colors[sc];}
};
Settings ss;
Settings sDef;

enum St { MENU, PLAY, PAUSED, GAMEOVER, SETTINGS };
St st = MENU;
bool fullscreen = false;
WINDOWPLACEMENT wpPrev = {sizeof(WINDOWPLACEMENT)};
bool tp = false;
int ls = 0, rs = 0, trem = 0;
UINT_PTR t1 = 0;

struct Pd { float x,y,w,h,dy; } lp, rp;
struct Bl { float x,y,s,vx,vy; } bl;
int psz, pspd, bspd;
bool k[256] = {};
HWND hw = NULL;

float rnd(float a, float b) { return a + (float)rand()/RAND_MAX*(b-a); }
float clf(float v, float lo, float hi) { return std::max(lo, std::min(hi, v)); }

void bp(int f, int d) { if(ss.snd) Beep(f,d); }
void sh()  { bp(440,40); }
void sw()  { bp(330,30); }
void ssc() { bp(220,100); }
void swin(){ bp(523,75); Sleep(80); bp(659,75); Sleep(80); bp(784,125); }

void apply() { psz=ss.psz; pspd=ss.pspd; bspd=ss.bspd; }

void rpdl() {
    lp = {20.f, (GH-psz)/2.f, (float)PW, (float)psz, 0};
    rp = {(float)(GW-20-PW), (float)(GH-psz)/2.f, (float)PW, (float)psz, 0};
}

void rbl(int dir) {
    float a = rnd(-0.4f, 0.4f);
    int d = dir ? dir : (rand()%2?1:-1);
    bl = {GW/2.f-BS/2.f, GH/2.f-BS/2.f, (float)BS, cosf(a)*bspd*d, sinf(a)*bspd};
}

void rgame() {
    if(t1){KillTimer(hw,t1);t1=0;}
    ls=rs=0; apply(); rpdl(); rbl(1);
    if(ss.tl>0){trem=ss.tl;t1=SetTimer(hw,1,1000,NULL);}else trem=0;
    st=PLAY; InvalidateRect(hw,NULL,TRUE);
}

void mai() {
    float t = bl.y+bl.s/2.f-rp.h/2.f, d = t-rp.y;
    float ms = 1.5f+(ss.ai/10.f)*5.f;
    float rc = 4.f+(ss.ai/10.f)*16.f;
    rp.dy = fabsf(d)>rc ? (d>0?1:-1)*ms : 0;
    if(rnd(0,1) < 0.02f-(ss.ai/10.f)*0.015f) rp.dy=0;
}

void upd() {
    if(st!=PLAY) return;
    if(!tp) mai();
    if(k['W']) lp.dy = -(float)pspd;
    else if(k['S']) lp.dy = (float)pspd;
    else lp.dy = 0;
    if(tp) {
        if(k[VK_UP]) rp.dy = -(float)pspd;
        else if(k[VK_DOWN]) rp.dy = (float)pspd;
        else rp.dy = 0;
    }
    lp.y = clf(lp.y+lp.dy, 0, GH-lp.h);
    rp.y = clf(rp.y+rp.dy, 0, GH-rp.h);
    bl.x += bl.vx; bl.y += bl.vy;
    if(bl.y<=0){bl.y=0;bl.vy=-bl.vy;sw();}
    if(bl.y+bl.s>=GH){bl.y=GH-bl.s;bl.vy=-bl.vy;sw();}
    if(bl.x+bl.s>=lp.x && bl.x<=lp.x+lp.w && bl.y+bl.s>=lp.y && bl.y<=lp.y+lp.h && bl.vx<0) {
        float hp=(bl.y+bl.s/2.f-lp.y)/lp.h, a=(hp-0.5f)*1.2f;
        bl.vx=fabsf(cosf(a))*bspd; bl.vy=sinf(a)*bspd;
        bl.x=lp.x+lp.w; sh();
    }
    if(bl.x+bl.s>=rp.x && bl.x<=rp.x+rp.w && bl.y+bl.s>=rp.y && bl.y<=rp.y+rp.h && bl.vx>0) {
        float hp=(bl.y+bl.s/2.f-rp.y)/rp.h, a=(hp-0.5f)*1.2f;
        bl.vx=-fabsf(cosf(a))*bspd; bl.vy=sinf(a)*bspd;
        bl.x=rp.x-bl.s; sh();
    }
    if(bl.x+bl.s<0){rs++;ssc();if(rs>=ss.sl){st=GAMEOVER;swin();}else rbl(-1);}
    if(bl.x>GW){ls++;ssc();if(ls>=ss.sl){st=GAMEOVER;swin();}else rbl(1);}
    InvalidateRect(hw,NULL,TRUE);
}

struct Btn { RECT r; const char* t; };
bool ph(POINT p, RECT r){return p.x>=r.left&&p.x<=r.right&&p.y>=r.top&&p.y<=r.bottom;}
int mx=0, my=0;
int hoverBtn = -1;

void drwBtn(HDC dc, Btn& b, bool hover, int cw) {
    HBRUSH br=hover?CreateSolidBrush(RGB(255,255,255)):CreateSolidBrush(RGB(0,0,0));
    HPEN pn=CreatePen(PS_SOLID,2,RGB(255,255,255));
    SelectObject(dc,pn); SelectObject(dc,br);
    RoundRect(dc,b.r.left,b.r.top,b.r.right,b.r.bottom,8,8);
    DeleteObject(br); DeleteObject(pn);
    SetBkMode(dc,TRANSPARENT);
    SetTextColor(dc,hover?RGB(0,0,0):RGB(255,255,255));
    HFONT bf=CreateFontA(16,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,FIXED_PITCH,"Courier New");
    SelectObject(dc,bf);
    int tw=(int)strlen(b.t)*9, tx=(b.r.left+b.r.right-tw)/2, ty=b.r.top+(b.r.bottom-b.r.top-14)/2;
    TextOutA(dc,tx,ty,b.t,(int)strlen(b.t));
    DeleteObject(bf);
}

void drawGame(HDC dc) {
    RECT cl; GetClientRect(hw,&cl);
    int cw=cl.right-cl.left, ch=cl.bottom-cl.top;
    static HDC mdc=NULL; static HBITMAP mb=NULL; static int mw=0,mh=0;
    if(!mdc||mw!=cw||mh!=ch){
        if(mb)DeleteObject(mb); if(mdc)DeleteDC(mdc);
        mdc=CreateCompatibleDC(dc); mb=CreateCompatibleBitmap(dc,cw,ch); SelectObject(mdc,mb); mw=cw; mh=ch;
    }
    int bg=ss.bg;
    HBRUSH bb=CreateSolidBrush(RGB(bg,bg,bg)); FillRect(mdc,&cl,bb); DeleteObject(bb);
    float sc=std::min((float)cw/GW,(float)ch/GH);
    int ox=(int)((cw-GW*sc)/2), oy=(int)((ch-GH*sc)/2);
    SetGraphicsMode(mdc,GM_ADVANCED);
    XFORM xf={sc,0,0,sc,(float)ox,(float)oy}; SetWorldTransform(mdc,&xf);
    HPEN dp=CreatePen(PS_DASH,2,RGB(51,51,51));
    HPEN op=(HPEN)SelectObject(mdc,dp);
    MoveToEx(mdc,GW/2,0,NULL); LineTo(mdc,GW/2,GH);
    SelectObject(mdc,op); DeleteObject(dp);
    HBRUSH pb; RECT r;
    pb=CreateSolidBrush(ss.p1C()); r={(int)lp.x,(int)lp.y,(int)(lp.x+lp.w),(int)(lp.y+lp.h)}; FillRect(mdc,&r,pb); DeleteObject(pb);
    pb=CreateSolidBrush(ss.p2C()); r={(int)rp.x,(int)rp.y,(int)(rp.x+rp.w),(int)(rp.y+rp.h)}; FillRect(mdc,&r,pb); DeleteObject(pb);
    pb=CreateSolidBrush(ss.bC()); r={(int)bl.x,(int)bl.y,(int)(bl.x+bl.s),(int)(bl.y+bl.s)}; FillRect(mdc,&r,pb); DeleteObject(pb);
    ModifyWorldTransform(mdc,NULL,MWT_IDENTITY);
    SetBkMode(mdc,TRANSPARENT); SetTextColor(mdc,ss.sC());
    std::string lss=std::to_string(ls), rss=std::to_string(rs);
    HFONT sf=CreateFontA((int)(48*sc),0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,FIXED_PITCH,"Courier New");
    HFONT of=(HFONT)SelectObject(mdc,sf);
    TextOutA(mdc,ox+(int)((GW/4-(int)lss.length()*14)*sc),oy,lss.c_str(),(int)lss.length());
    TextOutA(mdc,ox+(int)((GW*3/4-(int)rss.length()*14)*sc),oy,rss.c_str(),(int)rss.length());
    HFONT nf=CreateFontA((int)(11*sc),0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,FIXED_PITCH,"Courier New");
    SelectObject(mdc,nf);
    SetTextColor(mdc,RGB(GetRValue(ss.sC())*0.7,GetGValue(ss.sC())*0.7,GetBValue(ss.sC())*0.7));
    TextOutA(mdc,ox+(int)((GW/4-(int)ss.p1n.length()*3)*sc),oy+(int)(62*sc),ss.p1n.c_str(),(int)ss.p1n.length());
    std::string p2l=tp?ss.p2n:"CPU";
    TextOutA(mdc,ox+(int)((GW*3/4-(int)p2l.length()*3)*sc),oy+(int)(62*sc),p2l.c_str(),(int)p2l.length());
    SelectObject(mdc,of); DeleteObject(sf); DeleteObject(nf);
    if(ss.tl>0&&st==PLAY){
        char buf[32]; snprintf(buf,sizeof(buf),"%02d:%02d",trem/60,trem%60);
        HFONT tf=CreateFontA((int)(12*sc),0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,FIXED_PITCH,"Courier New");
        SelectObject(mdc,tf); SetTextColor(mdc,ss.sC());
        TextOutA(mdc,ox+(int)(GW/2*sc)-30,oy+(int)((GH-20)*sc),buf,(int)strlen(buf));
        DeleteObject(tf);
    }
    if(st==PAUSED){
        HFONT pf=CreateFontA((int)(20*sc),0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,FIXED_PITCH,"Courier New");
        SelectObject(mdc,pf); SetTextColor(mdc,RGB(255,255,255));
        TextOutA(mdc,ox+(int)(GW/2*sc)-70,oy+(int)(GH/2*sc),"-- PAUSED --",12);
        DeleteObject(pf);
    }
    BitBlt(dc,0,0,cw,ch,mdc,0,0,SRCCOPY);
}

void drawMenu(HDC dc) {
    RECT cl; GetClientRect(hw,&cl);
    int cw=cl.right-cl.left, ch=cl.bottom-cl.top;
    HBRUSH bb=CreateSolidBrush(RGB(0,0,0)); FillRect(dc,&cl,bb); DeleteObject(bb);
    SetBkMode(dc,TRANSPARENT);
    int fsz=std::min(cw,ch)/10;
    HFONT ttf=CreateFontA(std::max(28,fsz),0,0,0,FW_BOLD,FALSE,FALSE,FALSE,ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,FIXED_PITCH,"Courier New");
    SelectObject(dc,ttf); SetTextColor(dc,RGB(255,255,255));
    int tw=4*fsz/2; TextOutA(dc,(cw-tw)/2,ch/6,"PONG",4);
    if(st==GAMEOVER){
        std::string msg;
        if(ls>rs) msg=ss.p1n+" WINS!";
        else if(rs>ls) msg=(tp?ss.p2n:"COMPUTER")+" WINS!";
        else msg="TIE!";
        std::string scs=std::to_string(ls)+" - "+std::to_string(rs);
        HFONT wf=CreateFontA(std::max(18,fsz*3/5),0,0,0,FW_BOLD,FALSE,FALSE,FALSE,ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,FIXED_PITCH,"Courier New");
        SelectObject(dc,wf); SetTextColor(dc,RGB(255,200,100));
        int mw=(int)msg.length()*fsz/3; TextOutA(dc,(cw-mw)/2,ch/3,msg.c_str(),(int)msg.length());
        SetTextColor(dc,RGB(200,200,200));
        int sw=(int)scs.length()*fsz/3; TextOutA(dc,(cw-sw)/2,ch/3+fsz*2/3,scs.c_str(),(int)scs.length());
        DeleteObject(wf);
    }
    DeleteObject(ttf);
    int bw=std::min(220,cw*3/4), bh=std::max(28,ch/18), bx=(cw-bw)/2;
    Btn b[3] = {{{bx,ch*7/12,bx+bw,ch*7/12+bh},"1 PLAYER"},
                {{bx,ch*7/12+bh+10,bx+bw,ch*7/12+bh*2+10},"2 PLAYERS"},
                {{bx,ch*7/12+bh*2+24,bx+bw,ch*7/12+bh*3+24},"SETTINGS"}};
    for(int i=0;i<3;i++) drwBtn(dc,b[i],hoverBtn==i,cw);
    HFONT bf=CreateFontA(std::max(10,ch/40),0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,FIXED_PITCH,"Courier New");
    SelectObject(dc,bf); SetTextColor(dc,RGB(80,80,80));
    RECT ri={0,ch-36,cw,ch-10};
    DrawTextA(dc,"W/S - MOVE    |    P - PAUSE    |    ESC - MENU",-1,&ri,DT_CENTER|DT_VCENTER|DT_SINGLELINE);
    DeleteObject(bf);
}

void drawSettings(HDC dc) {
    RECT cl; GetClientRect(hw,&cl);
    int cw=cl.right-cl.left, ch=cl.bottom-cl.top;
    HBRUSH bb=CreateSolidBrush(RGB(10,10,10)); FillRect(dc,&cl,bb); DeleteObject(bb);
    SetBkMode(dc,TRANSPARENT);
    int fs=std::min(28,cw/20);
    HFONT tf=CreateFontA(fs,0,0,0,FW_BOLD,FALSE,FALSE,FALSE,ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,FIXED_PITCH,"Courier New");
    SelectObject(dc,tf); SetTextColor(dc,RGB(255,255,255));
    TextOutA(dc,(cw-9*fs/2)/2,ch/20,"SETTINGS",8); DeleteObject(tf);
    auto di=[&](int y,const char* l,const char* v,bool h=false){
        HFONT lf=CreateFontA(std::max(12,ch/35),0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,FIXED_PITCH,"Courier New");
        SelectObject(dc,lf); SetTextColor(dc,RGB(120,120,120));
        TextOutA(dc,cw/10,y,l,(int)strlen(l));
        RECT r={cw*4/10,y,cw*9/10,y+ch/25};
        SetTextColor(dc,h?RGB(100,255,100):RGB(200,200,200));
        DrawTextA(dc,v,-1,&r,DT_LEFT|DT_VCENTER|DT_SINGLELINE);
        DeleteObject(lf);
    };
    char buf[128]; int y=ch/20+fs+ch/20, stp=ch/20;
    snprintf(buf,sizeof(buf),"PLAYER 1  --  [1]%s",cN[ss.p1c]); di(y,"P1 COLOR",buf); y+=stp;
    snprintf(buf,sizeof(buf),"PLAYER 2  --  [2]%s",cN[ss.p2c]); di(y,"P2 COLOR",buf); y+=stp;
    snprintf(buf,sizeof(buf),"%s  [3]",cN[ss.bc]); di(y,"BALL COLOR",buf); y+=stp;
    snprintf(buf,sizeof(buf),"%s  [4]",cN[ss.sc]); di(y,"SCORE COLOR",buf); y+=stp;
    snprintf(buf,sizeof(buf),"%d  [5]",ss.sl); di(y,"SCORE TO WIN",buf); y+=stp;
    snprintf(buf,sizeof(buf),"%s  [6]",ss.tl?(std::to_string(ss.tl)+"s").c_str():"OFF"); di(y,"TIME LIMIT",buf); y+=stp;
    snprintf(buf,sizeof(buf),"%d  [B]",ss.bspd); di(y,"BALL SPEED",buf); y+=stp;
    snprintf(buf,sizeof(buf),"%dpx  [P]",ss.psz); di(y,"PADDLE SIZE",buf); y+=stp;
    snprintf(buf,sizeof(buf),"%s  [A]",ss.ai<=3?"EASY":ss.ai<=6?"NORMAL":ss.ai<=9?"HARD":"INSANE"); di(y,"AI DIFFICULTY",buf); y+=stp;
    snprintf(buf,sizeof(buf),"%d  [K]",ss.pspd); di(y,"PADDLE SPEED",buf); y+=stp;
    snprintf(buf,sizeof(buf),"%s  [S]",ss.snd?"ON":"OFF"); di(y,"SOUND",buf); y+=stp;
    int bw=std::min(200,cw/2), bh=std::max(28,ch/18), bx=(cw-bw)/2, by=ch-bh-ch/15;
    Btn bb2={{bx,by,bx+bw,by+bh},"BACK TO MENU"};
    drwBtn(dc,bb2,hoverBtn==0,cw);
    HFONT bf=CreateFontA(std::max(10,ch/40),0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,FIXED_PITCH,"Courier New");
    SelectObject(dc,bf); SetTextColor(dc,RGB(80,80,80));
    RECT rf={cw*6/10,ch/5,cw-20,ch*4/5};
    DrawTextA(dc,"Press [1-6] Colors/Score/Timer\n\n[B]Ball Speed\n[P]Paddle Size\n[A]AI Difficulty\n[K]Paddle Speed\n[S]Sound On/Off",-1,&rf,DT_RIGHT|DT_VCENTER|DT_WORDBREAK);
    DeleteObject(bf);
}

void hsk(int vk) {
    if(st!=SETTINGS) return;
    bool ch=false;
    auto cc=[&](int& idx){idx=(idx+1)%NC;ch=true;};
    if(vk=='1'){cc(ss.p1c);} else if(vk=='2'){cc(ss.p2c);}
    else if(vk=='3'){cc(ss.bc);} else if(vk=='4'){cc(ss.sc);}
    else if(vk=='5'){int n[]={3,5,7,9,11,15,21,27};int i=0;while(i<7&&n[i]<=ss.sl)i++;ss.sl=n[i%8];ch=true;}
    else if(vk=='6'){ss.tl=ss.tl?0:60;ch=true;}
    else if(vk=='B'||vk=='b'){ss.bspd=ss.bspd>=20?4:ss.bspd+2;ch=true;}
    else if(vk=='P'||vk=='p'){ss.psz=ss.psz>=140?50:ss.psz+10;ch=true;}
    else if(vk=='A'||vk=='a'){ss.ai=ss.ai>=10?1:ss.ai+1;ch=true;}
    else if(vk=='K'||vk=='k'){ss.pspd=ss.pspd>=12?3:ss.pspd+1;ch=true;}
    else if(vk=='S'||vk=='s'){ss.snd=!ss.snd;ch=true;}
    if(ch) InvalidateRect(hw,NULL,TRUE);
}

LRESULT CALLBACK WndProc(HWND h, UINT msg, WPARAM wp, LPARAM lp) {
    switch(msg){
    case WM_KEYDOWN:
        k[wp&0xFF]=true;
        if(st==SETTINGS){hsk((int)wp);break;}
        if(wp=='P'||wp=='p'){if(st==PLAY)st=PAUSED;else if(st==PAUSED)st=PLAY;InvalidateRect(h,NULL,TRUE);}
        if(wp==VK_ESCAPE){
            if(st==SETTINGS){st=MENU;InvalidateRect(h,NULL,TRUE);}
            else if(st==PLAY||st==PAUSED){st=MENU;if(t1){KillTimer(h,t1);t1=0;}InvalidateRect(h,NULL,TRUE);}
            else if(st==GAMEOVER){st=MENU;InvalidateRect(h,NULL,TRUE);}
        }
        if(wp==VK_RETURN||wp==VK_SPACE){if(st==GAMEOVER){st=MENU;InvalidateRect(h,NULL,TRUE);}}
        break;
    case WM_KEYUP: k[wp&0xFF]=false; break;
    case WM_ERASEBKGND: return TRUE;
    case WM_SIZE:
        InvalidateRect(h, NULL, TRUE);
        UpdateWindow(h);
        break;
    case WM_TIMER:
        if(wp==1&&st==PLAY){trem--;if(trem<=0){trem=0;KillTimer(h,t1);t1=0;st=GAMEOVER;swin();}}
        if(wp==2){upd();}
        break;
    case WM_MOUSEMOVE:{
        int nx=GET_X_LPARAM(lp), ny=GET_Y_LPARAM(lp);
        if(st==MENU||st==GAMEOVER){
            RECT cl; GetClientRect(h,&cl);
            int cw=cl.right-cl.left, ch=cl.bottom-cl.top;
            int bw=std::min(220,cw*3/4), bh=std::max(28,ch/18), bx=(cw-bw)/2;
            int oh=hoverBtn; hoverBtn=-1;
            if(ph({nx,ny},{bx,ch*7/12,bx+bw,ch*7/12+bh})) hoverBtn=0;
            if(ph({nx,ny},{bx,ch*7/12+bh+10,bx+bw,ch*7/12+bh*2+10})) hoverBtn=1;
            if(ph({nx,ny},{bx,ch*7/12+bh*2+24,bx+bw,ch*7/12+bh*3+24})) hoverBtn=2;
            if(hoverBtn!=oh) InvalidateRect(h,NULL,TRUE);
        } else if(st==SETTINGS){
            RECT cl; GetClientRect(h,&cl);
            int cw=cl.right-cl.left, ch=cl.bottom-cl.top;
            int bw=std::min(200,cw/2), bh=std::max(28,ch/18), bx=(cw-bw)/2, by=ch-bh-ch/15;
            int oh=hoverBtn; hoverBtn=ph({nx,ny},{bx,by,bx+bw,by+bh})?0:-1;
            if(hoverBtn!=oh) InvalidateRect(h,NULL,TRUE);
        }
        mx=nx; my=ny;
        break;
    }
    case WM_LBUTTONDOWN:{
        int x=GET_X_LPARAM(lp), y=GET_Y_LPARAM(lp);
        if(st==MENU||st==GAMEOVER){
            RECT cl; GetClientRect(h,&cl);
            int cw=cl.right-cl.left, ch=cl.bottom-cl.top;
            int bw=std::min(220,cw*3/4), bh=std::max(28,ch/18), bx=(cw-bw)/2;
            POINT pt={x,y};
            if(ph(pt,{bx,ch*7/12,bx+bw,ch*7/12+bh})){tp=false;rgame();}
            else if(ph(pt,{bx,ch*7/12+bh+10,bx+bw,ch*7/12+bh*2+10})){tp=true;rgame();}
            else if(ph(pt,{bx,ch*7/12+bh*2+24,bx+bw,ch*7/12+bh*3+24})){st=SETTINGS;InvalidateRect(h,NULL,TRUE);}
        }else if(st==SETTINGS){
            RECT cl; GetClientRect(h,&cl);
            int cw=cl.right-cl.left, ch=cl.bottom-cl.top;
            int bw=std::min(200,cw/2), bh=std::max(28,ch/18), bx=(cw-bw)/2, by=ch-bh-ch/15;
            if(ph({x,y},{bx,by,bx+bw,by+bh})){st=MENU;InvalidateRect(h,NULL,TRUE);}
        }
        break;
    }
    case WM_PAINT:{
        PAINTSTRUCT ps; HDC dc=BeginPaint(h,&ps);
        if(st==MENU||st==GAMEOVER) drawMenu(dc);
        else if(st==SETTINGS) drawSettings(dc);
        else drawGame(dc);
        EndPaint(h,&ps);
        break;
    }
    case WM_DESTROY: if(t1) KillTimer(h,t1); PostQuitMessage(0); break;
    default: return DefWindowProc(h,msg,wp,lp);
    }
    return 0;
}

void save(){
    std::ofstream f("pong_settings.ini"); if(!f)return;
    f<<ss.p1n<<std::endl<<ss.p2n<<std::endl;
    f<<ss.p1c<<std::endl<<ss.p2c<<std::endl<<ss.bc<<std::endl<<ss.sc<<std::endl;
    f<<ss.bg<<std::endl<<ss.sl<<std::endl<<ss.tl<<std::endl<<ss.bspd<<std::endl<<ss.psz<<std::endl<<ss.ai<<std::endl<<ss.pspd<<std::endl<<ss.snd<<std::endl;
}
void load(){
    std::ifstream f("pong_settings.ini"); if(!f)return;
    std::getline(f,ss.p1n); std::getline(f,ss.p2n);
    f>>ss.p1c>>ss.p2c>>ss.bc>>ss.sc;
    ss.p1c=std::min(NC-1,std::max(0,ss.p1c));
    ss.p2c=std::min(NC-1,std::max(0,ss.p2c));
    ss.bc=std::min(NC-1,std::max(0,ss.bc));
    ss.sc=std::min(NC-1,std::max(0,ss.sc));
    f>>ss.bg>>ss.sl>>ss.tl>>ss.bspd>>ss.psz>>ss.ai>>ss.pspd>>ss.snd;
}

int WINAPI WinMain(HINSTANCE hi, HINSTANCE, LPSTR, int ns) {
    srand((unsigned)time(NULL));
    sDef=ss; load(); apply();
    WNDCLASSA wc={}; wc.lpfnWndProc=WndProc; wc.hInstance=hi; wc.lpszClassName="PongWindow"; wc.hCursor=LoadCursor(NULL,IDC_HAND);
    RegisterClassA(&wc);
    RECT wr={0,0,800,500}; AdjustWindowRect(&wr,WS_OVERLAPPEDWINDOW,FALSE);
    hw=CreateWindowExA(0,"PongWindow","PONG",WS_OVERLAPPEDWINDOW,CW_USEDEFAULT,CW_USEDEFAULT,wr.right-wr.left,wr.bottom-wr.top,NULL,NULL,hi,NULL);
    if(!hw)return 0;
    ShowWindow(hw,ns); UpdateWindow(hw);
    SetTimer(hw,2,16,NULL);
    MSG msg={}; while(GetMessage(&msg,NULL,0,0)){TranslateMessage(&msg);DispatchMessage(&msg);}
    save(); return 0;
}
