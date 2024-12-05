#include <ncurses.h>
#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include <vector>


//当前光标的位置
int x=0;
int y=0;

//文本内容
std::vector<std::string> text;
std::string copied_line;

void insert_mode();
void command_mode();
void save_file();
void load_file();
void move_cursor(int dx, int dy);

int main(){
    initscr();   //开启
    raw();   //直接传送给程序去处理而不产生终端信号
    keypad(stdscr,TRUE);  //允许使用功能键
    noecho();   //在进行控制操作时不显示输入的控制字符
    curs_set(1);    //设置光标可见

    //加载文件内容
    load_file();

    int ch;
    bool delete_mode=false;
    while (1){
        clear();

        //显示当前文本内容
        for (int i = 0; i < text.size(); i++)
        {
            mvprintw(i,0,"%s",text[i].c_str());
        }
        
        //显示当前光标的内容
        move(y,x);
        refresh();

        ch=getch();  //读取键盘内容
        int next_ch;
        switch(ch){
            //进入insert模式
            case 'i':
                insert_mode();
                break;
            //进入命令模式
            case ':':
                command_mode();
                break;
            //光标左移
            case 'h':
                move_cursor(-1,0);
                break;
            //光标下移
            case 'j':
                move_cursor(0,1);
                break;
            //光标上移
            case 'k':
                move_cursor(0,-1);
                break;
            //光标右移
            case 'l':
                move_cursor(1,0);
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
            //跳转到行首
            case '0':
                x=0;
                break;
            //跳转到行尾
            case '$':
                while (x<text[y].length()){
                    x++;
                }
                break;
            //跳转到第一行
            case 'g':
                if (getch()=='g'){
                    y=0;
                    x=0;
                }
                break;
            //跳转到最后一行
            case 'G':
                y = text.size() - 1;
                x = 0;
                break;
            //删除当前行
            case 'd':
                next_ch = getch();
                if (next_ch=='d'){
                    if (!text.empty()&&y<text.size()){
                        text.erase(text.begin() + y);
                        if (y==text.size()-1){
                            y==text.size()-2;
                        }
                        x=0;
                    }
                }else{
                    ungetch(next_ch);    //若不是d，返回输入流
                }
                break;
            //复制当前行
            case 'y':
                next_ch = getch();  // 获取下一个按键
                if (next_ch == 'y') {  // 如果下一个按键是 'y'
                    // 复制当前行到 copied_line
                    if (!text.empty() && y < text.size()) {
                        copied_line = text[y];
                    }
                } else {
                    ungetch(next_ch);  // 将其放回输入流
                }
                break;
            //粘贴当前行
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
    endwin();
    return 0;
}


void load_file(){
    std::ifstream file("text.txt");
    if (file.is_open()){
        std::string line;
        while(getline(file,line)){
            text.push_back(line);
        }
        file.close();
    }
};

void insert_mode(){
    int ch;
    while (1){
        ch=getch();
        
        //如果按下的是ESC，则退回普通模式
        if (ch==27){
            return;
        }

        //按下backspace
        if (ch==KEY_BACKSPACE||ch==127||ch==8||ch=='^G'||ch=='\b'){
            if (x>0){
                text[y].erase(x-1,1);
                x--;
            }else if (y>0){
                x=text[y-1].length();
                text[y-1]+=text[y];
                text.erase(text.begin()+y);
                y--;
            }
        }
        //按下enter键
        else if (ch=='\n'){
            text.insert(text.begin()+y+1,text[y].substr(x));
            text[y]=text[y].substr(0,x);
            y++;
            x=0;
        }else{
            text[y].insert(x,1,ch);
            x++;
        }

        //实时更新文本内容
        clear();
        for (int i=0;i<text.size();i++){
            mvprintw(i,0,"%s",text[i].c_str());
        }
        move(y,x);
        refresh();
    }
}

void command_mode(){
    char command[100];
    echo();    //开启回显，使用户输入可见
    move(y,x);
    getstr(command);
    noecho();

    if (strcmp(command,":w")==0){
        save_file();
    }else if (strcmp(command,":q")==0){
        endwin();
        exit(0);
    }else if (strcmp(command,":wq")==0){
        save_file();
        endwin();
        exit(0);
    }
    refresh();
}

void save_file(){
    std::ofstream file("text.txt");
    if (file.is_open()){
        for (const auto &line : text) {
            file << line << std::endl;  // 写入每一行
        }
        file.close();
    }
    refresh();
}

void move_cursor(int dx, int dy) {
    x += dx;
    y += dy;

    if (x < 0){
        x = 0;
    }
    if (y < 0){
        y = 0;
    }
    if (y >= text.size()){
        y = text.size() - 1;
    } 
    if (x > text[y].length()){
        x = text[y].length();
    }

    move(y,x);
    refresh();
}