#include "pch.h"

int main()
{
	uint64 lastTick = 0;
	// 대부분의 게임은 main안에서 반복문이 무한히 반복
	while (true)
	{
#pragma region 프레임관리
		const uint64 currentTick = ::GetTickCount64();
		const uint64 deltaTick = currentTick - lastTick;
		lastTick = currentTick;
#pragma endregion

		// 입력 : 키보드, 마우스 입력

		// 로직 : 플레이어의 이동 등 게임로직

		// 렌더링 : 게임을 화면에 그려줌

		ConsoleHelper::SetCursorPosition(0, 0);
		ConsoleHelper::ShowConsoleCursor(false);
		ConsoleHelper::SetCursorColor(ConsoleColor::RED);

		const char* TILE = "■";
		for (int32 y = 0; y < 25; y++)
		{
			for (int32 x = 0; x < 25; x++)
			{
				cout << TILE;
			}
			cout << endl;
		}
	}
}