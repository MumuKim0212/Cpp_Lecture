#include "pch.h"
#include "Player.h"
#include "Board.h"
#include <stack>

void Player::Init(Board* board)
{
	_pos = board->GetEnterPos();
	_board = board;

	//RightHand(); // 우측벽 따라가기
	//Bfs(); // 너비 우선 탐색
	AStar(); // A* 알고리즘
}

void Player::Update(uint64 deltaTick)
{
    if (_pathIndex >= _path.size())
    {
        _board->GenerateMap();
        Init(_board);
        return;
    }
	_sumTick += deltaTick;
	if (_sumTick >= MOVE_TICK)
	{
		_sumTick = 0;

		_pos = _path[_pathIndex];
		_pathIndex++;
	}
}

bool Player::CanGo(Pos pos)
{
	TileType tileType = _board->GetTileType(pos);
	return tileType == TileType::EMPTY;
}

void Player::RightHand()
{
	Pos pos = _pos;

	_path.clear();
	_path.push_back(pos);

	// 목적지 도착하기 전에는 계속 실행
	Pos dest = _board->GetExitPos();

	Pos front[4] =
	{
		Pos { -1, 0},	// UP
		Pos { 0, -1},	// LEFT
		Pos { 1, 0},	// DOWN
		Pos { 0, 1}		// RIGHT
	};

	while (pos != dest)
	{
		// 1. 현재 바라보는 방향을 기준으로 오른쪽으로 갈 수 있는지 체크
		int32 newDir = (_dir - 1 + DIR_COUNT) % DIR_COUNT;
		if (CanGo(pos + front[newDir]))
		{
			// 오른쪽 방향으로 90도 회전
			_dir = newDir;
			// 앞으로 한보 전진
			pos += front[_dir];

			_path.push_back(pos);

		}
		// 2. 현재 바라보는 방향을 기준으로 전진할 수 있는지 체크
		else if (CanGo(pos + front[_dir]))
		{
			// 앞으로 한보 전진
			pos += front[_dir];

			_path.push_back(pos);
		}
		else
		{
			// 왼쪽 방향으로 90도 회전
			_dir = (_dir + 1) % DIR_COUNT;
			/*
			switch (_dir)
			{
			case DIR_UP:
				_dir = DIR_LEFT;
				break;
			case DIR_LEFT:
				_dir = DIR_DOWN;
				break;
			case DIR_DOWN:
				_dir = DIR_RIGHT;
				break;
			case DIR_RIGHT:
				_dir = DIR_UP;
				break;
			}
			*/
		}
	}

	stack<Pos> s;

	for (int i = 0; i < _path.size() - 1; i++)
	{
		// 되돌아가는중
		if (s.empty() == false && s.top() == _path[i + 1])
			s.pop();
		// 정상진행중
		else
			s.push(_path[i]);
	}

	// 목적지 도착
	if (_path.empty() == false)
		s.push(_path.back());

	vector<Pos> path;
	while (s.empty() == false)
	{
		path.push_back(s.top());
		s.pop();
	}

	std::reverse(path.begin(), path.end());
	_path = path;
}

void Player::Bfs()
{
	Pos pos = _pos;

	// 목적지 도착하기 전에는 계속 실행
	Pos dest = _board->GetExitPos();

	Pos front[4] =
	{
		Pos {-1, 0},	// UP
		Pos {0, -1},	// LEFT
		Pos {1, 0},		// DOWN
		Pos {0, 1},		// RIGHT
	};

	// 발견했는지 여부 추적
	const int32 size = _board->GetSize();
	vector<vector<bool>> discovered(size, vector<bool>(size, false));

	// vector를 이용한 방법
	//vector<vector<Pos>> parent;
	// map을 이용한 방법
	// parent[A] = B -> A는 B로 인해 발견함;
	map<Pos, Pos> parent;

	queue<Pos> q;
	q.push(pos);
	discovered[pos.y][pos.x] = true;
	parent[pos] = pos;

	while (q.empty() == false)
	{
		pos = q.front();
		q.pop();

		// 방문
		// 도착했다면
		if (pos == dest)
			break;
		// 도착하지 않았으면
		for (int32 dir = 0; dir < 4; dir++)
		{
			Pos nextPos = pos + front[dir];
			// 갈 수 있는 지역인지 확인
			if (CanGo(nextPos) == false)
				continue;
			// 이미 발견한 지역인지 확인
			if (discovered[nextPos.y][nextPos.x])
				continue;

			q.push(nextPos);
			discovered[nextPos.y][nextPos.x] = true;
			parent[nextPos] = pos;
		}
	}

	_path.clear();

	// 거꾸로 거슬러 올라간다.
	pos = dest;

	while (true)
	{
		_path.push_back(pos);

		// 시작점은 자기자신이 부모이다
		if (pos == parent[pos])
			break;

		pos = parent[pos];
	}
	std::reverse(_path.begin(), _path.end());
}

struct PQNode
{
    bool operator<(const PQNode& other) const { return f < other.f; }
    bool operator>(const PQNode& other) const { return f > other.f; }

    int32   f; // f = g + h
    int32   g;
    Pos     pos;
};

void Player::AStar()
{
	// 점수 메기기
	// F = G + H
	// F: 시작점에서 목적지까지의 예상 비용. 작을수록 좋음
	// G: 시작점에서 해당 좌표까지의 비용
	// H: 목적지까지의 예상 비용. 고정값

	Pos start = _pos;

	// 목적지 도착하기 전에는 계속 실행
	Pos dest = _board->GetExitPos();

    enum
    {
        DIR_COUNT = 4
    };

	Pos front[] =
	{
		Pos {-1, 0},	// UP
		Pos {0, -1},	// LEFT
		Pos {1, 0},		// DOWN
		Pos {0, 1},		// RIGHT
        Pos {-1, -1},	// UP LEFT
        Pos {1, -1},	// DOWN LEFT
        Pos {1, 1},		// DOWN RIGHT
        Pos {-1, 1}		// UP RIGHT
	};

    int32 cost[] = 
    {
        // 1.4를 정수로 관리하기 위해 10을 곱해서 관리
        10, // UP
        10, // LEFT
        10, // DOWN
        10, // RIGHT
        14, // UP LEFT
        14, // DOWN LEFT
        14, // DOWN RIGHT
        14  // UP RIGHT
    };

    const int32 size = _board->GetSize();

    // Closed List: closed[y][x] = true -> 이미 방문한 곳
    vector<vector<bool>> closed(size, vector<bool>(size, false));

    // best[y][x] = 시작점에서 해당 좌표까지의 최소 비용
    vector<vector<int32>> best(size, vector<int32>(size, INT32_MAX));

    // 부모 추적 용도
    map<Pos, Pos> parent;

    // OpenList: 우선순위 큐
    priority_queue<PQNode, vector<PQNode>, greater<PQNode>> pq;

    // 1) 예약(발견)시스템 구현
    //  - 한 정점에 연결되어있는 정점들을 발견해 cost를 계산하고, 예약
    // 2) 뒤늦게 더 좋은 경로가 발견될 수 있음 -> 예외처리 필요

	// 초기값
    {
        int32 g = 0;
        int32 h = 10 * (abs(dest.y - start.y) + abs(dest.x - start.x));
        pq.push(PQNode{g + h, g, start});
        best[start.y][start.x] = g + h;
        parent[start] = start;
    }

    while (pq.empty() == false)
    {
        // 제일 좋은 후보를 찾는다
        PQNode node = pq.top();
        pq.pop();

        // 동일한 좌표를 여러 경로로 찾아서 더 빠른 경로로 인해 이미 방문된경우 skip
        
        // [선택]
        if (closed[node.pos.y][node.pos.x])
            continue;
        if (best[node.pos.y][node.pos.x] < node.f)
            continue;

        // 방문
        closed[node.pos.y][node.pos.x] = true;

        // 목적지에 도착했으면 종료
        if (node.pos == dest)
            break;

        for (int32 dir = 0; dir < DIR_COUNT; dir++)
        {
            Pos nextPos = node.pos + front[dir];
            // 갈 수 있는 지역인지 확인
            if (CanGo(nextPos) == false)
                continue;
            // [선택] 이미 방문한 곳이면 skip
            if (closed[nextPos.y][nextPos.x])
                continue;

            // 비용 계산
            int32 g = node.g + cost[dir];
            int32 h = 10 * (abs(dest.y - nextPos.y) + abs(dest.x - nextPos.x));
            // 다른 경로에서 더 빠른 길을 찾았으면 skip
            if (best[nextPos.y][nextPos.x] <= g + h)
                continue;

            // 예약 진행
            best[nextPos.y][nextPos.x] = g + h;
            pq.push(PQNode{ g + h, g, nextPos });
            parent[nextPos] = node.pos;
        }
    }

    Pos pos = dest;

    // 거꾸로 거슬러 올라간다.
    _path.clear();
    _pathIndex = 0;
          
    while (true)
    {
        _path.push_back(pos);

        // 시작점은 자기자신이 부모이다
        if (pos == parent[pos])
            break;

        pos = parent[pos];
    }
    std::reverse(_path.begin(), _path.end());
}

