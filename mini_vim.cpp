#include <ncurses.h>
#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include <vector>
#include <stack>

class MiniVim {
private:
    int x = 0;
    int y = 0;
    std::vector<std::string> text;
    std::string copied_line;
    std::string command_str;
    std::string mode = "--NORMAL--";

    bool no_colors = false;  // 新增变量，用来跟踪是否处于无颜色模式

    // Undo/Redo Stacks
    std::stack<std::pair<std::vector<std::string>, std::pair<int, int>>> undo_stack;  // Text + Cursor
    std::stack<std::pair<std::vector<std::string>, std::pair<int, int>>> redo_stack;  // Text + Cursor

public:
    MiniVim() {
        initscr();
        raw();
        keypad(stdscr, TRUE);
        noecho();
        curs_set(1);
        init_colors();
        load_file();
    }

    ~MiniVim() {
        endwin();
    }

    void run() {
        int ch;
        while (true) {
            clear();
            display_text_with_line_numbers();
            display_mode();
            move(y, x + get_line_number_width() + 1);
            refresh();

            ch = getch();
            int next_ch;
            switch (ch) {
                case 'i':
                    mode = "--INSERT--";
                    display_mode();
                    insert_mode();
                    mode = "--NORMAL--";
                    break;
                case ':':
                    display_mode(false);
                    command_mode();
                    break;
                case 'h':
                    move_cursor(-1, 0);
                    break;
                case 'j':
                    move_cursor(0, 1);
                    break;
                case 'k':
                    move_cursor(0, -1);
                    break;
                case 'l':
                    move_cursor(1, 0);
                    break;
                case 'u':  // Undo
                    undo();
                    break;
                case 'r':  // Redo
                    redo();
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
                case '0':
                    x = 0;
                    break;
                case '$':
                    x = text[y].length();
                    break;
                case 'g':
                    if (getch() == 'g') {
                        y = 0;
                        x = 0;
                    }
                    break;
                case 'G':
                    y = text.size() - 1;
                    x = 0;
                    break;
                case 'd':
                    next_ch = getch();
                    if (next_ch == 'd') {
                        if (!text.empty() && y < text.size()) {
                            text.erase(text.begin() + y);
                            x = 0;
                        }
                    } else {
                        ungetch(next_ch);
                    }
                    break;
                case 'y':
                    next_ch = getch();
                    if (next_ch == 'y') {
                        if (!text.empty() && y < text.size()) {
                            copied_line = text[y];
                        }
                    } else {
                        ungetch(next_ch);
                    }
                    break;
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
    void init_colors() {
        if (has_colors()) {
            start_color();
            init_pair(1, COLOR_WHITE, COLOR_BLACK);   // 默认颜色（白色文字，黑色背景）
            init_pair(2, COLOR_BLACK, COLOR_WHITE);   // 反转色（黑色文字，白色背景）
            init_pair(3, COLOR_GREEN, COLOR_BLACK);   // 插入模式绿色
            init_pair(4, COLOR_BLUE, COLOR_BLACK);    // 命令模式蓝色
            init_pair(5, COLOR_RED, COLOR_BLACK);     // 错误信息红色
            init_pair(6, COLOR_YELLOW, COLOR_BLACK);  // 高亮当前行黄色
        }
    }

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
        move(y, x + get_line_number_width() + 1);
        while (1) {
            ch = getch();
            if (ch == 27) {
                return;
            }
            save_state_to_undo();  // Save state before editing

            if (ch == KEY_BACKSPACE || ch == 127) {
                if (x > 0) {
                    text[y].erase(x - 1, 1);
                    x--;
                } else if (y > 0) {
                    x = text[y - 1].length();
                    text[y - 1] += text[y];
                    text.erase(text.begin() + y);
                    y--;
                }
            } else if (ch == '\n') {
                text.insert(text.begin() + y + 1, text[y].substr(x));
                text[y] = text[y].substr(0, x);
                y++;
                x = 0;
            } else if (ch == KEY_LEFT) {
                move_cursor(-1, 0);
            } else if (ch == KEY_RIGHT) {
                move_cursor(1, 0);
            } else if (ch == KEY_UP) {
                move_cursor(0, -1);
            } else if (ch == KEY_DOWN) {
                move_cursor(0, 1);
            } else {
                text[y].insert(x, 1, ch);
                x++;
            }
            clear();
            display_text_with_line_numbers();
            display_mode();
            move(y, x + get_line_number_width() + 1);
            refresh();
        }
    }

    void command_mode() {
        int max_y, max_x;
        getmaxyx(stdscr, max_y, max_x);
        int command_line = max_y - 1;
        command_str = "";
        move(command_line, 0);
        mvprintw(command_line, 0, ":");
        refresh();

        while (true) {
            int ch = getch();
            if (ch == 27) {
                clear();
                return;
            }
            if (ch == KEY_BACKSPACE || ch == 127) {
                if (!command_str.empty()) {
                    command_str.pop_back();
                    move(command_line, 1 + command_str.length());
                    delch();
                }
            } else if (ch != '\n') {
                command_str.push_back(ch);
                addch(ch);
            } else if (ch == '\n') {
                noecho();
                process_command();
                clear();
                return;
            }
            refresh();
        }
    }

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
        } else if (command_str == "bg") {  // 切换颜色模式
            no_colors = !no_colors;  // 切换颜色模式
            display_mode();
        } else if (is_number(command_str)) {
            int line_number = std::stoi(command_str);
            if (line_number >= 1 && line_number <= text.size()) {
                y = line_number - 1;
                x = 0;
            } else {
                display_message("Line number out of range.");
            }
        } else {
            mvprintw(y, x, "Unknown command: %s", command_str.c_str());
            refresh();
        }
    }

    void save_file() {
        std::ofstream file("text.txt");
        if (file.is_open()) {
            for (const auto &line : text) {
                file << line << std::endl;
            }
            file.close();
        }
    }

    void move_cursor(int dx, int dy) {
        x += dx;
        y += dy;
        if (x < 0) x = 0;
        if (y < 0) y = 0;
        if (y >= text.size()) y = text.size() - 1;
        if (x > text[y].length()) x = text[y].length();
        move(y, x + get_line_number_width() + 1);
        refresh();
    }

    void display_mode(bool show = true) {
        int max_y, max_x;
        getmaxyx(stdscr, max_y, max_x);
        move(max_y - 1, 0);
        clrtoeol();
        if (show) {
            if (!no_colors) {
                // 使用颜色模式
                if (mode == "--INSERT--") {
                    attron(COLOR_PAIR(3));  // 插入模式绿色
                } else if (mode == "--NORMAL--") {
                    attron(COLOR_PAIR(4));  // 正常模式蓝色
                } else {
                    attron(COLOR_PAIR(2));  // 命令模式反转色
                }
                mvprintw(max_y - 1, 0, "%s", mode.c_str());
                attroff(COLOR_PAIR(3));
                attroff(COLOR_PAIR(4));
                attroff(COLOR_PAIR(2));
            } else {
                // 不使用颜色模式，直接显示文本
                mvprintw(max_y - 1, 0, "%s", mode.c_str());
            }
        }
        refresh();
    }

    void display_text_with_line_numbers() {
        int line_num_width = get_line_number_width();
        for (int i = 0; i < text.size(); ++i) {
            if (i == y) {
                attron(COLOR_PAIR(6));  // 高亮当前行
            }
            mvprintw(i, 0, "%*d ", line_num_width, i + 1);  // 打印行号
            mvprintw(i, line_num_width + 1, "%s", text[i].c_str());
            attroff(COLOR_PAIR(6));  // 取消高亮
        }
    }

    int get_line_number_width() {
        return std::to_string(text.size()).length() + 2;
    }

    bool is_number(const std::string &str) {
        for (char c : str) {
            if (!isdigit(c)) return false;
        }
        return true;
    }

    void display_message(const std::string &message) {
        int max_y, max_x;
        getmaxyx(stdscr, max_y, max_x);
        mvprintw(max_y - 2, 0, message.c_str());
        refresh();
        napms(2000);
    }

    void save_state_to_undo() {
        undo_stack.push({text, {x, y}});
        while (!redo_stack.empty()) {
            redo_stack.pop();  // 清空 redo 堆栈
        }
    }

    void undo() {
        if (!undo_stack.empty()) {
            auto last_state = undo_stack.top();
            undo_stack.pop();
            text = last_state.first;
            x = last_state.second.first;
            y = last_state.second.second;
            redo_stack.push(last_state);  // 保存到 redo 堆栈
        }
    }

    void redo() {
        if (!redo_stack.empty()) {
            auto last_state = redo_stack.top();
            redo_stack.pop();
            text = last_state.first;
            x = last_state.second.first;
            y = last_state.second.second;
            undo_stack.push(last_state);  // 保存到 undo 堆栈
        }
    }
};

int main() {
    MiniVim vim;
    vim.run();
    return 0;
}
