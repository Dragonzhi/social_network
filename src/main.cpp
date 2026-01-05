#define NOMINMAX  // 禁用 min/max 宏
#include "..\headc\SocialNetwork.h"
#include <iostream>
#include <string>
#include <limits>
#include <cstdlib>
#include <windows.h>
using namespace std;

// ASCII艺术标题
void showASCIITitle() {
    cout << "\n";
    cout << "  _______  _______  _______  _______  ___   _______  _______ " << endl;
    cout << " (  ____ \\(  ___  )(  ____ )(  ___  )/ __) (  ___  )(  ____ )" << endl;
    cout << " | (    \\/| (   ) || (    )|| (   ) |\\__ \ | (   ) || (    )|" << endl;
    cout << " | (_____ | (___) || (____)|| |   | |(__) )| |   | || (____)|" << endl;
    cout << " (_____  )|  ___  ||     __)| |   | |   | || |   | ||     __)" << endl;
    cout << "       ) || (   ) || (\\ (   | |   | |   | || |   | || (\\ (   " << endl;
    cout << " /\\____) || )   ( || ) \\ \\__| (___) |   | || (___) || ) \\ \\__" << endl;
    cout << " \\_______)|/     \\||/   \\__/(_______)   )_)(_______)|/   \\__/" << endl;
    cout << "\n";
    cout << "================================================================================" << endl;
    cout << "                   简易社交关系网系统 v1.0.19" << endl;
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
    cout << "  [7]  查询两人亲密度       [8]  显示社交达人(Top10)" << endl;
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

// 显示成功信息
void showSuccessMsg(const string& msg) {
    cout << "\n  [+] " << msg << endl;
}

// 显示错误信息
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
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    //cin.get();
}

int main() {
    // Windows下完整的UTF-8设置
    #ifdef _WIN32
    // 设置控制台编码为UTF-8
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    #endif

    SocialNetwork network;
    int choice;

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
            network.deletePerson();
            waitForUser();
            break;

        case 3:
            showOperationTitle("添加关系");
            network.addEdge();
            waitForUser();
            break;

        case 4:
            showOperationTitle("删除关系");
            network.deleteEdge();
            waitForUser();
            break;

        case 5:
            showOperationTitle("所有联系人关系");
            network.displayAll();
            waitForUser();
            break;

        case 6:
            showOperationTitle("按亲密程度排序好友");
            network.sortFriends();
            waitForUser();
            break;

        case 7:
            showOperationTitle("查询两人亲密度");
            network.displayBottleneckBeautiful();
            waitForUser();
            break;

        case 8:
            showOperationTitle("社交达人排行榜");
            network.displayTop10();
            waitForUser();
            break;

        case 9:
            showOperationTitle("保存到文件");
            network.saveToFile();
            waitForUser();
            break;

        case 10:
            showOperationTitle("从文件加载");
            network.loadFromFile();
            waitForUser();
            break;

        case 11:
            showOperationTitle("导出HTML可视化");
            network.exportToHTML();
            waitForUser();
            break;

        case 0:
            system("cls");
            cout << "\n================================================================================" << endl;
            cout << "                          感谢使用社交网络系统！" << endl;
            cout << "================================================================================" << endl;
            cout << "\n  再见！希望这个系统能帮助你更好地管理社交关系。" << endl;
            cout << "\n  系统功能总结:" << endl;
            cout << "    - 联系人管理: 添加、删除、查询" << endl;
            cout << "    - 关系分析: 亲密度计算、路径分析" << endl;
            cout << "    - 社交网络: 可视化显示、排行榜" << endl;
            cout << "    - 数据持久化: JSON格式保存/加载" << endl;
            cout << "\n================================================================================" << endl;
            return 0;

        default:
            showErrorMsg("无效的选择，请输入 0-11 之间的数字！");
            waitForUser();
        }
    }

    return 0;
}