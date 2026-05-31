#include <windows.h>
#include <tchar.h>
#include <string>
#include <vector>
#include <cwctype>
#include <algorithm>
#include "resource.h"

using namespace std;

HINSTANCE g_hInst;

// 계산기 상태 관리를 위한 전역 변수
std::wstring g_CurrentExpr = L"0";
std::wstring g_DecimalExpr = L""; // 이진수 변환 전 원래 식을 저장할 변수
bool g_isBinary = false;

BOOL CALLBACK Dlalog_Proc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

// -------------------------------
// 수식 문자열을 계산하여 double로 반환
double EvalExpression(std::wstring s) {
    s.erase(remove_if(s.begin(), s.end(), iswspace), s.end());
    if (s.empty()) return 0;

    vector<double> values;
    vector<wchar_t> ops;

    auto precedence = [](wchar_t op) {
        if (op == '+' || op == '-') return 1;
        if (op == '*' || op == '/') return 2;
        return 0;
        };

    auto applyOp = [&]() {
        if (values.size() < 2 || ops.empty()) return;
        double val2 = values.back(); values.pop_back();
        double val1 = values.back(); values.pop_back();
        wchar_t op = ops.back(); ops.pop_back();
        if (op == '+') values.push_back(val1 + val2);
        else if (op == '-') values.push_back(val1 - val2);
        else if (op == '*') values.push_back(val1 * val2);
        else if (op == '/') {
            if (val2 != 0) values.push_back(val1 / val2);
            else values.push_back(0); // 0으로 나누기 방지
        }
        };

    for (size_t i = 0; i < s.length(); i++) {
        if (s[i] == '-' && (i == 0 || s[i - 1] == '+' || s[i - 1] == '-' || s[i - 1] == '*' || s[i - 1] == '/')) {
            size_t j = i + 1;
            while (j < s.length() && (iswdigit(s[j]) || s[j] == '.')) j++;
            values.push_back(stod(s.substr(i, j - i)));
            i = j - 1;
        }
        else if (iswdigit(s[i]) || s[i] == '.') {
            size_t j = i;
            while (j < s.length() && (iswdigit(s[j]) || s[j] == '.')) j++;
            values.push_back(stod(s.substr(i, j - i)));
            i = j - 1;
        }
        else {
            while (!ops.empty() && precedence(ops.back()) >= precedence(s[i])) {
                applyOp();
            }
            ops.push_back(s[i]);
        }
    }
    while (!ops.empty()) applyOp();
    return values.empty() ? 0 : values.back();
}

// double 형을 소수점 불필요한 0을 제거한 문자열로 변환
std::wstring FormatDouble(double d) {
    std::wstring s = std::to_wstring(d);
    s.erase(s.find_last_not_of(L'0') + 1, std::wstring::npos);
    if (s.back() == L'.') s.pop_back();
    return s;
}

// 입력 보조 함수들
void CheckBinaryMode() {
    if (g_isBinary) { // 이진수 모드에서 다른 키를 누르면 원래 상태로 복구
        g_isBinary = false;
        g_CurrentExpr = g_DecimalExpr;
    }
}

void AppendStr(std::wstring str) {
    CheckBinaryMode();
    if (g_CurrentExpr == L"0" && str != L".") g_CurrentExpr = str;
    else g_CurrentExpr += str;
}

void AppendOp(std::wstring op) {
    CheckBinaryMode();
    if (g_CurrentExpr.empty()) return;
    wchar_t last = g_CurrentExpr.back();
    if (last == L'+' || last == L'-' || last == L'*' || last == L'/') {
        g_CurrentExpr.pop_back(); // 기존 연산자 덮어쓰기
    }
    g_CurrentExpr += op;
}
// -------------------------------------------------------------

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow)
{
    g_hInst = hInstance;
    DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, (DLGPROC)Dlalog_Proc);
    return 0;
}

BOOL CALLBACK Dlalog_Proc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    switch (iMsg) {
    case WM_INITDIALOG:
        g_CurrentExpr = L"0";
        SetDlgItemText(hDlg, IDC_EDIT1, g_CurrentExpr.c_str());
        SetDlgItemText(hDlg, IDC_BUTTON17, L"2\xC9C4\xC218");
        SetDlgItemText(hDlg, IDC_BUTTON25, L"\xC885\xB8CC");
        return TRUE;

    case WM_COMMAND:
    {
        if (HIWORD(wParam) != 0) return FALSE;

        int id = LOWORD(wParam);
        double val = 0.0;
        size_t pos = 0;
        std::wstring res = L"", temp = L"";

        switch (id) {
        case IDC_BUTTON1: // R 입력된 숫자들의 순서 바꾸기
            CheckBinaryMode();
            for (wchar_t c : g_CurrentExpr) {
                if (iswdigit(c) || c == L'.') temp = c + temp; // 역순 배치
                else {
                    res += temp + c;
                    temp = L"";
                }
            }
            res += temp;
            g_CurrentExpr = res;
            break;

        case IDC_BUTTON2: // CE 마지막 수식 제거 (예: 12+23+34 -> 12+23)
            CheckBinaryMode();
            pos = g_CurrentExpr.find_last_of(L"+-*/");
            if (pos != std::wstring::npos) g_CurrentExpr = g_CurrentExpr.substr(0, pos);
            else g_CurrentExpr = L"0"; // 연산자가 없으면 초기화
            break;

        case IDC_BUTTON3: // C 모두 지우기
            g_CurrentExpr = L"0";
            g_isBinary = false;
            break;

        case IDC_BUTTON4: // Backspace
            CheckBinaryMode();
            if (g_CurrentExpr.length() > 1) g_CurrentExpr.pop_back();
            else g_CurrentExpr = L"0";
            break;

            // 숫자 버튼 (5 ~ 18)
        case IDC_BUTTON5: AppendStr(L"7"); break;
        case IDC_BUTTON6: AppendStr(L"8"); break;
        case IDC_BUTTON7: AppendStr(L"9"); break;
        case IDC_BUTTON9: AppendStr(L"4"); break;
        case IDC_BUTTON10: AppendStr(L"5"); break;
        case IDC_BUTTON11: AppendStr(L"6"); break;
        case IDC_BUTTON13: AppendStr(L"1"); break;
        case IDC_BUTTON14: AppendStr(L"2"); break;
        case IDC_BUTTON15: AppendStr(L"3"); break;
        case IDC_BUTTON18: AppendStr(L"0"); break;

            // 연산자 버튼
        case IDC_BUTTON8: AppendOp(L"/"); break;
        case IDC_BUTTON12: AppendOp(L"*"); break;
        case IDC_BUTTON16: AppendOp(L"-"); break;
        case IDC_BUTTON20: AppendOp(L"+"); break;
        case IDC_BUTTON19: AppendStr(L"."); break; // 소수점

        case IDC_BUTTON17: // 2진수 변환
            if (g_isBinary) {
                g_CurrentExpr = g_DecimalExpr;
                g_isBinary = false;
            }
            else {
                g_DecimalExpr = g_CurrentExpr;
                long long num = (long long)EvalExpression(g_CurrentExpr);
                bool isNeg = num < 0;
                if (isNeg) num = -num;

                std::wstring binStr = L"";
                if (num == 0) binStr = L"0";
                else {
                    while (num > 0) {
                        binStr = (num % 2 == 0 ? L"0" : L"1") + binStr;
                        num /= 2;
                    }
                }
                if (isNeg) binStr = L"-" + binStr; // 음수 처리
                g_CurrentExpr = binStr;
                g_isBinary = true;
            }
            break;

        case IDC_BUTTON21: // 1/2
            CheckBinaryMode();
            val = EvalExpression(g_CurrentExpr) / 2.0;
            g_CurrentExpr = FormatDouble(val);
            break;

        case IDC_BUTTON22: // *10
            CheckBinaryMode();
            val = EvalExpression(g_CurrentExpr) * 10.0;
            g_CurrentExpr = FormatDouble(val);
            break;

        case IDC_BUTTON23: // ^2
            CheckBinaryMode();
            val = EvalExpression(g_CurrentExpr);
            val = val * val;
            g_CurrentExpr = FormatDouble(val);
            break;

        case IDC_BUTTON24: // = 수식 계산 후 결과 표시
            CheckBinaryMode();
            g_CurrentExpr = FormatDouble(EvalExpression(g_CurrentExpr));
            break;

        case IDC_BUTTON25: // 종료
            EndDialog(hDlg, 0);
            return TRUE;
        }

        SetDlgItemText(hDlg, IDC_EDIT1, g_CurrentExpr.c_str());
        return TRUE;
    }
    case WM_ERASEBKGND:
        return TRUE;
    case WM_CLOSE:
        EndDialog(hDlg, 0);
        return TRUE;
    }
    return FALSE;
}