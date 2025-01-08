#pragma once

#include <ncurses.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stack>

class MiniVim {
private:
    // 光标位置
    int x = 0; // 光标的列位置（相对于整个文件）
    int y = 0; // 光标的行位置（相对于整个文件）

    // 滚动视图的起始位置
    int view_start_x = 0; // 当前屏幕的起始列（用于横向滚动）
    int view_start_y = 0; // 当前屏幕的起始行（用于纵向滚动）

    // 屏幕尺寸
    int screen_width = 0;  // 屏幕显示区域的宽度（行内容可见的字符数）
    int screen_height = 0; // 屏幕显示区域的高度（可显示的行数，不包括模式行）

    // 文本数据
    std::vector<std::string> text; // 存储整个文件的内容
    std::string copied_line;       // 用于存储复制的行内容
    std::string command_str;       // 命令模式的输入字符串

    // 当前模式
    std::string mode = "--NORMAL--"; // 当前编辑器的模式

    // 撤销与重做栈
    std::stack<std::pair<std::vector<std::string>, std::pair<int, int>>> undo_stack; // 保存文本和光标位置的撤销栈
    std::stack<std::pair<std::vector<std::string>, std::pair<int, int>>> redo_stack; // 保存文本和光标位置的重做栈
    bool is_default_background = true;

    // 私有方法
    void init_colors();                         // 初始化颜色
    void load_file();                           // 加载文件内容
    void insert_mode();                         // 插入模式逻辑
    void command_mode();                        // 命令模式逻辑
    void process_command();                     // 处理命令输入
    void save_file();                           // 保存文件
    void move_cursor(int dx, int dy);           // 移动光标
    void display_mode(bool show = true);        // 显示模式行
    int get_line_number_width();                // 计算行号宽度
    void display_text_with_line_numbers();      // 显示文本和行号
    void save_state_to_undo();                  // 保存当前状态到撤销栈
    void undo();                                // 执行撤销
    void redo();                                // 执行重做
    bool is_number(const std::string& str);     // 判断字符串是否是数字
    void display_message(const std::string& message); // 显示状态或错误信息
    void delete_line();                         // 删除当前行
    void handle_find_and_replace();
    void toggle_background();
    
public:
    MiniVim();  // 构造函数
    ~MiniVim(); // 析构函数

    void run(); // 编辑器主程序入口
};
