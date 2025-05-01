#include <iostream>
#include <Windows.h>
#include <chrono>
#include <vector>
#include <algorithm>
#include <utility>

using namespace std;

int screenWidth = 120;
int screenHeight = 37;

float playerX = 14.7f;
float playerY = 5.09f;
float playerA = 0.0f;

int mapWidth = 16;
int mapHeight = 16;

float FOV = 3.14159f / 4.0f;
float depth = 16.0f;
float speed = 5.0f;

int main() {
	wchar_t* screen = new wchar_t[screenWidth * screenHeight];
	HANDLE console = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(console);
	DWORD dwBytesWritten = 0;

	wstring map;
	map += L"################";
	map += L"#..............#";
	map += L"#.......########";
	map += L"#..............#";
	map += L"#......##......#";
	map += L"#......##......#";
	map += L"#..............#";
	map += L"###............#";
	map += L"##.............#";
	map += L"#......####..###";
	map += L"#......#.......#";
	map += L"#......#.......#";
	map += L"#..............#";
	map += L"#......#########";
	map += L"#..............#";
	map += L"################";

	auto tp1 = chrono::system_clock::now();
	auto tp2 = chrono::system_clock::now();

	while (1) {
		tp2 = chrono::system_clock::now();
		chrono::duration<float> elapsedTime = tp2 - tp1;
		tp1 = tp2;
		float fElapsedTime = elapsedTime.count();

		if (GetAsyncKeyState((unsigned short)'A') & 0x8000)
			playerA -= (speed * 0.75f) * fElapsedTime;

		if (GetAsyncKeyState((unsigned short)'D') & 0x8000)
			playerA += (speed * 0.75f) * fElapsedTime;

		if (GetAsyncKeyState((unsigned short)'W') & 0x8000) {
			playerX += sinf(playerA) * speed * fElapsedTime;
			playerY += cosf(playerA) * speed * fElapsedTime;

			if (map.c_str()[(int)playerX * mapWidth + (int)playerY] == '#') {
				playerX -= sinf(playerA) * speed * fElapsedTime;
				playerY -= cosf(playerA) * speed * fElapsedTime;
			}
		}

		if (GetAsyncKeyState((unsigned short)'S') & 0x8000) {
			playerX -= sinf(playerA) * speed * fElapsedTime;
			playerY -= cosf(playerA) * speed * fElapsedTime;

			if (map.c_str()[(int)playerX * mapWidth + (int)playerY] == '#') {
				playerX += sinf(playerA) * speed * fElapsedTime;
				playerY += cosf(playerA) * speed * fElapsedTime;
			}
		}

		for (int x = 0; x < screenWidth; x++) {
			float rayAngle = (playerA - FOV / 2.0f) + ((float)x / (float)screenWidth) * FOV;

			float stepSize = 0.1f;
			float distanceToWall = 0.0f;

			bool hitWall = false;
			bool boundary = false;

			float eyeX = sinf(rayAngle);
			float eyeY = cosf(rayAngle);

			while (!hitWall && distanceToWall < depth) {
				distanceToWall += stepSize;

				int testX = (int)(playerX + eyeX * distanceToWall);
				int testY = (int)(playerY + eyeY * distanceToWall);

				if (testX < 0 || testX >= mapWidth || testY < 0 || testY >= mapHeight) {
					hitWall = true;
					distanceToWall = depth;
				}
				else {
					if (map.c_str()[testX * mapWidth + testY] == '#') {
						hitWall = true;

						vector<pair<float, float>> p;

						for (int tx = 0; tx < 2; tx++)
							for (int ty = 0; ty < 2; ty++) {
								float vy = (float)testY + ty - playerY;
								float vx = (float)testX + tx - playerX;
								float d = sqrt(vx * vx + vy * vy);
								float dot = (eyeX * vx / d) + (eyeY * vy / d);
								p.push_back(make_pair(d, dot));
							}

							sort(p.begin(), p.end(), [](const pair<float, float> &left, const pair<float, float> &right) {return left.first < right.first; });

							float bound = 0.01;
							if (acos(p.at(0).second) < bound) boundary = true;
							if (acos(p.at(1).second) < bound) boundary = true;
					}
				}
			}

			int ceiling = (float)(screenHeight / 2.0) - screenHeight / ((float)distanceToWall);
			int floor = screenHeight - ceiling;

			short shade = ' ';

			if (distanceToWall <= depth / 4.0f)			shade = 0x2588;
			else if (distanceToWall < depth / 3.0f)		shade = 0x2593;
			else if (distanceToWall < depth / 2.0f)		shade = 0x2592;
			else if (distanceToWall < depth)			shade = 0x2591;
			else										shade = ' ';

			if (boundary)	shade = ' ';

			for (int y = 0; y < screenHeight; y++) {
				if (y <= ceiling)
					screen[y * screenWidth + x] = ' ';
				else if (y > ceiling && y <= floor)
					screen[y * screenWidth + x] = shade;
				else
				{
					float b = 1.0f - (((float)y - screenHeight / 2.0f) / ((float)screenHeight / 2.0f));
					if (b < 0.25)		shade = '#';
					else if (b < 0.5)	shade = 'x';
					else if (b < 0.75)	shade = '.';
					else if (b < 0.9)	shade = '-';
					else				shade = ' ';
					screen[y * screenWidth + x] = shade;
				}
			}
		}

		for (int nx = 0; nx < mapWidth; nx++)
			for (int ny = 0; ny < mapWidth; ny++) {
				screen[(ny + 1) * screenWidth + nx] = map[ny * mapWidth + nx];
			}

		screen[((int)playerX + 1) * screenWidth + (int)playerY] = 'P';

		screen[screenWidth * screenHeight - 1] = '\0';
		WriteConsoleOutputCharacter(console, screen, screenWidth * screenHeight, { 0,0 }, &dwBytesWritten);
	}

	return 0;
}
