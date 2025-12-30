#include "SocialNetwork.h"
#include <iostream>
#include <limits>
using namespace std;

// 显示菜单
void showMenu() {
    cout << "\n======= 简易社交关系网系统 =======\n";
    cout << "1. 添加联系人\n";
    cout << "2. 删除联系人\n";
    cout << "3. 添加关系（亲密度）\n";
    cout << "4. 删除关系\n";
    cout << "5. 显示所有联系人关系\n";
    cout << "6. 按亲密程度排序好友\n";
    cout << "7. 查询两人亲密度\n";
    cout << "8. 显示社交大牛（Top10）\n";
    cout << "9. 保存到文件\n";
    cout << "10. 从文件加载\n";
    cout << "11. 退出系统\n";
    cout << "请选择操作 (1-11): ";
}

int main() {
    SocialNetwork network;
    int choice;
    string name1, name2, filename;
    int weight;
    bool ascending;

    while (true) {
        showMenu();
        cin >> choice;
        cin.ignore(numeric_limits<streamsize>::max(), '\n'); // 清除输入缓冲区

        switch (choice) {
        case 1:
            cout << "请输入联系人姓名: ";
            getline(cin, name1);
            network.addPerson(name1);
            break;

        case 2:
            cout << "请输入要删除的联系人姓名: ";
            getline(cin, name1);
            network.deletePerson(name1);
            break;

        case 3:
            cout << "请输入第一个人姓名: ";
            getline(cin, name1);
            cout << "请输入第二个人姓名: ";
            getline(cin, name2);
            cout << "请输入亲密度 (1-100): ";
            cin >> weight;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            network.addEdge(name1, name2, weight);
            break;

        case 4:
            cout << "请输入第一个人姓名: ";
            getline(cin, name1);
            cout << "请输入第二个人姓名: ";
            getline(cin, name2);
            network.deleteEdge(name1, name2);
            break;

        case 5:
            network.displayAll();
            break;

        case 6:
            cout << "请输入要排序的联系人姓名: ";
            getline(cin, name1);
            cout << "排序顺序 (0=降序, 1=升序): ";
            cin >> ascending;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            network.sortFriends(name1, ascending);
            break;

        case 7:
            cout << "请输入第一个人姓名: ";
            getline(cin, name1);
            cout << "请输入第二个人姓名: ";
            getline(cin, name2);
            {
                int closeness = network.getBottleneckPath(name1, name2);
                if (closeness == -1) {
                    cout << name1 << " 和 " << name2 << " 之间没有路径可达\n";
                }
                else if (closeness == 0) {
                    cout << name1 << " 和 " << name2 << " 是同一人\n";
                }
                else {
                    cout << name1 << " 和 " << name2 << " 的亲密度为: " << closeness << endl;
                }
            }
            break;

        case 8:
            network.displayTop10();
            break;

        case 9:
            cout << "请输入保存文件名: ";
            getline(cin, filename);
            network.saveToFile(filename);
            break;

        case 10:
            cout << "请输入加载文件名: ";
            getline(cin, filename);
            network.loadFromFile(filename);
            break;

        case 11:
            cout << "感谢使用，再见！\n";
            return 0;

        default:
            cout << "无效的选择，请重新输入！\n";
        }
    }

    return 0;
}