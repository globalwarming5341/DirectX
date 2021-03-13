#include <windows.h>

#define WINDOW_WIDTH	800
#define WINDOW_HEIGHT	600
#define WINDOW_TITLE	L"DirectX"


LRESULT CALLBACK WndProc( HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam );  // 窗口过程处理函数

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,LPSTR lpCmdLine, int nShowCmd) {
	WNDCLASSEX wndClass = { 0 };
	wndClass.cbSize = sizeof(WNDCLASSEX);			                // 结构体字节数大小
	wndClass.style = CS_HREDRAW | CS_VREDRAW;	                    // 窗口的样式，CS_HREDRAW当窗口水平方向的宽度变化时重绘整个窗口.CS_VREDRAW当窗口垂直方向的宽度变化时重绘整个窗口.
	wndClass.lpfnWndProc = WndProc;					                // 指向窗口过程处理函数的指针
	wndClass.cbClsExtra	= 0;						                // 窗口类的附加内存，默认填0即可
	wndClass.cbWndExtra	= 0;						                // 窗口附加内存，默认填0即可
	wndClass.hInstance = hInstance;					                // 包含窗口过程的程序的实例句柄，hInstance。
	wndClass.hIcon = (HICON)::LoadImage(NULL, L"icon.ico", IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE);  //本地加载自定义ico图标
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);                 // 窗口类光标句柄。
	wndClass.hbrBackground = (HBRUSH)GetStockObject(GRAY_BRUSH);    // 窗口初始背景色为灰色	
	wndClass.lpszMenuName = NULL;						            // 菜单资源的名字。
	wndClass.lpszClassName = L"FirstDirectX";		                // 窗口类的名字。

	if(!RegisterClassEx(&wndClass))				                    // 对窗口类进行注册
		return -1;		

	HWND hwnd = CreateWindow(L"FirstDirectX",WINDOW_TITLE,		    // 创建窗口
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, WINDOW_WIDTH,
		WINDOW_HEIGHT, NULL, NULL, hInstance, NULL);

	MoveWindow(hwnd,250,80,WINDOW_WIDTH,WINDOW_HEIGHT,true);		// 设置窗口位置以及大小
	ShowWindow( hwnd, nShowCmd );                                   // 显示窗口
	UpdateWindow(hwnd);						                        // 更新窗口

	MSG msg = { 0 };
	while(msg.message != WM_QUIT)
	{
		if( PeekMessage(&msg, 0, 0, 0, PM_REMOVE))                  // 查询应用程序消息队列
		{
			TranslateMessage(&msg);		                            // 虚拟键消息转换为字符消息
			DispatchMessage(&msg);		                            // 分发消息给窗口。
		}
	}

	UnregisterClass(L"FirstDirectX", wndClass.hInstance);           // 注销窗口类
	return 0;  
}


LRESULT CALLBACK WndProc( HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam ) {
	switch (message) {
        case WM_PAINT:					// 客户区重绘消息
            ValidateRect(hwnd, NULL);	// 更新客户区显示
            break;						
        case WM_KEYDOWN:                // 键盘消息
            if (wParam == VK_ESCAPE)    // 如果按下ESC
                DestroyWindow(hwnd);	// 销毁窗口, 并发送一条WM_DESTROY消息
            break;						// 跳出该switch语句
        case WM_DESTROY:				// 窗口销毁消息
            PostQuitMessage(0);		    // 向系统表明有个线程有终止请求
            break;
        default:
            return DefWindowProc(hwnd, message, wParam, lParam);		// 缺省窗口处理过程
	}
	return 0;
}