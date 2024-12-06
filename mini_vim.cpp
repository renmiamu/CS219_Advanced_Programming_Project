#include <ncurses.h>
#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include <vector>

class MiniVim {
private:
    int x = 0;
    int y = 0;
    std::vector<std::string> text;
    std::string copied_line;
    std::string command_str;
    std::string mode = "--NORMAL--";

public:
    MiniVim() {
        initscr();
        raw();
        keypad(stdscr, TRUE);
        noecho();
        curs_set(1);
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
        }else if(is_number(command_str)){
            int line_number=std::stoi(command_str);
            if (line_number>=1&&line_number<=text.size()){
                y=line_number-1;
                x=0;
            }else{
                refresh();
            }
        }else {
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
            mvprintw(max_y - 1, 0, "%s", mode.c_str());
        }
        refresh();
    }


    //让行号对齐文本内容
    int get_line_number_width() {
        int num_lines = text.size();
        int width = 1;
        while (num_lines >= 10) {
            num_lines /= 10;
            width++;
        }
        return width;
    }

    //将行号与文本内容分开显示
    void display_text_with_line_numbers() {
        int line_number_width = get_line_number_width();
        for (int i = 0; i < text.size(); i++) {
            mvprintw(i, 0, "%*d ", line_number_width, i + 1);
            mvprintw(i, line_number_width + 1, "%s", text[i].c_str());
        }
    }

    //判断是否为数字
    bool is_number(const std::string &str) {
    if (str.empty()) return false;  // 空字符串不是数字

    for (char ch : str) {
        if (!std::isdigit(ch)) {
            return false;  // 如果有任何非数字字符，返回 false
        }
    }
    return true;
}
};

int main() {
    MiniVim editor;
    editor.run();
    return 0;
}
