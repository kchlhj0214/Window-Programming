// ============================================================
 // main.cpp  ─  쇼본의 액션 Win32 변환판
 // ============================================================
#include "main.h"
#include <stdarg.h>

// ────────────────────────────────────────────────────────────
// 창 클래스 / 타이틀 문자열
// ────────────────────────────────────────────────────────────
static LPCTSTR lpszClass = L"SyobonCatMario";
static LPCTSTR lpszWindowName = L"쇼본의 액션 (Win32)";

// ============================================================
// §1  전역 변수 정의
// ============================================================

HINSTANCE g_hInst = NULL;
HWND      g_hWnd = NULL;
HDC       g_drawDC = NULL;   // RenderFrame 내에서만 유효

CImage    g_img[MAX_SHEETS];
int       g_imgCount = 0;

SpriteRect g_handles[MAX_SPRITE_HANDLES];
int        g_handleCount = 0;

COLORREF   g_transColor = RGB(153, 255, 255);

HFONT g_fontNormal = NULL;
HFONT g_fontLarge = NULL;

// ── 그리기 ──────────────────────────────────────────────────
COLORREF color = RGB(255, 255, 255);
int      mirror = 0;

// ── DxLib 스프라이트 테이블 ──────────────────────────────────
int grap[161][8];
int mgrap[51];

// ── 사운드 핸들 ─────────────────────────────────────────────
int oto[151];

// ── 게임 상태 ────────────────────────────────────────────────
// 100=타이틀, 10=목숨화면, 1=게임중, 2=스태프롤
int main_state = 100;
int maintm = 0;

// ── 스테이지 ─────────────────────────────────────────────────
int stagecolor = 0;
int sta = 1, stb = 1, stc = 0;

// ── 설정 ─────────────────────────────────────────────────────
int fast = 1, trap = 1, tyuukan = 0, ending = 0;
int stagerr = 0, stagepoint = 0;
int over = 0, stageonoff = 0;
int maint = 0;

// ── 루프 임시 변수 ───────────────────────────────────────────
int    t = 0, tt = 0, t1 = 0, t2 = 0, t3 = 0, t4 = 0;
int    zxon = 0, zzxon = 0;
int    key = 0, keytm = 0;
double pai = 3.1415926535;

// ── 지면 플랫폼 배열 ─────────────────────────────────────────
int sx = 0, sco = 0;
int sa[smax], sb[smax], sc[smax], sd[smax];
int stype[smax], sxtype[smax], sr[smax];
int sgtype[smax];

// ── 플레이어 ─────────────────────────────────────────────────
int mainmsgtype = 0;
int ma = 0, mb = 0, mnobia = 3000, mnobib = 3600, mhp = 1;
int mc = 0, md = 0, macttype = 0, atkon = 0, atktm = 0;
int mactsok = 0, msstar = 0, nokori = 2, mactp = 0, mact = 0;
int mtype = 0, mxtype = 0, mtm = 0, mzz = 0;
int mzimen = 0, mrzimen = 0, mkasok = 0;
int mmuki = 1, mmukitm = 0, mjumptm = 0;
int mkeytm = 0, mcleartm = 0;
int mmutekitm = 0, mmutekion = 0;
int mztm = 0, mztype = 0;
int actaon[7] = {};
int mmsgtm = 0, mmsgtype = 0;
int mascrollmax = 21000;

// ── 블록 배열 ────────────────────────────────────────────────
int tco = 0;
int ta[tmax], tb[tmax], tc[tmax], td[tmax];
int thp[tmax], ttype[tmax];
int titem[tmax], txtype[tmax];
int tmsgtm = 0, tmsgtype = 0, tmsgx = 0, tmsgy = 0;
int tmsgnobix = 0, tmsgnobiy = 0, tmsg = 0;

// ── 이펙트 배열 (코인·파편 등) ──────────────────────────────
int eco = 0;
int ea[emax], eb[emax], enobia[emax], enobib[emax];
int ec[emax], ed[emax], ee[emax], ef[emax], etm[emax];
int egtype[emax];

// ── 적 배열 ──────────────────────────────────────────────────
int aco = 0;
int aa[amax], ab[amax], anobia[amax], anobib[amax];
int ac[amax], ad[amax], ae[amax], af[amax];
int abrocktm[amax];
int aacta[amax], aactb[amax], azimentype[amax], axzimen[amax];
int atype[amax], axtype[amax], amuki[amax], ahp[amax];
int anotm[amax], anx[160], any[160];
int atm[amax], a2tm[amax];
int amsgtm[amax], amsgtype[amax];

// ── 탄환 배열 ────────────────────────────────────────────────
int bco = 0;
int ba[bmax], bb[bmax], btm[bmax];
int btype[bmax], bxtype[bmax], bz[bmax];

// ── 배경 오브젝트 배열 ───────────────────────────────────────
int nxxmax = 0, nco = 0;
int na[nmax], nb[nmax], nc[nmax], nd[nmax], ntype[nmax];
int ne[nmax], nf[nmax], ng[nmax], nx[nmax];

// ── 리프트 배열 ──────────────────────────────────────────────
int srco = 0;
int sra[srmax], srb[srmax], src[srmax], srd[srmax];
int sre[srmax], srf[srmax];
int srtype[srmax], srgtype[srmax], sracttype[srmax], srsp[srmax];
int srmuki[srmax], sron[srmax], sree[srmax];
int srsok[srmax], srmovep[srmax], srmove[srmax];

// ── 스크롤 / 화면 ────────────────────────────────────────────
int fx = 0, fy = 0, fzx = 0, fzy = 0, scrollx = 0, scrolly = 0;
int fma = 0, fmb = 0;
int kscroll = 0;
int fxmax = 48000, fymax = 42000;

// 스테이지 타일 데이터
byte stagedate[17][2001];

// 화면 페이드 아웃
int blacktm = 1, blackx = 0;

// 공용 임시 변수
int    xx[91];
double xd[11];
string xs[31];

// 타이머
long stime = 0;


// ============================================================
// §2  함수 전방 선언 (Forward Declarations)
// ============================================================

// Win32 진입점 / 창
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void InitGame();
void UpdateGame();
void RenderFrame(HDC hDC, RECT& rt);

// 스프라이트 드로우 내부 함수
void DrawSprite(int handle, int x, int y);
void DrawSpriteFlip(int handle, int x, int y);

// 그리기 헬퍼
void setcolor(int red, int green, int blue);
void setc0(); void setc1();
void setfont(int a); void setfont(int x, int y);
void drawpixel(int a, int b);
void drawline(int a, int b, int c, int d);
void drawrect(int a, int b, int c, int d);
void fillrect(int a, int b, int c, int d);
void drawarc(int a, int b, int c, int d);
void fillarc(int a, int b, int c, int d);
void drawimage(int mx, int a, int b);
void drawimage(int mx, int a, int b, int c, int d, int e, int f);
int  loadimage(string b);
int  loadimage(int a, int x, int y, int r, int z);
void setre(); void setre2(); void setno();
void str(string c, int a, int b);
void DrawFormatString(int x, int y, COLORREF col, const char* fmt, ...);

// 사운드 / 유틸
void ot(int x);
void bgmchange(int x);
void end();
static void wait(int interval);
static void wait2(long st, long et, int FLAME_TIME);
static int  rand(int Rand);

// 게임 오브젝트 생성
void tyobi(int x, int y, int type);
void brockbreak(int t_idx);
void eyobi(int xa, int xb, int xc, int xd_v, int xe, int xf,
    int xnobia, int xnobib, int xgtype, int xtm);
void ayobi(int xa, int xb, int xc, int xd_v, int xnotm,
    int xtype, int xxtype);
void ttmsg();
void txmsg(string x, int a);

// 게임 로직
void Mainprogram();
void rpaint();
void stagecls();
void stage();
void stagep();
void tekizimen();


// ============================================================
// §3  WinMain  ─  프로그램 진입점
// ============================================================
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    LPSTR lpszCmdParam, int nCmdShow)
{
    HWND hWnd;
    MSG  msg;
    WNDCLASSEX WndClass;

    g_hInst = hInstance;

    WndClass.cbSize = sizeof(WndClass);
    WndClass.style = CS_HREDRAW | CS_VREDRAW;
    WndClass.lpfnWndProc = (WNDPROC)WndProc;
    WndClass.cbClsExtra = 0;
    WndClass.cbWndExtra = 0;
    WndClass.hInstance = hInstance;
    WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    WndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    WndClass.lpszMenuName = NULL;
    WndClass.lpszClassName = lpszClass;
    WndClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    RegisterClassEx(&WndClass);

    // 클라이언트 영역이 정확히 480×420 이 되도록 창 크기 계산
    RECT rc = { 0, 0, WIN_W, WIN_H };
    AdjustWindowRect(&rc,
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        FALSE);

    hWnd = CreateWindow(
        lpszClass, lpszWindowName,
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT,
        rc.right - rc.left, rc.bottom - rc.top,
        NULL, NULL, hInstance, NULL);

    g_hWnd = hWnd;
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    RECT rt;
    GetClientRect(hWnd, &rt);

    while (true) {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) break;
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else {
            if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) break;

            UpdateGame();

            HDC hDC = GetDC(hWnd);
            RenderFrame(hDC, rt);
            ReleaseDC(hWnd, hDC);

            Sleep(33);   // ~30 fps
        }
    }

    if (g_fontNormal) DeleteObject(g_fontNormal);
    if (g_fontLarge)  DeleteObject(g_fontLarge);

    return (int)msg.wParam;
}


// ============================================================
// §4  WndProc  ─  윈도우 프로시저
// ============================================================
LRESULT CALLBACK WndProc(HWND hWnd, UINT iMsg,
    WPARAM wParam, LPARAM lParam)
{
    switch (iMsg) {
    case WM_CREATE:
        InitGame();
        return 0;

    case WM_ERASEBKGND:
        return 1;   // 더블버퍼링: 배경 지우기 억제

    case WM_GETMINMAXINFO: {
        MINMAXINFO* mmi = (MINMAXINFO*)lParam;
        RECT rc = { 0, 0, WIN_W, WIN_H };
        AdjustWindowRect(&rc,
            WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
            FALSE);
        mmi->ptMinTrackSize.x = rc.right - rc.left;
        mmi->ptMinTrackSize.y = rc.bottom - rc.top;
        mmi->ptMaxTrackSize = mmi->ptMinTrackSize;
        return 0;
    }

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hWnd, iMsg, wParam, lParam);
}


// ============================================================
// §5  InitGame / UpdateGame / RenderFrame  ─  프레임 구조
// ============================================================
void InitGame()
{
    srand((unsigned)timeGetTime());

    g_fontNormal = CreateFont(
        16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"굴림");

    g_fontLarge = CreateFont(
        20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"굴림");

    memset(grap, 0, sizeof(grap));
    memset(mgrap, 0, sizeof(mgrap));
    memset(oto, 0, sizeof(oto));
    memset(stagedate, 0, sizeof(stagedate));

    loadg();   // loadg.cpp 에서 이미지·사운드 로드
}

void UpdateGame()
{
    maint = 0;
    Mainprogram();
}

void RenderFrame(HDC hDC, RECT& rt)
{
    int w = rt.right - rt.left;
    int h = rt.bottom - rt.top;
    if (w <= 0 || h <= 0) return;

    HDC     memDC = CreateCompatibleDC(hDC);
    HBITMAP memBmp = CreateCompatibleBitmap(hDC, w, h);
    HBITMAP oldBmp = (HBITMAP)SelectObject(memDC, memBmp);
    HFONT   oldFnt = (HFONT)SelectObject(memDC, g_fontNormal);

    g_drawDC = memDC;
    rpaint();
    g_drawDC = NULL;

    BitBlt(hDC, 0, 0, w, h, memDC, 0, 0, SRCCOPY);

    SelectObject(memDC, oldFnt);
    SelectObject(memDC, oldBmp);
    DeleteObject(memBmp);
    DeleteDC(memDC);
}


// ============================================================
// §6  스프라이트 드로우 함수
// ============================================================
void DrawSprite(int handle, int x, int y)
{
    if (handle < 0 || handle >= g_handleCount) return;
    const SpriteRect& r = g_handles[handle];
    if (r.sheet < 0 || r.sheet >= g_imgCount) return;
    if (g_img[r.sheet].IsNull()) return;

    HDC srcDC = g_img[r.sheet].GetDC();
    TransparentBlt(g_drawDC, x, y, r.sw, r.sh,
        srcDC, r.sx, r.sy, r.sw, r.sh, g_transColor);
    g_img[r.sheet].ReleaseDC();
}

void DrawSpriteFlip(int handle, int x, int y)
{
    if (handle < 0 || handle >= g_handleCount) return;
    const SpriteRect& r = g_handles[handle];
    if (r.sheet < 0 || r.sheet >= g_imgCount) return;
    if (g_img[r.sheet].IsNull()) return;

    HDC srcDC = g_img[r.sheet].GetDC();

    HDC     tmpDC = CreateCompatibleDC(g_drawDC);
    HBITMAP tmpBmp = CreateCompatibleBitmap(g_drawDC, r.sw, r.sh);
    HBITMAP oldBmp = (HBITMAP)SelectObject(tmpDC, tmpBmp);

    // 투명색으로 배경 채우기
    HBRUSH br = CreateSolidBrush(g_transColor);
    RECT   rc = { 0, 0, r.sw, r.sh };
    FillRect(tmpDC, &rc, br);
    DeleteObject(br);

    // 수평 반전 복사 (음수 너비 → StretchBlt 좌우 뒤집기)
    StretchBlt(tmpDC, r.sw - 1, 0, -r.sw, r.sh,
        srcDC, r.sx, r.sy, r.sw, r.sh, SRCCOPY);

    g_img[r.sheet].ReleaseDC();

    TransparentBlt(g_drawDC, x, y, r.sw, r.sh,
        tmpDC, 0, 0, r.sw, r.sh, g_transColor);

    SelectObject(tmpDC, oldBmp);
    DeleteObject(tmpBmp);
    DeleteDC(tmpDC);
}


// ============================================================
// §7  그리기 헬퍼 함수
// ============================================================
void setcolor(int r, int g, int b) { color = RGB(r, g, b); }
void setc0() { color = RGB(0, 0, 0); }
void setc1() { color = RGB(255, 255, 255); }
void setfont(int a) {}
void setfont(int x, int y) {}
void setre() {}
void setre2() {}
void setno() {}

void drawpixel(int a, int b) { SetPixel(g_drawDC, a, b, color); }

void drawline(int a, int b, int c, int d) {
    HPEN p = CreatePen(PS_SOLID, 1, color);
    HPEN o = (HPEN)SelectObject(g_drawDC, p);
    MoveToEx(g_drawDC, a, b, NULL); LineTo(g_drawDC, c, d);
    SelectObject(g_drawDC, o); DeleteObject(p);
}

void drawrect(int a, int b, int c, int d) {
    HPEN   p = CreatePen(PS_SOLID, 1, color);
    HBRUSH br = (HBRUSH)GetStockObject(NULL_BRUSH);
    HPEN   op = (HPEN)SelectObject(g_drawDC, p);
    HBRUSH ob = (HBRUSH)SelectObject(g_drawDC, br);
    Rectangle(g_drawDC, a, b, a + c, b + d);
    SelectObject(g_drawDC, op); SelectObject(g_drawDC, ob);
    DeleteObject(p);
}

void fillrect(int a, int b, int c, int d) {
    HPEN   p = CreatePen(PS_SOLID, 1, color);
    HBRUSH br = CreateSolidBrush(color);
    HPEN   op = (HPEN)SelectObject(g_drawDC, p);
    HBRUSH ob = (HBRUSH)SelectObject(g_drawDC, br);
    Rectangle(g_drawDC, a, b, a + c, b + d);
    SelectObject(g_drawDC, op); SelectObject(g_drawDC, ob);
    DeleteObject(p); DeleteObject(br);
}

void drawarc(int a, int b, int c, int d) {
    HPEN   p = CreatePen(PS_SOLID, 1, color);
    HBRUSH br = (HBRUSH)GetStockObject(NULL_BRUSH);
    HPEN   op = (HPEN)SelectObject(g_drawDC, p);
    HBRUSH ob = (HBRUSH)SelectObject(g_drawDC, br);
    Ellipse(g_drawDC, a - c, b - d, a + c, b + d);
    SelectObject(g_drawDC, op); SelectObject(g_drawDC, ob);
    DeleteObject(p);
}

void fillarc(int a, int b, int c, int d) {
    HPEN   p = CreatePen(PS_SOLID, 1, color);
    HBRUSH br = CreateSolidBrush(color);
    HPEN   op = (HPEN)SelectObject(g_drawDC, p);
    HBRUSH ob = (HBRUSH)SelectObject(g_drawDC, br);
    Ellipse(g_drawDC, a - c, b - d, a + c, b + d);
    SelectObject(g_drawDC, op); SelectObject(g_drawDC, ob);
    DeleteObject(p); DeleteObject(br);
}

void drawimage(int mx, int a, int b) {
    if (mx < 0) return;
    if (mirror == 0) DrawSprite(mx, a, b);
    else             DrawSpriteFlip(mx, a, b);
}

void drawimage(int mx, int a, int b, int c, int d, int e, int f) {
    int h = DerivationGraph(c, d, e, f, mx);
    drawimage(h, a, b);
}

int loadimage(string b) { return LoadGraph(b.c_str()); }
int loadimage(int a, int x, int y, int r, int z) { return DerivationGraph(x, y, r, z, a); }

void str(string c, int a, int b) {
    if (!g_drawDC) return;
    int n = MultiByteToWideChar(CP_ACP, 0, c.c_str(), -1, NULL, 0);
    if (n <= 0) return;
    wchar_t* w = new wchar_t[n];
    MultiByteToWideChar(CP_ACP, 0, c.c_str(), -1, w, n);
    SetBkMode(g_drawDC, TRANSPARENT);
    SetTextColor(g_drawDC, color);
    TextOutW(g_drawDC, a, b, w, n - 1);
    delete[] w;
}

void DrawFormatString(int x, int y, COLORREF col, const char* fmt, ...) {
    char buf[256];
    va_list args; va_start(args, fmt);
    vsprintf_s(buf, sizeof(buf), fmt, args);
    va_end(args);
    int n = MultiByteToWideChar(CP_ACP, 0, buf, -1, NULL, 0);
    if (n <= 0) return;
    wchar_t* w = new wchar_t[n];
    MultiByteToWideChar(CP_ACP, 0, buf, -1, w, n);
    SetBkMode(g_drawDC, TRANSPARENT);
    SetTextColor(g_drawDC, col);
    TextOutW(g_drawDC, x, y, w, n - 1);
    delete[] w;
}


// ============================================================
// §8  사운드 / 유틸 함수
// ============================================================
void ot(int x) { PlaySoundMem(x, DX_PLAYTYPE_BACK); }

void bgmchange(int x) {
    if (oto[0] != 0) StopSoundMem(oto[0]);
    oto[0] = x;
}

void end() { PostQuitMessage(0); }

static void wait(int ms) { Sleep(ms); }

static void wait2(long st, long et, int FLAME_TIME) {
    if (et - st < FLAME_TIME) wait(FLAME_TIME - (int)(et - st));
}

static int rand(int Rand) { return GetRand(Rand); }


// ============================================================
// §9  오브젝트 생성 헬퍼 함수
// ============================================================
void tyobi(int x, int y, int type) {
    if (tco >= tmax) tco = 0;
    ta[tco] = x * 100; tb[tco] = y * 100; ttype[tco] = type; txtype[tco] = 0;
    tco++;
}

void brockbreak(int t_idx) {
    if (t_idx < 0 || t_idx >= tmax) return;
    ta[t_idx] = -9000000;
    ttype[t_idx] = 3;
}

void eyobi(int xa, int xb, int xc, int xd_v, int xe, int xf,
    int xnobia, int xnobib, int xgtype, int xtm)
{
    ea[eco] = xa; eb[eco] = xb; ec[eco] = xc; ed[eco] = xd_v;
    ee[eco] = xe; ef[eco] = xf;
    enobia[eco] = xnobia; enobib[eco] = xnobib;
    egtype[eco] = xgtype; etm[eco] = xtm;
    eco++; if (eco >= emax) eco = 0;
}

void ayobi(int xa, int xb, int xc, int xd_v, int xnotm,
    int xtype, int xxtype)
{
    aa[aco] = xa;    ab[aco] = xb;    ac[aco] = xc;   ad[aco] = xd_v;
    anotm[aco] = xnotm; atype[aco] = xtype; axtype[aco] = xxtype;
    amuki[aco] = 0;  azimentype[aco] = 1;
    anobia[aco] = anx[xtype]; anobib[aco] = any[xtype];
    amsgtm[aco] = 0; abrocktm[aco] = 0; atm[aco] = 0; a2tm[aco] = 0;
    aco++; if (aco >= amax) aco = 0;
}

void ttmsg() { /* 다음 단계에서 구현 */ }
void txmsg(string x, int a) {}


// ============================================================
// §10  stagecls()  ─  배열 초기화
// ============================================================
void stagecls() {
    for (t = 0; t < smax; t++) { sa[t] = -9000000; sb[t] = 1; sc[t] = 1; sd[t] = 1; sgtype[t] = 0; stype[t] = 0; sxtype[t] = 0; }
    for (t = 0; t < tmax; t++) { ta[t] = -9000000; tb[t] = 1; tc[t] = 1; td[t] = 1; titem[t] = 0; txtype[t] = 0; ttype[t] = 0; }
    for (t = 0; t < srmax; t++) {
        sra[t] = -9000000; srb[t] = 1; src[t] = 1; srd[t] = 1; sre[t] = 0; srf[t] = 0;

        srmuki[t] = 0; sron[t] = 0; sree[t] = 0; srsok[t] = 0; srmove[t] = 0; srmovep[t] = 0; srsp[t] = 0; sracttype[t] = 0;
    }
    for (t = 0; t < amax; t++) {
        aa[t] = -9000000; ab[t] = 1; ac[t] = 0; ad[t] = 1; azimentype[t] = 0; atype[t] = 0; axtype[t] = 0;
        ae[t] = 0; af[t] = 0; atm[t] = 0; a2tm[t] = 0; abrocktm[t] = 0; amsgtm[t] = 0;
    }
    for (t = 0; t < bmax; t++) { ba[t] = -9000000; bb[t] = 1; bz[t] = 1; btm[t] = 0; bxtype[t] = 0; }
    for (t = 0; t < emax; t++) { ea[t] = -9000000; eb[t] = 1; ec[t] = 1; ed[t] = 1; egtype[t] = 0; }
    for (t = 0; t < nmax; t++) { na[t] = -9000000; nb[t] = 1; nc[t] = 1; nd[t] = 1; ne[t] = 1; nf[t] = 1; ng[t] = 0; ntype[t] = 0; }
    sco = 0; tco = 0; aco = 0; bco = 0; eco = 0; nco = 0;
}


// ============================================================
// §11  stagep() / stage() / tekizimen()  ─  스텁
//       (다음 단계에서 채울 구현)
// ============================================================
void stagep() { /* 2단계: 스테이지 데이터 */ }
void tekizimen() { /* 3단계: 적 지면 충돌    */ }

void stage() {
    scrollx = 3600 * 100;
    stagep();

    for (tt = 0; tt <= 1000; tt++) {
        for (t = 0; t <= 16; t++) {
            int v = (int)stagedate[t][tt];
            if (!v) continue;
            int px = tt * 29, py = t * 29 - 12;

            if (v >= 1 && v <= 19 && v != 9) tyobi(px, py, v);
            if (v >= 20 && v <= 29 && srco < srmax) { sra[srco] = px * 100; srb[srco] = py * 100; src[srco] = 3000; srtype[srco] = 0; srco++; }
            if (v == 30 && sco < smax) { sa[sco] = px * 100; sb[sco] = py * 100; sc[sco] = 3000; sd[sco] = 6000; stype[sco] = 500; sco++; }
            if (v == 40 && sco < smax) { sa[sco] = px * 100; sb[sco] = py * 100; sc[sco] = 6000; sd[sco] = 3000; stype[sco] = 1; sco++; }
            if (v == 41 && sco < smax) {
                sa[sco] = px * 100 + 500; sb[sco] = py * 100; sc[sco] = 5000; sd[sco] = 3000; stype[sco] = 2; sco++;
            }
            if (v == 44 && sco < smax) {
                sa[sco] = px * 100; sb[sco] = py * 100 + 700; sc[sco] = 3900; sd[sco] = 5000; stype[sco] = 5; sco++;
            }
            if (v >= 50 && v <= 79 && bco < bmax) { ba[bco] = px * 100; bb[bco] = py * 100; btype[bco] = v - 50; bco++; }
            if (v >= 80 && v <= 89 && nco < nmax) { na[nco] = px * 100; nb[nco] = py * 100; ntype[nco] = v - 80; nco++; }
            if (v == 9)  tyobi(px, py, 800);
            if (v == 99 && sco < smax) { sa[sco] = px * 100; sb[sco] = py * 100; sc[sco] = 3000; sd[sco] = (12 - t) * 3000; stype[sco] = 300; sco++; }
        }
    }
}
// ============================================================
  // §12  rpaint()  ─  전체 렌더링 구현
  // ============================================================
void rpaint()
{
    if (!g_drawDC) return;

    // ── 배경색 결정 ──────────────────────────────────────────
    setcolor(0, 0, 0);
    if (stagecolor == 1) setcolor(160, 180, 250);
    if (stagecolor == 2) setcolor(10, 10, 10);
    if (stagecolor == 3) setcolor(160, 180, 250);
    if (stagecolor == 4) setcolor(10, 10, 10);
    if (stagecolor == 5) setcolor(160, 180, 250);
    fillrect(0, 0, fxmax / COORD_SCALE, fymax / COORD_SCALE);

    // ════════════════════════════════════════════════════════
    // 게임 중 렌더링 (main_state == 1, zxon >= 1)
    // ════════════════════════════════════════════════════════
    if (main_state == 1 && zxon >= 1) {

        // ── 배경 오브젝트 ───────────────────────────────────
        for (t = 0; t < nmax; t++) {
            xx[0] = na[t] - fx;  xx[1] = nb[t] - fy;
            xx[2] = 16000;       xx[3] = 16000;
            if (xx[0] + xx[2] >= -10 && xx[0] <= fxmax &&
                xx[1] + xx[3] >= -10 && xx[1] <= fymax) {
                if (ntype[t] != 3)
                    drawimage(grap[ntype[t]][4], xx[0] / 100, xx[1] / 100);
                else
                    drawimage(grap[ntype[t]][4], xx[0] / 100 - 5, xx[1] / 100);
            }
        }

        // ── 이펙트 (코인·파편·포탑 등) ─────────────────────
        for (t = 0; t < emax; t++) {
            xx[0] = ea[t] - fx;  xx[1] = eb[t] - fy;
            xx[2] = enobia[t] / 100; xx[3] = enobib[t] / 100;
            if (xx[0] + xx[2] * 100 >= -10 && xx[1] <= fxmax &&
                xx[1] + xx[3] * 100 >= -10 - 8000 && xx[3] <= fymax) {

                // 코인
                if (egtype[t] == 0)
                    drawimage(grap[0][2], xx[0] / 100, xx[1] / 100);

                // 블록 파편
                if (egtype[t] == 1) {
                    if (stagecolor <= 1 || stagecolor == 3) setcolor(9 * 16, 6 * 16, 3 * 16);
                    if (stagecolor == 2) setcolor(0, 120, 160);
                    if (stagecolor == 4) setcolor(192, 192, 192);
                    fillarc(xx[0] / 100, xx[1] / 100, 7, 7);
                    setcolor(0, 0, 0);
                    drawarc(xx[0] / 100, xx[1] / 100, 7, 7);
                }

                // 리프트 파편
                if (egtype[t] == 2 || egtype[t] == 3) {
                    if (egtype[t] == 3) mirror = 1;
                    drawimage(grap[0][5], xx[0] / 100, xx[1] / 100);
                    mirror = 0;
                }

                // 포탑
                if (egtype[t] == 4) {
                    setc1();
                    fillrect(xx[0] / 100 + 10, xx[1] / 100, 10, xx[3]);
                    setc0();
                    drawrect(xx[0] / 100 + 10, xx[1] / 100, 10, xx[3]);
                    setcolor(250, 250, 0);
                    fillarc(xx[0] / 100 + 14, xx[1] / 100, 10, 10);
                    setc0();
                    drawarc(xx[0] / 100 + 14, xx[1] / 100, 10, 10);
                }
            }
        }

        // ── 리프트 ─────────────────────────────────────────
        for (t = 0; t < srmax; t++) {
            xx[0] = sra[t] - fx;  xx[1] = srb[t] - fy;
            if (xx[0] + src[t] >= -10 && xx[1] <= fxmax + 12100 && src[t] / 100 >= 1) {
                xx[2] = (srsp[t] == 1) ? 12 : 14;

                if (srsp[t] <= 9 || srsp[t] >= 20) {
                    setcolor(220, 220, 0);
                    if (srsp[t] == 2)  setcolor(0, 220, 0);
                    if (srsp[t] == 21) setcolor(180, 180, 180);
                    fillrect(xx[0] / 100, xx[1] / 100, src[t] / 100, xx[2]);
                    setcolor(180, 180, 0);
                    if (srsp[t] == 2)  setcolor(0, 180, 0);
                    if (srsp[t] == 21) setcolor(150, 150, 150);
                    drawrect(xx[0] / 100, xx[1] / 100, src[t] / 100, xx[2]);
                }
                else if (srsp[t] <= 14 && src[t] >= 5000) {
                    setcolor(0, 200, 0);
                    fillrect(xx[0] / 100, xx[1] / 100, src[t] / 100, 30);
                    setcolor(0, 160, 0);
                    drawrect(xx[0] / 100, xx[1] / 100, src[t] / 100, 30);
                    setcolor(180, 120, 60);
                    fillrect(xx[0] / 100 + 20, xx[1] / 100 + 30, src[t] / 100 - 40, 480);
                    setcolor(100, 80, 20);
                    drawrect(xx[0] / 100 + 20, xx[1] / 100 + 30, src[t] / 100 - 40, 480);
                }

                // 벽돌 리프트
                if (srsp[t] == 15) {
                    for (t2 = 0; t2 <= 2; t2++)
                        drawimage(grap[1][1], xx[0] / 100 + t2 * 29, xx[1] / 100);
                }
            }
        }

        // ── 적 캐릭터 ──────────────────────────────────────
        for (t = 0; t < amax; t++) {
            xx[0] = aa[t] - fx;  xx[1] = ab[t] - fy;
            xx[2] = anobia[t] / 100; xx[3] = anobib[t] / 100;
            if (xx[0] + xx[2] * 100 >= -10 && xx[0] <= fxmax + 3000 &&
                xx[1] + xx[3] * 100 >= -10 - 9000 && xx[1] <= fymax) {
                if (amuki[t] == 0) mirror = 1;

                // 구리보 (아형 0)
                if (atype[t] == 0)
                    drawimage(grap[0][3], xx[0] / 100, xx[1] / 100);

                // 노코노코 서있음 (아형 1)
                if (atype[t] == 1)
                    drawimage(grap[1][3], xx[0] / 100, xx[1] / 100 - 13);

                // 노코노코 껍데기 (아형 2)
                if (atype[t] == 2)
                    drawimage(grap[2][3], xx[0] / 100, xx[1] / 100);

                // 파이어바 (아형 3)
                if (atype[t] == 3) {
                    setcolor(250, 100, 0);
                    fillarc(xx[0] / 100, xx[1] / 100, 8, 8);
                }

                // 해머브로 (아형 4)
                if (atype[t] == 4)
                    drawimage(grap[4][3], xx[0] / 100, xx[1] / 100);

                // 킬러 공 (아형 5, 6, 150)
                if (atype[t] == 5)
                    drawimage(grap[5][3], xx[0] / 100, xx[1] / 100);
                if (atype[t] == 6)
                    drawimage(grap[6][3], xx[0] / 100, xx[1] / 100);
                if (atype[t] == 150)
                    drawimage(grap[150][3], xx[0] / 100, xx[1] / 100);

                // 스파이크 (아형 7)
                if (atype[t] == 7)
                    drawimage(grap[7][3], xx[0] / 100, xx[1] / 100);

                // 킬러빌 (아형 8, 151)
                if (atype[t] == 8)
                    drawimage(grap[8][3], xx[0] / 100, xx[1] / 100);
                if (atype[t] == 151)
                    drawimage(grap[151][3], xx[0] / 100, xx[1] / 100);

                // 킬러빌 알 (아형 9)
                if (atype[t] == 9)
                    drawimage(grap[9][3], xx[0] / 100, xx[1] / 100);

                // 파이어바 가로 (아형 10)
                if (atype[t] == 10)
                    drawimage(grap[10][3], xx[0] / 100, xx[1] / 100);

                // 캐릭터 롤 (아형 30)
                if (atype[t] == 30) {
                    if (axtype[t] == 0)  drawimage(grap[30][3], xx[0] / 100, xx[1] / 100);
                    if (axtype[t] == 1)  drawimage(grap[155][3], xx[0] / 100, xx[1] / 100);
                }

                // 스테이지 클리어 파편 (아형 81)
                if (atype[t] == 81 && axtype[t] == 1)
                    drawimage(grap[130][3], xx[0] / 100, xx[1] / 100);

                // 노란 적 (아형 79)
                if (atype[t] == 79) {
                    setcolor(250, 250, 0);
                    fillrect(xx[0] / 100, xx[1] / 100, xx[2], xx[3]);
                    setc0();
                    drawrect(xx[0] / 100, xx[1] / 100, xx[2], xx[3]);
                }

                // 캐논 구 (아형 82, 83)
                if (atype[t] == 82 || atype[t] == 83) {
                    xx[9] = (stagecolor == 2) ? 30 : (stagecolor == 4) ? 60 : 0;
                    if (atype[t] == 82) {
                        xx[6] = (axtype[t] == 1) ? 4 + xx[9] : 5 + xx[9];
                        if (axtype[t] == 2)
                            drawimage(grap[1][5], xx[0] / 100, xx[1] / 100);
                        else
                            drawimage(grap[xx[6]][1], xx[0] / 100, xx[1] / 100);
                    }
                    else { // 83
                        xx[6] = (axtype[t] == 1) ? 4 + xx[9] : 5 + xx[9];
                        drawimage(grap[xx[6]][1], xx[0] / 100 + 10, xx[1] / 100 + 9);
                    }
                }

                // 포탑 (아형 85)
                if (atype[t] == 85) {
                    setc1();
                    fillrect(xx[0] / 100 + 10, xx[1] / 100, 10, xx[3]);
                    setc0();
                    drawrect(xx[0] / 100 + 10, xx[1] / 100, 10, xx[3]);
                    setcolor(0, 250, 200);
                    fillarc(xx[0] / 100 + 14, xx[1] / 100, 10, 10);
                    setc0();
                    drawarc(xx[0] / 100 + 14, xx[1] / 100, 10, 10);
                }

                // 뇨로즈 (아형 86)
                if (atype[t] == 86) {
                    if (ma >= aa[t] - fx - mnobia - 4000 && ma <= aa[t] - fx + anobia[t] + 4000)
                        drawimage(grap[152][3], xx[0] / 100, xx[1] / 100);
                    else
                        drawimage(grap[86][3], xx[0] / 100, xx[1] / 100);
                }

                // 아이템 스프라이트 (아형 100~110)
                if (atype[t] == 100)
                    drawimage(grap[100][3], xx[0] / 100, xx[1] / 100);
                if (atype[t] == 101)
                    drawimage(grap[101][3], xx[0] / 100, xx[1] / 100);
                if (atype[t] == 102)
                    drawimage(grap[102][3], xx[0] / 100, xx[1] / 100);
                if (atype[t] == 110)
                    drawimage(grap[110][3], xx[0] / 100, xx[1] / 100);

                // 사망 모션 (아형 200)
                if (atype[t] == 200)
                    drawimage(grap[0][3], xx[0] / 100, xx[1] / 100);

                mirror = 0;
            }
        }

        // ── 플레이어 ────────────────────────────────────────
        setcolor(0, 0, 255);
        if (mactp >= 2000) {
            mactp -= 2000;
            mact = (mact == 0) ? 1 : 0;
        }
        if (mmuki == 0) mirror = 1;

        if (mtype != 200 && mtype != 1) {
            if (mzimen == 1) {
                // 지면 위: 걷기/서있기 프레임
                if (mact == 0) drawimage(grap[0][0], ma / 100 - fx / 100, mb / 100 - fy / 100);
                if (mact == 1) drawimage(grap[1][0], ma / 100 - fx / 100, mb / 100 - fy / 100);
            }
            else {
                // 공중: 점프 프레임
                drawimage(grap[3][0], ma / 100 - fx / 100, mb / 100 - fy / 100);
            }
        }

        // 별 상태 (무적): 번쩍임
        if (mtype == 1) {
            if ((mtm / 3) % 2 == 0)
                drawimage(grap[41][0], ma / 100 - fx / 100 - 11, mb / 100 - fy / 100 - 37);
        }

        // 사망 모션
        if (mtype == 200) {
            drawimage(grap[2][0], ma / 100 - fx / 100, mb / 100 - fy / 100);
        }

        mirror = 0;

        // ── 블록 ────────────────────────────────────────────
        for (t = 0; t < tmax; t++) {
            xx[0] = ta[t] - fx;  xx[1] = tb[t] - fy;
            if (xx[0] + 32 * 100 >= -10 && xx[1] <= fxmax) {
                xx[9] = (stagecolor == 2) ? 30 : (stagecolor == 4) ? 60 : 0;

                // 일반 블록 (ttype < 100)
                if (ttype[t] < 100) {
                    xx[6] = ttype[t] + xx[9];
                    drawimage(grap[xx[6]][1], xx[0] / 100, xx[1] / 100);
                }

                if (txtype[t] != 10) {
                    // 아이템 블록 (물음표·버섯 등)
                    if (ttype[t] == 100 || ttype[t] == 101 || ttype[t] == 102 || ttype[t] == 103 ||
                        (ttype[t] == 104 && txtype[t] == 1) || (ttype[t] == 114 && txtype[t] == 1) ||
                        ttype[t] == 116) {
                        xx[6] = 2 + xx[9];
                        drawimage(grap[xx[6]][1], xx[0] / 100, xx[1] / 100);
                    }
                    if (ttype[t] == 112 || (ttype[t] == 104 && txtype[t] == 0) ||
                        (ttype[t] == 115 && txtype[t] == 1)) {
                        xx[6] = 1 + xx[9];
                        drawimage(grap[xx[6]][1], xx[0] / 100, xx[1] / 100);
                    }
                    if (ttype[t] == 111 || ttype[t] == 113 ||
                        (ttype[t] == 115 && txtype[t] == 0) || ttype[t] == 124) {
                        xx[6] = 3 + xx[9];
                        drawimage(grap[xx[6]][1], xx[0] / 100, xx[1] / 100);
                    }
                }

                // 점프대
                if (ttype[t] == 117 && txtype[t] == 1)
                    drawimage(grap[4][5], xx[0] / 100, xx[1] / 100);
                if (ttype[t] == 117 && txtype[t] >= 3)
                    drawimage(grap[3][5], xx[0] / 100, xx[1] / 100);

                // 점프 블록
                if (ttype[t] == 120 && txtype[t] != 1)
                    drawimage(grap[16][1], xx[0] / 100 + 3, xx[1] / 100 + 2);

                // ON-OFF 블록
                if (ttype[t] == 130) drawimage(grap[10][5], xx[0] / 100, xx[1] / 100);
                if (ttype[t] == 131) drawimage(grap[11][5], xx[0] / 100, xx[1] / 100);

                // 골 깃발 블록
                if (ttype[t] == 140) drawimage(grap[12][5], xx[0] / 100, xx[1] / 100);
                if (ttype[t] == 141) drawimage(grap[13][5], xx[0] / 100, xx[1] / 100);
                if (ttype[t] == 142) drawimage(grap[14][5], xx[0] / 100, xx[1] / 100);

                // 힌트·P스위치·코인
                if (ttype[t] == 300 || ttype[t] == 301)
                    drawimage(grap[1][5], xx[0] / 100, xx[1] / 100);
                if (ttype[t] == 400)
                    drawimage(grap[2][5], xx[0] / 100, xx[1] / 100);
                if (ttype[t] == 800)
                    drawimage(grap[0][2], xx[0] / 100 + 2, xx[1] / 100 + 1);
            }
        }

        // ── 지면 플랫폼 ─────────────────────────────────────
        for (t = 0; t < smax; t++) {
            if (sa[t] - fx + sc[t] >= -10 && sa[t] - fx <= fxmax + 1100) {
                int px = (sa[t] - fx) / 100, py = (sb[t] - fy) / 100;
                int pw = sc[t] / 100, ph = sd[t] / 100;

                // 일반 지면 (녹색 사각형)
                if (stype[t] == 0) {
                    setcolor(40, 200, 40);
                    fillrect(px, py, pw, ph);
                    drawrect(px, py, pw, ph);
                }
                // 풀밭
                if (stype[t] == 1) {
                    setcolor(0, 230, 0);
                    fillrect(px, py, pw, ph);
                    setc0(); drawrect(px, py, pw, ph);
                }
                if (stype[t] == 2) {
                    setcolor(0, 230, 0);
                    fillrect(px, py + 1, pw, ph);
                    setc0();
                    drawline(px, py, px, py + ph);
                    drawline(px + pw, py, px + pw, py + ph);
                }
                if (stype[t] == 5) {
                    setcolor(0, 230, 0);
                    fillrect(px, py + 1, pw, ph);
                    setc0();
                    drawline(px, py, px + pw, py);
                    drawline(px, py + ph, px + pw, py + ph);
                }

                // 사라지는 블록 다리 (stype 51)
                if (stype[t] == 51) {
                    if (sxtype[t] == 0) {
                        for (t3 = 0; t3 <= sc[t] / 3000; t3++)
                            drawimage(grap[1][1], px + 29 * t3, py);
                    }
                    if (sxtype[t] == 1 || sxtype[t] == 2) {
                        for (t3 = 0; t3 <= sc[t] / 3000; t3++)
                            drawimage(grap[31][1], px + 29 * t3, py);
                    }
                    if (sxtype[t] == 3 || sxtype[t] == 4) {
                        for (t3 = 0; t3 <= sc[t] / 3000; t3++)
                            for (t2 = 0; t2 <= sd[t] / 3000; t2++)
                                drawimage(grap[65][1], px + 29 * t3, py + 29 * t2);
                    }
                    if (sxtype[t] == 10) {
                        for (t3 = 0; t3 <= sc[t] / 3000; t3++)
                            drawimage(grap[65][1], px + 29 * t3, py);
                    }
                }

                // 벽 블록 (stype 52)
                if (stype[t] == 52) {
                    xx[29] = (stagecolor == 2) ? 30 : (stagecolor == 4) ? 60 : 0;
                    for (t3 = 0; t3 <= sc[t] / 3000; t3++) {
                        if (sxtype[t] == 0) {
                            drawimage(grap[5 + xx[29]][1], px + 29 * t3, py);
                            if (stagecolor != 4)
                                drawimage(grap[6 + xx[29]][1], px + 29 * t3, py + 29);
                            else
                                drawimage(grap[5 + xx[29]][1], px + 29 * t3, py + 29);
                        }
                        if (sxtype[t] == 1) {
                            for (t2 = 0; t2 <= sd[t] / 3000; t2++)
                                drawimage(grap[1 + xx[29]][1], px + 29 * t3, py + 29 * t2);
                        }
                        if (sxtype[t] == 2) {
                            for (t2 = 0; t2 <= sd[t] / 3000; t2++)
                                drawimage(grap[5 + xx[29]][1], px + 29 * t3, py + 29 * t2);
                        }
                    }
                }

                // 스테이지 트랩 표시 (trap==1 일 때)
                if (trap == 1 && stype[t] >= 100 && stype[t] <= 299) {
                    if (stagecolor <= 1 || stagecolor == 3) setc0();
                    else setc1();
                    // 트랩 종류별 마커 표시 (간략화)
                }
            }
        }

        // ── 플레이어 메시지 ──────────────────────────────────
        if (mmsgtm >= 1) {
            mmsgtm--;
            xs[0] = "";
            if (mmsgtype == 1)  xs[0] = "함정이다!!";
            if (mmsgtype == 2)  xs[0] = "잠깐... 잠깐...";
            if (mmsgtype == 3)  xs[0] = "조심해!!";
            if (mmsgtype == 10) xs[0] = "먹으면 안 되잖아!!";
            if (mmsgtype == 11) xs[0] = "나는 불꽃이니!!";
            if (mmsgtype == 50) xs[0] = "느껴... 있어...";
            if (mmsgtype == 51) xs[0] = "어서와~!!";
            if (mmsgtype == 52) xs[0] = "좋은 곳에 왔네";
            if (mmsgtype == 53) xs[0] = "재밌어, 기대해!!";
            if (mmsgtype == 54) xs[0] = "땅은 섭씨 800도!!";
            if (mmsgtype == 55) xs[0] = "지면과 밑이 좁아...";
            if (!xs[0].empty()) {
                setc0();
                str(xs[0], (ma + mnobia + 300) / 100 - 1 - fx / 100, mb / 100 - 1 - fy / 100);
                str(xs[0], (ma + mnobia + 300) / 100 + 1 - fx / 100, mb / 100 + 1 - fy / 100);
                setc1();
                str(xs[0], (ma + mnobia + 300) / 100 - fx / 100, mb / 100 - fy / 100);
            }
        }

        // ── 적 메시지 ────────────────────────────────────────
        for (t = 0; t < amax; t++) {
            if (amsgtm[t] >= 1) {
                amsgtm[t]--;
                xs[0] = "";
                // 1-1 구간 메시지
                if (amsgtype[t] == 1001) xs[0] = "돌격!!";
                if (amsgtype[t] == 1002) xs[0] = "왜? 이상하지 않아?";
                if (amsgtype[t] == 1003) xs[0] = "착한 아이 자리는 여기!";
                if (amsgtype[t] == 1004) xs[0] = "한번 더 빠지겠지";
                if (amsgtype[t] == 1005) xs[0] = "나 최강!!";
                if (amsgtype[t] == 1006) xs[0] = "내 몸이 나간다!!";
                if (amsgtype[t] == 1007) xs[0] = "게임 속 두 글자는 무시!!";
                if (amsgtype[t] == 1008) xs[0] = "핫하 !!";
                // 1-2 구간 메시지
                if (amsgtype[t] == 1011) xs[0] = "돌격!!";
                if (amsgtype[t] == 1012) xs[0] = "왜? 이상하지 않아?";
                if (amsgtype[t] == 1013) xs[0] = "착한 아이 자리는 여기!";
                if (amsgtype[t] == 1014) xs[0] = "내 몸도 모르겠다...";
                if (amsgtype[t] == 1015) xs[0] = "저의 실수를 사죄합니다";
                if (amsgtype[t] == 1016) xs[0] = "처음엔 신중하게";
                if (amsgtype[t] == 1017) xs[0] = "잠깐!!";
                if (amsgtype[t] == 1018) xs[0] = "중요한...";
                // 공통 메시지
                if (amsgtype[t] == 15)  xs[0] = "파이팅!!";
                if (amsgtype[t] == 18)  xs[0] = "꿈이 됩니다~";
                if (amsgtype[t] == 20)  xs[0] = "Zzz";
                if (amsgtype[t] == 21)  xs[0] = "쿠아크마~";
                if (amsgtype[t] == 24)  xs[0] = "?";
                if (amsgtype[t] == 25)  xs[0] = "먹으면 안 되잖아!!";
                if (amsgtype[t] == 30)  xs[0] = "잡아라!!";
                if (amsgtype[t] == 31)  xs[0] = "블록을 빼앗아줄까?";
                if (amsgtype[t] == 50)  xs[0] = "트라이!!";
                if (amsgtype[t] == 85)  xs[0] = "잘릴 거라 생각했어?";
                if (amsgtype[t] == 86)  xs[0] = "포탑 어택!!";
                if (!xs[0].empty()) {
                    xx[5] = (aa[t] + anobia[t] + 300 - fx) / 100;
                    xx[6] = (ab[t] - fy) / 100;
                    setc0();
                    str(xs[0], xx[5] - 1, xx[6] - 1);
                    str(xs[0], xx[5] + 1, xx[6] + 1);
                    setc1();
                    str(xs[0], xx[5], xx[6]);
                }
            }
        }

        // ── 화면 페이드 아웃 ────────────────────────────────
        if (blacktm > 0) {
            blacktm--;
            setc0();
            fillrect(0, 0, WIN_W, WIN_H);
            if (blacktm == 0 && blackx == 1) zxon = 0;
        }

    } // if (main_state == 1 && zxon >= 1)

    // ════════════════════════════════════════════════════════
    // 목숨 화면 (main_state == 10)
    // ════════════════════════════════════════════════════════
    if (main_state == 10) {
        setc0();
        fillrect(0, 0, WIN_W, WIN_H);
        drawimage(grap[0][0], 190, 190);
        DrawFormatString(230, 200, RGB(255, 255, 255), " x %d", nokori);
    }

    // ════════════════════════════════════════════════════════
    // 타이틀 화면 (main_state == 100)
    // ════════════════════════════════════════════════════════
    if (main_state == 100) {
        setcolor(160, 180, 250);
        fillrect(0, 0, WIN_W, WIN_H);

        drawimage(mgrap[30], 240 - 380 / 2, 60);
        drawimage(grap[0][4], 12 * 30, 10 * 29 - 12);
        drawimage(grap[1][4], 6 * 30, 12 * 29 - 12);
        drawimage(grap[0][0], 2 * 30, 12 * 29 - 12 - 6);

        for (t = 0; t <= 16; t++) {
            drawimage(grap[5][1], 29 * t, 13 * 29 - 12);
            drawimage(grap[6][1], 29 * t, 14 * 29 - 12);
        }

        setcolor(0, 0, 0);
        str("Enter 키를 눌러!!", 240 - 80, 250);
    }
}
// ============================================================
  // §13  Mainprogram()  ─  게임 로직 전체
  // ============================================================
void Mainprogram()
{
    stime = long(GetNowCount());
    if (ending == 1) main_state = 2;

    // 입력 읽기
    key = GetJoypadInputState(DX_INPUT_KEY_PAD1);

    // ════════════════════════════════════════════════════════
    // 타이틀 화면
    // ════════════════════════════════════════════════════════
    if (main_state == 100) {
        if (CheckHitKey(KEY_INPUT_ENTER) == 1) {
            main_state = 10;
            nokori = 2;
            maintm = 0;
        }
        return;
    }

    // ════════════════════════════════════════════════════════
    // 목숨 화면
    // ════════════════════════════════════════════════════════
    if (main_state == 10) {
        maintm++;
        if (maintm > 90) {
            if (nokori < 0) {
                // 게임 오버 → 타이틀로
                nokori = 2;
                sta = 1; stb = 1; stc = 0;
                main_state = 100;
            }
            else {
                zxon = 0;
                main_state = 1;
                maintm = 0;
            }
        }
        return;
    }

    // ════════════════════════════════════════════════════════
    // 게임 중  (main_state == 1)
    // ════════════════════════════════════════════════════════
    if (main_state != 1) return;

    // ── F1: 타이틀로 즉시 귀환 ──────────────────────────────
    if (CheckHitKey(KEY_INPUT_F1) == 1) main_state = 100;

    // ── 스테이지 최초 초기화 (zxon == 0) ────────────────────
    if (zxon == 0) {
        zxon = 1;
        mainmsgtype = 0;
        stagecolor = 1;
        ma = 5600; mb = 32000;
        mmuki = 1; mhp = 1;
        mc = 0;    md = 0;
        mnobia = 3000; mnobib = 3600;
        mtype = 0;
        fx = 0; fy = 0; fzx = 0;
        stageonoff = 0;
        blacktm = 1; blackx = 0;
        mkasok = 0; mjumptm = 0; mkeytm = 0;
        mzimen = 0; mrzimen = 0; mmutekitm = 0; mmutekion = 0;
        mztm = 0; mztype = 0; mtm = 0;       mtype = 0;
        mactp = 0; mact = 0;
        for (int i = 0; i < 7; i++) actaon[i] = 0;

        bgmchange(oto[100]);
        stagecls();
        stage();

        // 현재 BGM 재생
        if (oto[0] != 0)
            PlaySoundMem(oto[0], DX_PLAYTYPE_LOOP);
    }

    // ════════════════════════════════════════════════════════
    // 입력 → actaon 설정
    // ════════════════════════════════════════════════════════
    if (tmsgtype == 0) {
        xx[0] = 0; actaon[2] = 0; actaon[3] = 0;
        if (mkeytm <= 0) {
            if ((key & PAD_INPUT_LEFT) && keytm <= 0) { actaon[0] = -1; mmuki = 0; actaon[4] = -1; }
            if ((key & PAD_INPUT_RIGHT) && keytm <= 0) { actaon[0] = 1; mmuki = 1; actaon[4] = 1; }
            if (key & PAD_INPUT_DOWN) { actaon[3] = 1; }
        }

        if (mkeytm <= 0) {
            if ((key & PAD_INPUT_UP) || CheckHitKey(KEY_INPUT_Z) == 1) {
                if (actaon[1] == 10) { actaon[1] = 1; xx[0] = 1; }
                actaon[2] = 1;
            }
        }

        if ((key & PAD_INPUT_UP) || CheckHitKey(KEY_INPUT_Z) == 1) {
            if (mjumptm == 8 && md >= -900) {
                md = -1300;
                xx[22] = 200; if (mc >= xx[22] || mc <= -xx[22]) md = -1400;
                xx[22] = 600; if (mc >= xx[22] || mc <= -xx[22]) md = -1500;
            }
            if (xx[0] == 0) actaon[1] = 10;
        }
    }

    // ════════════════════════════════════════════════════════
    // 플레이어 수평 가속
    // ════════════════════════════════════════════════════════
    xx[0] = 40; xx[1] = 700; xx[8] = 500; xx[9] = 700;
    xx[12] = 1; xx[13] = 2;
    if (mrzimen == 1) { xx[0] = 20; xx[12] = 9; xx[13] = 10; }

    if (actaon[0] == -1) {
        if (!(mzimen == 0 && mc < -xx[8])) {
            if (mc >= -xx[9]) { mc -= xx[0]; if (mc < -xx[9]) mc = -xx[9] - 1; }
            if (mc < -xx[9] && atktm <= 0) mc -= xx[0] / 10;
        }
        if (mrzimen != 1) {
            if (mc > 100 && mzimen == 0) mc -= xx[0] * 2 / 3;
            if (mc > 100 && mzimen == 1) { mc -= xx[0]; mc -= xx[0] / 2; }
            actaon[0] = 3; mkasok++;
        }
    }
    if (actaon[0] == 1) {
        if (!(mzimen == 0 && mc > xx[8])) {
            if (mc <= xx[9]) { mc += xx[0]; if (mc > xx[9]) mc = xx[9] + 1; }
            if (mc > xx[9] && atktm <= 0) mc += xx[0] / 10;
        }
        if (mrzimen != 1) {
            if (mc < -100 && mzimen == 0) mc += xx[0] * 2 / 3;
            if (mc < -100 && mzimen == 1) { mc += xx[0]; mc += xx[0] / 2; }
            actaon[0] = 3; mkasok++;
        }
    }
    if (actaon[0] == 0 && mkasok > 0) mkasok -= 2;
    if (mkasok > 8) mkasok = 8;
    if (mzimen != 1) mrzimen = 0;

    // ════════════════════════════════════════════════════════
    // 점프
    // ════════════════════════════════════════════════════════
    if (mjumptm >= 0) mjumptm--;
    if (actaon[1] == 1 && mzimen == 1) {
        mb -= 400;
        md = -1200;
        mjumptm = 10;
        ot(oto[1]);
        mzimen = 0;
    }
    if (actaon[1] <= 9) actaon[1] = 0;

    if (mmutekitm >= -1) mmutekitm--;

    // ════════════════════════════════════════════════════════
    // HP 0 → 사망 처리
    // ════════════════════════════════════════════════════════
    if (mhp <= 0 && mhp >= -9) {
        mkeytm = 12; mhp = -20; mtype = 200; mtm = 0;
        ot(oto[12]);
        StopSoundMem(oto[0]);
    }
    if (mtype == 200) {
        if (mtm <= 11) { mc = 0; md = 0; }
        if (mtm == 12) md = -1200;
        if (mtm >= 12) mc = 0;
        mtm++;
        if (mtm >= 100 || fast == 1) {
            zxon = 0; main_state = 10; mtm = 0; mkeytm = 0;
            nokori--;
            if (fast == 1) mtype = 0;
        }
    }

    // ── 파이프 진입 (mtype == 2) ─────────────────────────────
    if (mtype == 2) {
        mtm++;
        mkeytm = 2; md = -1500;
        if (mb <= -6000) {
            blackx = 1; blacktm = 20; stc += 5; stagerr = 0;
            StopSoundMem(oto[0]); mtm = 0; mtype = 0; mkeytm = -1;
        }
    }

    // ── 스테이지 클리어 파이프 (mtype == 300) ─────────────────
    if (mtype == 300) {
        mtm++;
        mkeytm = 3;
        if (mtm <= 1) { mc = 0; md = 0; }
        if (mtm >= 2 && mtm <= 42) { md = 600; mmuki = 1; }
        if (mtm > 43 && mtm <= 108) mc = 300;
        if (mtm == 110) { mb = -80000000; mc = 0; }
        if (mtm == 250) {
            stb++; stc = 0; zxon = 0; tyuukan = 0;
            main_state = 10; maintm = 0;
        }
    }

    // ── 스테이지 전체 클리어 (mtype == 301) ───────────────────
    if (mtype == 301) {
        mtm++;
        mkeytm = 3;
        if (mtm <= 1) { mc = 0; md = 0; }
        if (mtm >= 2 && mtm <= 102) { ma -= 500; fx += 500; fzx += 500; }
        if (mtm >= 2 && mtm <= 100) { mc = 250; mmuki = 1; }
        if (mtm == 200)             ot(oto[17]);
        if (mtm == 440) {
            // 1-2 클리어 → 타이틀 복귀 (구현 범위: 1-1, 1-2)
            sta = 1; stb = 1; stc = 0;
            main_state = 100; nokori = 2;
        }
    }

    // ── 이동 적용 ────────────────────────────────────────────
    if (mkeytm >= 1) mkeytm--;
    ma += mc; mb += md;
    if (mc < 0) mactp += (-mc);
    else        mactp += mc;

    if (mtype <= 9 || mtype == 200 || mtype == 300 || mtype == 301) md += 100;

    // 속도 상한
    if (mtype == 0) {
        xx[0] = 800; xx[1] = 1600;
        if (mc > xx[0] && mc < xx[0] + 200) mc = xx[0];
        if (mc > xx[0] + 200)                mc -= 200;
        if (mc < -xx[0] && mc > -xx[0] - 200) mc = -xx[0];
        if (mc < -xx[0] - 200)               mc += 200;
        if (md > xx[1]) md = xx[1];
    }

    // 지면 마찰 (공중이 아닐 때)
    if (mzimen == 1 && actaon[0] != 3) {
        if (mtype <= 9 || mtype == 300 || mtype == 301) {
            if (mrzimen == 0) {
                xx[2] = 30; xx[1] = 60; xx[3] = 30;
                if (mc >= -xx[3] && mc <= xx[3]) mc = 0;
                if (mc >= xx[2]) mc -= xx[1];
                if (mc <= -xx[2]) mc += xx[1];
            }
            else {
                xx[2] = 5; xx[1] = 10; xx[3] = 5;
                if (mc >= -xx[3] && mc <= xx[3]) mc = 0;
                if (mc >= xx[2]) mc -= xx[1];
                if (mc <= -xx[2]) mc += xx[1];
            }
        }
    }

    // 지면 플래그 리셋
    mzimen = 0;

    // 좌우 경계 클램프
    if (mtype <= 9 && mhp >= 1) {
        if (ma < 100) { ma = 100;           mc = 0; }
        if (ma + mnobia > fxmax) { ma = fxmax - mnobia;  mc = 0; }
    }
    if (mb >= 52000 && mhp >= 0) mhp = -2;

    // ════════════════════════════════════════════════════════
    // 블록 충돌 (플레이어 ↔ 블록)
    // ════════════════════════════════════════════════════════
    xx[15] = 0;
    for (t = 0; t < tmax; t++) {
        xx[0] = 200; xx[1] = 3000; xx[2] = 1000; xx[3] = 3000;
        xx[8] = ta[t] - fx; xx[9] = tb[t] - fy;
        if (ta[t] - fx + xx[1] < -10 - xx[3] || ta[t] - fx > fxmax + 12000 + xx[3]) continue;
        if (mtype == 200 || mtype == 1 || mtype == 2) continue;

        if (ttype[t] < 1000 && ttype[t] != 800 && ttype[t] != 140 && ttype[t] != 141) {
            if (!(mztype == 1)) {
                xx[16] = 0; xx[17] = 0;

                // 위(착지) 충돌
                if (ttype[t] != 7 && ttype[t] != 110 && ttype[t] != 114) {
                    if (ma + mnobia > xx[8] + xx[0] * 2 + 100 &&
                        ma        < xx[8] + xx[1] - xx[0] * 2 - 100 &&
                        mb + mnobib > xx[9] &&
                        mb + mnobib < xx[9] + xx[1] && md >= -100) {
                        if (ttype[t] != 115 && ttype[t] != 400 && ttype[t] != 117 && ttype[t] != 120) {
                            mb = xx[9] - mnobib + 100; md = 0; mzimen = 1; xx[16] = 1;
                        }
                        else if (ttype[t] == 115) {
                            // 부서지는 블록
                            ot(oto[3]);
                            eyobi(ta[t] + 1200, tb[t] + 1200, 300, -1000, 0, 160, 1000, 1000, 1, 120);
                            eyobi(ta[t] + 1200, tb[t] + 1200, -300, -1000, 0, 160, 1000, 1000, 1, 120);
                            eyobi(ta[t] + 1200, tb[t] + 1200, 240, -1400, 0, 160, 1000, 1000, 1, 120);
                            eyobi(ta[t] + 1200, tb[t] + 1200, -240, -1400, 0, 160, 1000, 1000, 1, 120);
                            brockbreak(t);
                        }
                        else if (ttype[t] == 120) {
                            // 점프대
                            md = -2400; mtype = 3; mtm = 0;
                        }
                    }
                }

                // 아래(머리) 충돌
                xx[21] = 0; xx[22] = 1;
                if (mzimen == 1 || mjumptm >= 10) { xx[21] = 3; xx[22] = 0; }
                for (t3 = 0; t3 <= 1; t3++) {
                    if (t3 == xx[21] && mtype != 100 && ttype[t] != 117) {
                        if (ma + mnobia > xx[8] + xx[0] * 2 + 800 &&
                            ma        < xx[8] + xx[1] - xx[0] * 2 - 800 &&
                            mb        > xx[9] - xx[0] * 2 &&
                            mb < xx[9] + xx[1] - xx[0] * 2 && md <= 0) {
                            xx[16] = 1; xx[17] = 1;
                            mb = xx[9] + xx[1] + xx[0];
                            if (md < 0) md = -md * 2 / 3;
                            // 블록 타격 효과
                            if (ttype[t] == 1 && mzimen == 0) {
                                ot(oto[3]);
                                eyobi(ta[t] + 1200, tb[t] + 1200, 300, -1000, 0, 160, 1000, 1000, 1, 120);
                                eyobi(ta[t] + 1200, tb[t] + 1200, -300, -1000, 0, 160, 1000, 1000, 1, 120);
                                eyobi(ta[t] + 1200, tb[t] + 1200, 240, -1400, 0, 160, 1000, 1000, 1, 120);
                                eyobi(ta[t] + 1200, tb[t] + 1200, -240, -1400, 0, 160, 1000, 1000, 1, 120);
                                brockbreak(t);
                            }
                            if (ttype[t] == 2 && mzimen == 0) {
                                ot(oto[4]);
                                eyobi(ta[t] + 10, tb[t], 0, -800, 0, 40, 3000, 3000, 0, 16);
                                ttype[t] = 3;
                            }
                            if (ttype[t] == 7) {
                                ot(oto[4]);
                                eyobi(ta[t] + 10, tb[t], 0, -800, 0, 40, 3000, 3000, 0, 16);
                                mb = xx[9] + xx[1] + xx[0]; ttype[t] = 3;
                                if (md < 0) md = -md * 2 / 3;
                            }
                            // 아이템 블록 타격
                            if (ttype[t] == 101 && xx[17] == 1) {
                                ot(oto[8]); ttype[t] = 3; abrocktm[aco] = 16;
                                ayobi(ta[t], tb[t], 0, 0, 0, 0, 0);
                            }
                            if (ttype[t] == 102 && xx[17] == 1) {
                                ot(oto[8]); ttype[t] = 3; abrocktm[aco] = 16;
                                if (txtype[t] == 0) ayobi(ta[t], tb[t], 0, 0, 0, 100, 0);
                                if (txtype[t] == 2) ayobi(ta[t], tb[t], 0, 0, 0, 100, 2);
                            }
                            if (ttype[t] == 104 && xx[17] == 1) {
                                ot(oto[8]); ttype[t] = 3; abrocktm[aco] = 16;
                                ayobi(ta[t], tb[t], 0, 0, 0, 110, 0);
                            }
                        }
                    }
                    // 좌우 충돌
                    if (t3 == xx[22] && xx[15] == 0) {
                        if (ttype[t] != 7 && ttype[t] != 110 && ttype[t] != 117 && ttype[t] != 114) {
                            if (ta[t] >= -20000) {
                                if (ma + mnobia > xx[8] && ma < xx[8] + xx[2] &&
                                    mb + mnobib > xx[9] + xx[1] / 2 - xx[0] && mb < xx[9] + xx[2] && mc >= 0)
                                {
                                    ma = xx[8] - mnobia; mc = 0; xx[16] = 1;
                                }
                                if (ma + mnobia > xx[8] + xx[2] && ma < xx[8] + xx[1] &&
                                    mb + mnobib > xx[9] + xx[1] / 2 - xx[0] && mb < xx[9] + xx[2] && mc <= 0)
                                {
                                    ma = xx[8] + xx[1]; mc = 0; xx[16] = 1;
                                }
                            }
                        }
                    }
                }
            }
        }

        // 골 깃발 블록 터치
        if (ttype[t] == 140) {
            if (mb > xx[9] - xx[0] * 2 - 2000 && mb < xx[9] + xx[1] - xx[0] * 2 + 2000 &&
                ma + mnobia > xx[8] - 400 && ma < xx[8] + xx[1]) {
                ta[t] = -800000;
                sracttype[20] = 1; sron[20] = 1;
                StopSoundMem(oto[0]); mtype = 301; mtm = 0; ot(oto[16]);
            }
        }
    }

    // ════════════════════════════════════════════════════════
    // 지면 플랫폼 충돌 (플레이어 ↔ sa 배열)
    // ════════════════════════════════════════════════════════
    for (t = 0; t < smax; t++) {
        if (sa[t] - fx + sc[t] < -12000 || sa[t] - fx > fxmax) continue;
        xx[0] = 200; xx[1] = 2400; xx[2] = 1000;
        xx[8] = sa[t] - fx; xx[9] = sb[t] - fy;
        if ((stype[t] <= 99 || stype[t] == 200) && mtype < 10) {
            // 착지
            if (ma + mnobia > xx[8] + xx[0] && ma < xx[8] + sc[t] - xx[0] &&
                mb + mnobib > xx[9] && mb + mnobib < xx[9] + sd[t] - xx[1] && md >= -100) {
                mb = xx[9] - mnobib + 100 + fy; md = 0; mzimen = 1;
            }
            // 천장 충돌
            if (ma + mnobia > xx[8] + xx[0] && ma < xx[8] + sc[t] - xx[0] &&
                mb > xx[9] + sd[t] - xx[1] && mb < xx[9] + sd[t] + xx[0]) {
                mb = xx[9] + sd[t] + xx[0] + fy;
                if (md < 0) md = -md * 2 / 3;
            }
            // 좌우 충돌
            if (ma + mnobia > xx[8] && ma < xx[8] + xx[2] &&
                mb + mnobib > xx[9] + xx[1] * 3 / 4 && mb < xx[9] + sd[t] - xx[2]) {
                ma = xx[8] - mnobia + fx; mc = 0;
            }
            if (ma + mnobia > xx[8] + sc[t] - xx[0] && ma < xx[8] + sc[t] + xx[0] &&
                mb + mnobib > xx[9] + xx[1] * 3 / 4 && mb < xx[9] + sd[t] - xx[2]) {
                ma = xx[8] + sc[t] + xx[0] + fx; mc = 0;
            }
        }
    }

    // ════════════════════════════════════════════════════════
    // 이펙트 이동
    // ════════════════════════════════════════════════════════
    for (t = 0; t < emax; t++) {
        if (ea[t] <= -9000000) continue;
        ea[t] += ec[t]; eb[t] += ed[t];
        if (egtype[t] == 0 || egtype[t] >= 2) ed[t] += 40;
        if (egtype[t] == 1) ed[t] += 40;
        etm[t]--;
        if (etm[t] <= 0) ea[t] = -9000000;
    }

    // ════════════════════════════════════════════════════════
    // 스크롤 처리
    // ════════════════════════════════════════════════════════
    if (mtype <= 9 || mtype == 300 || mtype == 301) {
        xx[0] = fxmax / 2;
        if (ma > fx + xx[0]) {
            int diff = ma - (fx + xx[0]);
            if (fx + diff <= fzx) { fx += diff; }
            else if (fzx > fx) { fx = fzx; }
        }
    }
    // 스크롤 상한 (스테이지 길이에 맞춰 조정 가능)
    if (fx > scrollx) fx = scrollx;
    if (fx < 0)       fx = 0;

    // ════════════════════════════════════════════════════════
    // 탄환(포탄) 이동
    // ════════════════════════════════════════════════════════
    for (t = 0; t < bmax; t++) {
        if (ba[t] <= -9000000) continue;
        btm[t]++;
        if (btype[t] == 0) {
            // 구리보 계열 → 생성 타이밍
            if (btm[t] >= 120) {
                btm[t] = 0;
                if (ba[t] - fx > -3000 && ba[t] - fx < fxmax + 3000) {
                    int dir = (ma < ba[t]) ? 0 : 1;
                    ayobi(ba[t], bb[t], 0, 0, 0, 0, dir);
                }
            }
        }
        if (btype[t] == 80) {
            // 캐논 포탄 → 포탄 발사
            if (btm[t] >= 180) {
                btm[t] = 0;
                if (ba[t] - fx > -3000 && ba[t] - fx < fxmax + 3000) {
                    int dir = (ma < ba[t]) ? 0 : 1;
                    int spd = (dir == 0) ? -800 : 800;
                    ayobi(ba[t], bb[t], spd, 0, 0, 9, dir);
                }
            }
        }
    }
}
// ============================================================
// §11-A  stagep()  ─  스테이지별 타일 데이터 + 오브젝트 배치
// ============================================================
void stagep()
{
    scrollx = 3600 * 100;

    // ════════════════════════════════════════════════════════
    // 1-1  오버월드
    // ════════════════════════════════════════════════════════
    if (sta == 1 && stb == 1 && stc == 0) {

        byte stagedatex[17][1001] = {
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0} ,
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,82, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0} ,
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,82, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,98, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0,99, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0} ,
        { 0, 0, 0, 0, 0, 0,82, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,50, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0} ,
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0} ,
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,98, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,98,98,98, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,98, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0} ,
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
4, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0} ,
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,50, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,30, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4,
4, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0} ,
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,98,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 4,
4, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0} ,
        { 0, 0, 0, 0, 0, 0, 0, 0,98, 0, 0, 0, 1,98, 1, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 1,98, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,98, 0, 0, 0, 0, 0, 0, 1,98, 0, 0, 0, 2, 0, 0, 2,
0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 4, 4, 4,
4, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0} ,
        { 0,80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0,40, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,80, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0, 4, 0, 7, 7, 7, 7, 7,40, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 4, 4, 4,
4, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,83, 0, 0, 0, 0, 0} ,
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,41, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 4, 4, 0, 0, 0, 4, 4, 0, 0, 0, 0, 0,41, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,40, 0, 0, 4, 4, 4, 4, 4,
4, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0} ,
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,50, 0, 0, 0, 0, 0,50, 0, 0,81,41, 0, 0, 0, 0, 0,81,98, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0,81, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,50, 0,50, 0, 0,51, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0,81, 0, 0, 0, 4, 4, 4, 0, 0, 0, 4, 4, 0, 0, 0, 0, 0,41, 0, 0, 0, 0, 0,50, 0,50, 0, 0,41, 0, 4, 4, 4, 4, 4, 4,
4, 4, 4, 0, 0, 0, 0, 0, 0, 4,81, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0} ,
        { 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
5, 5, 5, 5, 0, 0, 0, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 0, 0, 0, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 0, 0, 0,
0, 0, 5, 5, 5, 5, 5, 5, 5, 5, 0, 0, 0, 5, 5, 5, 5, 5, 5, 0, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 0, 5, 5, 5, 5, 5, 5, 5} ,
        { 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
6, 6, 6, 6, 0, 0, 0, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 0, 0, 0, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 0, 0, 0,
0, 0, 6, 6, 6, 6, 6, 6, 6, 6, 0, 0, 0, 6, 6, 6, 6, 6, 6, 0, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 0, 6, 6, 6, 6, 6, 6, 6, 6, 0, 0, 0, 0} ,
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
        };

        // 아이템·특수 블록 추가 배치
        tyobi(8 * 29, 9 * 29 - 12, 100); txtype[tco - 1] = 2;
        tyobi(13 * 29, 9 * 29 - 12, 102); txtype[tco - 1] = 0;
        tyobi(14 * 29, 5 * 29 - 12, 101);
        tyobi(35 * 29, 8 * 29 - 12, 110);
        tyobi(47 * 29, 9 * 29 - 12, 103);
        tyobi(59 * 29, 9 * 29 - 12, 112);
        tyobi(67 * 29, 9 * 29 - 12, 104);

        // 지면 플랫폼 (sa 배열)
        sco = 0;
        t = sco; sa[t] = 20 * 29 * 100 + 500; sb[t] = -6000;       sc[t] = 5000; sd[t] = 70000; stype[t] = 100; sco++;
        t = sco; sa[t] = 54 * 29 * 100 - 500; sb[t] = -6000;       sc[t] = 7000; sd[t] = 70000; stype[t] = 101; sco++;
        t = sco; sa[t] = 112 * 29 * 100 + 1000; sb[t] = -6000;      sc[t] = 3000; sd[t] = 70000; stype[t] = 102; sco++;
        t = sco; sa[t] = 117 * 29 * 100;    sb[t] = (2 * 29 - 12) * 100 - 1500; sc[t] = 15000; sd[t] = 3000; stype[t] = 103; sco++;
        t = sco; sa[t] = 125 * 29 * 100;    sb[t] = -6000;       sc[t] = 9000; sd[t] = 70000; stype[t] = 101; sco++;
        t = 28;  sa[t] = 29 * 29 * 100 + 500; sb[t] = (9 * 29 - 12) * 100; sc[t] = 6000; sd[t] = 12000 - 200; stype[t] = 50; sco++;
        t = sco; sa[t] = 49 * 29 * 100;     sb[t] = (5 * 29 - 12) * 100; sc[t] = 9000 - 1; sd[t] = 3000;   stype[t] = 51; sgtype[t] = 0; sco++;
        t = sco; sa[t] = 72 * 29 * 100;     sb[t] = (13 * 29 - 12) * 100; sc[t] = 3000 * 5 - 1; sd[t] = 3000; stype[t] = 52; sco++;

        // 탄환·포탄 배치 (ba 배열)
        bco = 0;
        t = bco; ba[t] = 27 * 29 * 100;  bb[t] = (9 * 29 - 12) * 100;       btype[t] = 0;  bxtype[t] = 0; bco++;
        t = bco; ba[t] = 103 * 29 * 100; bb[t] = (5 * 29 - 12 + 10) * 100;    btype[t] = 80; bxtype[t] = 0; bco++;

        // 타일 데이터 복사
        for (tt = 0; tt <= 1000; tt++)
            for (t = 0; t <= 16; t++)
                stagedate[t][tt] = stagedatex[t][tt];

    } // 1-1
}