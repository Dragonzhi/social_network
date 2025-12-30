#pragma once

#include <vector>
#include <list>
#include <map>
#include <string>
#include <iostream>
#include <algorithm>
#include <fstream>

#include "Edge.h"
#include "Person.h"

// 用于排序比较
struct FriendInfo {
	string name;
	int weight;
	int index;

	FriendInfo(string n, int c, int i) : name(n), weight(c), index(i) {}

};

class SocialNetwork
{
public:
	SocialNetwork();
	~SocialNetwork();

	// A side
	void addPerson(string name);
	void deletePerson(string name);
	void addEdge(string name1, string name2, int weight);
	void deleteEdge(string name1, string name2);
	void saveToFile(string filename);
	void loadFromFile(string filename);
	void displayAll();

	// B side
	void sortFriends(string name, bool ascending);
	int getBottleneckPath(string startName, string endName);		//路径亲密度下界最大值
	void displayTop10();
	int findIndex(string name);

    // 美化显示函数
    void displayAllBeautiful();
    void displayTop10Beautiful();
    void displaySortFriendsBeautiful(string name, bool ascending);
    void displayBottleneckBeautiful(string startName, string endName);
    void displayGraphASCII();
    void exportToHTML(string filename);
private:
	vector<Person> vertList;
	vector<list<Edge>> adjList;
	map<string, int> nameToIndex;

	// 美化部分
    // ASCII装饰字符
    const string LINE_HORIZ = "================================================================================";
    const string LINE_THIN = "--------------------------------------------------------------------------------";
    const string LINE_DASH = "- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -";

    // ASCII艺术符号
    const string HEART_ICON = "<3";
    const string STAR_ICON = "(*)";
    const string CROWN_ICON = "[C]";
    const string USER_ICON = "[U]";
    const string FRIEND_ICON = "[F]";
    const string NEW_ICON = "[N]";
    const string TOP1_ICON = "[1]";
    const string TOP2_ICON = "[2]";
    const string TOP3_ICON = "[3]";
    const string ADD_ICON = "[+]";
    const string DEL_ICON = "[-]";
    const string SEARCH_ICON = "[?]";
    const string SAVE_ICON = "[S]";
    const string LOAD_ICON = "[L]";
    const string EXIT_ICON = "[X]";
    const string STATS_ICON = "[#]";
    const string ERROR_ICON = "[!]";
    const string SUCCESS_ICON = "[+]";
    const string INFO_ICON = "[i]";

    // 进度条字符
    const string BAR_FULL = "#";
    const string BAR_MID = "=";
    const string BAR_LOW = "-";
    const string BAR_EMPTY = ".";

    // 表格字符
    const string TABLE_CROSS = "+";
    const string TABLE_HORIZ = "-";
    const string TABLE_VERT = "|";
    const string TABLE_T_LEFT = "+";
    const string TABLE_T_RIGHT = "+";
    const string TABLE_T_TOP = "+";
    const string TABLE_T_BOTTOM = "+";

    // 美化显示辅助函数
    string createBoxedText(const string& text, int width = 70);
    string createSectionHeader(const string& title);
    string createProgressBar(float percentage, int length = 20);
    string createSmallProgressBar(float percentage, int length = 10);
    string centerText(const string& text, int width);

};

