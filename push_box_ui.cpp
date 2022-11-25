#include <graphics.h>		// 引用图形库头文件
#include <conio.h>
#include <windows.h>
#include <fstream>

#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <stack>

using namespace std;

class Pos
{
public:
	int x;
	int y;
	Pos() {};
	Pos(int x, int y) :x(x), y(y) {};
	bool operator == (const Pos& b) const
	{
		return x == b.x && y == b.y;
	}
	bool operator < (const Pos& b) const
	{
		if (x < b.x)
		{
			return true;
		}
		else if (x == b.x)
		{
			if (y < b.y)
			{
				return true;
			}
			else
			{
				return false;
			}
		}
		else
		{
			return false;
		}
	}
	friend ostream& operator<<(ostream& output,
		const Pos& pos)
	{
		output << "(" << pos.x << "," << pos.y << ")";
		return output;
	}
};
class State
{
public:
	Pos player;
	vector<Pos> box;
	int from_state = -1;
	int from_action = -1;//上下左右 0,1,2,3
	int score = 0;

	State() {};
	State(Pos player, vector<Pos> box) :player(player), box(box.begin(), box.end()) {
		sort(box.begin(), box.end());
	};
	/*
	~State() {
		vector<Pos>().swap(box);
	}
	*/

	void sort_box() {
		sort(box.begin(), box.end());
	}
	void update_score(State& end_state) {
		int tmp = 0;
		for (int i = 0; i < box.size(); i++)
		{
			tmp = tmp + abs(box[i].x - end_state.box[i].x) + abs(box[i].y - end_state.box[i].y);
		}
		score = tmp;
	}
	bool operator == (const State& b) const
	{
		if (this->player == b.player && this->box == b.box)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	bool operator < (const State& b) const
	{
		return score < b.score;

	}
	bool operator > (const State& b) const
	{
		return score > b.score;

	}
	friend ostream& operator<<(ostream& output,
		State& state)
	{

		output << "------" << endl << state.player << endl;
		for (vector<Pos>::iterator iter = state.box.begin(); iter != state.box.end(); iter++)
		{
			output << *iter << endl;
		}
		output << "------" << endl;
		return output;
	}
};

State start_state;
State end_state;
int** map_matrix;
vector<Pos> wall_pos;
vector<Pos> des_pos;
int box_width = 64;
IMAGE player, wall, box, des;

void draw(vector<State> &searched,int i) {
	cleardevice();
	wchar_t s[] = L"求解完毕，按方向键左右查看结果";
	
	outtextxy(200, 600, s);

	for (vector<Pos>::iterator iter = wall_pos.begin(); iter != wall_pos.end(); iter++)
	{
		putimage(box_width * (*iter).y, box_width * (*iter).x, box_width, box_width, &wall, 0, 0);
	}
	for (vector<Pos>::iterator iter = des_pos.begin(); iter != des_pos.end(); iter++)
	{
		putimage(box_width * (*iter).y, box_width * (*iter).x, box_width, box_width, &des, -16, -16);
	}
	for (vector<Pos>::iterator iter = searched[i].box.begin(); iter != searched[i].box.end(); iter++)
	{
		putimage(box_width * (*iter).y, box_width * (*iter).x, box_width, box_width, &box, 0, 0);
	}
	putimage(box_width * searched[i].player.y, box_width * searched[i].player.x, box_width, box_width, &player, -16, 0);

	switch (searched[i].from_action)
	{
	case 0:
		outtextxy(600, 200, _T("上"));
		break;
	case 1:
		outtextxy(600, 200, _T("下"));
		break;
	case 2:
		outtextxy(600, 200, _T("左"));
		break;
	case 3:
		outtextxy(600, 200, _T("右"));
		break;
	default:
		break;
	}


	//wchar_t s[] = L"看好了我只示范一次 (•̀ᴗ• ))";
	//outtextxy(200, 600, s);
	// 
	//wchar_t s[] = L"看好了我只示范一次 (•̀ᴗ• ))";
	//outtextxy(200, 600, s);

	wchar_t s1[] = L"made by zy";
	outtextxy(700, 780, s1);
}
void get_pos(int direction, Pos& pos) {
	switch (direction)
	{
	case 0:
		pos.x--;
		break;
	case 1:
		pos.x++;
		break;
	case 2:
		pos.y--;
		break;
	case 3:
		pos.y++;
		break;
	default:
		break;
	}
}
vector<State> move(State* state_p) {
	vector<State> search_list;
	for (int i = 0; i < 4; i++)
	{
		Pos player_move(state_p->player);
		get_pos(i, player_move);
		if (map_matrix[player_move.x][player_move.y] == '#')//玩家碰到墙
		{
			continue;
		}
		else
		{
			vector<Pos>::iterator box_index = find(state_p->box.begin(), state_p->box.end(), player_move);
			if (box_index != state_p->box.end())//玩家碰到箱子
			{
				Pos box_move(*box_index);
				get_pos(i, box_move);
				vector<Pos>::iterator box2_index = find(state_p->box.begin(), state_p->box.end(), box_move);
				if (map_matrix[box_move.x][box_move.y] == '#') {//箱子碰到墙
					continue;
				}
				else if (box2_index != state_p->box.end()) {//箱子碰到箱子
					continue;
				}
				else//推箱子
				{
					// vector<Pos> box_list(state_p->box);
					// box_list[box_index-state_p->box.begin()]=box_move;
					State tmp(*state_p);
					tmp.box[box_index - state_p->box.begin()] = box_move;
					tmp.sort_box();
					tmp.update_score(end_state);

					tmp.player = player_move;
					tmp.from_action = i;
					search_list.push_back(tmp);
				}
			}
			else//玩家没有碰到箱子
			{
				State tmp(*state_p);
				tmp.player = player_move;
				tmp.from_action = i;

				search_list.push_back(tmp);

			}
		}
	}
	return search_list;
}

int main()
{
	initgraph(800, 800);

	loadimage(&player, _T("./img/player.png"));
	loadimage(&wall, _T("./img/wall.png"));
	loadimage(&box, _T("./img/box.png"));
	loadimage(&des, _T("./img/des.png"));


	ifstream infile;
	infile.open("./map.txt");
	int map_height, map_width;
	infile>>map_height>> map_width;
	map_matrix=new int*[map_height];

	for (int i = 0; i < map_height; i++)
	{
		map_matrix[i] = new int[map_width];
	}

	for (int i = 0; i < map_height; i++)
	{
		for (int j = 0; j < map_width; j++) {
			char tmp_s;
			infile >> tmp_s;
			switch (tmp_s)
			{
			case '#':
				map_matrix[i][j] = '#';
				putimage(box_width * j, box_width * i, box_width, box_width, &wall, 0, 0);
				wall_pos.push_back(Pos(i, j));
				break;
			case 'X':
				map_matrix[i][j] = 'X';
				start_state.box.push_back(Pos(i,j));
				putimage(box_width * j, box_width * i, box_width, box_width, &box, 0, 0);
				break;
			case 'o':
				map_matrix[i][j] = 'o';			
				break;
			case 'd':
				map_matrix[i][j] = 'd';
				end_state.box.push_back(Pos(i, j));
				des_pos.push_back(Pos(i, j));
				putimage(box_width * j, box_width * i, box_width, box_width, &des, -16, -16);
				break;
			case 'p':
				map_matrix[i][j] = 'p';
				putimage(box_width*j, box_width * i, box_width, box_width, &player, -16, 0);
				start_state.player = Pos(i, j);
				break;
			case 'P':
				map_matrix[i][j] = 'P';
				putimage(box_width * j, box_width * i, box_width, box_width, &player, -16, 0);
				start_state.player = Pos(i, j);
				end_state.box.push_back(Pos(i, j));
				des_pos.push_back(Pos(i, j));

				break;
			case 'u':
				map_matrix[i][j] = 'u';
				putimage(box_width * j, box_width * i, box_width, box_width, &box, 0, 0);
				start_state.box.push_back(Pos(i, j));
				end_state.box.push_back(Pos(i, j));
				des_pos.push_back(Pos(i, j));
				break;
			default:
				break;
			}
		}
	}
	
	cout << "正在求解" << endl;
	
	wchar_t s[] = L"正在求解";
	outtextxy(200, 600, s);
	wchar_t s1[] = L"made by zy";
	outtextxy(700, 780, s1);

	//开始求解
	State state_p = start_state;

	priority_queue <State, vector<State>, greater<State>> unSearch;
	vector<State> searched;

	int ctn = 0;
	while (state_p.box != end_state.box)
	{

		//cout << "epoch:" << ctn << endl;
		//cout << state_p.score << endl;
		ctn++;

		vector<State> search_list = move(&state_p);
		searched.push_back(state_p);

		for (vector<State>::iterator iter = search_list.begin(); iter != search_list.end(); iter++)
		{
			if (find(searched.begin(), searched.end(), *iter) != searched.end())//已经搜索过了
			{
				continue;
			}
			else
			{
				// cout<<"add ";
				(*iter).from_state = searched.size() - 1;
				unSearch.push(*iter);
			}
		}

		cout << "\n";

		if (unSearch.empty())
		{
			cout << "not find" << endl;
			break;
		}
		else
		{
			state_p = unSearch.top();
			unSearch.pop();
		}
	}

	searched.push_back(state_p);
	vector<int> step_list_r;
	int index = searched.size() - 1;
	while (index != -1)
	{
		step_list_r.push_back(index);

		index = searched[index].from_state;

	}
	
	//开始展示结果
	ExMessage m;
	int index_i = step_list_r.size() - 1;

	while (true)
	{
		draw(searched, step_list_r[index_i]);

		// 获取一条鼠标或按键消息
		m = getmessage(EX_KEY);

		switch (m.message)
		{
		case WM_KEYDOWN:
			if (m.vkcode == VK_ESCAPE)
				return 0;	// 按 ESC 键退出程序
			else if(m.vkcode == VK_RIGHT)
			{
				int tmp = index_i - 1;
				if (tmp >=0)
				{
					index_i = tmp;
				}
			}
			else if (m.vkcode == VK_LEFT)
			{
				int tmp = index_i + 1;
				if (tmp< step_list_r.size())
				{
					index_i = tmp;
				}
			}
		}
	}



	//Sleep(1000);
	//cleardevice();
	
	_getch();				// 按任意键继续
	closegraph();			// 关闭绘图窗口
	return 0;
}
