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
// 디테일한 설정들과 완성도로 가산점

#define LEN 1000
#define HEI 1000
#define JUMP_DUR 1200
#define PAC_SIZE 50
#define FOOD_INIT 20
#define SHOOT_COOL 200
#define BRICK_INIT 8

using namespace std;

random_device rd;
mt19937 g_rng(rd());
uniform_int_distribution<> disX{ 100, 900 };
uniform_int_distribution<> disY{ 100, 900 };

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

// 총알 크기: 사진 크기 48×8, 히트박스 40×6 (원본 비율 162:26 근사).
struct Bullet {
    float x = 0.f, y = 0.f;
    Dir   dir      = Dir::RIGHT;
    float speed    = 600.f;
    int   colorIdx = 3;
    bool  alive    = true;

    RECT GetHitBox() const {
        float cx = x + 24.f, cy = y + 4.f;
        if (dir == Dir::RIGHT || dir == Dir::LEFT) {
            return { (LONG)(cx - 20), (LONG)(cy - 3), (LONG)(cx + 20), (LONG)(cy + 3) };
        } else {
            return { (LONG)(cx - 3), (LONG)(cy - 20), (LONG)(cx + 3), (LONG)(cy + 20) };
        }
    }
};

struct Pacman {
    float x = 0.f, y = 0.f;
    int w = PAC_SIZE, h = PAC_SIZE;
    float speed = 200.f;
    Dir dir = Dir::RIGHT;
    int colorIdx = 3;
    int animFrame = 0;
    DWORD lastAnimTime = 0;
    DWORD lastShotTime = 0;
    // 점프 상태
    bool isJumping = false;
    DWORD jumpStart = 0;
    static constexpr DWORD JUMP_DURATION = JUMP_DUR;  // 점프 지속 시간
    bool isPaused = false;

    RECT GetHitBox() const {
        return { (LONG)x, (LONG)y, (LONG)(x + w), (LONG)(y + h) };
    }
};

// 복제 팩맨 관리
struct Clone {
    int slot;          // 1 / 2 / 3  >>  지연 = slot * CLONE_DELAY 레코드
    int colorIdx;
    int w, h;
    float x = 0.f, y = 0.f;
    Dir dir = Dir::RIGHT;
    int animFrame = 0;
    bool valid = false;
};

// ─── 장애물(Brick) ────────────────────────────────────────────────────────────
// type 0 : 50×50  (작은 정사각형, 소스 16×16)
// type 1 : 100×50 (넓은 직사각형, 소스 32×16), 회전 없음
// type 2 : 50×100 (높은 직사각형, 소스 32×16), R90CW 회전
//
// HP = colorIdx*2 (type 0) | colorIdx*3 (type 1/2)
struct Brick {
    float x = 0.f, y = 0.f;
    int type = 0;   // 0 / 1 / 2
    int colorIdx = 1;   // 1~5
    int hp = 0;
    int maxHp = 0;
    bool alive = true;

    int GetW() const { return (type == 1) ? 100 : 50; }
    int GetH() const { return (type == 2) ? 100 : 50; }

    RECT GetHitBox() const {
        return { (LONG)x, (LONG)y,
                 (LONG)(x + GetW()), (LONG)(y + GetH()) };
    }

    // 스프라이트 파손 상태
    int GetState() const {
        if (type == 0) {
            return (hp * 2 <= maxHp) ? 1 : 0;  // HP 절반 이하면 금 간 상태
        }
        if (hp * 3 <= maxHp) return 2;   // HP ≤ 1/3
        if (hp * 3 <= maxHp * 2) return 1;   // HP ≤ 2/3
        return 0;
    }
};

// ─── 먹이(Food) ───────────────────────────────────────────────────────────────
struct Food {
    float x = 0.f, y = 0.f;
    float centerX = 0.f, centerY = 0.f;
    int colorIdx = 1;
    int w = 40, h = 40;
    float speed = 100.f;
    bool alive = true;
    bool isHorizontal = true;
    Dir dir = Dir::RIGHT;
    int animFrame = 0;
    DWORD lastAnimTime = 0;

    RECT GetHitBox() const {
        return { (LONG)x, (LONG)y, (LONG)(x + w), (LONG)(y + h) };
    }
};

// ─── 이펙트(Effect) ───────────────────────────────────────────────────────────
struct Effect {
    float cx = 0.f, cy = 0.f;   // 화면 중심 좌표
    int animFrame = 0;
    DWORD lastAnimTime = 0;
    bool alive = true;
    int srcRow = 0;
    bool isPauseEffect = false; // 해당 애니메이션중 팩맨 일시정지
};

// ─── 클론 / 히스토리 상수 ────────────────────────────────────────────────────
constexpr int CLONE_DELAY     = 4;               // 슬롯당 지연 레코드 수 (4 = 200ms ≈ 40px, 중심 기준 ~35px 뒤)
constexpr int RECORD_INTERVAL = 50;              // 히스토리 스냅샷 간격 (ms)
constexpr int MAX_HISTORY     = CLONE_DELAY * 3 + 10;

// ─── 이미지 에셋 (색상별 인덱스 1-5) ────────────────────────────────────────
CImage img_pacman[6];
CImage img_bullet[6];
CImage img_ghost[6];
CImage img_effect;
CImage img_brick;

// ─── 전역 상태 ───────────────────────────────────────────────────────────────
HINSTANCE g_hInst;
HWND g_hWnd = nullptr;
HDC g_hBackDC = nullptr;
HBITMAP g_hBackBmp = nullptr;
HBITMAP g_hOldBmp = nullptr;

Pacman g_pacman;
vector<Bullet> g_bullets;
vector<Clone> g_clones;
vector<Brick> g_bricks;
vector<Food> g_foods;
vector<Effect> g_effects;
deque<PosRecord> g_posHistory;

DWORD g_prevTick = 0;
DWORD g_lastRecordTime = 0;

// 각 먹이는 스폰 중심에서 ±g_foodOscillateRange px 범위로 왕복
int g_foodOscillateRange = 40;
int g_foodRespawnPending = 0;
DWORD g_lastFoodRespawnTime = 0;

bool g_showDebug = false;
bool g_hKeyPressed = false;
bool g_jKeyPressed = false;
bool g_tKeyPressed = false;
bool g_aKeyPressed = false;

// ─── 스프라이트 선택 함수 ─────────────────────────────────────────────────────
SpriteInfo GetPacmanSprite(int frame) { return { frame * 16, 0, 16, 16 }; }
SpriteInfo GetBulletSprite()          { return { 29, 59, 162, 26 }; }

// 장애물 스프라이트
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

// 유령/먹이 스프라이트
SpriteInfo GetGhostSprite(int frame) { return { (frame % 8) * 16, 0, 16, 16 }; }

// 폭발 이펙트
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

// ─── 그리기 ─────────────────────────────────────────────────────────────

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

    // ── 소스 영역을 픽셀 버퍼에 복사 ───────────────
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

    // ── 픽셀 회전 ────────────────────────────────────────────────────
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

    // ── 반전 (회전된 버퍼에 적용) ───────────────────────────────────
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

    // ── 픽셀별 알파 조절 ─────────────────────────────────────────────
    if (alpha < 255) {
        for (auto& px : dstPx) {
            BYTE a = (BYTE)((unsigned)(px >> 24) * alpha / 255u);
            px = (px & 0x00FFFFFFu) | ((DWORD)a << 24);
        }
    }

    // ── 출력 DIB에 쓰고 화면에 알파 블렌딩 ──────────────────────────
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

// ─── 먹이 / 이펙트 ─────────────────────────────────────────────────────

void SpawnEffect(float cx, float cy, int srcRow = 0, bool isPauseEffect = false)
{
    Effect e;
    e.cx = cx;
    e.cy = cy;
    e.animFrame = 0;
    e.lastAnimTime = GetTickCount();
    e.alive = true;
    e.srcRow = srcRow;
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

// 'candidate' 영역이 맵 안에 완전히 있고, 팩맨/살아있는 먹이/살아있는 장애물과 겹치지 않으면 true 반환.
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

            // guard 충돌 체크
            if (IntersectRect(&dummy, &nr, &guard)) {
                blocked = true;
            }

            // 장애물 충돌 체크
            if (!blocked) {
                for (const auto& br : g_bricks) {
                    RECT brickBox = br.GetHitBox();
                    if (IntersectRect(&dummy, &nr, &brickBox)) {
                        blocked = true;
                        break;
                    }
                }
            }

            // 기존 먹이 충돌 체크
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

            // 어딘가에 부딪혔다면다음 시도로 넘어감
            if (blocked) continue;

            // 반복문 탈출
            g_foods.push_back(SpawnFoodAt(fx, fy, colorDis(g_rng)));
            break;
        }
    }
}

// ─── SpawnBricks ─────────────────────────────────────────────────────────────
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

    for (int i = 0; i < BRICK_INIT; i++) {
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
        PremultiplyAlpha(img_bullet[i]);
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
    SpawnFoodBatch(FOOD_INIT);
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

    // ── J: 점프 ────────────────────────────────────────────────────────────
    if (GetAsyncKeyState('J') & 0x8000) {
        if (!g_jKeyPressed && !g_pacman.isJumping) {
            g_jKeyPressed = true;
            g_pacman.isJumping = true;
            g_pacman.jumpStart = now;
        }
    }
    else { g_jKeyPressed = false; }

    if (g_pacman.isJumping && now - g_pacman.jumpStart >= Pacman::JUMP_DURATION)
        g_pacman.isJumping = false;

    // ── T: 클론 순환  0→1→2→3→0 ─────────────────────────────────────────────
    if (GetAsyncKeyState('T') & 0x8000) {
        if (!g_tKeyPressed) {
            g_tKeyPressed = true;
            if ((int)g_clones.size() >= 3) {
                g_clones.clear();
            }
            else {
                Clone c;
                c.slot = (int)g_clones.size() + 1;
                c.colorIdx = g_pacman.colorIdx;
                c.w = g_pacman.w;
                c.h = g_pacman.h;
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
            g_foodRespawnPending = 20;
            g_lastFoodRespawnTime = now;
        }
    }
    else { g_aKeyPressed = false; }

    if (g_foodRespawnPending > 0 && now - g_lastFoodRespawnTime >= 200) {
        g_lastFoodRespawnTime = now;
        SpawnFoodBatch(1);
        g_foodRespawnPending--;
    }

    if (!g_pacman.isPaused) {

        // ── 방향 입력 ────────────────────────────────────────────────────────────
        if (GetAsyncKeyState(VK_RIGHT) & 0x8000) g_pacman.dir = Dir::RIGHT;
        else if (GetAsyncKeyState(VK_LEFT) & 0x8000) g_pacman.dir = Dir::LEFT;
        else if (GetAsyncKeyState(VK_UP) & 0x8000) g_pacman.dir = Dir::UP;
        else if (GetAsyncKeyState(VK_DOWN) & 0x8000) g_pacman.dir = Dir::DOWN;

        // ── 발사 (Enter, 200ms 쿨타임) ───────────────────────────────────────────
        if ((GetAsyncKeyState(VK_RETURN) & 0x8000) && now - g_pacman.lastShotTime >= SHOOT_COOL) {
            g_pacman.lastShotTime = now;
            Bullet b;
            // 중심을 팩맨 중심에 맞춤: 중심 = (x+24, y+4)
            b.x = g_pacman.x + (g_pacman.w - 48) / 2.f;
            b.y = g_pacman.y + (g_pacman.h - 8) / 2.f;
            b.dir = g_pacman.dir;
            b.colorIdx = g_pacman.colorIdx;
            g_bullets.push_back(b);
        }

        // ── 플레이어 이동 ─────────────────────────────────────────────────────────
        switch (g_pacman.dir) {
        case Dir::RIGHT: g_pacman.x += g_pacman.speed * dt; break;
        case Dir::LEFT: g_pacman.x -= g_pacman.speed * dt; break;
        case Dir::UP: g_pacman.y -= g_pacman.speed * dt; break;
        case Dir::DOWN: g_pacman.y += g_pacman.speed * dt; break;
        }

        // ── 경계 반사 ────────────────────────────────────────────────────────────
        if (g_pacman.x < 0.f) {
            g_pacman.x = 0.f;
            g_pacman.dir = Dir::RIGHT;
        }
        else if (g_pacman.x + g_pacman.w > LEN) {
            g_pacman.x = (float)(LEN - g_pacman.w);
            g_pacman.dir = Dir::LEFT;
        }
        if (g_pacman.y < 0.f) {
            g_pacman.y = 0.f;
            g_pacman.dir = Dir::DOWN;
        }
        else if (g_pacman.y + g_pacman.h > HEI) {
            g_pacman.y = (float)(HEI - g_pacman.h);
            g_pacman.dir = Dir::UP;
        }

        // ── 팩맨-장애물 충돌 (점프 중에는 장애물을 통과) ───────────────────────
        if (!g_pacman.isJumping) {
            RECT pBox = g_pacman.GetHitBox();
            for (const auto& br : g_bricks) {
                if (!br.alive) continue;
                RECT brBox = br.GetHitBox();
                RECT dummy;
                if (!IntersectRect(&dummy, &pBox, &brBox)) continue;

                switch (g_pacman.dir) {
                case Dir::RIGHT:
                    g_pacman.x = (float)(brBox.left - g_pacman.w);
                    g_pacman.dir = Dir::LEFT;  break;
                case Dir::LEFT:
                    g_pacman.x = (float)brBox.right;
                    g_pacman.dir = Dir::RIGHT; break;
                case Dir::UP:
                    g_pacman.y = (float)brBox.bottom;
                    g_pacman.dir = Dir::DOWN;  break;
                case Dir::DOWN:
                    g_pacman.y = (float)(brBox.top - g_pacman.h);
                    g_pacman.dir = Dir::UP;    break;
                }
                // 밀어낸 후 화면 밖으로 나가지 않도록 보정
                g_pacman.x = max(0.f, min(g_pacman.x, (float)(LEN - g_pacman.w)));
                g_pacman.y = max(0.f, min(g_pacman.y, (float)(HEI - g_pacman.h)));
                break;
            }

        }

        // ── 스프라이트 애니메이션 ──────────────────────────────
        if (now - g_pacman.lastAnimTime > 100) {
            g_pacman.animFrame = (g_pacman.animFrame + 1) % 8;
            g_pacman.lastAnimTime = now;
        }

        // ── 위치 히스토리 스냅샷 (클론 경로 추적용) ──────────────────────────────
        if (now - g_lastRecordTime >= (DWORD)RECORD_INTERVAL) {
            g_lastRecordTime = now;
            g_posHistory.push_back({ g_pacman.x, g_pacman.y, g_pacman.dir, g_pacman.animFrame });
            while ((int)g_posHistory.size() > MAX_HISTORY)
                g_posHistory.pop_front();
        }

        // ── 히스토리에서 클론 위치 갱신 ─────────────────────────────────────────
        for (auto& c : g_clones) {
            int histIdx = (int)g_posHistory.size() - 1 - c.slot * CLONE_DELAY;
            if (histIdx >= 0) {
                const PosRecord& rec = g_posHistory[histIdx];
                c.x = rec.x;
                c.y = rec.y;
                c.dir = rec.dir;
                c.animFrame = rec.animFrame;
                c.valid = true;
            }
        }

        // ── 총알 이동 ────────────────────────────────────────────────────────────
        for (auto& b : g_bullets) {
            switch (b.dir) {
            case Dir::RIGHT: b.x += b.speed * dt; break;
            case Dir::LEFT: b.x -= b.speed * dt; break;
            case Dir::UP: b.y -= b.speed * dt; break;
            case Dir::DOWN: b.y += b.speed * dt; break;
            }
            // 중심 기준으로 화면 이탈 검사
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
                f.animFrame = (f.animFrame + 1) % 8;
                f.lastAnimTime = now;
            }

            // 축 방향 이동
            if (f.dir == Dir::RIGHT) f.x += f.speed * dt;
            else if (f.dir == Dir::LEFT) f.x -= f.speed * dt;
            else if (f.dir == Dir::DOWN) f.y += f.speed * dt;
            else f.y -= f.speed * dt;

            // 진동 범위
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

            // 장애물 반사
            RECT fBox = f.GetHitBox();
            for (const auto& br : g_bricks) {
                RECT brBox = br.GetHitBox();
                RECT dummy;
                if (!IntersectRect(&dummy, &fBox, &brBox)) continue;
                if (f.isHorizontal) {
                    if (f.dir == Dir::RIGHT) { f.x = (float)(brBox.left - f.w); f.dir = Dir::LEFT; }
                    else { f.x = (float)brBox.right; f.dir = Dir::RIGHT; }
                }
                else {
                    if (f.dir == Dir::DOWN) { f.y = (float)(brBox.top - f.h); f.dir = Dir::UP; }
                    else { f.y = (float)brBox.bottom; f.dir = Dir::DOWN; }
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
                g_pacman.w = (int)(g_pacman.w * 1.05f);
                g_pacman.h = (int)(g_pacman.h * 1.05f);
                SpawnEffect(f.x + f.w * 0.5f, f.y + f.h * 0.5f);
                f.alive = false;
                break;
            }
        }

        // ── 총알-먹이 충돌 (분열) ────────────────────────────────────────────────
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

                    bool placed = false;
                    for (int att = 0; att < 8 && !placed; att++) {
                        float nx = max(0.f, min((float)(LEN - 40), f.x + offsetDis(g_rng)));
                        float ny = max(0.f, min((float)(HEI - 40), f.y + offsetDis(g_rng)));
                        RECT nr = { (LONG)nx, (LONG)ny, (LONG)(nx + 40), (LONG)(ny + 40) };
                        bool ok = true;
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

        // ── 소멸된 총알, 장애물, 먹이 제거 ──────────────────────────────
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
    }

    // ── 이펙트 애니메이션 (isPaused 상태에서도 항상 진행) ────────────────────
    for (auto& e : g_effects) {
        if (!e.alive) continue;
        if (now - e.lastAnimTime > 70) { e.animFrame++; e.lastAnimTime = now; }
        if (e.animFrame >= 10) {
            e.alive = false;
            if (e.isPauseEffect) g_pacman.isPaused = false;
        }
    }

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

    // ── 장애물 ─────────────
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
        }
        else if (br.type == 1) {
            // 넓은 100×50: 소스 32×16, 회전 없음
            SpriteInfo sp = GetBrickSprite(1, state, br.colorIdx);
            if (!img_brick.IsNull()) {
                DrawSprite(hdc, drawX, drawY, 100, 50,
                    img_brick, sp.x, sp.y, sp.w, sp.h,
                    false, false, Rot::R0, 255);
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
        }
    }

    // ── 일반 이펙트 ─────────────────────────────────────────────
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
    }

    // ── 클론 ──────────────────────
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
                255);
        }
    }

    // ── 팩맨 ─────────────────────────────────────────────────────────────────
    {
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
    }

    // ── 좌클릭 폭발 이펙트 ─────────────────────────────────────────────────────
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

        // 먹이 히트박스
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

        // 장애물 히트박스 + HP 표시
        HPEN wPen = CreatePen(PS_SOLID, 1, RGB(200, 200, 200));
        SelectObject(hdc, wPen);
        SelectObject(hdc, GetStockObject(NULL_BRUSH));
        SetTextColor(hdc, RGB(255, 255, 255));

        SetBkMode(hdc, TRANSPARENT);

        for (const auto& br : g_bricks) {
            RECT hb = br.GetHitBox();
            Rectangle(hdc, hb.left, hb.top, hb.right, hb.bottom);
            wchar_t hp[16];
            swprintf_s(hp, L"%d/%d", br.hp, br.maxHp);
            TextOut(hdc, hb.left + 2, hb.top + 2, hp, (int)wcslen(hp));
        }
        DeleteObject(wPen);

        // 팩맨 히트박스
        HPEN rPen= CreatePen(PS_SOLID, 1, RGB(255, 50, 50));
        HPEN oldPen = (HPEN)SelectObject(hdc, rPen);
        RECT phb = g_pacman.GetHitBox();
        Rectangle(hdc, phb.left, phb.top, phb.right, phb.bottom);

        // 클론 히트박스
        HPEN cPen = CreatePen(PS_SOLID, 1, RGB(0, 210, 210));
        SelectObject(hdc, cPen);
        DeleteObject(rPen);
        for (const auto& c : g_clones) {
            if (c.valid)
                Rectangle(hdc, (int)c.x, (int)c.y,
                          (int)(c.x + c.w), (int)(c.y + c.h));
        }

        // 총알 히트박스
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
            break;
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
