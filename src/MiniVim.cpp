  #include "MiniVim.h"
MiniVim::MiniVim(const std::string &filename) {
    initscr();
    raw();
    keypad(stdscr, TRUE);
    noecho();
    curs_set(1);
    init_colors();
    file_name = "../files/" + filename + ".txt";

    std::ifstream new_file(file_name);

    if (!new_file.is_open()) {
        std::ofstream create_file(file_name);
        if (!create_file.is_open()) {
            display_message("cannot create file: " + file_name);
            return;
        }
        create_file << "this is a new file.\n";
        create_file.close();
        display_message("file doesn't exist, new file created: " + file_name);
    } else {
        if (new_file.peek() == EOF) {
            new_file.close(); 

            std::ofstream out_file(file_name, std::ios::out);
            if (out_file.is_open()) {
                out_file << " ";
                out_file.close();
            }
        } else {
            display_message("entered " + file_name);
        }
    }

    new_file.close();

    load_file();

    view_start_x = 0;
    view_start_y = 0;
    screen_width = COLS - get_line_number_width() - 2;
    screen_height = LINES - 1;
}

  MiniVim::~MiniVim() {
      endwin();
  }

  void MiniVim::run() {
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
              case 'u':  // 撤销
                  undo();
                  break;
              case 18:  // 重做
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
                    delete_line();
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
                save_state_to_undo();
                text.insert(text.begin() + y + 1, copied_line);
                y++;
      }
                  break;
              default:
                  break;
          }
          // 如果光标超出内容显示区域的最后一行，滚动视图
            if (y >= view_start_y + screen_height - 1) {
                view_start_y++;
                y = view_start_y + screen_height - 2;
            }
            // 重新渲染屏幕
        clear();
        display_text_with_line_numbers();
        display_mode();
        move(y - view_start_y, x - view_start_x + get_line_number_width() + 1);
        refresh();
      }
  }
  
  void MiniVim::delete_line() {
    if (!text.empty() && y < text.size()) {
        if (get_line_number_width()-1 != text[y].length())
        {
            save_state_to_undo(); 
        }
        if (y == 0 && text.size() == 1) {
            text[y].erase(get_line_number_width() - 1, text[y].length());
        }
        else {
            text.erase(text.begin() + y);
                y--;
                if (y < 0) {
                    y = 0;
                }
            }
        x = 0;
    }
  }

void MiniVim::init_colors() {
    if (has_colors()) {
        start_color();
        init_pair(1, COLOR_WHITE, COLOR_BLACK);   // 默认颜色（白色文字，黑色背景）
        init_pair(2, COLOR_BLACK, COLOR_WHITE);   // 反转颜色（黑色文字，白色背景）
        init_pair(3, COLOR_GREEN, COLOR_BLACK);   // 插入模式绿色
        init_pair(4, COLOR_BLUE, COLOR_BLACK);    // 命令模式蓝色
        init_pair(5, COLOR_RED, COLOR_BLACK);     // 错误信息红色
        init_pair(6, COLOR_YELLOW, COLOR_BLACK);  // 高亮当前行黄色字体，黑色背景
        init_pair(7, COLOR_BLACK, COLOR_YELLOW);  // 高亮当前行黑色字体，黄色背景（白色模式下）
        init_pair(8, COLOR_BLUE, COLOR_WHITE);    // 模式行深蓝色字体，白色背景（白色模式下）
        init_pair(9, COLOR_RED, COLOR_WHITE);
    }
}

  void MiniVim::load_file() {
      std::ifstream file(file_name);
      if (file.is_open()) {
          std::string line;
          while (getline(file, line)) {
              text.push_back(line);
          }
          file.close();
      }
  }

void MiniVim::insert_mode() {
    int ch;
    move(y - view_start_y, x - view_start_x + get_line_number_width() + 1);

    while (true) {
        ch = getch();
        if (ch == 27) {  // ESC 键退出插入模式
            if (y >= view_start_y + screen_height - 1) {
                view_start_y++;
                y = view_start_y + screen_height - 2;
            }
            return;
        }

        save_state_to_undo();

        if (ch == KEY_BACKSPACE || ch == 127) { // 处理退格键
            if (x > 0) {
                text[y].erase(x - 1, 1);
                x--;
            } else if (y > 0) {
                x = text[y - 1].length();
                text[y - 1] += text[y];
                text.erase(text.begin() + y);
                y--;
            }
        } else if (ch == '\n') { // 按回车键
            text.insert(text.begin() + y + 1, text[y].substr(x));
            text[y] = text[y].substr(0, x);
            y++;
            x = 0;

            // 如果光标超出内容显示区域的最后一行，滚动视图
            if (y >= view_start_y + screen_height - 1) {
                view_start_y++;
                y = view_start_y + screen_height - 2;
            }
        } else if (ch == KEY_LEFT) {
            move_cursor(-1, 0);
        } else if (ch == KEY_RIGHT) {
            move_cursor(1, 0);
        } else if (ch == KEY_UP) {
            move_cursor(0, -1);
        } else if (ch == KEY_DOWN) {
            move_cursor(0, 1);
        } else { // 插入普通字符
            text[y].insert(x, 1, ch);
            x++;

            // 水平滚动逻辑
            if (x >= view_start_x + screen_width - 1) {
                view_start_x++;
            }
        }

        // 重新渲染屏幕
        clear();
        display_text_with_line_numbers();
        display_mode();
        move(y - view_start_y, x - view_start_x + get_line_number_width() + 1);
        refresh();
    }
}

  void MiniVim::command_mode() {
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

  void MiniVim::process_command() {
    if (command_str == "w") {
        save_file(file_name);
    } else if (command_str == "q") {
        endwin();
        exit(0);
    } else if (command_str == "wq") {
        save_file(file_name);
        endwin();
        exit(0);
    } else if (command_str == "background") {
        toggle_background();
    } else if (command_str.rfind("s/", 0) == 0) {
        handle_find_and_replace();
    }else if (is_number(command_str)) {
    int line_number = std::stoi(command_str);
    if (line_number >= 1 && line_number <= text.size()) {
        y = line_number - 1;
        x = 0;

            if (y >= view_start_y + screen_height - 1) {
                view_start_y++;
                y = view_start_y + screen_height - 2;
            }
        clear();
        display_text_with_line_numbers();
        display_mode();
        refresh();
    } else {
        display_message("line number exceeded.");
    }
} else if (command_str.rfind("cd/", 0) == 0) {
        std::string filename = "../files/" + command_str.substr(3) + ".txt";
        file_name = filename;
        change_file(filename);
    } else {
        display_message("unknown command: " + command_str);
    }
}

void MiniVim::handle_find_and_replace() {
    size_t first_slash = command_str.find('/', 2);
    size_t second_slash = command_str.find('/', first_slash + 1);
    size_t third_slash = command_str.find('/', second_slash + 1);

    if (first_slash == std::string::npos || second_slash == std::string::npos) {
        display_message("command format invalid, please use: s/old/new/g");
        return;
    }

    std::string old_str = command_str.substr(2, first_slash - 2); 
    std::string new_str = command_str.substr(first_slash + 1, second_slash - first_slash - 1); 

    bool global_replace = (third_slash != std::string::npos && command_str.substr(third_slash) == "/g");

    if (old_str.empty()) {
        display_message("old string cannot be empty");
        return;
    }

    int replace_count = 0;
    for (std::string &line : text) {
        size_t pos = 0;
        while ((pos = line.find(old_str, pos)) != std::string::npos) {
            line.replace(pos, old_str.length(), new_str);
            replace_count++;
            if (!global_replace) break;
            pos += new_str.length();
        }
    }

    if (replace_count > 0) {
        display_message("successfully replaced, total replaced in " + std::to_string(replace_count) + " places.");
    } else {
        display_message("cannot find old string.");
    }
}

  void MiniVim::save_file(const std::string &filename) {
      std::ofstream file(filename);
      if (file.is_open()) {
          for (const auto &line : text) {
              file << line << std::endl;
          }
          file.close();
      }
  }

void MiniVim::move_cursor(int dx, int dy) {
    x += dx;
    y += dy;

    // 限制光标在文件的有效范围内
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    if (y >= text.size()) y = text.size() - 1; // 光标不能超过文本的最后一行
    if (x > text[y].length()) x = text[y].length(); // 光标不能超过当前行的长度

    // **垂直滚动：调整 view_start_y**
    if (y < view_start_y) {
        view_start_y = y; // 光标在屏幕上方，向上滚动
    } else if (y >= view_start_y + screen_height - 1) {
        view_start_y = y - (screen_height - 2); // 光标在屏幕下方，向下滚动
    }

    // **水平滚动：调整 view_start_x**
    if (x < view_start_x) {
        view_start_x = x; // 光标在屏幕左侧，向左滚动
    } else if (x >= view_start_x + screen_width - 1) {
        view_start_x = x - (screen_width - 1); // 光标在屏幕右侧，向右滚动
    }

    // 更新光标在屏幕上的实际位置
    move(y - view_start_y, x - view_start_x + get_line_number_width() + 1);
    refresh();
}

  void MiniVim::display_mode(bool show) {
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);
    move(max_y - 1, 0); // 移动到模式行位置
    clrtoeol();         // 清空当前行

    if (show) {
        if (is_default_background) {
            // 黑色背景模式
            attron(COLOR_PAIR(3)); // 绿色字体，黑色背景
        } else {
            // 白色背景模式
            attron(COLOR_PAIR(8)); // 深蓝色字体，白色背景
        }

        mvprintw(max_y - 1, 0, "%s", mode.c_str()); // 打印模式字符串

        if (is_default_background) {
            attroff(COLOR_PAIR(3));
        } else {
            attroff(COLOR_PAIR(8));
        }
    }

    refresh();
}

  int MiniVim::get_line_number_width() {
      int num_lines = text.size();
      int width = 1;
      while (num_lines >= 10) {
          num_lines /= 10;
          width++;
      }
      return width;
  }

void MiniVim::display_text_with_line_numbers() {
    int line_number_width = get_line_number_width();

    for (int i = 0; i < screen_height - 1; ++i) { // 遍历内容显示区域
        int text_line = i + view_start_y; // 当前显示的文本行号
        if (text_line >= text.size()) break; // 超出文本内容时停止渲染

        // 获取当前行内容，从 view_start_x 开始截取屏幕宽度
        std::string line = text[text_line];
        if (view_start_x < line.length()) {
            line = line.substr(view_start_x, screen_width); // 截取可见范围内的内容
        } else {
            line = ""; // 如果起始列超出行长度，显示空内容
        }

        // 动态选择颜色对
        if (text_line == y) { // 如果是当前光标所在的行
            if (is_default_background) {
                attron(COLOR_PAIR(6)); // 黑色背景：黄色字体，高亮
            } else {
                attron(COLOR_PAIR(7)); // 白色背景：黑色字体，黄色背景，高亮
            }
            mvprintw(i, 0, "%*d %s", line_number_width, text_line + 1, line.c_str());
            if (is_default_background) {
                attroff(COLOR_PAIR(6));
            } else {
                attroff(COLOR_PAIR(7));
            }
        } else { // 非高亮行
            if (is_default_background) {
                attron(COLOR_PAIR(1)); // 默认背景
            } else {
                attron(COLOR_PAIR(2)); // 反转背景
            }
            mvprintw(i, 0, "%*d %s", line_number_width, text_line + 1, line.c_str());
            if (is_default_background) {
                attroff(COLOR_PAIR(1));
            } else {
                attroff(COLOR_PAIR(2));
            }
        }
    }

    // 显示模式行
    display_mode();
}

  void MiniVim::save_state_to_undo() {
      //保存当前文本和光标位置到撤销栈
      undo_stack.push({text, {x, y}});
      // 清空重做栈
      while (!redo_stack.empty()) {
          redo_stack.pop();
      }
  }

  void MiniVim::undo() {
      if (!undo_stack.empty()) {
          // 保存当前状态到重做栈
          redo_stack.push({text, {x, y}});
          // 恢复撤销栈状态
          text = undo_stack.top().first;
          x = undo_stack.top().second.first;
          y = undo_stack.top().second.second;
          undo_stack.pop();
          refresh();
      }
  }

  void MiniVim::redo() {
      if (!redo_stack.empty()) {
          // 保存当前状态到撤销栈
          undo_stack.push({text, {x, y}});
          // 恢复重做栈状态
          text = redo_stack.top().first;
          x = redo_stack.top().second.first;
          y = redo_stack.top().second.second;
          redo_stack.pop();
          refresh();
      }
  }

  bool MiniVim::is_number(const std::string &str) {
      if (str.empty()) return false;  // 空字符串不是数字

      for (char ch : str) {
          if (!std::isdigit(ch)) {
              return false;  // 如果有任何非数字字符，返回 false
          }
      }
      return true;
  }

  void MiniVim::display_message(const std::string &message) {
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);
    move(max_y - 2, 0); // 移动到消息行位置
    clrtoeol();         // 清空当前行

    if (is_default_background) {
        attron(COLOR_PAIR(5));
    } else {
        attron(COLOR_PAIR(9));
    }

    mvprintw(max_y - 2, 0, "%s", message.c_str());

    if (is_default_background) {
        attroff(COLOR_PAIR(5));
    } else {
        attroff(COLOR_PAIR(9));
    }

    refresh();
    napms(1000);
    clear();
}

  void MiniVim::toggle_background() {
    is_default_background = !is_default_background; // 切换背景状态

    // 设置全局背景颜色
    if (is_default_background) {
        bkgd(COLOR_PAIR(1)); // 默认背景：黑色背景，白色文字
        display_message("BLACK");
    } else {
        bkgd(COLOR_PAIR(2)); // 白色背景，黑色文字
        display_message("WHITE");
    }

    // 清屏并重新渲染所有内容
    clear();
    display_text_with_line_numbers();
    display_mode();
    refresh();
}

void MiniVim::change_file(const std::string &filename) {
    std::ifstream new_file(filename);

    if (!new_file.is_open()) {
        std::ofstream create_file(filename);
        if (!create_file.is_open()) {
            display_message("cannot create file: " + filename);
            return;
        }
        create_file.close();
        display_message("file doesn't exists, new file created: " + filename);
    } else {
        new_file.close();
    }

    text.clear();

    std::ifstream open_file(filename);
    std::string line;
    while (getline(open_file, line)) {
        text.push_back(line);
    }
    open_file.close();

    if (text.empty()) {
        text.push_back(filename);
    }

    x = 0;
    y = 0;
    view_start_x = 0;
    view_start_y = 0;

    clear();
    display_text_with_line_numbers();
    display_mode();
    refresh();

    display_message("enter file: " + filename);
}
