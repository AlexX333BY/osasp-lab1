#include <windows.h>

#define WND_CLASS_NAME "LaboratoryWork1Class"
#define WM_LOAD_SPRITE WM_USER
#define VK_L 0x4c

bool PostLoadSpriteMessage(HWND hWnd)
{
	return PostMessage(hWnd, WM_LOAD_SPRITE, NULL, NULL);
}

SIZE GetDeviceContextDimensions(HDC hdc)
{
	BITMAP structBitmapHeader;
	memset(&structBitmapHeader, 0, sizeof(BITMAP));
	GetObject(GetCurrentObject(hdc, OBJ_BITMAP), sizeof(BITMAP), &structBitmapHeader);
	SIZE size;
	size.cx = structBitmapHeader.bmWidth;
	size.cy = structBitmapHeader.bmHeight;
	return size;
}

int FillDeviceContextWithColor(HDC hdc, HBRUSH hBrush)
{
	SIZE size = GetDeviceContextDimensions(hdc);
	RECT rect;
	rect.left = 0;
	rect.top = 0;
	rect.right = size.cx;
	rect.bottom = size.cy;
	return FillRect(hdc, &rect, hBrush);
}

bool PutSpriteOnWindow(HDC wndDC, HDC spriteDC)
{
	SIZE bitmapSize = GetDeviceContextDimensions(spriteDC);
	return BitBlt(wndDC, 0, 0, bitmapSize.cx, bitmapSize.cy, spriteDC, 0, 0, SRCCOPY);
}

bool LoadSprite(HWND hWnd, HDC &spriteDC)
{
	char fileName[MAX_PATH] = { NULL };

	OPENFILENAME openFileName;
	openFileName.lStructSize = sizeof(OPENFILENAME);
	openFileName.hwndOwner = hWnd;
	openFileName.hInstance = NULL;
	openFileName.lpstrFilter = "Bitmap images\0*.bmp\0\0";
	openFileName.lpstrCustomFilter = NULL;
	openFileName.nFilterIndex = 1;
	openFileName.lpstrFile = fileName;
	openFileName.nMaxFile = sizeof(fileName);
	openFileName.lpstrFileTitle = NULL;
	openFileName.lpstrInitialDir = NULL;
	openFileName.lpstrTitle = "Select sprite image";
	openFileName.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
	openFileName.lpstrDefExt = ".bmp";

	if (GetOpenFileName(&openFileName))
	{
		HANDLE sprite = LoadImage(NULL, fileName, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_LOADTRANSPARENT);
		if (sprite == NULL)
		{
			MessageBox(hWnd, "Error while loading image", "Error", MB_OK | MB_ICONERROR);
			return false;
		}

		HDC wndDC = GetDC(hWnd);
		spriteDC = CreateCompatibleDC(wndDC);
		SelectObject(spriteDC, sprite);

		FillDeviceContextWithColor(wndDC, (HBRUSH)(COLOR_WINDOW + 1));
		return PutSpriteOnWindow(wndDC, spriteDC);
	}
	return false;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HDC spriteDC = NULL;

	switch (message)
	{
	case WM_LOAD_SPRITE:
		LoadSprite(hWnd, spriteDC);
		break;
	case WM_KEYDOWN:
		if (wParam == VK_L)
		{
			PostLoadSpriteMessage(hWnd);
			break;
		}
		if (wParam != VK_ESCAPE)
		{
			break;
		}
	case WM_CLOSE:
		if (MessageBox(hWnd, "Are you sure you want to exit?", "Exit question", MB_YESNO | MB_ICONQUESTION) == IDYES)
		{
			PostQuitMessage(0);
		}
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	WNDCLASSEX wndClassEx;
	wndClassEx.cbSize = sizeof(WNDCLASSEX);
	wndClassEx.style = CS_DBLCLKS;
	wndClassEx.lpfnWndProc = WndProc;
	wndClassEx.cbClsExtra = 0;
	wndClassEx.cbWndExtra = 0;
	wndClassEx.hInstance = hInstance;
	wndClassEx.hIcon = NULL;
	wndClassEx.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClassEx.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wndClassEx.lpszMenuName = NULL;
	wndClassEx.lpszClassName = WND_CLASS_NAME;
	wndClassEx.hIconSm = NULL;
	RegisterClassEx(&wndClassEx);

	HWND hWnd = CreateWindow(WND_CLASS_NAME, "Lab 1", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	MSG msg;
	PostLoadSpriteMessage(hWnd);
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}
