#pragma once

class Board;

class Player
{
	enum
	{
		MOVE_TICK = 100
	};
public:
	void			Init(Board* board);
	void			Update(uint64 deltaTick);

	void			SetPos(Pos pos) { _pos = pos; }
	Pos				GetPos() { return _pos; }

	bool			CanGo(Pos pos);

private:
	void			RightHand();
	void			Bfs();
	void			AStar();

private:
	Pos				_pos = {};
	int32			_dir = DIR_UP;
	Board*			_board = nullptr;
	
	vector<Pos>		_path;
	int32			_pathIndex = 0; // 경로를 기준으로 얼만큼 이동했는지
	uint64			_sumTick = 0;
};

