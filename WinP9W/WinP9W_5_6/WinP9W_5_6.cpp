#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <tchar.h>
#include <atlimage.h>
#include <vector>
#include <deque>
#include <algorithm>
#include <random>
#include <cmath>
#include <windowsx.h>
#pragma comment(lib, "msimg32.lib")

#define LEN 1000
#define HEI 1000

using namespace std;

// ─── 색상 테이블 (인덱스 1-5: 빨강 / 주황 / 노랑 / 초록 / 파랑) ───────────
static const COLORREF g_colorTable[] = {
    RGB(  0,   0,   0),
    RGB(255,  60,  60),   // 1 빨강
    RGB(255, 165,   0),   // 2 주황
    RGB(255, 220,   0),   // 3 노랑 (팩맨 기본색)
    RGB( 60, 200,  60),   // 4 초록
    RGB( 60, 120, 255),   // 5 파랑
};

// ─── 난수 ────────────────────────────────────────────────────────────────────
random_device rd;
mt19937 g_rng(rd());
uniform_int_distribution<> disX{ 100, 900 };
uniform_int_distribution<> disY{ 100, 900 };

// ─── 열거형 ──────────────────────────────────────────────────────────────────
enum class Dir { RIGHT, LEFT, UP, DOWN };

// R90CW  : 오른쪽 방향 스프라이트 → 아래 방향 (dstRow=srcCol, dstCol=(sh-1)-srcRow)
// R90CCW : 오른쪽 방향 스프라이트 → 위 방향   (dstRow=(sw-1)-srcCol, dstCol=srcRow)
enum class Rot { R0, R90CW, R180, R90CCW };

// ─── 구조체 ──────────────────────────────────────────────────────────────────
struct SpriteInfo { int x, y, w, h; };

// 클론 경로 추적을 위해 기록되는 팩맨의 위치/상태 스냅샷
struct PosRecord {
    float x, y;
    Dir   dir;
    int   animFrame;
};

// 총알 표시 크기: 가로 방향 48×8, 세로 방향 8×48 (원본 비율 162:26 근사).
// x,y = 오른쪽 방향 기준 좌상단, 중심 = (x+24, y+4).
struct Bullet {
    float x = 0.f, y = 0.f;
    Dir   dir      = Dir::RIGHT;
    float speed    = 600.f;
    int   colorIdx = 3;
    bool  alive    = true;

    RECT GetHitBox() const {
        float cx = x + 24.f, cy = y + 4.f;
        if (dir == Dir::RIGHT || dir == Dir::LEFT) {
            // 가로탄: 히트박스 40×6
            return { (LONG)(cx - 20), (LONG)(cy - 3), (LONG)(cx + 20), (LONG)(cy + 3) };
        } else {
            // 세로탄: 히트박스 6×40
            return { (LONG)(cx - 3), (LONG)(cy - 20), (LONG)(cx + 3), (LONG)(cy + 20) };
        }
    }
};

struct Pacman {
    float x = 0.f, y = 0.f;
    int   w = 50, h = 50;
    float speed    = 200.f;
    Dir   dir      = Dir::RIGHT;
    int   colorIdx = 3;
    int   animFrame    = 0;
    DWORD lastAnimTime = 0;
    DWORD lastShotTime = 0;
    // 점프 상태
    bool  isJumping = false;
    DWORD jumpStart = 0;
    static constexpr DWORD JUMP_DURATION = 1200;  // ms (긴 장애물도 뛰어넘을 수 있도록 충분한 시간)
    bool  isPaused  = false;

    RECT GetHitBox() const {
        return { (LONG)x, (LONG)y, (LONG)(x + w), (LONG)(y + h) };
    }
};

// 클론은 팩맨의 기록된 경로를 고정 지연으로 따라간다.
// colorIdx / w / h 는 생성 시점에 고정된다.
struct Clone {
    int   slot;          // 1 / 2 / 3  →  지연 = slot * CLONE_DELAY 레코드
    int   colorIdx;      // 생성 시 고정
    int   w, h;          // 생성 시 고정
    float x = 0.f, y = 0.f;
    Dir   dir      = Dir::RIGHT;
    int   animFrame = 0;
    bool  valid    = false;   // 히스토리가 충분히 쌓이기 전까지 false
};

// ─── 장애물(Brick) ────────────────────────────────────────────────────────────
// type 0 : 50×50  (작은 정사각형)   스프라이트 size=0 (소스 16×16)
// type 1 : 100×50 (넓은 직사각형) 스프라이트 size=1 (소스 32×16), 회전 없음
// type 2 : 50×100 (높은 직사각형) 스프라이트 size=1 (소스 32×16), R90CW 회전
//
// HP = colorIdx*2 (type 0) | colorIdx*3 (type 1/2)
// 파손 상태: 큰 블록 → 3단계 (0 / ≤2/3 / ≤1/3), 작은 블록 → 2단계 (0 / ≤1/2)
struct Brick {
    float x = 0.f, y = 0.f;
    int   type     = 0;   // 0 / 1 / 2
    int   colorIdx = 1;   // 1-5
    int   hp    = 0;
    int   maxHp = 0;
    bool  alive = true;

    int GetW() const { return (type == 1) ? 100 : 50; }
    int GetH() const { return (type == 2) ? 100 : 50; }

    RECT GetHitBox() const {
        return { (LONG)x, (LONG)y,
                 (LONG)(x + GetW()), (LONG)(y + GetH()) };
    }

    // 스프라이트 파손 상태
    // type 0 (50×50): 스프라이트 2개만 존재 → 0=멀쩡, 1=금 간 상태
    // type 1/2 (큰 블록): 스프라이트 3개 → 0=멀쩡, 1=약간 파손, 2=심하게 파손
    int GetState() const {
        if (type == 0) {
            return (hp * 2 <= maxHp) ? 1 : 0;  // HP 절반 이하면 금 간 상태
        }
        if (hp * 3 <= maxHp)     return 2;   // HP ≤ 1/3
        if (hp * 3 <= maxHp * 2) return 1;   // HP ≤ 2/3
        return 0;
    }
};

// ─── 먹이(Food) ───────────────────────────────────────────────────────────────
// 한 축(수평 또는 수직)으로 이동하며 아래 범위에서 왕복:
//   [center - g_foodOscillateRange, center + g_foodOscillateRange]
// 화면 경계와 장애물 히트박스에도 반사된다.
// 팩맨 접촉 시 : 먹이 제거, 팩맨 색상 변경, 팩맨 크기 증가.
// 총알 접촉 시 : 먹이 유지, 근처에 새 먹이 1개 생성(분열), 총알 소멸.
struct Food {
    float x = 0.f, y = 0.f;
    float centerX = 0.f, centerY = 0.f;  // spawn top-left anchor
    int   colorIdx = 1;
    int   w = 40, h = 40;
    float speed = 100.f;
    bool  alive        = true;
    bool  isHorizontal = true;    // true = X axis, false = Y axis
    Dir   dir          = Dir::RIGHT;
    int   animFrame    = 0;
    DWORD lastAnimTime = 0;

    RECT GetHitBox() const {
        return { (LONG)x, (LONG)y, (LONG)(x + w), (LONG)(y + h) };
    }
};

// ─── 이펙트(Effect) ───────────────────────────────────────────────────────────
// 일회성 폭발 연기 (10프레임 × 70ms, cx/cy 중심으로 100×100 크기 출력).
struct Effect {
    float cx = 0.f, cy = 0.f;   // 화면 중심 좌표
    int   animFrame     = 0;
    DWORD lastAnimTime  = 0;
    bool  alive         = true;
    int   srcRow        = 0;     // 사용할 스프라이트 시트 행
    bool  isPauseEffect = false; // true이면 애니메이션 종료 시 팩맨 일시정지 해제
};

// ─── 클론 / 히스토리 상수 ────────────────────────────────────────────────────
constexpr int CLONE_DELAY     = 4;               // 슬롯당 지연 레코드 수 (4 = 200ms ≈ 40px, 중심 기준 ~35px 뒤)
constexpr int RECORD_INTERVAL = 50;              // 히스토리 스냅샷 간격 (ms)
constexpr int MAX_HISTORY     = CLONE_DELAY * 3 + 10;

// ─── 이미지 에셋 (색상별 인덱스 1-5) ────────────────────────────────────────
CImage img_pacman[6];
CImage img_bullet[6];
CImage img_ghost[6];     // food sprites (index 1-5 per color)
CImage img_effect;
CImage img_brick;

// ─── 전역 상태 ───────────────────────────────────────────────────────────────
HINSTANCE g_hInst;
HWND      g_hWnd     = nullptr;
HDC       g_hBackDC  = nullptr;
HBITMAP   g_hBackBmp = nullptr;
HBITMAP   g_hOldBmp  = nullptr;

Pacman           g_pacman;
vector<Bullet>   g_bullets;
vector<Clone>    g_clones;
vector<Brick>    g_bricks;
vector<Food>     g_foods;
vector<Effect>   g_effects;
deque<PosRecord> g_posHistory;

DWORD g_prevTick       = 0;
DWORD g_lastRecordTime = 0;

// 먹이 진동: 각 먹이는 스폰 중심에서 ±g_foodOscillateRange px 범위로 왕복
int   g_foodOscillateRange    = 40;
int   g_foodRespawnPending    = 0;    // how many foods still need to be re-spawned
DWORD g_lastFoodRespawnTime   = 0;

bool  g_showDebug   = false;
bool  g_hKeyPressed = false;
bool  g_jKeyPressed = false;
bool  g_tKeyPressed = false;
bool  g_aKeyPressed = false;

// ─── 스프라이트 선택 함수 ─────────────────────────────────────────────────────
// 시트 레이아웃: 8프레임 × 16×16 px 수평 배열, 오른쪽 방향
SpriteInfo GetPacmanSprite(int frame) { return { frame * 16, 0, 16, 16 }; }
SpriteInfo GetBulletSprite()          { return { 29, 59, 162, 26 }; }

// 장애물 스프라이트 시트 레이아웃 (bricks.png):
//   size=0 (작은 16×16): x = 224 + state*16,  y = (96 - color*16) - 16
//   size=1 (큰 32×16):   x = 112 + state*32,  y = (96 - color*16) - 16
// state: 0=멀쩡, 1=약간 파손, 2=심하게 파손  /  color: 1-5
SpriteInfo GetBrickSprite(int size, int state, int color)
{
    if (size == 0) {
        int x = 224 + state * 16;
        int y = 96 - color * 16;
        return { x, y - 16, 16, 16 };
    }
    int x = 112 + state * 32;
    int y = 96 - color * 16;
    return { x, y - 16, 32, 16 };
}

// 유령/먹이 스프라이트: 8프레임 × 16×16 px 수평 배열 (팩맨과 동일한 레이아웃)
SpriteInfo GetGhostSprite(int frame) { return { (frame % 8) * 16, 0, 16, 16 }; }

// 폭발 이펙트: y=128에서 시작, 각 프레임 64×64, x방향으로 64씩 증가, 총 10프레임
SpriteInfo GetEffectSprite(int frame, int srcRow = 0) {
    return { frame * 64, 128, 64, 64 };
}

// 오른쪽 방향 스프라이트를 'dir' 방향으로 회전/반전하기 위한 {Rot, flipH} 반환.
//   RIGHT → 변환 없음
//   LEFT  → 수평 반전 (거울)
//   UP    → 90° CCW 회전 (스프라이트 오른쪽이 위를 향함)
//   DOWN  → 90° CW  회전 (스프라이트 오른쪽이 아래를 향함)
pair<Rot, bool> GetSpriteTransform(Dir dir)
{
    switch (dir) {
    default:
    case Dir::RIGHT: return { Rot::R0,     false };
    case Dir::LEFT:  return { Rot::R0,     true  };
    case Dir::UP:    return { Rot::R90CCW, false };
    case Dir::DOWN:  return { Rot::R90CW,  false };
    }
}

// ─── 그리기 헬퍼 ─────────────────────────────────────────────────────────────

void DrawAlphaImage(HDC hdcDest, int dx, int dy, int dw, int dh,
                    CImage& src, int sx, int sy, int sw, int sh)
{
    if (src.IsNull()) return;
    BLENDFUNCTION bf = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };
    HDC hdcSrc = src.GetDC();
    GdiAlphaBlend(hdcDest, dx, dy, dw, dh, hdcSrc, sx, sy, sw, sh, bf);
    src.ReleaseDC();
}

// 통합 스프라이트 그리기: 픽셀 수준 회전 + 반전 + 알파 상수 적용.
//
// 알고리즘 (원시 BGRA 픽셀 배열 처리):
//   1. CImage::GetPixelAddress로 소스 영역 픽셀(BGRA)을 버퍼에 직접 복사 (알파 보존).
//   2. 'rot'에 따라 픽셀을 출력 버퍼에 재배치.
//      회전 공식 (srcRow=r, srcCol=c, 소스 크기 sw×sh):
//        R90CW  → dstRow=c,         dstCol=(sh-1)-r   outW=sh, outH=sw
//        R90CCW → dstRow=(sw-1)-c,  dstCol=r           outW=sh, outH=sw
//        R180   → dstRow=(sh-1)-r,  dstCol=(sw-1)-c   outW=sw, outH=sh
//   3. 회전된 버퍼에 flipH / flipV 적용 (행/열 제자리 교환).
//   4. 'alpha' 상수로 픽셀별 알파 조절 (반투명 클론 등에 사용).
//   5. 출력 버퍼를 새 DIB에 쓰고 GdiAlphaBlend로 화면에 출력.
// colorKey=true 이면 소스 좌상단(sx,sy) 픽셀의 BGRA 값과 완전히 일치하는 픽셀을 투명으로 처리.
// PNG 이미지에 알파 채널 없는 단색 배경이 있을 때 배경을 제거하는 용도로 사용.
void DrawSprite(HDC hdcDest, int dx, int dy, int dw, int dh,
                CImage& src, int sx, int sy, int sw, int sh,
                bool flipH = false, bool flipV = false,
                Rot rot = Rot::R0, BYTE alpha = 255, bool colorKey = false)
{
    if (src.IsNull()) return;

    // 빠른 경로 — 변환 없음, 완전 불투명, 컬러키 없음
    if (!flipH && !flipV && rot == Rot::R0 && alpha == 255 && !colorKey) {
        DrawAlphaImage(hdcDest, dx, dy, dw, dh, src, sx, sy, sw, sh);
        return;
    }

    // ── Step 1: 소스 영역을 픽셀 버퍼에 복사 (알파 채널 보존) ───────────────
    // BitBlt는 알파 채널을 0으로 덮어써서 반투명 클론 색상이 깨지므로,
    // CImage::GetPixelAddress로 원본 픽셀(BGRA)에 직접 접근하여 복사한다.
    vector<DWORD> srcPx((size_t)sw * sh, 0);
    {
        for (int r = 0; r < sh; r++) {
            for (int c = 0; c < sw; c++) {
                const DWORD* pPx = (const DWORD*)src.GetPixelAddress(sx + c, sy + r);
                srcPx[(size_t)r * sw + c] = pPx ? *pPx : 0;
            }
        }
    }

    // ── 컬러키 적용: 좌상단 픽셀과 BGRA가 완전히 일치하는 픽셀을 투명화 ─────
    if (colorKey && !srcPx.empty()) {
        DWORD key = srcPx[0];
        for (auto& px : srcPx)
            if (px == key) px = 0;
    }

    // ── Step 2: 픽셀 회전 ────────────────────────────────────────────────────
    // 90도 회전 후 너비와 높이가 교환된다.
    int outW = (rot == Rot::R90CW || rot == Rot::R90CCW) ? sh : sw;
    int outH = (rot == Rot::R90CW || rot == Rot::R90CCW) ? sw : sh;

    vector<DWORD> dstPx((size_t)outW * outH, 0);
    for (int r = 0; r < sh; r++) {
        for (int c = 0; c < sw; c++) {
            DWORD pixel = srcPx[(size_t)r * sw + c];
            int dstR, dstC;
            switch (rot) {
            default:
            case Rot::R0:
                dstR = r;          dstC = c;          break;
            case Rot::R90CW:
                dstR = c;          dstC = (sh - 1) - r; break;
            case Rot::R180:
                dstR = (sh - 1) - r; dstC = (sw - 1) - c; break;
            case Rot::R90CCW:
                dstR = (sw - 1) - c; dstC = r;          break;
            }
            dstPx[(size_t)dstR * outW + dstC] = pixel;
        }
    }

    // ── Step 3: 반전 (회전된 버퍼에 적용) ───────────────────────────────────
    if (flipH) {
        for (int r = 0; r < outH; r++) {
            int l = 0, ri = outW - 1;
            while (l < ri)
                swap(dstPx[(size_t)r * outW + l++], dstPx[(size_t)r * outW + ri--]);
        }
    }
    if (flipV) {
        for (int t = 0, b = outH - 1; t < b; t++, b--)
            for (int c = 0; c < outW; c++)
                swap(dstPx[(size_t)t * outW + c], dstPx[(size_t)b * outW + c]);
    }

    // ── Step 4: 픽셀별 알파 조절 ─────────────────────────────────────────────
    if (alpha < 255) {
        for (auto& px : dstPx) {
            BYTE a = (BYTE)((unsigned)(px >> 24) * alpha / 255u);
            px = (px & 0x00FFFFFFu) | ((DWORD)a << 24);
        }
    }

    // ── Step 5: 출력 DIB에 쓰고 화면에 알파 블렌딩 ──────────────────────────
    BITMAPINFO bi = {};
    bi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth       = outW;
    bi.bmiHeader.biHeight      = -outH;
    bi.bmiHeader.biPlanes      = 1;
    bi.bmiHeader.biBitCount    = 32;
    bi.bmiHeader.biCompression = BI_RGB;

    void*   pOut = nullptr;
    HBITMAP hOut = CreateDIBSection(nullptr, &bi, DIB_RGB_COLORS, &pOut, nullptr, 0);
    if (!hOut) return;

    memcpy(pOut, dstPx.data(), (size_t)outW * outH * 4);

    HDC     hOutDC  = CreateCompatibleDC(nullptr);
    HBITMAP hOutOld = (HBITMAP)SelectObject(hOutDC, hOut);

    BLENDFUNCTION bf = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };
    GdiAlphaBlend(hdcDest, dx, dy, dw, dh, hOutDC, 0, 0, outW, outH, bf);

    SelectObject(hOutDC, hOutOld);
    DeleteObject(hOut);
    DeleteDC(hOutDC);
}

// ─── 알파 프리멀티플라이 ──────────────────────────────────────────────────────
// GdiAlphaBlend(AC_SRC_ALPHA)는 프리멀티플라이드 알파를 요구한다.
// 글로우 효과 PNG처럼 A=0이면서 RGB≠0인 픽셀이 있으면 배경이 색으로 칠해지므로,
// 로딩 직후 이 함수를 호출해 CImage DIB를 프리멀 형식으로 변환한다.
void PremultiplyAlpha(CImage& img)
{
    if (img.IsNull()) return;
    int w = img.GetWidth(), h = img.GetHeight();
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            DWORD* p = (DWORD*)img.GetPixelAddress(x, y);
            if (!p) continue;
            BYTE a = (BYTE)(*p >> 24);
            if (a == 255) continue;
            if (a == 0)   { *p = 0; continue; }
            BYTE r = (BYTE)((((*p) >> 16) & 0xFF) * a / 255u);
            BYTE g = (BYTE)((((*p) >>  8) & 0xFF) * a / 255u);
            BYTE b = (BYTE)(( (*p)         & 0xFF) * a / 255u);
            *p = ((DWORD)a << 24) | ((DWORD)r << 16) | ((DWORD)g << 8) | b;
        }
    }
}

// ─── 먹이 / 이펙트 헬퍼 ─────────────────────────────────────────────────────

void SpawnEffect(float cx, float cy, int srcRow = 0, bool isPauseEffect = false)
{
    Effect e;
    e.cx            = cx;
    e.cy            = cy;
    e.animFrame     = 0;
    e.lastAnimTime  = GetTickCount();
    e.alive         = true;
    e.srcRow        = srcRow;
    e.isPauseEffect = isPauseEffect;
    g_effects.push_back(e);
}

// 좌상단 (cx, cy)에 먹이 1개 생성. colorIdx=-1이면 랜덤 색상.
Food SpawnFoodAt(float cx, float cy, int colorIdx = -1)
{
    uniform_int_distribution<> colorDis{ 1, 5 };
    Food f;
    f.x = cx; f.y = cy;
    f.centerX = cx; f.centerY = cy;
    f.colorIdx = (colorIdx < 1 || colorIdx > 5) ? colorDis(g_rng) : colorIdx;
    f.isHorizontal = (uniform_int_distribution<>{ 0, 1 }(g_rng) == 0);
    bool negDir    = (uniform_int_distribution<>{ 0, 1 }(g_rng) == 0);
    f.dir = f.isHorizontal
        ? (negDir ? Dir::LEFT  : Dir::RIGHT)
        : (negDir ? Dir::UP    : Dir::DOWN);
    f.lastAnimTime = GetTickCount();
    return f;
}

// 'candidate' 영역이 맵 안에 완전히 있고, 팩맨/살아있는 먹이/살아있는 장애물('exclude' 제외)과
// 겹치지 않으면 true 반환.
bool RectSafe(RECT candidate, const Brick* exclude = nullptr)
{
    if (candidate.left < 0 || candidate.top < 0 ||
        candidate.right > LEN || candidate.bottom > HEI) return false;

    RECT dummy;
    RECT phb = g_pacman.GetHitBox();
    if (IntersectRect(&dummy, &candidate, &phb)) return false;

    for (const auto& f : g_foods) {
        if (f.alive) {
            RECT foodBox = f.GetHitBox();
            if (IntersectRect(&dummy, &candidate, &foodBox)) {
                return false;
            }
        }
    }

    for (const auto& br : g_bricks) {
        if (&br == exclude || !br.alive) continue;

        RECT brickBox = br.GetHitBox();
        if (IntersectRect(&dummy, &candidate, &brickBox)) {
            return false;
        }
    }
    return true;
}

// 겹치지 않는 랜덤 위치에 먹이 'count'개 스폰.
// 장애물, 기존 먹이, 팩맨 시작 구역을 피한다.
void SpawnFoodBatch(int count)
{
    const int FOOD_W = 40, FOOD_H = 40;
    const int MARGIN      = 5;
    const int MAX_ATTEMPT = 200;
    RECT guard = { 0, 0, 120, 120 };

    uniform_int_distribution<> colorDis{ 1, 5 };

    for (int i = 0; i < count; i++) {
        for (int att = 0; att < MAX_ATTEMPT; att++) {
            float fx = (float)uniform_int_distribution<>{ 0, LEN - FOOD_W }(g_rng);
            float fy = (float)uniform_int_distribution<>{ 0, HEI - FOOD_H }(g_rng);

            RECT nr = { (LONG)fx, (LONG)fy, (LONG)(fx + FOOD_W), (LONG)(fy + FOOD_H) };
            RECT dummy;
            bool blocked = false;

            // 1. guard 충돌 체크 (흐름을 blocked 변수로 통일)
            if (IntersectRect(&dummy, &nr, &guard)) {
                blocked = true;
            }

            // 2. 장애물(Brick) 충돌 체크 (l-value 문제 해결)
            if (!blocked) {
                for (const auto& br : g_bricks) {
                    RECT brickBox = br.GetHitBox(); // 실제 변수에 복사하여 l-value로 만듦
                    if (IntersectRect(&dummy, &nr, &brickBox)) {
                        blocked = true;
                        break;
                    }
                }
            }

            // 3. 기존 먹이(Food) 충돌 체크
            if (!blocked) {
                for (const auto& ef : g_foods) {
                    RECT er = ef.GetHitBox();
                    RECT ex = { er.left - MARGIN, er.top - MARGIN, er.right + MARGIN, er.bottom + MARGIN };
                    if (IntersectRect(&dummy, &nr, &ex)) {
                        blocked = true;
                        break;
                    }
                }
            }

            // 어딘가에 부딪혔다면 이번 좌표(att)는 실패이므로 다음 시도로 넘어감
            if (blocked) continue;

            // 모든 충돌을 통과했다면 안전하게 스폰 후 att 반복문 탈출(다음 아이템 생성으로)
            g_foods.push_back(SpawnFoodAt(fx, fy, colorDis(g_rng)));
            break;
        }
    }
}

// ─── SpawnBricks ─────────────────────────────────────────────────────────────
// 겹치지 않는 랜덤 위치에 장애물 8개 배치.
// 충돌 감지는 각 장애물 히트박스를 6px 마진으로 확장하여 사용.
// (0,0) 주변 120×120 보호 구역으로 팩맨 스폰 지점을 보호.
void SpawnBricks()
{
    uniform_int_distribution<> typeDis{ 0, 2 };
    uniform_int_distribution<> colorDis{ 1, 5 };
    const int MARGIN      = 6;
    const int MAX_ATTEMPT = 300;

    RECT spawnGuard = { 0, 0, 120, 120 };   // 팩맨 시작 지점 보호

    g_bricks.clear();

    auto overlap = [&](RECT a, RECT b) -> bool {
        RECT dummy;
        return IntersectRect(&dummy, &a, &b) != 0;
    };
    auto expand = [](RECT r, int m) -> RECT {
        return { r.left - m, r.top - m, r.right + m, r.bottom + m };
    };

    for (int i = 0; i < 8; i++) {
        for (int attempt = 0; attempt < MAX_ATTEMPT; attempt++) {
            int type = typeDis(g_rng);
            int col  = colorDis(g_rng);
            int bw   = (type == 1) ? 100 : 50;
            int bh   = (type == 2) ? 100 : 50;

            int bx = uniform_int_distribution<>{ 0, LEN - bw }(g_rng);
            int by = uniform_int_distribution<>{ 0, HEI - bh }(g_rng);

            RECT newRect = { (LONG)bx, (LONG)by,
                              (LONG)(bx + bw), (LONG)(by + bh) };

            if (overlap(expand(newRect, MARGIN), spawnGuard)) continue;

            bool blocked = false;
            for (const auto& ex : g_bricks) {
                if (overlap(expand(newRect, MARGIN), expand(ex.GetHitBox(), MARGIN))) {
                    blocked = true;
                    break;
                }
            }
            if (blocked) continue;

            Brick b;
            b.x        = (float)bx;
            b.y        = (float)by;
            b.type     = type;
            b.colorIdx = col;
            b.maxHp    = (type == 0) ? col * 2 : col * 3;
            b.hp       = b.maxHp;
            g_bricks.push_back(b);
            break;
        }
    }
}

// ─── 게임 초기화 ─────────────────────────────────────────────────────────────
void InitGame()
{
    static const wchar_t* pacNames[] = {
        nullptr,
        L"PacMan_red.png", L"PacMan_orange.png", L"PacMan_yellow.png",
        L"PacMan_green.png", L"PacMan_blue.png"
    };
    static const wchar_t* razNames[] = {
        nullptr,
        L"red_razer.png", L"orange_razer.png", L"yellow_razer.png",
        L"green_razer.png", L"blue_razer.png"
    };

    for (int i = 1; i <= 5; i++) {
        img_pacman[i].Load(pacNames[i]);
        img_bullet[i].Load(razNames[i]);
        PremultiplyAlpha(img_bullet[i]);  // 글로우 PNG의 비프리멀 알파 → 프리멀 변환
    }
    static const wchar_t* ghostNames[] = {
        nullptr,
        L"redGhost.png", L"orangeGhost.png", L"yellowGhost.png",
        L"greenGhost.png", L"blueGhost.png"
    };
    for (int i = 1; i <= 5; i++) img_ghost[i].Load(ghostNames[i]);

    img_effect.Load(L"Free_Smoke_Fx_Pixel 05.png");
    img_brick.Load(L"bricks.png");

    g_pacman = Pacman{};
    g_bullets.clear();
    g_clones.clear();
    g_foods.clear();
    g_effects.clear();
    g_posHistory.clear();
    g_foodRespawnPending  = 0;
    g_lastFoodRespawnTime = 0;
    SpawnBricks();
    SpawnFoodBatch(20);
    g_prevTick = g_lastRecordTime = GetTickCount();
}

// ─── 게임 업데이트 ───────────────────────────────────────────────────────────
void UpdateGame(float dt)
{
    DWORD now = GetTickCount();

    // ── H: 디버그 토글 ───────────────────────────────────────────────────────
    if (GetAsyncKeyState('H') & 0x8000) {
        if (!g_hKeyPressed) { g_showDebug = !g_showDebug; g_hKeyPressed = true; }
    }
    else { g_hKeyPressed = false; }

    // ── J: 점프 (키 누를 때 1회, 애니메이션 종료까지 대기) ────────────────────
    // 스케일 애니메이션(시각적 효과만, 히트박스 불변)이 JUMP_DURATION ms 동안 실행.
    if (GetAsyncKeyState('J') & 0x8000) {
        if (!g_jKeyPressed && !g_pacman.isJumping) {
            g_jKeyPressed      = true;
            g_pacman.isJumping = true;
            g_pacman.jumpStart = now;
        }
    }
    else { g_jKeyPressed = false; }

    if (g_pacman.isJumping && now - g_pacman.jumpStart >= Pacman::JUMP_DURATION)
        g_pacman.isJumping = false;

    // ── T: 클론 순환  0→1→2→3→0 ─────────────────────────────────────────────
    // 누를 때마다 클론 1개 추가 (생성 시점의 colorIdx/크기 고정).
    // 4번째 누름(이미 클론 3개)에서 모든 클론 제거.
    if (GetAsyncKeyState('T') & 0x8000) {
        if (!g_tKeyPressed) {
            g_tKeyPressed = true;
            if ((int)g_clones.size() >= 3) {
                g_clones.clear();
            }
            else {
                Clone c;
                c.slot     = (int)g_clones.size() + 1;
                c.colorIdx = g_pacman.colorIdx;
                c.w        = g_pacman.w;
                c.h        = g_pacman.h;
                g_clones.push_back(c);
            }
        }
    }
    else { g_tKeyPressed = false; }

    // ── A: 모든 먹이 폭발, 이후 20개를 200ms 간격으로 순차 재생성 ────────────
    if (GetAsyncKeyState('A') & 0x8000) {
        if (!g_aKeyPressed) {
            g_aKeyPressed = true;
            for (const auto& f : g_foods) {
                if (f.alive) SpawnEffect(f.x + f.w * 0.5f, f.y + f.h * 0.5f);
            }
            g_foods.clear();
            g_foodRespawnPending  = 20;
            g_lastFoodRespawnTime = now;
        }
    }
    else { g_aKeyPressed = false; }

    // 순차 재생성: 대기 수가 0이 될 때까지 200ms마다 먹이 1개 생성
    if (g_foodRespawnPending > 0 && now - g_lastFoodRespawnTime >= 200) {
        g_lastFoodRespawnTime = now;
        SpawnFoodBatch(1);
        g_foodRespawnPending--;
    }

    // ── 이동, 입력, 충돌 (팩맨 일시정지 중 모두 차단) ─────────────────────────
    if (!g_pacman.isPaused) {

    // ── 방향 입력 ────────────────────────────────────────────────────────────
    if      (GetAsyncKeyState(VK_RIGHT) & 0x8000) g_pacman.dir = Dir::RIGHT;
    else if (GetAsyncKeyState(VK_LEFT)  & 0x8000) g_pacman.dir = Dir::LEFT;
    else if (GetAsyncKeyState(VK_UP)    & 0x8000) g_pacman.dir = Dir::UP;
    else if (GetAsyncKeyState(VK_DOWN)  & 0x8000) g_pacman.dir = Dir::DOWN;

    // ── 발사 (Enter, 200ms 쿨타임) ───────────────────────────────────────────
    if ((GetAsyncKeyState(VK_RETURN) & 0x8000) && now - g_pacman.lastShotTime >= 200) {
        g_pacman.lastShotTime = now;
        Bullet b;
        // 중심을 팩맨 중심에 맞춤: 중심 = (x+24, y+4)
        b.x        = g_pacman.x + (g_pacman.w - 48) / 2.f;
        b.y        = g_pacman.y + (g_pacman.h - 8) / 2.f;
        b.dir      = g_pacman.dir;
        b.colorIdx = g_pacman.colorIdx;
        g_bullets.push_back(b);
    }

    // ── 플레이어 이동 ─────────────────────────────────────────────────────────
    switch (g_pacman.dir) {
    case Dir::RIGHT: g_pacman.x += g_pacman.speed * dt; break;
    case Dir::LEFT:  g_pacman.x -= g_pacman.speed * dt; break;
    case Dir::UP:    g_pacman.y -= g_pacman.speed * dt; break;
    case Dir::DOWN:  g_pacman.y += g_pacman.speed * dt; break;
    }

    // ── 경계 반사 ────────────────────────────────────────────────────────────
    if (g_pacman.x < 0.f) {
        g_pacman.x   = 0.f;
        g_pacman.dir = Dir::RIGHT;
    }
    else if (g_pacman.x + g_pacman.w > LEN) {
        g_pacman.x   = (float)(LEN - g_pacman.w);
        g_pacman.dir = Dir::LEFT;
    }
    if (g_pacman.y < 0.f) {
        g_pacman.y   = 0.f;
        g_pacman.dir = Dir::DOWN;
    }
    else if (g_pacman.y + g_pacman.h > HEI) {
        g_pacman.y   = (float)(HEI - g_pacman.h);
        g_pacman.dir = Dir::UP;
    }

    // ── 팩맨-장애물 충돌 (점프 중에는 장애물을 통과) ───────────────────────
    // 이동 및 경계 처리 후 실행하여 위치가 확정된 상태에서 검사.
    // 충돌 시: 팩맨을 장애물 면에 붙이고 방향 반전.
    if (!g_pacman.isJumping) {
        RECT pBox = g_pacman.GetHitBox();
        for (const auto& br : g_bricks) {
            if (!br.alive) continue;
            RECT brBox = br.GetHitBox();
            RECT dummy;
            if (!IntersectRect(&dummy, &pBox, &brBox)) continue;

            switch (g_pacman.dir) {
            case Dir::RIGHT:
                g_pacman.x   = (float)(brBox.left - g_pacman.w);
                g_pacman.dir = Dir::LEFT;  break;
            case Dir::LEFT:
                g_pacman.x   = (float)brBox.right;
                g_pacman.dir = Dir::RIGHT; break;
            case Dir::UP:
                g_pacman.y   = (float)brBox.bottom;
                g_pacman.dir = Dir::DOWN;  break;
            case Dir::DOWN:
                g_pacman.y   = (float)(brBox.top - g_pacman.h);
                g_pacman.dir = Dir::UP;    break;
            }
            // 밀어낸 후 화면 밖으로 나가지 않도록 클램프
            g_pacman.x = max(0.f, min(g_pacman.x, (float)(LEN - g_pacman.w)));
            g_pacman.y = max(0.f, min(g_pacman.y, (float)(HEI - g_pacman.h)));
            break;   // 프레임당 최대 1회 충돌 처리
        }
    }

    } // end if (!g_pacman.isPaused)

    // ── 스프라이트 애니메이션 (8프레임 @ 10fps) ──────────────────────────────
    if (now - g_pacman.lastAnimTime > 100) {
        g_pacman.animFrame    = (g_pacman.animFrame + 1) % 8;
        g_pacman.lastAnimTime = now;
    }

    // ── 위치 히스토리 스냅샷 (클론 경로 추적용) ──────────────────────────────
    // RECORD_INTERVAL ms마다 기록; MAX_HISTORY 개로 상한.
    // 클론 N은 history[size - 1 - N*CLONE_DELAY]를 읽어 고정 시간 지연 구현.
    if (now - g_lastRecordTime >= (DWORD)RECORD_INTERVAL) {
        g_lastRecordTime = now;
        g_posHistory.push_back({ g_pacman.x, g_pacman.y,
                                  g_pacman.dir, g_pacman.animFrame });
        while ((int)g_posHistory.size() > MAX_HISTORY)
            g_posHistory.pop_front();
    }

    // ── 히스토리에서 클론 위치 갱신 ─────────────────────────────────────────
    for (auto& c : g_clones) {
        int histIdx = (int)g_posHistory.size() - 1 - c.slot * CLONE_DELAY;
        if (histIdx >= 0) {
            const PosRecord& rec = g_posHistory[histIdx];
            c.x        = rec.x;
            c.y        = rec.y;
            c.dir      = rec.dir;
            c.animFrame = rec.animFrame;
            c.valid    = true;
        }
    }

    // ── 총알 이동 ────────────────────────────────────────────────────────────
    for (auto& b : g_bullets) {
        switch (b.dir) {
        case Dir::RIGHT: b.x += b.speed * dt; break;
        case Dir::LEFT:  b.x -= b.speed * dt; break;
        case Dir::UP:    b.y -= b.speed * dt; break;
        case Dir::DOWN:  b.y += b.speed * dt; break;
        }
        // 중심 기준으로 화면 이탈 검사 (충분한 여유 포함)
        float bcx = b.x + 24.f, bcy = b.y + 4.f;
        if (bcx < -50.f || bcx > LEN + 50.f || bcy < -50.f || bcy > HEI + 50.f)
            b.alive = false;
    }

    // ── 총알-장애물 충돌 ─────────────────────────────────────────────────────
    // 피해량 = 총알의 colorIdx. 총알 1발당 장애물 1개 타격 후 소멸.
    for (auto& b : g_bullets) {
        if (!b.alive) continue;
        RECT bBox = b.GetHitBox();
        for (auto& br : g_bricks) {
            if (!br.alive) continue;
            RECT brBox = br.GetHitBox();
            RECT dummy;
            if (IntersectRect(&dummy, &bBox, &brBox)) {
                br.hp -= b.colorIdx;
                if (br.hp <= 0) {
                    br.alive = false;
                    g_pacman.speed *= 1.1f;  // 장애물 파괴 시 속도 1.1배 증가
                }
                b.alive = false;
                break;
            }
        }
    }

    // ── 먹이 애니메이션 + 진동 이동 ──────────────────────────────────────────
    for (auto& f : g_foods) {
        if (!f.alive) continue;

        // 애니메이션
        if (now - f.lastAnimTime > 150) {
            f.animFrame    = (f.animFrame + 1) % 8;
            f.lastAnimTime = now;
        }

        // 축 방향 이동
        if      (f.dir == Dir::RIGHT) f.x += f.speed * dt;
        else if (f.dir == Dir::LEFT)  f.x -= f.speed * dt;
        else if (f.dir == Dir::DOWN)  f.y += f.speed * dt;
        else                           f.y -= f.speed * dt;

        // 진동 범위 클램프 (화면 경계도 적용)
        if (f.isHorizontal) {
            float xMin = max(0.f, f.centerX - (float)g_foodOscillateRange);
            float xMax = min((float)(LEN - f.w), f.centerX + (float)g_foodOscillateRange);
            if (f.x < xMin) { f.x = xMin; f.dir = Dir::RIGHT; }
            else if (f.x > xMax) { f.x = xMax; f.dir = Dir::LEFT; }
        }
        else {
            float yMin = max(0.f, f.centerY - (float)g_foodOscillateRange);
            float yMax = min((float)(HEI - f.h), f.centerY + (float)g_foodOscillateRange);
            if (f.y < yMin) { f.y = yMin; f.dir = Dir::DOWN; }
            else if (f.y > yMax) { f.y = yMax; f.dir = Dir::UP; }
        }

        // 장애물 반사 (방향 반전 + 밀어내기)
        RECT fBox = f.GetHitBox();
        for (const auto& br : g_bricks) {
            RECT brBox = br.GetHitBox();
            RECT dummy;
            if (!IntersectRect(&dummy, &fBox, &brBox)) continue;
            if (f.isHorizontal) {
                if (f.dir == Dir::RIGHT) { f.x = (float)(brBox.left - f.w); f.dir = Dir::LEFT; }
                else                     { f.x = (float)brBox.right;         f.dir = Dir::RIGHT; }
            }
            else {
                if (f.dir == Dir::DOWN)  { f.y = (float)(brBox.top - f.h); f.dir = Dir::UP; }
                else                     { f.y = (float)brBox.bottom;       f.dir = Dir::DOWN; }
            }
            break;
        }
    }

    // ── 팩맨-먹이 충돌 ───────────────────────────────────────────────────────
    // 먹이 획득 → 팩맨 색상 변경, 팩맨 크기 5% 증가
    {
        RECT pBox = g_pacman.GetHitBox();
        for (auto& f : g_foods) {
            if (!f.alive) continue;
            RECT fBox = f.GetHitBox();
            RECT dummy;
            if (!IntersectRect(&dummy, &pBox, &fBox)) continue;

            g_pacman.colorIdx = f.colorIdx;
            g_pacman.w        = (int)(g_pacman.w * 1.05f);
            g_pacman.h        = (int)(g_pacman.h * 1.05f);
            SpawnEffect(f.x + f.w * 0.5f, f.y + f.h * 0.5f);
            f.alive = false;
            break;   // 프레임당 최대 1개 획득
        }
    }

    // ── 총알-먹이 충돌 (분열) ────────────────────────────────────────────────
    // 총알이 먹이에 맞으면 → 총알 소멸, 같은 색 새 먹이 1개 근처에 생성
    {
        uniform_int_distribution<> offsetDis{ -60, 60 };
        for (auto& b : g_bullets) {
            if (!b.alive) continue;
            RECT bBox = b.GetHitBox();
            for (auto& f : g_foods) {
                if (!f.alive) continue;
                RECT fBox = f.GetHitBox();
                RECT dummy;
                if (!IntersectRect(&dummy, &bBox, &fBox)) continue;

                // 원본 주변 랜덤 오프셋에 새 먹이 생성 시도
                bool placed = false;
                for (int att = 0; att < 8 && !placed; att++) {
                    float nx = max(0.f, min((float)(LEN - 40), f.x + offsetDis(g_rng)));
                    float ny = max(0.f, min((float)(HEI - 40), f.y + offsetDis(g_rng)));
                    RECT  nr = { (LONG)nx, (LONG)ny, (LONG)(nx+40), (LONG)(ny+40) };
                    bool  ok = true;
                    for (const auto& br : g_bricks) {
                        RECT brBox2 = br.GetHitBox(); RECT d2;
                        if (IntersectRect(&d2, &nr, &brBox2)) { ok = false; break; }
                    }
                    if (ok) { g_foods.push_back(SpawnFoodAt(nx, ny, f.colorIdx)); placed = true; }
                }

                b.alive = false;
                break;
            }
        }
    }

    // ── 이펙트 애니메이션 ────────────────────────────────────────────────────
    for (auto& e : g_effects) {
        if (!e.alive) continue;
        if (now - e.lastAnimTime > 70) { e.animFrame++; e.lastAnimTime = now; }
        if (e.animFrame >= 10) {
            e.alive = false;
            if (e.isPauseEffect) g_pacman.isPaused = false;
        }
    }

    // ── 소멸된 총알, 장애물, 먹이, 이펙트 제거 ──────────────────────────────
    g_bullets.erase(
        remove_if(g_bullets.begin(), g_bullets.end(),
            [](const Bullet& b) { return !b.alive; }),
        g_bullets.end());
    g_bricks.erase(
        remove_if(g_bricks.begin(), g_bricks.end(),
            [](const Brick& br) { return !br.alive; }),
        g_bricks.end());
    g_foods.erase(
        remove_if(g_foods.begin(), g_foods.end(),
            [](const Food& f) { return !f.alive; }),
        g_foods.end());
    g_effects.erase(
        remove_if(g_effects.begin(), g_effects.end(),
            [](const Effect& e) { return !e.alive; }),
        g_effects.end());
}

// ─── 프레임 렌더링 ───────────────────────────────────────────────────────────
void RenderFrame(HDC hdc)
{
    // 배경
    RECT screen = { 0, 0, LEN, HEI };
    HBRUSH bgBr = CreateSolidBrush(RGB(20, 20, 30));
    FillRect(hdc, &screen, bgBr);
    DeleteObject(bgBr);

    // ── 장애물 (최하위 레이어 — 먼저 그려서 엔티티가 위에 표시됨) ─────────────
    for (const auto& br : g_bricks) {
        int state = br.GetState();
        int drawX = (int)br.x, drawY = (int)br.y;

        if (br.type == 0) {
            // 작은 50×50: 소스 16×16, 회전 없음
            SpriteInfo sp = GetBrickSprite(0, state, br.colorIdx);
            if (!img_brick.IsNull()) {
                DrawSprite(hdc, drawX, drawY, 50, 50,
                    img_brick, sp.x, sp.y, sp.w, sp.h,
                    false, false, Rot::R0, 255);
            }
            else {
                HBRUSH br2 = CreateSolidBrush(g_colorTable[br.colorIdx]);
                auto   ob  = (HBRUSH)SelectObject(hdc, br2);
                auto   op  = (HPEN)  SelectObject(hdc, GetStockObject(NULL_PEN));
                Rectangle(hdc, drawX, drawY, drawX + 50, drawY + 50);
                SelectObject(hdc, ob); SelectObject(hdc, op);
                DeleteObject(br2);
            }
        }
        else if (br.type == 1) {
            // 넓은 100×50: 소스 32×16, 회전 없음
            SpriteInfo sp = GetBrickSprite(1, state, br.colorIdx);
            if (!img_brick.IsNull()) {
                DrawSprite(hdc, drawX, drawY, 100, 50,
                    img_brick, sp.x, sp.y, sp.w, sp.h,
                    false, false, Rot::R0, 255);
            }
            else {
                HBRUSH br2 = CreateSolidBrush(g_colorTable[br.colorIdx]);
                auto   ob  = (HBRUSH)SelectObject(hdc, br2);
                auto   op  = (HPEN)  SelectObject(hdc, GetStockObject(NULL_PEN));
                Rectangle(hdc, drawX, drawY, drawX + 100, drawY + 50);
                SelectObject(hdc, ob); SelectObject(hdc, op);
                DeleteObject(br2);
            }
        }
        else {
            // 높은 50×100: 소스 32×16을 R90CW 회전 → 논리적 16×32, 50×100으로 확대
            SpriteInfo sp = GetBrickSprite(1, state, br.colorIdx);
            if (!img_brick.IsNull()) {
                DrawSprite(hdc, drawX, drawY, 50, 100,
                    img_brick, sp.x, sp.y, sp.w, sp.h,
                    false, false, Rot::R90CW, 255);
            }
            else {
                HBRUSH br2 = CreateSolidBrush(g_colorTable[br.colorIdx]);
                auto   ob  = (HBRUSH)SelectObject(hdc, br2);
                auto   op  = (HPEN)  SelectObject(hdc, GetStockObject(NULL_PEN));
                Rectangle(hdc, drawX, drawY, drawX + 50, drawY + 100);
                SelectObject(hdc, ob); SelectObject(hdc, op);
                DeleteObject(br2);
            }
        }
    }

    // ── 일반 이펙트 (장애물 위, 엔티티 아래 — isPauseEffect=false만 출력) ────
    for (const auto& e : g_effects) {
        if (!e.alive || e.isPauseEffect) continue;
        SpriteInfo sp = GetEffectSprite(e.animFrame, e.srcRow);
        DrawAlphaImage(hdc,
            (int)(e.cx - 50.f), (int)(e.cy - 50.f), 100, 100,
            img_effect, sp.x, sp.y, sp.w, sp.h);
    }

    // ── 먹이 ─────────────────────────────────────────────────────────────────
    for (const auto& f : g_foods) {
        if (!f.alive) continue;

        pair<Rot, bool> tf = GetSpriteTransform(f.dir);
        CImage&    img = img_ghost[f.colorIdx];
        SpriteInfo sp  = GetGhostSprite(f.animFrame);

        if (!img.IsNull()) {
            DrawSprite(hdc,
                (int)f.x, (int)f.y, f.w, f.h,
                img, sp.x, sp.y, sp.w, sp.h,
                tf.second, false, tf.first, 255);
        }
        else {
            HBRUSH br  = CreateSolidBrush(g_colorTable[f.colorIdx]);
            HPEN   pn  = CreatePen(PS_SOLID, 2, RGB(255, 255, 255));
            auto   obr = (HBRUSH)SelectObject(hdc, br);
            auto   opn = (HPEN)  SelectObject(hdc, pn);
            Ellipse(hdc, (int)f.x, (int)f.y,
                    (int)(f.x + f.w), (int)(f.y + f.h));
            SelectObject(hdc, obr); SelectObject(hdc, opn);
            DeleteObject(br); DeleteObject(pn);
        }
    }

    // ── 클론 (팩맨보다 먼저 그려서 팩맨이 위에 표시됨) ──────────────────────
    for (const auto& c : g_clones) {
        if (!c.valid) continue;

        pair<Rot, bool> tf = GetSpriteTransform(c.dir);
        Rot  rot   = tf.first;
        bool flipH = tf.second;

        CImage&    img = img_pacman[c.colorIdx];
        SpriteInfo sp  = GetPacmanSprite(c.animFrame);

        if (!img.IsNull()) {
            DrawSprite(hdc,
                (int)c.x, (int)c.y, c.w, c.h,
                img, sp.x, sp.y, sp.w, sp.h,
                flipH, false, rot,
                255);   // 팩맨과 동일한 불투명도로 출력
        }
        else {
            // GDI 폴백 (이미지 없을 때)
            HBRUSH br  = CreateSolidBrush(g_colorTable[c.colorIdx]);
            HPEN   pn  = CreatePen(PS_DOT, 1, RGB(200, 200, 200));
            auto   obr = (HBRUSH)SelectObject(hdc, br);
            auto   opn = (HPEN)  SelectObject(hdc, pn);
            Ellipse(hdc, (int)c.x, (int)c.y,
                    (int)(c.x + c.w), (int)(c.y + c.h));
            SelectObject(hdc, obr); SelectObject(hdc, opn);
            DeleteObject(br); DeleteObject(pn);
        }
    }

    // ── 팩맨 ─────────────────────────────────────────────────────────────────
    {
        // 점프: JUMP_DURATION ms 동안 사인 곡선 스케일 펄스.
        // 중간 지점에서 최대 1.4배; 히트박스(g_pacman.w/h)는 변하지 않음.
        int drawW = g_pacman.w, drawH = g_pacman.h;
        int drawX = (int)g_pacman.x, drawY = (int)g_pacman.y;

        if (g_pacman.isJumping) {
            float t = (float)(GetTickCount() - g_pacman.jumpStart)
                      / (float)Pacman::JUMP_DURATION;
            if (t > 1.f) t = 1.f;
            float scale = 1.f + 0.4f * sinf(t * 3.14159265f);
            drawW = (int)(g_pacman.w * scale);
            drawH = (int)(g_pacman.h * scale);
            // 확대된 스프라이트를 원래 좌상단 기준으로 중앙 정렬
            drawX = (int)(g_pacman.x + (g_pacman.w - drawW) / 2.f);
            drawY = (int)(g_pacman.y + (g_pacman.h - drawH) / 2.f);
        }

        pair<Rot, bool> tf = GetSpriteTransform(g_pacman.dir);
        Rot  rot   = tf.first;
        bool flipH = tf.second;

        CImage&    img = img_pacman[g_pacman.colorIdx];
        SpriteInfo sp  = GetPacmanSprite(g_pacman.animFrame);

        if (!img.IsNull()) {
            DrawSprite(hdc,
                drawX, drawY, drawW, drawH,
                img, sp.x, sp.y, sp.w, sp.h,
                flipH, false, rot, 255);
        }
        else {
            HBRUSH br  = CreateSolidBrush(g_colorTable[g_pacman.colorIdx]);
            HPEN   pn  = CreatePen(PS_SOLID, 2, RGB(255, 255, 255));
            auto   obr = (HBRUSH)SelectObject(hdc, br);
            auto   opn = (HPEN)  SelectObject(hdc, pn);
            Ellipse(hdc, drawX, drawY, drawX + drawW, drawY + drawH);
            SelectObject(hdc, obr); SelectObject(hdc, opn);
            DeleteObject(br); DeleteObject(pn);
        }
    }

    // ── 총알 ─────────────────────────────────────────────────────────────────
    for (const auto& b : g_bullets) {
        pair<Rot, bool> tf = GetSpriteTransform(b.dir);
        Rot  rot   = tf.first;
        bool flipH = tf.second;

        CImage&    img = img_bullet[g_pacman.colorIdx];
        SpriteInfo sp  = GetBulletSprite();

        if (!img.IsNull()) {
            // 방향에 따라 가로(48×8) / 세로(8×48) 크기 선택
            bool isVert = (b.dir == Dir::UP || b.dir == Dir::DOWN);
            float bcx = b.x + 24.f, bcy = b.y + 4.f;
            int bW = isVert ? 8 : 48, bH = isVert ? 48 : 8;
            int bDrawX = (int)(bcx - bW * 0.5f), bDrawY = (int)(bcy - bH * 0.5f);
            DrawSprite(hdc, bDrawX, bDrawY, bW, bH,
                img, sp.x, sp.y, sp.w, sp.h,
                flipH, false, rot);  // razer PNG는 이미 알파 채널로 투명 처리되어 있음
        }
        else {
            RECT   hb  = b.GetHitBox();
            HBRUSH br  = CreateSolidBrush(g_colorTable[b.colorIdx]);
            auto   obr = (HBRUSH)SelectObject(hdc, br);
            auto   opn = (HPEN)  SelectObject(hdc, GetStockObject(NULL_PEN));
            Ellipse(hdc, hb.left, hb.top, hb.right, hb.bottom);
            SelectObject(hdc, obr); SelectObject(hdc, opn);
            DeleteObject(br);
        }
    }

    // ── 좌클릭 폭발 이펙트 (팩맨 위에 — isPauseEffect=true만 출력) ────────────
    for (const auto& e : g_effects) {
        if (!e.alive || !e.isPauseEffect) continue;
        SpriteInfo sp = GetEffectSprite(e.animFrame, e.srcRow);
        DrawAlphaImage(hdc,
            (int)(e.cx - 50.f), (int)(e.cy - 50.f), 100, 100,
            img_effect, sp.x, sp.y, sp.w, sp.h);
    }

    // ── 디버그 오버레이 ───────────────────────────────────────────────────────
    if (g_showDebug) {
        HBRUSH oldBr = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));

        // 먹이 히트박스 (노란 외곽선)
        HPEN yPen = CreatePen(PS_SOLID, 1, RGB(255, 230, 0));
        SelectObject(hdc, yPen);
        SelectObject(hdc, GetStockObject(NULL_BRUSH));
        for (const auto& f : g_foods) {
            if (f.alive) {
                RECT fhb = f.GetHitBox();
                Rectangle(hdc, fhb.left, fhb.top, fhb.right, fhb.bottom);
            }
        }
        DeleteObject(yPen);

        // 장애물 히트박스 (흰색) + HP 표시
        HPEN wPen = CreatePen(PS_SOLID, 1, RGB(200, 200, 200));
        SelectObject(hdc, wPen);
        SelectObject(hdc, GetStockObject(NULL_BRUSH));
        SetTextColor(hdc, RGB(255, 255, 255));
        for (const auto& br : g_bricks) {
            RECT hb = br.GetHitBox();
            Rectangle(hdc, hb.left, hb.top, hb.right, hb.bottom);
            wchar_t hp[16];
            swprintf_s(hp, L"%d/%d", br.hp, br.maxHp);
            TextOut(hdc, hb.left + 2, hb.top + 2, hp, (int)wcslen(hp));
        }
        DeleteObject(wPen);

        // 팩맨 히트박스 (빨간색) — 점프 중에도 기본 w/h 사용
        HPEN rPen   = CreatePen(PS_SOLID, 1, RGB(255, 50, 50));
        HPEN oldPen = (HPEN)SelectObject(hdc, rPen);
        RECT phb    = g_pacman.GetHitBox();
        Rectangle(hdc, phb.left, phb.top, phb.right, phb.bottom);

        // 클론 히트박스 (청록색)
        HPEN cPen = CreatePen(PS_SOLID, 1, RGB(0, 210, 210));
        SelectObject(hdc, cPen);
        DeleteObject(rPen);
        for (const auto& c : g_clones) {
            if (c.valid)
                Rectangle(hdc, (int)c.x, (int)c.y,
                          (int)(c.x + c.w), (int)(c.y + c.h));
        }

        // 총알 히트박스 (초록색)
        HPEN gPen = CreatePen(PS_SOLID, 1, RGB(50, 255, 50));
        SelectObject(hdc, gPen);
        DeleteObject(cPen);
        for (const auto& b : g_bullets) {
            RECT hb = b.GetHitBox();
            Rectangle(hdc, hb.left, hb.top, hb.right, hb.bottom);
        }

        SelectObject(hdc, oldPen);
        SelectObject(hdc, oldBr);
        DeleteObject(gPen);
    }
}

// ─── 윈도우 프로시저 ─────────────────────────────────────────────────────────
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE) PostQuitMessage(0);
        break;

    case WM_LBUTTONDOWN: {
        POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };

        bool hitPacOrClone = false;
        RECT phb = g_pacman.GetHitBox();
        if (PtInRect(&phb, pt)) hitPacOrClone = true;
        if (!hitPacOrClone) {
            for (const auto& c : g_clones) {
                if (!c.valid) continue;
                RECT chb = { (LONG)c.x, (LONG)c.y,
                             (LONG)(c.x + c.w), (LONG)(c.y + c.h) };
                if (PtInRect(&chb, pt)) { hitPacOrClone = true; break; }
            }
        }

        if (hitPacOrClone && !g_pacman.isPaused) {
            g_pacman.isPaused = true;
            // 팩맨 중심 바로 위에 폭발 이펙트 생성 (완료 시 일시정지 해제)
            float cx = g_pacman.x + g_pacman.w * 0.5f;
            float cy = g_pacman.y - 50.f;
            if (cy < 50.f) cy = 50.f;
            SpawnEffect(cx, cy, 2, true);
        }
        break;
    }

    case WM_RBUTTONDOWN: {
        POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        uniform_int_distribution<> offDis{ -50, 50 };

        for (auto& br : g_bricks) {
            if (!br.alive) continue;
            RECT brBox = br.GetHitBox();
            if (!PtInRect(&brBox, pt)) continue;

            bool moved = false;
            for (int att = 0; att < 100 && !moved; att++) {
                float nx = br.x + (float)offDis(g_rng);
                float ny = br.y + (float)offDis(g_rng);
                RECT candidate = { (LONG)nx, (LONG)ny,
                                   (LONG)(nx + br.GetW()), (LONG)(ny + br.GetH()) };
                if (RectSafe(candidate, &br)) {
                    br.x  = nx;
                    br.y  = ny;
                    moved = true;
                }
            }
            break;   // 가장 위에 있는 클릭된 장애물만 처리
        }
        break;
    }

    case WM_ERASEBKGND:
        return 1;
    case WM_GETMINMAXINFO: {
        RECT rc = { 0, 0, LEN, HEI };
        AdjustWindowRect(&rc, (DWORD)GetWindowLong(hWnd, GWL_STYLE), FALSE);
        LPMINMAXINFO p = (LPMINMAXINFO)lParam;
        p->ptMinTrackSize.x = p->ptMaxTrackSize.x = rc.right  - rc.left;
        p->ptMinTrackSize.y = p->ptMaxTrackSize.y = rc.bottom - rc.top;
        return 0;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

LPCTSTR lpszClass = L"My Window Class";
LPCTSTR lpszWindowName = L"windows program 2";

// ─── 메인 진입점 ─────────────────────────────────────────────────────────────
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
    g_hInst = hInstance;

    WNDCLASSEX wc    = {};
    wc.cbSize        = sizeof(wc);
    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInstance;
    wc.hIcon         = LoadIcon(nullptr, IDI_APPLICATION);
    wc.hCursor       = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszClassName = lpszClass;
    wc.hIconSm       = LoadIcon(nullptr, IDI_APPLICATION);
    RegisterClassEx(&wc);

    const DWORD winStyle = WS_OVERLAPPEDWINDOW & ~(WS_THICKFRAME | WS_MAXIMIZEBOX);
    RECT rc = { 0, 0, LEN, HEI };
    AdjustWindowRect(&rc, winStyle, FALSE);

    g_hWnd = CreateWindow(lpszClass, lpszWindowName, WS_OVERLAPPEDWINDOW, 0, 0, LEN, HEI, NULL, (HMENU)NULL, hInstance, NULL);
    ShowWindow(g_hWnd, nCmdShow);

    ShowWindow(g_hWnd, nCmdShow);
    UpdateWindow(g_hWnd);

    HDC hdc    = GetDC(g_hWnd);
    g_hBackDC  = CreateCompatibleDC(hdc);
    g_hBackBmp = CreateCompatibleBitmap(hdc, LEN, HEI);
    g_hOldBmp  = (HBITMAP)SelectObject(g_hBackDC, g_hBackBmp);
    ReleaseDC(g_hWnd, hdc);

    InitGame();

    MSG msg = {};
    while (true) {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) break;
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else {
            DWORD now = GetTickCount();
            float dt  = (now - g_prevTick) / 1000.f;
            if (dt > 0.05f) dt = 0.05f;
            g_prevTick = now;

            UpdateGame(dt);
            RenderFrame(g_hBackDC);

            HDC winDC = GetDC(g_hWnd);
            BitBlt(winDC, 0, 0, LEN, HEI, g_hBackDC, 0, 0, SRCCOPY);
            ReleaseDC(g_hWnd, winDC);

            Sleep(1);
        }
    }

    SelectObject(g_hBackDC, g_hOldBmp);
    DeleteObject(g_hBackBmp);
    DeleteDC(g_hBackDC);

    return (int)msg.wParam;
}
