#include <ncurses.h>
#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include <vector>

class MiniVim {
private:
    // 当前光标的位置
    int x = 0;
    int y = 0;

    // 文本内容
    std::vector<std::string> text;
    std::string copied_line;
    std::string command_str;  // Stores the command entered in command mode

public:
    // 构造函数
    MiniVim() {
        initscr();   // 开启
        raw();       // 直接传送给程序去处理而不产生终端信号
        keypad(stdscr, TRUE);  // 允许使用功能键
        noecho();    // 在进行控制操作时不显示输入的控制字符
        curs_set(1); // 设置光标可见

        // 加载文件内容
        load_file();
    }

    // 析构函数
    ~MiniVim() {
        endwin();  // 结束ncurses
    }

    // 启动编辑器
    void run() {
        int ch;
        while (true) {
            clear();
            // 显示当前文本内容
            for (int i = 0; i < text.size(); i++) {
                mvprintw(i, 0, "%s", text[i].c_str());
            }

            // 显示当前光标的位置
            move(y, x);
            refresh();

            ch = getch();  // 读取键盘输入

            int next_ch;
            switch (ch) {
                // 进入insert模式
                case 'i':
                    insert_mode();
                    break;
                // 进入命令模式
                case ':':
                    command_mode();
                    break;
                // 光标左移
                case 'h':
                    move_cursor(-1, 0);
                    break;
                // 光标下移
                case 'j':
                    move_cursor(0, 1);
                    break;
                // 光标上移
                case 'k':
                    move_cursor(0, -1);
                    break;
                // 光标右移
                case 'l':
                    move_cursor(1, 0);
                    break;
                case KEY_LEFT:
                    move_cursor(-1, 0);
                    break;
                case KEY_RIGHT:
                    move_cursor(1, 0);
                    break;
                case KEY_UP:
                    move_cursor(0, -1);
                    break;
                case KEY_DOWN:
                    move_cursor(0, 1);
                    break;
                // 跳转到行首
                case '0':
                    x = 0;
                    break;
                // 跳转到行尾
                case '$':
                    while (x < text[y].length()) {
                        x++;
                    }
                    break;
                // 跳转到第一行
                case 'g':
                    if (getch() == 'g') {
                        y = 0;
                        x = 0;
                    }
                    break;
                // 跳转到最后一行
                case 'G':
                    y = text.size() - 1;
                    x = 0;
                    break;
                // 删除当前行
                case 'd':
                    next_ch = getch();
                    if (next_ch == 'd') {
                        if (!text.empty() && y < text.size()) {
                            text.erase(text.begin() + y);
                            if (y == text.size() - 1) {
                                y == text.size() - 2;
                            }
                            x = 0;
                        }
                    } else {
                        ungetch(next_ch);  // 如果不是d，返回输入流
                    }
                    break;
                // 复制当前行
                case 'y':
                    next_ch = getch();
                    if (next_ch == 'y') {
                        if (!text.empty() && y < text.size()) {
                            copied_line = text[y];
                        }
                    } else {
                        ungetch(next_ch);  // 将其放回输入流
                    }
                    break;
                // 粘贴当前行
                case 'p':
                    if (!copied_line.empty()) {
                        text.insert(text.begin() + y + 1, copied_line);
                        y++;
                    }
                    break;
                default:
                    break;
            }
        }
    }

private:
    // 加载文件
    void load_file() {
        std::ifstream file("text.txt");
        if (file.is_open()) {
            std::string line;
            while (getline(file, line)) {
                text.push_back(line);
            }
            file.close();
        }
    }

    void insert_mode() {
        int ch;
        while (1) {
            // 获取按键输入
            ch = getch();

            // 如果按下的是ESC，则退回普通模式
            if (ch == 27) {
                return;
            }

            // 按下backspace
            if (ch == KEY_BACKSPACE || ch == 127 ) {
                if (x > 0) {
                    text[y].erase(x - 1, 1);
                    x--;
                } else if (y > 0) {
                    x = text[y - 1].length();
                    text[y - 1] += text[y];
                    text.erase(text.begin() + y);
                    y--;
                }
            }
            // 按下enter键
            else if (ch == '\n') {
                text.insert(text.begin() + y + 1, text[y].substr(x));
                text[y] = text[y].substr(0, x);
                y++;
                x = 0;
            } else {
                text[y].insert(x, 1, ch);
                x++;
            }

            // 实时更新文本内容
            clear();  // 每次清空屏幕
            for (int i = 0; i < text.size(); i++) {
                mvprintw(i, 0, "%s", text[i].c_str());
            }
            move(y, x);  // 更新光标位置
            refresh();
        }
    }

    void command_mode() {
        int max_y, max_x;
        getmaxyx(stdscr, max_y, max_x);  // 获取终端的大小
        int command_line = max_y - 1;  // 保留最后一行用于输入命令

        command_str = "";  // 清空之前的命令字符串
        move(command_line, 0);  // 移动到命令行输入的位置

        // 显示命令提示符
        mvprintw(command_line, 0, ":");
        refresh();

        while (true) {
            int ch = getch();  // 获取按键输入
            
            // 检查是否是 ESC 退出命令模式
            if (ch == 27) {  // ESC 键
                clear();
                return;  // 退出命令模式
            }

            // 处理退格键
            if (ch == KEY_BACKSPACE || ch == 127) {
                if (!command_str.empty()) {
                    command_str.pop_back();  // 移除命令字符串中的最后一个字符
                    move(command_line, 1 + command_str.length());  // 光标向后移动
                    delch();  // 删除字符
                }
            }
            // 处理字符输入（将字符追加到命令字符串中）
            else if (ch != '\n') {  // 忽略回车键
                command_str.push_back(ch);  // 将字符添加到命令中
                addch(ch);  // 在屏幕上显示字符
            }
            // 当按下回车键时，处理命令
            else if (ch == '\n') {
                noecho();  // 禁用回显
                process_command();
                clear();  // 清空屏幕
                return;  // 退出命令模式
            }

            refresh();  // 刷新屏幕以更新命令行
        }
    }


    // Process the entered command
    void process_command() {
        if (command_str == "w") {
            save_file();
        } else if (command_str == "q") {
            endwin();
            exit(0);
        } else if (command_str == "wq") {
            save_file();
            endwin();
            exit(0);
        } else {
            mvprintw(y, x, "Unknown command: %s", command_str.c_str());
            refresh();
        }
    }

    // 保存文件
    void save_file() {
        std::ofstream file("text.txt");
        if (file.is_open()) {
            for (const auto &line : text) {
                file << line << std::endl;  // 写入每一行
            }
            file.close();
        }
        refresh();
    }

    // 移动光标
    void move_cursor(int dx, int dy) {
        x += dx;
        y += dy;

        if (x < 0) {
            x = 0;
        }
        if (y < 0) {
            y = 0;
        }
        if (y >= text.size()) {
            y = text.size() - 1;
        }
        if (x > text[y].length()) {
            x = text[y].length();
        }

        move(y, x);
        refresh();
    }
};

// 程序入口
int main() {
    MiniVim editor;
    editor.run();
    return 0;
}
