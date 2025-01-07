  #include "MiniVim.h"

  MiniVim::MiniVim() {
      initscr();
      raw();
      keypad(stdscr, TRUE);
      noecho();
      curs_set(1);
      init_colors();
      load_file();
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
          init_pair(2, COLOR_BLACK, COLOR_WHITE);   // 反转色（黑色文字，白色背景）
          init_pair(3, COLOR_GREEN, COLOR_BLACK);   // 插入模式绿色
          init_pair(4, COLOR_BLUE, COLOR_BLACK);    // 命令模式蓝色
          init_pair(5, COLOR_RED, COLOR_BLACK);     // 错误信息红色
          init_pair(6, COLOR_YELLOW, COLOR_BLACK);  // 高亮当前行黄色
      }
  }

  void MiniVim::load_file() {
      std::ifstream file("../text.txt");
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
      move(y, x + get_line_number_width() + 1);
      while (1) {
          ch = getch();
          if (ch == 27) {
              return;
          }
          save_state_to_undo();  // 保存状态以便撤销

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
          save_file();
      } else if (command_str == "q") {
          endwin();
          exit(0);
      } else if (command_str == "wq") {
          save_file();
          endwin();
          exit(0);
      } else if (is_number(command_str)) {
          int line_number = std::stoi(command_str);
          if (line_number >= 1 && line_number <= text.size()) {
              y = line_number - 1;
              x = 0;
          } else {
              display_message("行号超出范围。");
          }
      } else {
          mvprintw(y, x, "未知命令: %s", command_str.c_str());
          refresh();
      }
  }

  void MiniVim::save_file() {
      std::ofstream file("../text.txt");
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
      if (x < 0) x = 0;
      if (y < 0) y = 0;
      if (y >= text.size()) y = text.size() - 1;
      if (x > text[y].length()) x = text[y].length();
      move(y, x + get_line_number_width() + 1);
      refresh();
  }

  void MiniVim::display_mode(bool show) {
      int max_y, max_x;
      getmaxyx(stdscr, max_y, max_x);
      move(max_y - 1, 0);
      clrtoeol();
      if (show) {
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
      for (int i = 0; i < text.size(); i++) {
          if (i == y) {
              attron(COLOR_PAIR(6));  // 高亮当前行黄色
          } else {
              attron(COLOR_PAIR(1));  // 默认颜色
          }

          mvprintw(i, 0, "%*d ", line_number_width, i + 1);
          mvprintw(i, line_number_width + 1, "%s", text[i].c_str());

          attroff(COLOR_PAIR(6));  // 取消高亮当前行
          attroff(COLOR_PAIR(1));  // 取消默认颜色
      }
  }

  void MiniVim::save_state_to_undo() {
      // 保存当前文本和光标位置到撤销栈
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
      move(max_y - 2, 0);
      clrtoeol();
      attron(COLOR_PAIR(5));  // 错误信息红色
      mvprintw(max_y - 2, 0, "%s", message.c_str());
      attroff(COLOR_PAIR(5));
      refresh();
      napms(1000);  // 显示1秒
      clear();
  }
  