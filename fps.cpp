#include <iostream>
#include <Windows.h>
#include <chrono>
#include <vector>
#include <algorithm>

using namespace std;

int nScreenWidth = 120;
int nScreenHeight = 40;

float fPlayerX = 8.0f;
float fPlayerY = 8.0f;
float fPlayerA = 0.0f; // angle

int nMapHeight = 16;
int nMapWidth = 16;

float fFOV = 3.1415926 / 4.0; // field of view 90° or pi/4
float fDepth = 16.0f;

int main() {
    wchar_t* screen = new wchar_t[nScreenWidth * nScreenHeight];

    HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
    SetConsoleActiveScreenBuffer(hConsole);
    DWORD dwBytesWritten = 0;

    wstring GameMap;
    GameMap += L"################";
    GameMap += L"#..............#";
    GameMap += L"#..............#";
    GameMap += L"#..........##..#";
    GameMap += L"#..........##..#";
    GameMap += L"#..........##..#";
    GameMap += L"#..............#";
    GameMap += L"#..............#";
    GameMap += L"#..............#";
    GameMap += L"#..............#";
    GameMap += L"#..............#";
    GameMap += L"#..###.........#";
    GameMap += L"#..............#";
    GameMap += L"#..............#";
    GameMap += L"#..............#";
    GameMap += L"################";

    auto tp1 = chrono::system_clock::now();
    auto tp2 = chrono::system_clock::now();

    // Game Loop
    while (1) {
        tp2 = chrono::system_clock::now();
        chrono::duration<float> elapsedTime = tp2 - tp1;
        tp1 = tp2;
        float fElapsedTime = elapsedTime.count();

        // Controls
        if (GetAsyncKeyState((unsigned short)'Q') & 0x8000) { // 左转
            fPlayerA -= (0.5f) * fElapsedTime;
        }
        if (GetAsyncKeyState((unsigned short)'E') & 0x8000) { // 右转
            fPlayerA += (0.5f) * fElapsedTime;
        }
        if (GetAsyncKeyState((unsigned short)'D') & 0x8000) { // 
            fPlayerX += cosf(fPlayerA) * 5.0f * fElapsedTime;
            fPlayerY += -sinf(fPlayerA) * 5.0f * fElapsedTime;
            if (GameMap[(int)fPlayerY * nMapWidth + (int)fPlayerX] == '#') {
                // 是否撞墙
                fPlayerX -= cosf(fPlayerA) * 5.0f * fElapsedTime;
                fPlayerY -= -sinf(fPlayerA) * 5.0f * fElapsedTime;
            }
        }
        if (GetAsyncKeyState((unsigned short)'A') & 0x8000) { // 左走
            fPlayerX -= cosf(fPlayerA) * 5.0f * fElapsedTime;
            fPlayerY -= -sinf(fPlayerA) * 5.0f * fElapsedTime;
            if (GameMap[(int)fPlayerY * nMapWidth + (int)fPlayerX] == '#') {
                // 是否撞墙
                fPlayerX += cosf(fPlayerA) * 5.0f * fElapsedTime;
                fPlayerY += -sinf(fPlayerA) * 5.0f * fElapsedTime;
            }
        }
        if (GetAsyncKeyState((unsigned short)'W') & 0x8000) { // 前进
            fPlayerX += sinf(fPlayerA) * 5.0f * fElapsedTime;
            fPlayerY += cosf(fPlayerA) * 5.0f * fElapsedTime;
            if (GameMap[(int)fPlayerY * nMapWidth + (int)fPlayerX] == '#') {
                // 是否撞墙
                fPlayerX -= sinf(fPlayerA) * 5.0f * fElapsedTime;
                fPlayerY -= cosf(fPlayerA) * 5.0f * fElapsedTime;
            }
        }
        if (GetAsyncKeyState((unsigned short)'S') & 0x8000) { // 后退
            fPlayerX -= sinf(fPlayerA) * 5.0f * fElapsedTime;
            fPlayerY -= cosf(fPlayerA) * 5.0f * fElapsedTime;
            if (GameMap[(int)fPlayerY * nMapWidth + (int)fPlayerX] == '#') {
                // 是否撞墙
                fPlayerX += sinf(fPlayerA) * 5.0f * fElapsedTime;
                fPlayerY += cosf(fPlayerA) * 5.0f * fElapsedTime;
            }
        }
        // 每轮一竖 输出画面 共120竖(nScreenWidth)
        for (int x = 0; x < nScreenWidth; x++) {
            float fRayAngle = (fPlayerA - fFOV / 2.0f) + ((float)x / (float)nScreenWidth) * fFOV;
            //
            float fDistanceToWall = 0;
            bool bHitWall = false;
            bool bBoundary = false;

            float fEyeX = sinf(fRayAngle); // fEyeX & fEyeY 即 player looking direction
            float fEyeY = cosf(fRayAngle); // 视野方向 得到单位向量
            while (!bHitWall && fDistanceToWall < fDepth) {
                fDistanceToWall += 0.1f;
                int nTestX = (int)(fPlayerX + fEyeX * fDistanceToWall);
                int nTestY = (int)(fPlayerY + fEyeY * fDistanceToWall);

                if (nTestX < 0 || nTestX >= nMapWidth || nTestY < 0 || nTestY >= nMapHeight) {
                    // test ray is out of bounds
                    bHitWall = true;
                    fDistanceToWall = fDepth; // set distance to maximum
                }
                else {
                    // within bounds
                    // check the cells individually on the map
                    if (GameMap[nTestY * nMapWidth + nTestX] == '#') {
                        bHitWall = true;

                        vector<pair<float, float>> p; // distance, dot product
                        for (int tx = 0; tx < 2; tx++) {
                            for (int ty = 0; ty < 2; ty++) {
                                float vy = (float)nTestY + ty - fPlayerY;
                                float vx = (float)nTestX + tx - fPlayerX;
                                float d = sqrt(vx * vx + vy * vy);
                                float dot = (fEyeX * vx / d) + (fEyeY * vy / d);
                                p.push_back(make_pair(d, dot));
                            }
                        }
                        // sort pairs
                        sort(p.begin(), p.end(),
                            [](const pair<float, float>& left, const pair<float, float>& right) {
                                return left.first < right.first;
                            }
                        );
                        float fBound = 0.05f;
                        if (acos(p.at(0).second) < fBound) bBoundary = true;
                        if (acos(p.at(1).second) < fBound) bBoundary = true;
                    }
                }
            }
            // Calculate distance to ceiling and floor
            int nCeiling = (float)(nScreenHeight / 2.0) - nScreenHeight / ((float)fDistanceToWall);
            int nFloor = nScreenHeight - nCeiling;

            // 墙 远近渲染
            short nShade = ' ';

            for (int y = 0; y < nScreenHeight; y++) {
                if (y < nCeiling) {
                    screen[y * nScreenWidth + x] = ' '; // draw sky
                }
                else if (y > nCeiling && y <= nFloor) {
                    if (fDistanceToWall <= fDepth / 4.0f) nShade = 0x2588; // very close
                    else if (fDistanceToWall <= fDepth / 3.0f) nShade = 0x2593;
                    else if (fDistanceToWall <= fDepth / 2.0f) nShade = 0x2592;
                    else if (fDistanceToWall <= fDepth) nShade = 0x2591;
                    else nShade = ' ';
                    if (bBoundary) nShade = ' ';
                    screen[y * nScreenWidth + x] = nShade; // draw wall
                }
                else {

                    float b = 1.0f - (((float)y - nScreenHeight / 2.0f) / ((float)nScreenHeight / 2.0f));
                    if (b < 0.25f)       nShade = '#';
                    else if (b < 0.5f)   nShade = 'x';
                    else if (b < 0.75f)  nShade = '-';
                    else if (b < 0.9f)   nShade = '.';
                    else                 nShade = ' ';

                    screen[y * nScreenWidth + x] = nShade; // draw floor
                }
            }
        }

        // 状态 & 小地图
        swprintf_s(screen, 40, L"X=%3.2f, Y=%3.2f, A=%3.2f FPS=%3.2f ", fPlayerX, fPlayerY, fPlayerA, 1.0f / fElapsedTime);
        for (int nx = 0; nx < nMapWidth; nx++) {
            for (int ny = 0; ny < nMapWidth; ny++) {
                screen[(ny+1)*nScreenWidth+nx] = GameMap[ny * nMapWidth + nx];
            }
        }
        screen[((int)fPlayerY + 1) * nScreenWidth + (int)fPlayerX] = 'P';

        screen[nScreenWidth * nScreenHeight - 1] = '\0';
        WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten);
    }

    return 2;
}