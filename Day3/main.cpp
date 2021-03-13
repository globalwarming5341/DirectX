/*
 * @Author: Zhuang Zemin 
 * @Date: 2015-08-12 12:01:08 
 * @Last Modified by: Zhuang Zemin
 */
#include <windows.h>
#include <tchar.h>
#include <time.h>
#pragma comment(lib,"winmm.lib")
#pragma comment(lib,"Msimg32.lib")

#define WINDOW_WIDTH	1024
#define WINDOW_HEIGHT	768
#define WINDOW_TITLE	L"Game"
#define PARTICLE_NUMBER	150


struct CHARACTER {
	int	CurHP;
	int	MaxHP;
	int	CurMP;
	int	MaxMP;
	int	Level;
	int	Strength;
	int	Agility;
	int	Intelligence;
};

struct SNOW {
	int x;
	int y;
	BOOL exist;
};

enum ActionTypes {
	ACTION_TYPE_NORMAL = 0,
	ACTION_TYPE_CRITICAL = 1,
	ACTION_TYPE_MAGIC = 2,
	ACTION_TYPE_MISS = 3,
	ACTION_TYPE_RECOVER = 4,
};


HDC	g_hdc = NULL;
HDC g_mdc = NULL;
HDC g_bufdc = NULL;
DWORD g_tPre=0, g_tNow=0;	
RECT g_rect;
int g_iFrameNum, g_iTxtNum;
wchar_t	text[8][100];
BOOL g_bCanAttack, g_bIsGameOver;
SNOW ParticleList[PARTICLE_NUMBER];
int	g_ParticleNum = 0;
CHARACTER hero, enemy;
ActionTypes	HeroActionType, EnemyActionType;
HBITMAP	g_hBackGround, g_hLose,g_hWin, g_hParticle;  
HBITMAP	g_hEnemyBitmap, g_hHeroBitmap, g_hHealSkill;  
HBITMAP	g_hSkillButton1, g_hSkillButton2, g_hSkillButton3;  
HBITMAP	g_hHeroSkill1, g_hHeroSkill2, g_hHeroSkill3;  
HBITMAP	g_hEnemySkill1, g_hEnemySkill2, g_hEnemySkill3;  

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam); // 窗口过程处理函数
BOOL GameInit(HWND hwnd);
VOID GameLoop( HWND hwnd);
BOOL CleanGameRes(HWND hwnd );
VOID CheckState(int CurHP,bool isHero);
VOID PrintMessage(wchar_t* str);
VOID CheckHeroAction();
VOID DrawHeroAction();
VOID CheckEnemyAction();
VOID DrawEnemyAction();
VOID DrawParticle();

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,LPSTR lpCmdLine, int nShowCmd) {

    // 定义一个窗口类
	WNDCLASSEX wndClass = { 0 };
	wndClass.cbSize = sizeof(WNDCLASSEX);
	wndClass.style = CS_HREDRAW | CS_VREDRAW;
	wndClass.lpfnWndProc = WndProc;
	wndClass.cbClsExtra	= 0;
	wndClass.cbWndExtra	= 0;
	wndClass.hInstance = hInstance;
	wndClass.hIcon = (HICON)::LoadImage(NULL, L"icon.ico", IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE);
	wndClass.hCursor = LoadCursor( NULL, IDC_ARROW );
	wndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndClass.lpszMenuName = NULL;
	wndClass.lpszClassName = L"FirstDirectX";

    // 注册该窗口类
	if(!RegisterClassEx(&wndClass)) return -1;

    // 完成窗口类注册后，使用该窗口类的lpszClassName进行注册
	HWND hwnd = CreateWindow( L"FirstDirectX",WINDOW_TITLE,
	    WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, WINDOW_WIDTH,
		WINDOW_HEIGHT, NULL, NULL, hInstance, NULL );

    // 调整窗口位置，大小
	MoveWindow(hwnd,250,80,WINDOW_WIDTH,WINDOW_HEIGHT,true);

    // 显示该窗口
	ShowWindow(hwnd, nShowCmd);

    // 强制刷新界面，防止界面没有刷新
	UpdateWindow(hwnd);

	// 初始化游戏资源，例如图片，人物信息、字体
	if (!GameInit(hwnd)) return FALSE;
	
    // 播放背景音乐
	PlaySound(L"bgm.wav", NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);


    // = { 0 }、memset和Zeromemory方式都可以对结构体进行清零 
	MSG msg = { 0 };

    // 程序主循环
	while (msg.message != WM_QUIT) {
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) { // 有消息就返回TRUE，无消息返回FALSE
			TranslateMessage(&msg); // 用于产生字符键消息
			DispatchMessage(&msg);
		} else {
			g_tNow = GetTickCount(); // GetTickCount用于获取开机后的毫秒数，不含系统暂停时间
			if (g_tNow - g_tPre >= 50) GameLoop(hwnd); // 每隔60ms执行一次游戏循环体
		}

	}

	// 不需要用的窗口类就注销掉，释放内存
	UnregisterClass(L"FirstDirectX", wndClass.hInstance);  
	return 0;  
}




LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam ) {
	switch (message) {
	case WM_KEYDOWN:
		if(wParam==VK_ESCAPE) PostQuitMessage(0); // 按下ESC键退出
		break;
	case WM_LBUTTONDOWN: // 鼠标左键点击一个技能
		if (!g_bCanAttack) {
			int x = LOWORD(lParam);		
			int y = HIWORD(lParam);

			// 判断鼠标点击的位置是否在图标位置上
			if(x >= 530 && x <= 570 && y >=420 && y <= 470) {
				g_bCanAttack = true;
				HeroActionType=ACTION_TYPE_NORMAL;
			} else if(x >= 590 && x <= 640 && y >=420 && y <= 470) {
				g_bCanAttack = true;
				HeroActionType=ACTION_TYPE_MAGIC;
			} else if(x >= 650 && x <= 700 && y >=420 && y <= 470) {
				g_bCanAttack = true;
				HeroActionType=ACTION_TYPE_RECOVER;
			}
			
		}
		break;
	case WM_DESTROY:					
		CleanGameRes(hwnd);			
		PostQuitMessage(0);			
		break;									

	default:										
		return DefWindowProc( hwnd, message, wParam, lParam );		
	}

	return 0;									
}






BOOL GameInit(HWND hwnd) {
	srand((unsigned)time(NULL));      
	HBITMAP bmp;
	g_hdc = GetDC(hwnd);  
	g_mdc = CreateCompatibleDC(g_hdc);  
	g_bufdc = CreateCompatibleDC(g_hdc);
	bmp = CreateCompatibleBitmap(g_hdc,WINDOW_WIDTH,WINDOW_HEIGHT); 
	SelectObject(g_mdc,bmp);
	
	g_hLose = (HBITMAP)LoadImage(NULL,L"data\\lose.bmp",IMAGE_BITMAP,1086,396,LR_LOADFROMFILE);
	g_hWin = (HBITMAP)LoadImage(NULL,L"data\\win.bmp",IMAGE_BITMAP,800,600,LR_LOADFROMFILE);
	g_hBackGround = (HBITMAP)LoadImage(NULL,L"data\\bg.bmp",IMAGE_BITMAP,800,600,LR_LOADFROMFILE);
	g_hEnemyBitmap = (HBITMAP)LoadImage(NULL,L"data\\enemy.bmp",IMAGE_BITMAP,360,360,LR_LOADFROMFILE);
	g_hHeroBitmap = (HBITMAP)LoadImage(NULL,L"data\\hero.bmp",IMAGE_BITMAP,360,360,LR_LOADFROMFILE);
	g_hHeroSkill1 = (HBITMAP)LoadImage(NULL,L"data\\hero_skill_1.bmp",IMAGE_BITMAP,364,140,LR_LOADFROMFILE);
	g_hHeroSkill2 = (HBITMAP)LoadImage(NULL,L"data\\hero_skill_2.bmp",IMAGE_BITMAP,374,288,LR_LOADFROMFILE);
	g_hHeroSkill3 = (HBITMAP)LoadImage(NULL,L"data\\hero_skill_3.bmp",IMAGE_BITMAP,574,306,LR_LOADFROMFILE);
	g_hSkillButton1 = (HBITMAP)LoadImage(NULL,L"data\\skill_button_1.bmp",IMAGE_BITMAP,50,50,LR_LOADFROMFILE);
	g_hSkillButton2 = (HBITMAP)LoadImage(NULL,L"data\\skill_button_2.bmp",IMAGE_BITMAP,50,50,LR_LOADFROMFILE);
	g_hSkillButton3 = (HBITMAP)LoadImage(NULL,L"data\\skill_button_3.bmp",IMAGE_BITMAP,50,50,LR_LOADFROMFILE);
	g_hEnemySkill1 = (HBITMAP)LoadImage(NULL,L"data\\enemy_skill_1.bmp",IMAGE_BITMAP,234,188,LR_LOADFROMFILE);
	g_hEnemySkill2 = (HBITMAP)LoadImage(NULL,L"data\\enemy_skill_2.bmp",IMAGE_BITMAP,387,254,LR_LOADFROMFILE);
	g_hEnemySkill3 = (HBITMAP)LoadImage(NULL,L"data\\enemy_skill_3.bmp",IMAGE_BITMAP,574,306,LR_LOADFROMFILE);
	g_hParticle = (HBITMAP)LoadImage(NULL,L"data\\particle.bmp",IMAGE_BITMAP,30,30,LR_LOADFROMFILE);
	g_hHealSkill = (HBITMAP)LoadImage(NULL,L"data\\heal.bmp",IMAGE_BITMAP,150,150,LR_LOADFROMFILE);

	GetClientRect(hwnd, &g_rect);		

	hero.MaxHP = 200;
	hero.CurHP = 200;	
	hero.Level = 6;	
	hero.MaxMP = 60;				
	hero.CurMP = 60;    
	hero.Strength = 10;			
	hero.Agility = 20;				
	hero.Intelligence = 10;		

	enemy.MaxHP = 300;
	enemy.CurHP = 300;	
	enemy.Level = 10;						
	enemy.Strength = 10;				
	enemy.Agility = 10;                   
	enemy.Intelligence = 10;			

	g_iTxtNum = 0;		

	
	HFONT hFont;
	hFont = CreateFont(20, 0, 0, 0, 700, 0, 0, 0, GB2312_CHARSET, 0, 0, 0, 0, TEXT("微软雅黑"));
	SelectObject(g_mdc,hFont);
	SetBkMode(g_mdc, TRANSPARENT);

	GameLoop(hwnd);  
	return TRUE;
}




VOID GameLoop(HWND hwnd) {
	wchar_t str[100];

	// 贴上背景图
	SelectObject(g_bufdc,g_hBackGround);
	BitBlt(g_mdc,0,0,WINDOW_WIDTH,WINDOW_HEIGHT,g_bufdc,0,0,SRCCOPY);

	if (!g_bIsGameOver) {
		DrawParticle(); // 绘制粒子效果
	}

	// 文字显示
	SetTextColor(g_mdc,RGB(255,255,255));
	for(int i=0;i<g_iTxtNum;i++) TextOut(g_mdc,20,410+i*18,text[i],wcslen(text[i]));

	
	// 判断敌人的生命值
	if (enemy.CurHP>0) {
		SelectObject(g_bufdc,g_hEnemyBitmap);
		TransparentBlt(g_mdc,0,50,360,360,g_bufdc,0,0,360,360,RGB(0,0,0));
		swprintf_s(str,L"%d / %d",enemy.CurHP,enemy.MaxHP);
		SetTextColor(g_mdc,RGB(255,10,10));
		TextOut(g_mdc,100,370,str,wcslen(str));
	}

	// 判断自己的英雄生命值
	if(hero.CurHP>0) {
		
		SelectObject(g_bufdc,g_hHeroBitmap);
		TransparentBlt(g_mdc,400,50,360,360,g_bufdc,0,0,360,360,RGB(0,0,0));
		
		swprintf_s(str,L"%d / %d",hero.CurHP,hero.MaxHP);
		SetTextColor(g_mdc,RGB(255,10,10));
		TextOut(g_mdc,600,350,str, wcslen(str));
		
		swprintf_s(str,L"%d / %d",hero.CurMP,hero.MaxMP);
		SetTextColor(g_mdc,RGB(10,10,255));
		TextOut(g_mdc,600,370,str, wcslen(str));
	}

	// 判断游戏是否结束
	if(g_bIsGameOver) {
		if(hero.CurHP <= 0) {
			// 失败
			SelectObject(g_bufdc,g_hLose);
			BitBlt(g_mdc,120,50,543,396,g_bufdc,543,0,SRCAND);
			BitBlt(g_mdc,120,50,543,396,g_bufdc,0,0,SRCPAINT);
		} else {
			// 胜利
			SelectObject(g_bufdc,g_hWin);
			TransparentBlt(g_mdc,0,0,800,600,g_bufdc,0,0,800,600,RGB(0,0,0));
		}
	}
	
	// 是否处于可攻击（可点击）状态
	else if(!g_bCanAttack) {
		SelectObject(g_bufdc,g_hSkillButton1);
		BitBlt(g_mdc,530,420,50,50,g_bufdc,0,0,SRCCOPY);
		SelectObject(g_bufdc,g_hSkillButton2);
		BitBlt(g_mdc,590,420,50,50,g_bufdc,0,0,SRCCOPY);
		SelectObject(g_bufdc,g_hSkillButton3);
		BitBlt(g_mdc,650,420,50,50,g_bufdc,0,0,SRCCOPY);
	} else {
		g_iFrameNum++; // 帧数递增
		if(g_iFrameNum >= 5 && g_iFrameNum <= 10) {
			if(g_iFrameNum == 5) {			
				CheckHeroAction();
				CheckState(enemy.CurHP,false);
			}	
			DrawHeroAction();
		}

		
		if(g_iFrameNum == 15)
		{
			CheckEnemyAction();
		}

		
		if(g_iFrameNum>=26  && g_iFrameNum<=30)
		{
			DrawEnemyAction();
		}

		if(g_iFrameNum == 30)			
		{
			g_bCanAttack = false;
			g_iFrameNum = 0;

			
			if(!g_bIsGameOver)
			{
				int MpRecover=2*(rand()%hero.Intelligence)+6;
				hero.CurMP+=MpRecover;

				if (hero.CurMP>=hero.MaxMP)
				{
					hero.CurMP=hero.MaxMP;
				}
			
			swprintf_s(str,L"Recover %d mana",MpRecover);
			PrintMessage(str);
			}
		}
	}

	
	BitBlt(g_hdc,0,0,WINDOW_WIDTH,WINDOW_HEIGHT,g_mdc,0,0,SRCCOPY);
	g_tPre = GetTickCount();     
}




// 打印文本信息
void PrintMessage(wchar_t* str)
{
	
	if(g_iTxtNum < 8)
	{
		swprintf_s(text[g_iTxtNum],str);
		g_iTxtNum++;
	}
	
	else
	{
		for(int i=0;i<g_iTxtNum;i++)
			swprintf_s(text[i],text[i+1]);
			swprintf_s(text[7],str);
	}
}




// 检查人物状态
void CheckState(int CurHP,bool isHero) {
	wchar_t str[100];
	
	if(CurHP <= 0) {
		g_bIsGameOver = true;
		if (isHero) {
			PlaySound(L"GameMedia\\failure.wav", NULL, SND_FILENAME | SND_ASYNC); 
			swprintf_s(str,L"You lose!");  
			PrintMessage(str);  
		} else {
			PlaySound(L"GameMedia\\victory.wav", NULL, SND_FILENAME | SND_ASYNC); 
			swprintf_s(str,L"You win！");  
			PrintMessage(str);  
		}
	}
}



// 检查人物动作，攻击 Or 回复 
VOID CheckHeroAction() {
		int damage=0;
		wchar_t str[100];

		switch(HeroActionType) {
			case ACTION_TYPE_NORMAL:

				if (1==rand() % 4) {
					HeroActionType=ACTION_TYPE_CRITICAL;
					damage = (int)(4.5f*(float)(3*(rand()%hero.Agility) + hero.Level*hero.Strength+20));
					enemy.CurHP -= (int)damage;
					swprintf_s(str,L"Deals【%d】damage",damage);
				} else {
					damage = 3*(rand()%hero.Agility) + hero.Level*hero.Strength+20;
					enemy.CurHP -= (int)damage;
					swprintf_s(str,L"Deals【%d】damage",damage);
				}
				PrintMessage(str);
				break;

			case ACTION_TYPE_MAGIC:  
				if(hero.CurMP>=30) {
					damage = 5*(2*(rand()%hero.Agility) + hero.Level*hero.Intelligence);
					enemy.CurHP -= (int)damage;
					hero.CurMP-=30;
					swprintf_s(str,L"Deals【%d】damage",damage);
				} else {
					HeroActionType=ACTION_TYPE_MISS;
					swprintf_s(str,L"Insufficient mana！");
				}
				PrintMessage(str);
				break;

			case ACTION_TYPE_RECOVER:
					if(hero.CurMP>=40) {
						hero.CurMP-=40;
						int HpRecover=5*(5*(rand()%hero.Intelligence)+40);
						hero.CurHP+=HpRecover;
						if (hero.CurHP>=hero.MaxHP) {
							hero.CurHP=hero.MaxHP;
						}
						swprintf_s(str,L"Heals【%d】hit point。",HpRecover);
					} else {
						HeroActionType=ACTION_TYPE_MISS;
						swprintf_s(str,L"Insufficient mana！");
					}
					PrintMessage(str);
					break;
		}
}




// 绘制英雄动作
VOID DrawHeroAction() {
	switch (HeroActionType)
	{
	case ACTION_TYPE_NORMAL:   
		SelectObject(g_bufdc,g_hHeroSkill1);
		TransparentBlt(g_mdc,50,170,364,140,g_bufdc,0,0,364,140,RGB(0,0,0));
		break;

	case ACTION_TYPE_CRITICAL:  
		SelectObject(g_bufdc,g_hHeroSkill3);
		TransparentBlt(g_mdc,20,60,574,306,g_bufdc,0,0,574,306,RGB(0,0,0));
		break;

	case ACTION_TYPE_MAGIC:  
		SelectObject(g_bufdc,g_hHeroSkill2);
		TransparentBlt(g_mdc,50,100,374,288,g_bufdc,0,0,374,288,RGB(0,0,0));
		break;

	case ACTION_TYPE_RECOVER:   
		SelectObject(g_bufdc,g_hHealSkill);
		TransparentBlt(g_mdc,560,170,150,150,g_bufdc,0,0,150,150,RGB(0,0,0));
		break;
	}
}



// 检查敌人动作
VOID CheckEnemyAction() {
	srand((unsigned)time(NULL));      
	if(enemy.CurHP > (enemy.MaxHP/2))				
	{
		switch(rand()%3)
		{
		case 0:						
			EnemyActionType = ACTION_TYPE_NORMAL;
			break;
		case 1:						
			EnemyActionType = ACTION_TYPE_CRITICAL;
			break;
		case 2:						
			EnemyActionType = ACTION_TYPE_MAGIC;
			break;
		}
	} else {
		switch(rand() % 3) {
		case 0:						
			EnemyActionType = ACTION_TYPE_MAGIC;
			break;
		case 1:						
			EnemyActionType = ACTION_TYPE_CRITICAL;
			break;
		case 2:						
			EnemyActionType = ACTION_TYPE_RECOVER;
			break;
		}
	}
}



// 绘制敌人动作
VOID DrawEnemyAction() {
	int damage=0,recover=0;
	wchar_t str[100];
	switch(EnemyActionType) {
	case ACTION_TYPE_NORMAL:							
		SelectObject(g_bufdc,g_hEnemySkill1);
		TransparentBlt(g_mdc,500,150,234,188,g_bufdc,0,0,234,188,RGB(0,0,0));
		if(g_iFrameNum == 30) {
			damage = rand()%enemy.Agility+ enemy.Level*enemy.Strength;
			hero.CurHP -= (int)damage;

			swprintf_s(str,L"Deals【 %d】damage",damage);
			PrintMessage(str);

			CheckState(hero.CurHP,true);
		}
		break;

	case ACTION_TYPE_MAGIC:							
		SelectObject(g_bufdc,g_hEnemySkill2);
		TransparentBlt(g_mdc,450,150,387,254,g_bufdc,0,0,387,254,RGB(0,0,0));
		
		if(g_iFrameNum == 30) {
			damage = 5 * enemy.Intelligence;  
			hero.CurHP -= damage;	   
			recover	=(int)((float)damage*0.3f);   
			enemy.CurHP+=recover;
			swprintf_s(str,L"Deals %d damage and heals【%d】hit point",damage,recover);   
			PrintMessage(str);
			CheckState(hero.CurHP,true);
		}
		break;

	case ACTION_TYPE_CRITICAL:
		SelectObject(g_bufdc,g_hEnemySkill3);
		TransparentBlt(g_mdc,280,100,574,306,g_bufdc,0,0,574,306,RGB(0,0,0));
		if(g_iFrameNum == 30) {
			damage = 2 * (rand() % enemy.Agility + enemy.Level * enemy.Strength);
			hero.CurHP -= (int)damage;

			swprintf_s(str,L"Deal【%d】damage.",damage);
			PrintMessage(str);

			CheckState(hero.CurHP,true);
		}
		break;

	case ACTION_TYPE_RECOVER:
		SelectObject(g_bufdc, g_hHealSkill);
		TransparentBlt(g_mdc,150,150,150,150,g_bufdc,0,0,150,150,RGB(0,0,0));
		if(g_iFrameNum == 30) {
			recover = 2 * enemy.Intelligence * enemy.Intelligence;
			enemy.CurHP +=recover;
			swprintf_s(str,L"heals【%d】hit point",recover);
			PrintMessage(str);
		}
		break;
	}
}

// 绘制粒子效果
VOID DrawParticle() {
	if(g_ParticleNum< PARTICLE_NUMBER) {
		ParticleList[g_ParticleNum].x = rand()%g_rect.right;
		ParticleList[g_ParticleNum].y = 0;
		ParticleList[g_ParticleNum].exist = true; 
		g_ParticleNum++;
	}

	for (int i=0;i < PARTICLE_NUMBER; i++) {
		if (ParticleList[i].exist) {
			SelectObject(g_bufdc,g_hParticle);
			TransparentBlt(g_mdc,ParticleList[i].x,ParticleList[i].y,30,30,g_bufdc,0,0,30,30,RGB(0,0,0));
			if (rand()%2==0) ParticleList[i].x+=rand()%6;
			else ParticleList[i].x-=rand()%6;
			ParticleList[i].y+=10;
			if (ParticleList[i].y > g_rect.bottom) {
				ParticleList[i].x = rand()%g_rect.right;
				ParticleList[i].y = 0;
			}
		}

	}
}

// 清理资源
BOOL CleanGameRes(HWND hwnd) {
	DeleteObject(g_hBackGround);
	DeleteObject(g_hBackGround);
	DeleteObject(g_hLose);
	DeleteObject(g_hWin);
	DeleteObject(g_hParticle);
	DeleteObject(g_hEnemyBitmap);
	DeleteObject(g_hHeroBitmap);
	DeleteObject(g_hHealSkill);
	DeleteObject(g_hSkillButton1);
	DeleteObject(g_hSkillButton2);
	DeleteObject(g_hSkillButton3);
	DeleteObject(g_hHeroSkill1);
	DeleteObject(g_hHeroSkill2);
	DeleteObject(g_hHeroSkill3);
	DeleteObject(g_hEnemySkill1);
	DeleteObject(g_hEnemySkill2);
	DeleteObject(g_hEnemySkill3);
	DeleteDC(g_bufdc);
	DeleteDC(g_mdc);
	ReleaseDC(hwnd,g_hdc);
	return TRUE;
}