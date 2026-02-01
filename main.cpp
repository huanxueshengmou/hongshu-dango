#include <chrono>
#include <iostream>
#include <thread>
#include <vector>
#include <windows.h>

#include "doge_frames.h"

using namespace std;

HANDLE consoleHandle;

/**
* 初始化控制台为动画播放环境
* 配置显示模式、光标及缓冲区防止交互阻塞
*/
void initializeConsole() {
    consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);

    system("color F0");
    system("Sweet Potato Dog");

    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(consoleHandle, &cursorInfo);
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(consoleHandle, &cursorInfo);

    DWORD consoleMode;
    GetConsoleMode(consoleHandle, &consoleMode);
    SetConsoleMode(consoleHandle, consoleMode & ~ENABLE_QUICK_EDIT_MODE);

    system("cls");
}

int main() {
    initializeConsole();

    if (doge_animation.empty()) {
        cerr << "错误: 动画帧数据为空。" << endl;
        return EXIT_FAILURE;
    }

    int currentFrame = 0;
    const int frameCount = doge_animation.size();
    const int animWidth = ANIME_WIDTH;
    const int animHeight = ANIME_HEIGHT;
    // 属性字节: 背景亮白(0xF), 前景黑(0x0)
 const WORD displayAttribute = 0xF0;
    vector<CHAR_INFO> displayBuffer;

    while (true) {
        // 获取动态窗口尺寸并匹配缓冲区大小，消除滚动条
 CONSOLE_SCREEN_BUFFER_INFO bufferInfo;
        GetConsoleScreenBufferInfo(consoleHandle, &bufferInfo);

        short windowWidth = bufferInfo.srWindow.Right - bufferInfo.srWindow.Left + 1;
        short windowHeight = bufferInfo.srWindow.Bottom - bufferInfo.srWindow.Top + 1;

        if (bufferInfo.dwSize.X != windowWidth || bufferInfo.dwSize.Y != windowHeight) {
            COORD newBufferSize = {windowWidth, windowHeight };
            SetConsoleScreenBufferSize(consoleHandle, newBufferSize);
        }

        // 重置缓冲区：用空格填满整个屏幕区域，应用统一颜色属性
 displayBuffer.assign(windowWidth * windowHeight, CHAR_INFO{' ', displayAttribute
        }
        );

        // 计算动画在窗口中的居中位置
 int renderWidth = animWidth * 2;
        int startCol = (windowWidth - renderWidth) / 2;
        int startRow = (windowHeight - animHeight) / 2;

        // 将当前帧的像素数据绘制到缓冲区
 const auto& frameData = doge_animation[currentFrame];
        for (int row = 0; row < animHeight; ++row) {
            int screenRow = startRow + row;
            if (screenRow < 0 || screenRow >= windowHeight || row >= frameData.size())
            continue;

            for (int col = 0; col < animWidth; ++col) {
                if (frameData[row][col] != 1) continue;

                int screenCol = startCol + (col * 2);
                if (screenCol < 0 || screenCol >= windowWidth - 1)
                continue;

                int bufferIndex = screenRow * windowWidth + screenCol;
                displayBuffer[bufferIndex].Char.AsciiChar = '#';
                displayBuffer[bufferIndex + 1].Char.AsciiChar = '#';
            }
        }

        // 单次原子写入整个缓冲区至控制台，实现双缓冲无闪烁输出
 COORD bufferDimensions = {windowWidth, windowHeight };
        COORD bufferStart = {0, 0 };
        SMALL_RECT writeRegion = {0, 0, static_cast<SHORT>(windowWidth - 1),
            static_cast<SHORT>(windowHeight - 1) };
        WriteConsoleOutputA(consoleHandle, displayBuffer.data(),
        bufferDimensions, bufferStart, &writeRegion);

        currentFrame = (currentFrame + 1) % frameCount;
        this_thread::sleep_for(chrono::milliseconds(60));
    }

    return EXIT_SUCCESS;
}
