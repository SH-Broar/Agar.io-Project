#define _WIN32_WINNT 0x0400

#include <windows.h> // 윈도우 헤더 파일
#include <tchar.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <math.h>
#include <WinUser.h>

#define windowX 750
#define windowY 750

#define CandyMovementManage 1
#define VirusMovementManage 2
#define TrapMovementManage 3

typedef struct _Player_Data
{
	float rad = 0;
	float x;
	float y;
	double FromX;
	double FromY;
}Player_Data;

typedef struct _Players
{
	int MAXradius;
	int PlayerNumbers;
	Player_Data data[64];
}Players;

typedef struct _Virus_Data
{
	float rad = 0;
	float x;
	float y;
}Virus_Data;

typedef struct _Virus
{
	int VirusNumbers;
	Virus_Data data[10];
}Virus;

typedef struct _Trap_Data
{
	float x;
	float y;
	const float rad = 20;
	float dir;
}Trap_Data;

typedef struct _Trap
{
	int TrapNumbers;
	Trap_Data data[10];
}Trap;

typedef struct _SL_NODE
{
	POINT pt;
	COLORREF c;
	int id;
	int Tick;
	float Tickx;
	float Ticky;
	struct _SL_NODE* pNext;
	struct _SL_NODE* pPrev;
}SL_NODE;

typedef struct _SL_LIST
{
	unsigned int nDatanums;
	SL_NODE* head;
	SL_NODE* now;
}SL_LIST;

HINSTANCE g_hInst;
LPCTSTR lpszClass = _T("Window Class Name");

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow) //메인
{
	srand((unsigned int)time(NULL));
	HWND hWnd;
	MSG Message;
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
	WndClass.hbrBackground =
		(HBRUSH)GetStockObject(WHITE_BRUSH);
	WndClass.lpszMenuName = NULL;
	WndClass.lpszClassName = lpszClass;
	WndClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	RegisterClassEx(&WndClass);
	hWnd = CreateWindow(lpszClass, L"window program", WS_OVERLAPPEDWINDOW, 0, 0, windowX, windowY, NULL, (HMENU)NULL, hInstance, NULL);

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	while (GetMessage(&Message, 0, 0, 0))
	{
		TranslateMessage(&Message);
		DispatchMessage(&Message);
	}
	return Message.wParam;
}
//--- (x1, y1)과 (x2, y2)간의 길이
float LengthPts(int x1, int y1, int x2, int y2)
{
	return (sqrt((x2 - x1)*(x2 - x1) + (y2 - y1)*(y2 - y1)));
}
//--- (x, y)와 (mx, my)의 길이가 반지름보다 짧으면 true, 아니면 false
BOOL InCircle(int x, int y, int mx, int my, float BSIZE)
{
	if (LengthPts(x, y, mx, my) < BSIZE)
		return TRUE;
	else
		return FALSE;
}

static RECT clientRECT;
RECT tmpRECT;
static Players pls;
static Virus vis;
static Trap trs;
static POINT destination;
static int destinatoin_Tick;

static POINT Seed[60];
static float Tickx, Ticky;
static float Vickx, Vicky;
static float Cickx, Cicky;
static SL_LIST MySeed;
static int ClickTick;
static BOOL isGameOver;
SL_NODE* tmpNode;
static BOOL FixMode = FALSE;

static int PandemicTick;
static int TrapTick;

void CALLBACK TimerProc(HWND hWnd, UINT uMsg, UINT idEvent, DWORD dwTime)
{
	double V_MinDistance;
	int V_MinDistanceSeed;

	switch (idEvent)
	{
	case CandyMovementManage:
	{
		ClickTick++;
		for (int i = 0; i < pls.PlayerNumbers; i++)
		{
			float angle;
			if (FixMode == FALSE)
			{
				angle = atan2(destination.x - pls.data[i].FromX, destination.y - pls.data[i].FromY);
			}
			else
			{
				angle = atan2(destination.x - clientRECT.right / 2, destination.y - clientRECT.bottom / 2);
			}

			//Tickx = ((float)destination.x - (float)pls.data[i].FromX) / pls.data[i].rad;
			//Ticky = ((float)destination.y - (float)pls.data[i].FromY) / pls.data[i].rad;
			Tickx = sin(angle) * 200 / pls.data[i].rad;
			Ticky = cos(angle) * 200 / pls.data[i].rad;

			if ((pls.data[i].x - pls.data[i].rad > 0 && pls.data[i].x + pls.data[i].rad < clientRECT.right) || (pls.data[i].x - pls.data[i].rad <= 0 && Tickx > 0) || (pls.data[i].x + pls.data[i].rad >= clientRECT.right &&  Tickx < 0))
			{
				pls.data[i].x += Tickx / 2;
			}
			if ((pls.data[i].y - pls.data[i].rad > 0 && pls.data[i].y + pls.data[i].rad < clientRECT.bottom) || (pls.data[i].y - pls.data[i].rad <= 0 && Ticky > 0) || (pls.data[i].y + pls.data[i].rad >= clientRECT.bottom &&  Ticky < 0))
			{
				pls.data[i].y += Ticky / 2;
			}
			for (int j = 0; j < 60; j++)
			{
				if (InCircle(pls.data[i].x, pls.data[i].y, Seed[j].x, Seed[j].y, pls.data[i].rad + 5))
				{
					Seed[j].x = rand() % clientRECT.right;
					Seed[j].y = rand() % clientRECT.bottom;
					if (pls.data[i].rad < 270)
					{
						pls.data[i].rad += 2;
						if (pls.MAXradius < pls.data[i].rad)
							pls.MAXradius = pls.data[i].rad;
					}
				}
			}

			if (MySeed.head != NULL)
			{
				tmpNode = MySeed.head;
				MySeed.now = MySeed.head;
				if (InCircle(pls.data[i].x, pls.data[i].y, tmpNode->pt.x, tmpNode->pt.y, pls.data[i].rad + 5))
				{
					if (tmpNode->pNext == tmpNode)
					{
						MySeed.head = NULL;
						MySeed.now = NULL;
						MySeed.nDatanums = 0;
						pls.data[i].rad += 2;
						free(tmpNode);
						break;
					}
					else
					{
						MySeed.head = tmpNode->pNext;
						MySeed.now = MySeed.head;
						MySeed.nDatanums--;
						tmpNode->pNext->pPrev = tmpNode->pPrev;
						tmpNode->pPrev->pNext = tmpNode->pNext;
						pls.data[i].rad += 2;
						free(tmpNode);
						break;
					}
				}
				for (MySeed.now = MySeed.head; MySeed.now->pNext != MySeed.head; MySeed.now = MySeed.now->pNext)
				{
					tmpNode = MySeed.now;
					if (InCircle(pls.data[i].x, pls.data[i].y, tmpNode->pt.x, tmpNode->pt.y, pls.data[i].rad + 5))
					{
						MySeed.nDatanums--;
						tmpNode->pNext->pPrev = tmpNode->pPrev;
						tmpNode->pPrev->pNext = tmpNode->pNext;
						pls.data[i].rad += 2;
						MySeed.now = MySeed.now->pPrev;
						free(tmpNode);
						break;
					}
				}
				if (InCircle(pls.data[i].x, pls.data[i].y, MySeed.now->pt.x, MySeed.now->pt.y, pls.data[i].rad + 5))
				{
					MySeed.nDatanums--;
					tmpNode = MySeed.now;
					MySeed.now = MySeed.now->pPrev;
					tmpNode->pNext->pPrev = tmpNode->pPrev;
					tmpNode->pPrev->pNext = tmpNode->pNext;
					pls.data[i].rad += 2;
					free(tmpNode);
					break;
				}

				if (MySeed.nDatanums == 0)
				{
					MySeed.head = NULL;
					MySeed.now = NULL;
				}
			}
			if (ClickTick > 15)
			{
				for (int j = i + 1; j < pls.PlayerNumbers; j++)
				{
					if (InCircle(pls.data[i].x, pls.data[i].y, pls.data[j].x, pls.data[j].y, ((pls.data[i].rad + pls.data[i].rad) / 2)))
					{
						pls.data[i].rad = sqrt(pls.data[i].rad*pls.data[i].rad + pls.data[j].rad*pls.data[j].rad);
						if (pls.data[i].rad > 250)
							pls.data[i].rad = 250;
						for (int k = j; k < pls.PlayerNumbers - 1; k++)
						{
							pls.data[k].rad = pls.data[k + 1].rad;
							pls.data[k].x = pls.data[k + 1].x;
							pls.data[k].y = pls.data[k + 1].y;
						}
						pls.PlayerNumbers--;
					}
				}
			}
		}

		InvalidateRect(hWnd, &tmpRECT, false);
		break;
	}

	case VirusMovementManage:
	{
		V_MinDistance = 10000;
		if (isGameOver != TRUE)
			PandemicTick++;
		if (PandemicTick > 50)
		{
			if (vis.VirusNumbers < 9)
			{
				vis.VirusNumbers++;
				vis.data[vis.VirusNumbers].x = rand() % clientRECT.right;
				vis.data[vis.VirusNumbers].y = rand() % clientRECT.bottom;
				vis.data[vis.VirusNumbers].rad = 10;
			}
			PandemicTick = 0;
		}
		for (int i = 0; i < vis.VirusNumbers; i++)
		{
			for (int j = 0; j < 60; j++)
			{
				if (LengthPts(vis.data[i].x, vis.data[i].y, Seed[j].x, Seed[j].y) < V_MinDistance)
				{
					V_MinDistance = LengthPts(vis.data[i].x, vis.data[i].y, Seed[j].x, Seed[j].y);
					V_MinDistanceSeed = j;
				}
			}

			Vickx = (Seed[V_MinDistanceSeed].x - vis.data[i].x) / (vis.data[i].rad * 2);
			Vicky = (Seed[V_MinDistanceSeed].y - vis.data[i].y) / (vis.data[i].rad * 2);
			if (InCircle(vis.data[i].x, vis.data[i].y, pls.data[0].x, pls.data[0].y, vis.data[i].rad + pls.data[0].rad + 100))
			{
				if (vis.data[i].rad > pls.data[0].rad)
				{
					Vickx = (pls.data[0].x - vis.data[i].x) / (vis.data[i].rad * 2);
					Vicky = (pls.data[0].y - vis.data[i].y) / (vis.data[i].rad * 2);
				}
				else
				{
					Vickx = (pls.data[0].x - vis.data[i].x) / (vis.data[i].rad * 2) * -1 / 1.5f;
					Vicky = (pls.data[0].y - vis.data[i].y) / (vis.data[i].rad * 2) * -1 / 1.5f;
				}
			}

			if (vis.data[i].x + Vickx - vis.data[i].rad >= 0 && vis.data[i].x + vis.data[i].rad <= clientRECT.right)
				vis.data[i].x += Vickx;
			if (vis.data[i].y + Vicky - vis.data[i].rad >= 0 && vis.data[i].y + vis.data[i].rad <= clientRECT.bottom)
				vis.data[i].y += Vicky;

			for (int j = 0; j < 60; j++)
			{
				if (InCircle(vis.data[i].x, vis.data[i].y, Seed[j].x, Seed[j].y, vis.data[i].rad + 5))
				{
					Seed[j].x = rand() % clientRECT.right;
					Seed[j].y = rand() % clientRECT.bottom;
					if (vis.data[i].rad < 200)
					{
						vis.data[i].rad += 2;
					}
				}
			}

			if (MySeed.head != NULL)
			{
				tmpNode = MySeed.head;
				MySeed.now = MySeed.head;
				if (InCircle(vis.data[i].x, vis.data[i].y, tmpNode->pt.x, tmpNode->pt.y, vis.data[i].rad + 5))
				{
					if (tmpNode->pNext == tmpNode)
					{
						MySeed.head = NULL;
						MySeed.now = NULL;
						MySeed.nDatanums = 0;
						vis.data[i].rad += 2;
						free(tmpNode);
						break;
					}
					else
					{
						MySeed.head = tmpNode->pNext;
						MySeed.now = MySeed.head;
						MySeed.nDatanums--;
						tmpNode->pNext->pPrev = tmpNode->pPrev;
						tmpNode->pPrev->pNext = tmpNode->pNext;
						vis.data[i].rad += 2;
						free(tmpNode);
						break;
					}
				}
				for (MySeed.now = MySeed.head; MySeed.now->pNext != MySeed.head; MySeed.now = MySeed.now->pNext)
				{
					tmpNode = MySeed.now;
					if (InCircle(vis.data[i].x, vis.data[i].y, tmpNode->pt.x, tmpNode->pt.y, vis.data[i].rad + 5))
					{
						MySeed.nDatanums--;
						tmpNode->pNext->pPrev = tmpNode->pPrev;
						tmpNode->pPrev->pNext = tmpNode->pNext;
						vis.data[i].rad += 2;
						MySeed.now = MySeed.now->pPrev;
						free(tmpNode);
					}
				}
				if (InCircle(vis.data[i].x, vis.data[i].y, MySeed.now->pt.x, MySeed.now->pt.y, vis.data[i].rad + 5))
				{
					MySeed.nDatanums--;
					tmpNode = MySeed.now;
					MySeed.now = MySeed.now->pPrev;
					tmpNode->pNext->pPrev = tmpNode->pPrev;
					tmpNode->pPrev->pNext = tmpNode->pNext;
					vis.data[i].rad += 2;
					free(tmpNode);
				}

				if (MySeed.nDatanums == 0)
				{
					MySeed.head = NULL;
					MySeed.now = NULL;
				}
			}
			for (int j = 0; j < pls.PlayerNumbers; j++)
			{
				if (InCircle(vis.data[i].x, vis.data[i].y, pls.data[j].x, pls.data[j].y, vis.data[i].rad + pls.data[j].rad))
				{
					if (vis.data[i].rad > pls.data[j].rad)
					{
						if (vis.data[i].rad < 250)
							vis.data[i].rad += pls.data[j].rad / 2;
						for (int k = j; k < pls.PlayerNumbers - 1; k++)
						{
							pls.data[k].x = pls.data[k + 1].x;
							pls.data[k].y = pls.data[k + 1].y;
							pls.data[k].rad = pls.data[k + 1].rad;
							pls.data[k].FromX = pls.data[k + 1].FromX;
							pls.data[k].FromY = pls.data[k + 1].FromY;

						}
						pls.PlayerNumbers--;
						if (pls.PlayerNumbers == 0)
						{
							isGameOver = true;
							FixMode = FALSE;
						}
					}
					else
					{
						if (pls.data[j].rad < 270)
							pls.data[j].rad += vis.data[i].rad / 2;
						for (int k = i; k < vis.VirusNumbers - 1; k++)
						{
							vis.data[k].x = vis.data[k + 1].x;
							vis.data[k].y = vis.data[k + 1].y;
							vis.data[k].rad = vis.data[k + 1].rad;
						}
						vis.VirusNumbers--;
					}
				}
			}

			V_MinDistance = 10000;
		}
	}
	break;

	case TrapMovementManage:
	{
		TrapTick++;
		if (TrapTick > 99)
		{
			if (trs.TrapNumbers >= 10)
				break;
			TrapTick = 0;
			trs.data[trs.TrapNumbers].x = rand() % clientRECT.right - 80 + 40;
			trs.data[trs.TrapNumbers].y = rand() % clientRECT.bottom - 80 + 40;
			trs.data[trs.TrapNumbers].dir = atan2(trs.data[trs.TrapNumbers].x - rand() % clientRECT.right, trs.data[trs.TrapNumbers].y - rand() % clientRECT.bottom);
			trs.TrapNumbers++;
		}
		for (int i = 0; i < trs.TrapNumbers; i++)
		{
			Cickx = sin(trs.data[i].dir) * 5;
			Cicky = cos(trs.data[i].dir) * 5;

			trs.data[i].x += Cickx;
			trs.data[i].y += Cicky;
			if (trs.data[i].y < trs.data[i].rad)
			{
				trs.data[i].dir /= 2;
			}
			else if (trs.data[i].y > clientRECT.bottom - trs.data[i].rad)
			{
				trs.data[i].dir += trs.data[i].dir;
			}
			else if (trs.data[i].x < trs.data[i].rad || trs.data[i].x > clientRECT.right - trs.data[i].rad)
			{
				trs.data[i].dir *= -1;
			}
			int tmp = pls.PlayerNumbers;
			for (int j = 0; j < tmp; j++)
			{
				if (pls.data[j].rad > 20)
				{
					if (InCircle(trs.data[i].x, trs.data[i].y, pls.data[j].x, pls.data[j].y, trs.data[i].rad + pls.data[j].rad))
					{
						pls.data[j].rad *= 0.6;
						for (int k = 0; k < 4; k++)
						{
							pls.data[pls.PlayerNumbers].x = pls.data[j].x + ((rand() % 100 + pls.data[j].rad) - 100);
							pls.data[pls.PlayerNumbers].y = pls.data[j].y + ((rand() % 200 + pls.data[j].rad) - 100);
							pls.data[pls.PlayerNumbers].rad = pls.data[j].rad / (rand()%2 + 2);
							pls.PlayerNumbers++;
						}
						for (int k = i; k < trs.TrapNumbers-1; k++)
						{
							trs.data[k].x = trs.data[k + 1].x;
							trs.data[k].y = trs.data[k + 1].y;
							trs.data[k].dir = trs.data[k + 1].dir;
						}
						trs.TrapNumbers--;
					}
				}
			}
			for (int j = 0; j < vis.VirusNumbers; j++)
			{
				if (InCircle(trs.data[i].x, trs.data[i].y, vis.data[j].x, vis.data[j].y, trs.data[i].rad + vis.data[j].rad))
				{
					if (vis.data[j].rad > 20)
					{
						vis.data[j].rad /= 2;
						if (vis.VirusNumbers < 7)
						{
							vis.data[vis.VirusNumbers].x = vis.data[j].x + ((rand() % 100 + vis.data[j].rad) - 100);
							vis.data[vis.VirusNumbers].y = vis.data[j].y + ((rand() % 100 + vis.data[j].rad) - 100);
							vis.data[vis.VirusNumbers].rad = vis.data[j].rad / 1.414;
							vis.VirusNumbers++;
							vis.data[vis.VirusNumbers].x = vis.data[j].x + ((rand() % 100 + vis.data[j].rad) - 100);
							vis.data[vis.VirusNumbers].y = vis.data[j].y + ((rand() % 100 + vis.data[j].rad) - 100);
							vis.data[vis.VirusNumbers].rad = vis.data[j].rad / 1.414;
							vis.VirusNumbers++;
						}
						for (int k = i; k < trs.TrapNumbers - 1; k++)
						{
							trs.data[k].x = trs.data[k + 1].x;
							trs.data[k].y = trs.data[k + 1].y;
							trs.data[k].dir = trs.data[k + 1].dir;
						}
						trs.TrapNumbers--;
					}
					else
					{
						for (int k = j; k < vis.VirusNumbers - 1; k++)
						{
							vis.data[k].x = vis.data[k + 1].x;
							vis.data[k].y = vis.data[k + 1].y;
							vis.data[k].rad = vis.data[k + 1].rad;
						}
						vis.VirusNumbers--;
						for (int k = i; k < trs.TrapNumbers - 1; k++)
						{
							trs.data[k].x = trs.data[k + 1].x;
							trs.data[k].y = trs.data[k + 1].y;
							trs.data[k].dir = trs.data[k + 1].dir;
						}
						trs.TrapNumbers--;
					}
				}
			}
		}



	}
	break;
	}
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hDC;
	HDC memDC;
	HPEN hPen, oldPen;
	HBRUSH hBrush, oldBrush;
	HBITMAP hBitmap;
	int tmp;
	static COLORREF SeedColor[60];
	static POINT shot;

	static double ZoomIn;

	//static WCHAR text[100];
	//static int ttmp = 0;
	int x = 0, y = 0;
	switch (iMessage)
	{
	case WM_CREATE:
	RESET:
		pls.MAXradius = 10;
		pls.PlayerNumbers = 1;
		pls.data[0].rad = 10;
		pls.data[0].x = 375;
		pls.data[0].y = 375;
		vis.VirusNumbers = 1;
		PandemicTick = 0;
		vis.data[0].rad = 10;
		vis.data[0].x = 100;
		vis.data[0].y = 100;
		destination.x = 375;
		destination.y = 375;
		pls.data[0].FromX = 375;
		pls.data[0].FromY = 375;
		ZoomIn = 0;
		GetClientRect(hWnd, &clientRECT);
		GetClientRect(hWnd, &tmpRECT);
		tmpRECT.left -= 600;
		tmpRECT.top -= 600;
		tmpRECT.bottom += 600;
		tmpRECT.right += 600;
		trs.TrapNumbers = 3;
		TrapTick = 0;
		for (int i = 0; i < trs.TrapNumbers; i++)
		{
			trs.data[i].x = rand() % clientRECT.right - 80 + 40;
			trs.data[i].y = rand() % clientRECT.bottom - 80 + 40;
			trs.data[i].dir = atan2(trs.data[i].x - rand() % clientRECT.right, trs.data[i].y - rand() % clientRECT.bottom);
		}

		for (int i = 0; i < 60; i++)
		{
			Seed[i].x = rand() % clientRECT.right;
			Seed[i].y = rand() % clientRECT.bottom;
			SeedColor[i] = RGB(rand() % 255, rand() % 255, rand() % 255);
		}
		for (int i = 1; i < 10; i++)
		{
			vis.data[i].rad = 0;
		}

		MySeed.nDatanums = 0;
		MySeed.head = NULL;
		MySeed.now = NULL;

		SetTimer(hWnd, CandyMovementManage, 50, TimerProc);
		SetTimer(hWnd, VirusMovementManage, 50, TimerProc);
		SetTimer(hWnd, TrapMovementManage, 50, TimerProc);
		break;
	case WM_PAINT: //Paint 메세지 불렸을 때
		//
		hDC = BeginPaint(hWnd, &ps);
		memDC = CreateCompatibleDC(hDC);
		hBitmap = CreateCompatibleBitmap(hDC, clientRECT.right + 800, clientRECT.bottom + 800);
		SelectObject(memDC, (HBITMAP)hBitmap);
		//
		hBrush = CreateSolidBrush(RGB(255, 255, 255));
		oldBrush = (HBRUSH)SelectObject(memDC, hBrush);
		Rectangle(memDC, tmpRECT.left, tmpRECT.top, tmpRECT.right, tmpRECT.bottom);
		Rectangle(memDC, clientRECT.left, clientRECT.top, clientRECT.right + ZoomIn * 10, clientRECT.bottom + ZoomIn * 10);
		SelectObject(memDC, oldBrush);
		DeleteObject(hBrush);
		//
		//함정 출력할 위치
		//TextOut(hDC, 0, 10, text, 5);
		//
		if (FixMode == FALSE)
		{
			for (int i = 0; i < 60; i++)
			{
				hBrush = CreateSolidBrush(SeedColor[i]);
				oldBrush = (HBRUSH)SelectObject(memDC, hBrush);
				Ellipse(memDC, Seed[i].x - 5, Seed[i].y - 5, Seed[i].x + 5, Seed[i].y + 5);								//Seed
				SelectObject(memDC, oldBrush);
				DeleteObject(hBrush);
			}
			//
			if (MySeed.head != NULL)																					//MySeed
			{
				for (tmpNode = MySeed.head; tmpNode->pNext != MySeed.head; tmpNode = tmpNode->pNext)
				{
					if (tmpNode->pNext == NULL)
						break;
					if (tmpNode->Tick < 7)
					{
						tmpNode->pt.x += tmpNode->Tickx;
						tmpNode->pt.y += tmpNode->Ticky;
						tmpNode->Tick++;
					}
					hBrush = CreateSolidBrush(tmpNode->c);
					oldBrush = (HBRUSH)SelectObject(memDC, hBrush);
					Ellipse(memDC, tmpNode->pt.x - 5, tmpNode->pt.y - 5, tmpNode->pt.x + 5, tmpNode->pt.y + 5);
					SelectObject(memDC, oldBrush);
					DeleteObject(hBrush);

				}
				if (tmpNode->Tick < 7)
				{
					tmpNode->pt.x += tmpNode->Tickx;
					tmpNode->pt.y += tmpNode->Ticky;
					tmpNode->Tick++;
				}
				hBrush = CreateSolidBrush(tmpNode->c);
				oldBrush = (HBRUSH)SelectObject(memDC, hBrush);
				Ellipse(memDC, tmpNode->pt.x - 5, tmpNode->pt.y - 5, tmpNode->pt.x + 5, tmpNode->pt.y + 5);
				SelectObject(memDC, oldBrush);
				DeleteObject(hBrush);

			}
			//
			hBrush = CreateHatchBrush(HS_FDIAGONAL, RGB(0, 200, 50));
			oldBrush = (HBRUSH)SelectObject(memDC, hBrush);
			SetBkMode(memDC, OPAQUE);
			SetBkColor(memDC, RGB(240, 120, 0));
			for (int i = 0; i < pls.PlayerNumbers; i++)
			{
				Ellipse(memDC, pls.data[i].x - pls.data[i].rad, pls.data[i].y - pls.data[i].rad, pls.data[i].x + pls.data[i].rad, pls.data[i].y + pls.data[i].rad); //Player
			}
			SelectObject(memDC, oldBrush);
			DeleteObject(hBrush);
			//
			hBrush = CreateHatchBrush(HS_BDIAGONAL, RGB(50, 50, 0));
			oldBrush = (HBRUSH)SelectObject(memDC, hBrush);
			SetBkColor(memDC, RGB(100, 120, 240));
			for (int i = 0; i < vis.VirusNumbers; i++)
			{
				Ellipse(memDC, vis.data[i].x - vis.data[i].rad, vis.data[i].y - vis.data[i].rad, vis.data[i].x + vis.data[i].rad, vis.data[i].y + vis.data[i].rad); //Virus
			}
			SelectObject(memDC, oldBrush);
			DeleteObject(hBrush);
			//
			hPen = CreatePen(PS_DASH, 2, RGB(255, 0, 0));
			oldPen = (HPEN)SelectObject(memDC, hPen);
			for (int i = 0; i < trs.TrapNumbers; i++)
			{
				Ellipse(memDC, trs.data[i].x - trs.data[i].rad, trs.data[i].y - trs.data[i].rad, trs.data[i].x + trs.data[i].rad, trs.data[i].y + trs.data[i].rad); //Trap
			}
			SelectObject(memDC, oldPen);
			DeleteObject(hPen);
			//
			if (isGameOver == true)
			{
				SetBkColor(memDC, RGB(255, 255, 255));
				TextOut(memDC, clientRECT.right / 2 - 100, clientRECT.bottom / 2 - 10, L"Game Over. Press r to Restart.", 30);
			}
			BitBlt(hDC, 0, 0, clientRECT.right, clientRECT.bottom, memDC, 0, 0, SRCCOPY);
		}
		else //Fix
		{
			float Fixation;
			static int FQ = 400;
			Fixation = (clientRECT.right + ZoomIn * 10) / clientRECT.right;

			hBrush = CreateSolidBrush(RGB(255, 255, 255));
			oldBrush = (HBRUSH)SelectObject(memDC, hBrush);
			Rectangle(memDC, tmpRECT.left + 500, tmpRECT.top + 500, tmpRECT.right + 500, tmpRECT.bottom + FQ);
			Rectangle(memDC, clientRECT.left + FQ, clientRECT.top + FQ, clientRECT.right + ZoomIn * 10 + FQ, clientRECT.bottom + ZoomIn * 10 + FQ);
			SelectObject(memDC, oldBrush);
			DeleteObject(hBrush);

			for (int i = 0; i < 60; i++)
			{
				hBrush = CreateSolidBrush(SeedColor[i]);
				oldBrush = (HBRUSH)SelectObject(memDC, hBrush);
				Ellipse(memDC, Seed[i].x*Fixation - 5 - Fixation + FQ, Seed[i].y*Fixation - 5 - Fixation + FQ, Seed[i].x*Fixation + 5 + Fixation + FQ, Seed[i].y*Fixation + 5 + Fixation + FQ);				//Seed
				SelectObject(memDC, oldBrush);
				DeleteObject(hBrush);
			}
			//

			if (MySeed.head != NULL)																					//MySeed
			{
				for (tmpNode = MySeed.head; tmpNode->pNext != MySeed.head; tmpNode = tmpNode->pNext)
				{
					if (tmpNode->pNext == NULL)
						break;
					if (tmpNode->Tick < 7)
					{
						tmpNode->pt.x += tmpNode->Tickx;
						tmpNode->pt.y += tmpNode->Ticky;
						tmpNode->Tick++;
					}
					hBrush = CreateSolidBrush(tmpNode->c);
					oldBrush = (HBRUSH)SelectObject(memDC, hBrush);
					Ellipse(memDC, tmpNode->pt.x*Fixation - 5 - Fixation + FQ, tmpNode->pt.y*Fixation - 5 - Fixation + FQ, tmpNode->pt.x*Fixation + 5 + Fixation + FQ, tmpNode->pt.y*Fixation + 5 + Fixation + FQ);
					SelectObject(memDC, oldBrush);
					DeleteObject(hBrush);

				}
				if (tmpNode->Tick < 7)
				{
					tmpNode->pt.x += tmpNode->Tickx;
					tmpNode->pt.y += tmpNode->Ticky;
					tmpNode->Tick++;
				}
				hBrush = CreateSolidBrush(tmpNode->c);
				oldBrush = (HBRUSH)SelectObject(memDC, hBrush);
				Ellipse(memDC, tmpNode->pt.x*Fixation - 5 - Fixation + FQ, tmpNode->pt.y*Fixation - 5 - Fixation + FQ, tmpNode->pt.x*Fixation + 5 + Fixation + FQ, tmpNode->pt.y*Fixation + 5 + Fixation + FQ);
				SelectObject(memDC, oldBrush);
				DeleteObject(hBrush);

			}
			//
			hBrush = CreateHatchBrush(HS_FDIAGONAL, RGB(0, 200, 50));
			oldBrush = (HBRUSH)SelectObject(memDC, hBrush);
			SetBkMode(memDC, OPAQUE);
			SetBkColor(memDC, RGB(240, 120, 0));
			for (int i = 0; i < pls.PlayerNumbers; i++)
			{
				Ellipse(memDC, pls.data[i].x*Fixation - pls.data[i].rad*Fixation + FQ, pls.data[i].y*Fixation - pls.data[i].rad*Fixation + FQ, pls.data[i].x*Fixation + pls.data[i].rad*Fixation + FQ, pls.data[i].y*Fixation + pls.data[i].rad*Fixation + FQ); //Player
			}
			SelectObject(memDC, oldBrush);
			DeleteObject(hBrush);
			//
			hBrush = CreateHatchBrush(HS_BDIAGONAL, RGB(50, 50, 0));
			oldBrush = (HBRUSH)SelectObject(memDC, hBrush);
			SetBkColor(memDC, RGB(100, 120, 240));
			for (int i = 0; i < vis.VirusNumbers; i++)
			{
				Ellipse(memDC, vis.data[i].x*Fixation - vis.data[i].rad*Fixation + FQ, vis.data[i].y*Fixation - vis.data[i].rad*Fixation + FQ, vis.data[i].x*Fixation + vis.data[i].rad*Fixation + FQ, vis.data[i].y*Fixation + vis.data[i].rad*Fixation + FQ); //Virus
			}
			SelectObject(memDC, oldBrush);
			DeleteObject(hBrush);
			//
			hPen = CreatePen(PS_DASH, 2, RGB(255, 0, 0));
			oldPen = (HPEN)SelectObject(memDC, hPen);
			for (int i = 0; i < trs.TrapNumbers; i++)
			{
				Ellipse(memDC, trs.data[i].x*Fixation - trs.data[i].rad*Fixation + FQ, trs.data[i].y*Fixation - trs.data[i].rad*Fixation + FQ, trs.data[i].x*Fixation + trs.data[i].rad*Fixation + FQ, trs.data[i].y*Fixation + trs.data[i].rad*Fixation + FQ); //Trap
			}
			SelectObject(memDC, oldPen);
			DeleteObject(hPen);
			//
			if (isGameOver == true)
			{
				SetBkColor(memDC, RGB(255, 255, 255));
				TextOut(memDC, clientRECT.right / 2 - 100, clientRECT.bottom / 2 - 10, L"Game Over. Press r to Restart.", 30);
			}
			BitBlt(hDC, 0, 0, clientRECT.right, clientRECT.bottom, memDC, (pls.data[0].x)*Fixation - clientRECT.right / 2 + FQ, (pls.data[0].y)*Fixation - clientRECT.bottom / 2 + FQ, SRCCOPY);
		}


		EndPaint(hWnd, &ps);
		DeleteDC(memDC);
		DeleteObject(hBitmap);

		break;

	case WM_LBUTTONDOWN:
		if (isGameOver == true)
			break;
		tmp = pls.PlayerNumbers;
		ClickTick = 0;
		if (tmp > 32)
			break;
		for (int i = 0; i < tmp; i++)
		{
			if (pls.data[i].rad > 20)
			{
				pls.data[i].rad /= 1.414f;
				pls.data[pls.PlayerNumbers].rad = pls.data[i].rad;
				pls.data[pls.PlayerNumbers].x = pls.data[i].x + (Tickx*pls.data[i].rad / 3);
				pls.data[pls.PlayerNumbers].y = pls.data[i].y + (Ticky*pls.data[i].rad / 3);
				pls.data[pls.PlayerNumbers].FromX = pls.data[i].FromX;
				pls.data[pls.PlayerNumbers].FromY = pls.data[i].FromY;
				pls.PlayerNumbers++;
			}
		}
		break;

	case WM_RBUTTONDOWN:
		shot.x = LOWORD(lParam);
		shot.y = HIWORD(lParam);
		if (isGameOver == true)
			break;
		if (pls.data[0].rad > 10)
		{
			if (MySeed.head == NULL)
			{
				tmpNode = (SL_NODE*)malloc(sizeof(SL_NODE));

				tmpNode->c = RGB(rand() % 255, rand() % 255, rand() % 255);
				tmpNode->id = MySeed.nDatanums;
				tmpNode->pNext = tmpNode;
				tmpNode->pPrev = tmpNode;
				tmpNode->Tick = 0;
				if (FixMode == FALSE)
				{
					tmpNode->Tickx = (shot.x - pls.data[0].x) / 16;
					tmpNode->Ticky = (shot.y - pls.data[0].y) / 16;
				}
				else
				{
					tmpNode->Tickx = (shot.x - clientRECT.right / 2) / 16;
					tmpNode->Ticky = (shot.y - clientRECT.bottom / 2) / 16;
				}
				tmpNode->pt.x = pls.data[0].x + tmpNode->Tickx * 2;
				tmpNode->pt.y = pls.data[0].y + tmpNode->Ticky * 2;

				MySeed.head = tmpNode;
				MySeed.now = MySeed.head;
				MySeed.nDatanums++;
				pls.data[0].rad -= 2;
			}
			else
			{
				MySeed.now = MySeed.head;
				MySeed.now = MySeed.now->pPrev;
				if (MySeed.now->Tick == 7)
				{
					tmpNode = (SL_NODE*)malloc(sizeof(SL_NODE));

					MySeed.now = MySeed.head->pPrev;
					tmpNode->c = RGB(rand() % 255, rand() % 255, rand() % 255);
					tmpNode->id = MySeed.nDatanums;
					tmpNode->pNext = MySeed.head;
					tmpNode->pPrev = MySeed.now;
					MySeed.now->pNext = tmpNode;
					MySeed.head->pPrev = tmpNode;
					tmpNode->Tick = 0;
					if (FixMode == FALSE)
					{
						tmpNode->Tickx = (shot.x - pls.data[0].x) / 16;
						tmpNode->Ticky = (shot.y - pls.data[0].y) / 16;
					}
					else
					{
						tmpNode->Tickx = (shot.x - clientRECT.right / 2) / 16;
						tmpNode->Ticky = (shot.y - clientRECT.bottom / 2) / 16;
					}
					tmpNode->pt.x = pls.data[0].x + tmpNode->Tickx * 2;
					tmpNode->pt.y = pls.data[0].y + tmpNode->Ticky * 2;

					MySeed.now = tmpNode;
					MySeed.nDatanums++;

					pls.data[0].rad -= 2;
				}
			}
		}
		InvalidateRect(hWnd, NULL, false);
		break;

	case WM_MOUSEMOVE:
		destination.x = LOWORD(lParam);
		destination.y = HIWORD(lParam);
		for (int i = 0; i < pls.PlayerNumbers; i++)
		{
			pls.data[i].FromX = pls.data[i].x;
			pls.data[i].FromY = pls.data[i].y;
		}
		break;

	case WM_CHAR:
		switch (wParam)
		{
		case 'r':
			if (isGameOver == TRUE)
			{
				isGameOver = FALSE;
				goto RESET;
			}
			break;
		}
		break;

	case WM_MOUSEWHEEL:
	{
		int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
		if (isGameOver == true)
			break;
		if (zDelta > 0)
		{
			if (ZoomIn < 30)
			{
				ZoomIn++;
			}
			FixMode = TRUE;
		}
		else
		{
			ZoomIn--;
			if (ZoomIn <= 0)
			{
				ZoomIn = 0;
				FixMode = FALSE;
			}
		}

	}
	break;
	case WM_DESTROY: //Destroy 메세지 불렸을 때
		PostQuitMessage(0); //창 종료
		return 0;
	}
	return(DefWindowProc(hWnd, iMessage, wParam, lParam)); //처리되지 않은 메세지는 여기서 처리
}