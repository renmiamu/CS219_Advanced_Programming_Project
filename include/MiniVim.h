     #pragma once
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

         // Undo/Redo Stacks
         std::stack<std::pair<std::vector<std::string>, std::pair<int, int>>> undo_stack;  // 文本 + 光标
         std::stack<std::pair<std::vector<std::string>, std::pair<int, int>>> redo_stack;  // 文本 + 光标

         void init_colors();
         void load_file();
         void insert_mode();
         void command_mode();
         void process_command();
         void save_file();
         void move_cursor(int dx, int dy);
         void display_mode(bool show = true);
         int get_line_number_width();
         void display_text_with_line_numbers();
         void save_state_to_undo();
         void undo();
         void redo();
         bool is_number(const std::string &str);
         void display_message(const std::string &message);
        void delete_line();
     public:
         MiniVim();
         ~MiniVim();
         void run();
     };
     