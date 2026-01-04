#define NOMINMAX  // 禁用 min/max 宏
#include "..\head\SocialNetwork.h"
#include <iostream>
#include <limits>
#include <cstdlib>
#include <windows.h>
#include <locale>
using namespace std;

// ASCII艺术标题
void showASCIITitle() {
    cout << "\n";
    cout << "  _______  _______  _______  _______  ___   _______  _______ " << endl;
    cout << " (  ____ \\(  ___  )(  ____ )(  ___  )/ __) (  ___  )(  ____ )" << endl;
    cout << " | (    \\/| (   ) || (    )|| (   ) |\\__ \\ | (   ) || (    )|" << endl;
    cout << " | (_____ | (___) || (____)|| |   | |(__) )| |   | || (____)|" << endl;
    cout << " (_____  )|  ___  ||     __)| |   | |   | || |   | ||     __)" << endl;
    cout << "       ) || (   ) || (\\ (   | |   | |   | || |   | || (\\ (   " << endl;
    cout << " /\\____) || )   ( || ) \\ \\__| (___) |   | || (___) || ) \\ \\__" << endl;
    cout << " \\_______)|/     \\||/   \\__/(_______)   )_)(_______)|/   \\__/" << endl;
    cout << "\n";
    cout << "================================================================================" << endl;
    cout << "                   简易社交关系网系统 v1.0.15" << endl;
    cout << "================================================================================" << endl;
    cout << " 功能: 联系人管理 | 关系分析 | 社交网络可视化 | 数据持久化" << endl;
    cout << "================================================================================" << endl;
}

// ASCII菜单系统
void showASCIIMenu() {
    system("cls");

    string line(80, '=');
    string thinLine(80, '-');
    string dashLine(80, '.');

    cout << line << endl;
    cout << "                              主菜单                              " << endl;
    cout << thinLine << endl;

    cout << "  [1]  添加联系人           [2]  删除联系人" << endl;
    cout << "  [3]  添加关系(亲密度)     [4]  删除关系" << endl;
    cout << "  [5]  显示所有联系人关系   [6]  按亲密程度排序好友" << endl;
    cout << "  [7]  查询两人亲密度       [8]  显示社交大牛(Top10)" << endl;
    cout << "  [9]  保存到文件          [10]  从文件加载" << endl;
    cout << "  [11] 导出HTML可视化       [0]  退出系统" << endl;

    cout << thinLine << endl;
    cout << "  提示: 输入数字选择对应功能，按Enter键确认" << endl;
    cout << dashLine << endl;
    cout << "  请选择操作 (0-11): ";
}

// 显示操作标题
void showOperationTitle(const string& operation) {
    system("cls");
    string line(80, '=');
    cout << line << endl;
    cout << "                       " << operation << "                      " << endl;
    cout << line << endl;
}

// 显示成功消息
void showSuccessMsg(const string& msg) {
    cout << "\n  [+] " << msg << endl;
}

// 显示错误消息
void showErrorMsg(const string& msg) {
    cout << "\n  [!] " << msg << endl;
}

// 显示信息消息
void showInfoMsg(const string& msg) {
    cout << "\n  [i] " << msg << endl;
}

// 等待用户按键
void waitForUser() {
    cout << "\n 按Enter键继续...";
    //cin.ignore();
    cin.get();
}

void testEncoding() {
    std::string test = "社交网络状态";
    std::cout << "测试字符串: " << test << std::endl;
    std::cout << "字符串长度: " << size_t(test.length()) << std::endl;
    std::cout << "第一个字符: 0x" << std::hex << (int)(unsigned char)test[0] << std::endl;
}

int main() {
    // Windows下设置控制台编码
#ifdef _WIN32
    // 使用系统默认编码（中文Windows通常是GBK/CP936）
    // 而不是UTF-8
    //system("chcp 65001 > nul");
    SetConsoleOutputCP(CP_ACP);  // 使用ANSI代码页
    SetConsoleCP(CP_ACP);        // 使用ANSI代码页
#endif


    SocialNetwork network;
    int choice;
    string name1, name2, filename;
    int weight;
    bool ascending;

    // 显示标题
    showASCIITitle();
    cout << "\n 按Enter键开始使用系统...";
    cin.get();

    while (true) {
        showASCIIMenu();

        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            showErrorMsg("请输入有效的数字！");
            waitForUser();
            continue;
        }

        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        switch (choice) {
        case 1:
            showOperationTitle("添加联系人");
           
            network.addPersons();
            waitForUser();
            break;

        case 2:
            showOperationTitle("删除联系人");
            cout << "\n  请输入要删除的联系人姓名: ";
            getline(cin, name1);
            network.deletePerson(name1);
            waitForUser();
            break;

        case 3:
            showOperationTitle("添加关系");
            cout << "\n  请输入第一个人姓名: ";
            getline(cin, name1);
            cout << "  请输入第二个人姓名: ";
            getline(cin, name2);
            cout << "  请输入亲密度 (1-100): ";
            cin >> weight;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            network.addEdge(name1, name2, weight);
            waitForUser();
            break;

        case 4:
            showOperationTitle("删除关系");
            cout << "\n  请输入第一个人姓名: ";
            getline(cin, name1);
            cout << "  请输入第二个人姓名: ";
            getline(cin, name2);
            network.deleteEdge(name1, name2);
            waitForUser();
            break;

        case 5:
            showOperationTitle("所有联系人关系");
            network.displayAll();
            waitForUser();
            break;

        case 6:
            showOperationTitle("按亲密程度排序好友");
            cout << "\n  请输入要排序的联系人姓名: ";
            getline(cin, name1);
            cout << "  排序顺序 (0=降序, 1=升序): ";
            cin >> ascending;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            network.sortFriends(name1, ascending);
            waitForUser();
            break;

        case 7:
            showOperationTitle("查询两人亲密度");
            cout << "\n  请输入第一个人姓名: ";
            getline(cin, name1);
            cout << "  请输入第二个人姓名: ";
            getline(cin, name2);
            {
                int closeness = network.getBottleneckPath(name1, name2);
                if (closeness == -1) {
                    showErrorMsg(name1 + " 和 " + name2 + " 之间没有路径可达");
                }
                else if (closeness == 0) {
                    showInfoMsg(name1 + " 和 " + name2 + " 是同一人");
                }
                else {
                    showSuccessMsg(name1 + " 和 " + name2 + " 的亲密度为: " + to_string(closeness));
                }
            }
            waitForUser();
            break;

        case 8:
            showOperationTitle("社交大牛排行榜");
            network.displayTop10();
            waitForUser();
            break;

        case 9:
            showOperationTitle("保存到文件");
            cout << "\n  请输入保存文件名 (例如: network.json): ";
            getline(cin, filename);
            network.saveToFile(filename);
            waitForUser();
            break;

        case 10:
            showOperationTitle("从文件加载");
            cout << "\n  请输入加载文件名 (例如: network.json): ";
            getline(cin, filename);
            network.loadFromFile(filename);
            waitForUser();
            break;

        case 11:
            showOperationTitle("导出HTML可视化");
            cout << "\n  请输入HTML文件名 (例如: network.html): ";
            getline(cin, filename);
            network.exportToHTML(filename);
            waitForUser();
            break;

        case 0:
            system("cls");
            cout << "\n================================================================================" << endl;
            cout << "                          感谢使用社交网络系统！" << endl;
            cout << "================================================================================" << endl;
            cout << "\n  再见！希望这个系统能帮助你更好地管理社交关系！" << endl;
            cout << "\n  系统功能总结:" << endl;
            cout << "    - 联系人管理: 添加、删除、查询" << endl;
            cout << "    - 关系分析: 亲密度计算、路径分析" << endl;
            cout << "    - 社交网络: 可视化显示、排行榜" << endl;
            cout << "    - 数据持久化: JSON格式保存/加载" << endl;
            cout << "\n================================================================================" << endl;
            return 0;

        case 114:
            testEncoding();
            waitForUser();
            break;

        default:
            showErrorMsg("无效的选择，请输入 0-11 之间的数字！");
            waitForUser();
        }
    }

    return 0;
}