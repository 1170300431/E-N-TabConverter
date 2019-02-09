#pragma once
#include "stdafx.h"

using namespace std;

void form::run() {
	//�ڴ˴�����
	MSG msg;
	memset(&msg, 0, sizeof(msg));

	forAllControl([](control * me) {me->create(); });
	show();
	
	if (this->Event_Load_Complete) this->Event_Load_Complete(this);

	int loopret = 0;
	while ((loopret = GetMessage(&msg, this->Hwnd, 0, 0)) > 0) {
		if (loopret == -1) {
			popError();
			exit(1);
		}
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	fset.remove(id);
	window* w = fset.find(Classname);
	if (!w) UnregisterClass(Classname, hi);
}

control* form::getControl(HWND controlHwnd) {
	window* r = tab.find(controlHwnd);
	return (control*)r;
}

static LRESULT CALLBACK WinProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam){

	form* t = NULL;
	switch (message) {
	case WM_CREATE:
		t = (form*)(((LPCREATESTRUCT)lParam)->lpCreateParams);
		fset.add(t);
		break;
	default: t = (form*)fset.find(hwnd);
	}
	
	if (t) return t->winproc(hwnd, message, wParam, lParam);
	else return DefWindowProc(hwnd, message, wParam, lParam);
}

LRESULT CALLBACK form::winproc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
	case WM_PAINT:
		if (bitmapName) {
			HBITMAP hbm;
			BITMAP bminfo;
			hbm = LoadBitmap(GetModuleHandle(NULL), bitmapName);
			GetObject(hbm, sizeof(bminfo), &bminfo);
			PAINTSTRUCT ps;
			hdc = BeginPaint(hwnd, &ps);
			HDC memdc = CreateCompatibleDC(hdc);
			SelectObject(memdc, hbm);
			BitBlt(hdc, 0, 0, bminfo.bmWidth, bminfo.bmHeight, memdc, 0, 0, SRCCOPY);
			DeleteDC(memdc);
			EndPaint(hwnd, &ps);
		}
		if (Event_On_Paint) Event_On_Paint(this);
		break;
	case WM_CONTEXTMENU:
	{
		RECT rect;
		POINT pt;
		pt.x = LOWORD(lParam);
		pt.y = HIWORD(lParam);
		GetClientRect((HWND)wParam, &rect);
		//����Ļ����תΪ�ͻ�������  
		ScreenToClient((HWND)wParam, &pt);
		if (PtInRect(&rect, pt))
			if (!TrackPopupMenu(RBmenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON, LOWORD(lParam), HIWORD(lParam), 0, (HWND)wParam, NULL)) {
				if (RBmenu) MessageBox(NULL, TEXT("�����˵�ʧ��"), NULL, MB_OK);
			}
			else return DefWindowProc(hwnd, message, wParam, lParam);
		break;
	}
	case WM_TIMER:
		((Timer*)tab[wParam - 1])->Event_Timer(this);
		break;
	case WM_SIZE:
	{
		w = LOWORD(lParam);
		h = HIWORD(lParam);
		if (Event_Window_Resize) Event_Window_Resize(this);
		break;
	}
	case WM_MOVE:
		x = LOWORD(lParam);
		y = HIWORD(lParam);
		break;
	case WM_COMMAND:
		if (lParam)
		{
			//���ǿؼ�����¼�
			void* p = getControl((HWND)lParam);
			if (p) switch (((control*)p)->type) {
			case 'b':
				if (((Button*)p)->Event_On_Click) ((Button*)p)->Event_On_Click((Button*)p);
				break;
			case 'l':
				if (((Label*)p)->Event_On_Click) ((Label*)p)->Event_On_Click((Label*)p);
				break;
			case 'p':
				if (((Picture*)p)->Event_On_Click) ((Picture*)p)->Event_On_Click((Picture*)p);
				break;
			case 'r':
				if (((Radio*)p)->Event_On_Check) ((Radio*)p)->Event_On_Check((Radio*)p);
				break;
			case 'c':
				if (((Checkbox*)p)->Event_On_Check) ((Checkbox*)p)->Event_On_Check((Checkbox*)p);
				break;
			}
		}
		else {
			Event_Menu_Click(LOWORD(wParam));
		}
		break;
	case WM_CTLCOLORSTATIC://����WM_CTLCOLORSTATIC��Ϣ
		SetBkMode((HDC)wParam, TRANSPARENT);//���ñ���͸��
		return (INT_PTR)GetStockObject(brush);//���ظ�����ˢ
		break;
	case WM_CLOSE:
		if (Event_On_Unload) Event_On_Unload(this);
		DestroyWindow(hwnd);
		PostQuitMessage(0);
		break;
	case WM_CHAR:
		break;
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}

size_t window::create() {
	Hwnd = CreateWindow(
		this->Classname, this->Name,
		this->feature,
		this->x, this->y, this->w, this->h,
		Parent ? ((window*)Parent)->Hwnd : NULL,
		Menu, hi, this
	);
	if (Hwnd) return id;
	else {
		popError();
		exit(1);
	}
}

window::window(char type, LPTSTR classname, window* p, int x, int y, int w, int h)
	: x(x), y(y), w(w), h(h), Parent(p), Classname(classname), type(type),
		Text(Hwnd){
	menu.setContainer(this);
	menu.setter(&window::setMenu);
}

form::~form() {
	if (Parent) ((form*)Parent)->top();
	if (hdc) {
		//TODO
	}
}

form::form(form* parent, const TCHAR* clsName, const TCHAR* title, int x, int y, int w, int h) 
	: window('f', (LPTSTR)clsName, parent, x, y, w, h) {
	this->feature = WS_OVERLAPPEDWINDOW;
	this->name = (TCHAR*) title;
	
}

void form::forAllControl(void(*todo)(control*)) {
	for (window* i : (vector<window*>)tab) todo((control*)i);
}

bool form::regist() {
	WNDCLASSEX wndclass;
	wndclass.cbSize = sizeof(wndclass);
	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = WinProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hbrBackground = (HBRUSH)GetStockObject(brush);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	//ע�⣺�˴������䣬MSDN�м��أ�����Ǳ�׼ͼ�꣬��һ������Ӧ����NULL������hInstance. �˴���δ����������������ͨ�������ҷ���
	wndclass.hIcon = LoadIcon(hi, this->icon);
	wndclass.hIconSm = LoadIcon(hi, this->smallIcon);
	wndclass.hInstance = hi;
	wndclass.lpszClassName = Classname;
	wndclass.lpszMenuName = NULL;
	if (RegisterClassEx(&wndclass)) return true;
	else {
		DWORD x = GetLastError();
		if (x == 1410) return true;
		popError();
		return false;
	}
}

size_t form::create() {
	if (!regist()) return 0;
	window::create();
	if (x < 0) {
		RECT r;
		GetWindowRect(Hwnd, &r);
		x = r.left;
		y = r.top;
		w = r.right - x;
		h = r.bottom - y;
	}
	if (this->Event_On_Create) this->Event_On_Create(this);
	return id;
}

void form::paintLine(int x1, int y1, int x2, int y2) {
	//if x1 = x2 = y1 = y2 = 0 then end paint.
	PAINTSTRUCT ps;
	hdc = BeginPaint(Hwnd, &ps);
	MoveToEx(hdc, x1, y1, NULL);
	LineTo(hdc, x2, y2);
	EndPaint(Hwnd, &ps);
	hdc = NULL;
}

void form::paintLine(int x1, int y1, int x2, int y2, RECT* rect) {
	//if x1 = x2 = y1 = y2 = 0 then end paint.
	static PAINTSTRUCT ps;
	if (!hdc) {
		hdc = BeginPaint(Hwnd, &ps);
	}
	if (x1 || x2 || y1 || y2) {
		MoveToEx(hdc, x1, y1, NULL);
		LineTo(hdc, x2, y2);
	}
	else {
		EndPaint(Hwnd, &ps);
		hdc = NULL;
		UpdateWindow(Hwnd);
	}
}

window* windowSet::find(HWND hWnd) {
	std::vector<window*>::iterator r = find_if(pool.begin(), pool.end(), [hWnd](window* x) -> bool {return x && hWnd == x->hWnd(); });
	return r == pool.end() ? NULL : *r;
}

window* windowSet::find(LPTSTR clsname) {
	static std::vector<window*>::iterator r;
	if (clsname) r = pool.begin();
	else if (r == pool.end()) return NULL;
	else r++;

	r = find_if(r, pool.end(), [clsname](window* x) -> bool {
		return x && _tcscmp(x->classname(), clsname) == 0;
	});
	return r == pool.end() ? NULL : *r;
}

void windowSet::add(window* which) {
	std::vector<window*>::iterator r = std::find(pool.begin(), pool.end(), nullptr);
	if (r == pool.end()) {
		pool.push_back(which);
		which->id = pool.size();
	}
	else {
		which->id = r - pool.begin() + 1;
		pool[which->id - 1] = which;
	}
	size++;
}