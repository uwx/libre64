#pragma once
#include <Project64/WTLApp.h>

template <class T>
class ATL_NO_VTABLE CProject64Dialog :
    public CDialogImpl < T >
{
protected:
    static int CALLBACK WindowProc(HWND hDlg, uint32_t uMsg, uint32_t wParam, uint32_t lParam)
    {
        T * _this = (T *)::GetWindowLongPtr(hDlg, GWLP_USERDATA);
        BOOL res = false;
        if (uMsg == WM_INITDIALOG)
        {
            _this = (T *)lParam;
            _this->m_hWnd = hDlg;
            ::SetWindowLongPtr(hDlg, GWLP_USERDATA, (LONG_PTR)_this);
        }
        if (_this)
        {
            LRESULT result;
            res = _this->ProcessWindowMessage(hDlg, uMsg, wParam, lParam, result);
            if (uMsg == WM_DESTROY)
            {
                _this->m_hWnd = NULL;
                ::SetWindowLongPtr(hDlg, GWLP_USERDATA, (LONG_PTR)NULL);
            }
        }
        return res;
    }
};