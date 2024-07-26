#include "pch.h"
#include <iostream>
#include "ConsoleHelper.h"
#include "Board.h"
#include "Player.h"
// 1. 맵 설정, 플레이어 배치
// 2. 
Board board;
Player player;

int main()
{

	// 맵 설정을 위한 시드 지정
	::srand(static_cast<unsigned>(time(nullptr)));
	// 25칸 보드로 초기화
	board.Init(25, &player);
	player.Init(&board);

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
		player.Update(deltaTick);

		// 렌더링 : 게임을 화면에 그려줌
		board.Render();
	}
}