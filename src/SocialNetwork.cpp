#define NOMINMAX  // 禁用 min/max 宏
#include "..\headc\SocialNetwork.h"
#include <queue>
#include <fstream>
#include "..\headc\json.hpp" 
#include <set>
#include <sstream> 
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <chrono>

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#endif
using json = nlohmann::json;

// 在类中添加一个辅助函数
static wstring utf8ToWide(const string& utf8) {
#ifdef _WIN32
    int wlen = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), (int)utf8.size(), NULL, 0);
    if (wlen <= 0) return L"";
    wstring ws(wlen, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), (int)utf8.size(), &ws[0], wlen);
    return ws;
#else
    // 在非Windows平台上，简单转换（假设系统使用UTF-8）
    return wstring(utf8.begin(), utf8.end());
#endif
}

SocialNetwork::SocialNetwork()
{
    // 构造函数，初始化数据结构
}

SocialNetwork::~SocialNetwork()
{
    // 析构函数，清理资源
}

// 替换全角空格 U+3000 -> ASCII space
static void replaceFullWidthSpaces(string& s) {
    const string fullWidth = "\xE3\x80\x80";
    size_t pos = 0;
    while ((pos = s.find(fullWidth, pos)) != string::npos) {
        s.replace(pos, fullWidth.size(), " ");
        pos += 1;
    }
}

static void ltrim(string& s) {
    s.erase(s.begin(), find_if(s.begin(), s.end(), [](unsigned char ch) { return !isspace(ch); }));
}
static void rtrim(string& s) {
    s.erase(find_if(s.rbegin(), s.rend(), [](unsigned char ch) { return !isspace(ch); }).base(), s.end());
}

#ifdef _WIN32
// 将 UTF-16 wide string 转成 UTF-8 string
static string wideToUtf8(const wstring& ws) {
    if (ws.empty()) return {};
    int len = WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), (int)ws.size(), NULL, 0, NULL, NULL);
    if (len <= 0) return {};
    string out(len, '\0');
    WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), (int)ws.size(), &out[0], len, NULL, NULL);
    return out;
}
#endif

// 调试：打印字节十六进制
static void debug_print_hex(const string& s, const char* label = nullptr) {
#ifdef DEBUG
    if (label) cout << label;
    cout << "DEBUG: raw bytes (len=" << s.size() << "): ";
    cout << hex << setfill('0');
    for (unsigned char c : s) {
        cout << setw(2) << (int)c << " ";
    }
    cout << dec << "\n";
#endif
}

void SocialNetwork::addPersons() {
#ifdef _WIN32
    // 切换 stdin/stdout 到 UTF-16 模式，使得 wcin/wcout 能正确读取/写入控制台（Windows 控制台）
    // 注意：若输入被重定向（例如文件或管道），_setmode 可能不适用。通常对交互式控制台有效。
    _setmode(_fileno(stdin), _O_U16TEXT);
    _setmode(_fileno(stdout), _O_U16TEXT);

    wstring winput;
    wcout << L"请输入要添加的联系人名称，多个名称用空格分隔：\n";
    getline(wcin, winput);

    if (winput.empty()) {
        wcout << L"输入不能为空！\n";
        // 可将模式恢复为默认（可选）
        _setmode(_fileno(stdin), _O_TEXT);
        _setmode(_fileno(stdout), _O_TEXT);
        return;
    }

    // 将宽字符串转换为 UTF-8 string，后续逻辑以 UTF-8 字符串处理
    string input = wideToUtf8(winput);

    // 可选：恢复到窄模式（如果你的程序中后续大量使用 cout，建议恢复）
    _setmode(_fileno(stdin), _O_TEXT);
    _setmode(_fileno(stdout), _O_TEXT);
#else
    string input;
    cout << "请输入要添加的联系人名称，多个名称用空格分隔：\n";
    getline(cin, input);
    if (input.empty()) {
        cout << "输入不能为空！\n";
        return;
    }
#endif

    // DEBUG（若需要看转换后的字节）
    // debug_print_hex(input, "After read/convert: ");

    // 归一化：把全角空格替换为普通空格（防止无法分割）
    replaceFullWidthSpaces(input);

    // 去掉前后空白
    ltrim(input);
    rtrim(input);

    if (input.empty()) {
        cout << "未检测到有效姓名！\n";
        return;
    }

    // 按 ASCII 空白分割
    stringstream ss(input);
    string name;
    vector<string> names;
    int addedCount = 0;
    int existCount = 0;

    while (ss >> name) {
        if (name.empty()) continue;
        names.push_back(name);
    }

    if (names.empty()) {
        cout << "未检测到有效姓名！\n";
        return;
    }

    cout << "\n正在添加以下联系人：\n";
    for (const auto& n : names) {
        cout << n << " ";
    }
    cout << "\n\n";

    for (const auto& n : names) {
        if (n.empty()) continue;

        if (findIndex(n) != -1) {
            cout << "联系人 " << n << " 已存在，跳过添加。\n";
            existCount++;
            continue;
        }

        Person p;
        p.setName(n); // setName 接受 UTF-8 string
        vertList.push_back(p);
        adjList.push_back(list<Edge>());
        nameToIndex[n] = static_cast<int>(vertList.size()) - 1;
        addedCount++;
        cout << "联系人 " << n << " 添加成功！\n";
    }

    cout << "\n批量添加完成：\n";
    cout << "成功添加 " << addedCount << " 个联系人。\n";
    if (existCount > 0) {
        cout << "跳过 " << existCount << " 个已存在的联系人。\n";
    }
}
// 删除联系人
void SocialNetwork::deletePerson() {  // 注意：移除了参数，改为从控制台读取
#ifdef _WIN32
    // 切换 stdin/stdout 到 UTF-16 模式
    _setmode(_fileno(stdin), _O_U16TEXT);
    _setmode(_fileno(stdout), _O_U16TEXT);

    wstring winput;
    wcout << L"请输入要删除的联系人姓名: ";
    getline(wcin, winput);

    if (winput.empty()) {
        wcout << L"输入不能为空！\n";
        _setmode(_fileno(stdin), _O_TEXT);
        _setmode(_fileno(stdout), _O_TEXT);
        return;
    }

    // 将宽字符串转换为 UTF-8 string
    string name = wideToUtf8(winput);

    // 归一化：把全角空格替换为普通空格
    replaceFullWidthSpaces(name);

    // 去掉前后空白
    ltrim(name);
    rtrim(name);

    if (name.empty()) {
        wcout << L"未检测到有效姓名！\n";
        _setmode(_fileno(stdin), _O_TEXT);
        _setmode(_fileno(stdout), _O_TEXT);
        return;
    }

    int index = findIndex(name);
    if (index == -1) {
        wstring wname = utf8ToWide(name);
        wcout << L"联系人 " << wname << L" 不存在！\n";
        _setmode(_fileno(stdin), _O_TEXT);
        _setmode(_fileno(stdout), _O_TEXT);
        return;
    }

    // 1. 删除该人的所有关系（从其他人的邻接表中删除）
    for (int i = 0; i < adjList.size(); i++) {
        if (i == index) continue;

        auto& friends = adjList[i];
        for (auto it = friends.begin(); it != friends.end();) {
            if (it->getTo() == index) {
                it = friends.erase(it);
            }
            else {
                ++it;
            }
        }
    }

    // 2. 删除该人的邻接表
    adjList.erase(adjList.begin() + index);

    // 3. 删除该人
    vertList.erase(vertList.begin() + index);

    // 4. 更新nameToIndex映射
    nameToIndex.erase(name);
    for (auto& pair : nameToIndex) {
        if (pair.second > index) {
            pair.second--;
        }
    }

    wstring wname = utf8ToWide(name);
    wcout << L"联系人 " << wname << L" 删除成功！\n";
    _setmode(_fileno(stdin), _O_TEXT);
    _setmode(_fileno(stdout), _O_TEXT);
#else
    string name;
    cout << "请输入要删除的联系人姓名: ";
    getline(cin, name);

    if (name.empty()) {
        cout << "输入不能为空！\n";
        return;
    }

    // 归一化：把全角空格替换为普通空格
    replaceFullWidthSpaces(name);

    // 去掉前后空白
    ltrim(name);
    rtrim(name);

    if (name.empty()) {
        cout << "未检测到有效姓名！\n";
        return;
    }

    int index = findIndex(name);
    if (index == -1) {
        cout << "联系人 " << name << " 不存在！\n";
        return;
    }

    // 1. 删除该人的所有关系（从其他人的邻接表中删除）
    for (int i = 0; i < adjList.size(); i++) {
        if (i == index) continue;

        auto& friends = adjList[i];
        for (auto it = friends.begin(); it != friends.end();) {
            if (it->getTo() == index) {
                it = friends.erase(it);
            }
            else {
                ++it;
            }
        }
    }

    // 2. 删除该人的邻接表
    adjList.erase(adjList.begin() + index);

    // 3. 删除该人
    vertList.erase(vertList.begin() + index);

    // 4. 更新nameToIndex映射
    nameToIndex.erase(name);
    for (auto& pair : nameToIndex) {
        if (pair.second > index) {
            pair.second--;
        }
    }

    cout << "联系人 " << name << " 删除成功！\n";
#endif
}

// 添加关系 - 同样需要修改为从控制台读取
void SocialNetwork::addEdge() {  // 移除参数
#ifdef _WIN32
    // 切换 stdin/stdout 到 UTF-16 模式
    _setmode(_fileno(stdin), _O_U16TEXT);
    _setmode(_fileno(stdout), _O_U16TEXT);

    wstring wname1, wname2;
    int weight;

    wcout << L"请输入第一个人名: ";
    getline(wcin, wname1);
    wcout << L"请输入第二个人名: ";
    getline(wcin, wname2);
    wcout << L"请输入亲密度 (整数): ";
    wcin >> weight;
    wcin.ignore(); // 清除换行符

    if (wname1.empty() || wname2.empty()) {
        wcout << L"输入不能为空！\n";
        _setmode(_fileno(stdin), _O_TEXT);
        _setmode(_fileno(stdout), _O_TEXT);
        return;
    }

    // 将宽字符串转换为 UTF-8 string
    string name1 = wideToUtf8(wname1);
    string name2 = wideToUtf8(wname2);

    // 归一化：把全角空格替换为普通空格
    replaceFullWidthSpaces(name1);
    replaceFullWidthSpaces(name2);

    // 去掉前后空白
    ltrim(name1); rtrim(name1);
    ltrim(name2); rtrim(name2);

    if (name1.empty() || name2.empty()) {
        wcout << L"未检测到有效姓名！\n";
        _setmode(_fileno(stdin), _O_TEXT);
        _setmode(_fileno(stdout), _O_TEXT);
        return;
    }

    int index1 = findIndex(name1);
    int index2 = findIndex(name2);

    if (index1 == -1 || index2 == -1) {
        wcout << L"输入的人员不存在！\n";
        _setmode(_fileno(stdin), _O_TEXT);
        _setmode(_fileno(stdout), _O_TEXT);
        return;
    }

    if (index1 == index2) {
        wcout << L"不能与自己建立关系！\n";
        _setmode(_fileno(stdin), _O_TEXT);
        _setmode(_fileno(stdout), _O_TEXT);
        return;
    }

    // 检查关系是否已存在
    auto& friends1 = adjList[index1];
    for (auto& edge : friends1) {
        if (edge.getTo() == index2) {
            wcout << wname1 << L" 和 " << wname2 << L" 的关系已存在，亲密度为: "
                << edge.getWeight() << L"\n";
            _setmode(_fileno(stdin), _O_TEXT);
            _setmode(_fileno(stdout), _O_TEXT);
            return;
        }
    }

    // 添加双向关系（无向图）
    Edge edge1, edge2;
    edge1.setTo(index2);
    edge1.setWeight(weight);
    edge2.setTo(index1);
    edge2.setWeight(weight);

    adjList[index1].push_back(edge1);
    adjList[index2].push_back(edge2);

    wcout << L"已建立 " << wname1 << L" 和 " << wname2 << L" 的关系，亲密度: " << weight << L"\n";
    _setmode(_fileno(stdin), _O_TEXT);
    _setmode(_fileno(stdout), _O_TEXT);
#else
    string name1, name2;
    int weight;

    cout << "请输入第一个人名: ";
    getline(cin, name1);
    cout << "请输入第二个人名: ";
    getline(cin, name2);
    cout << "请输入亲密度 (整数): ";
    cin >> weight;
    cin.ignore(); // 清除换行符

    if (name1.empty() || name2.empty()) {
        cout << "输入不能为空！\n";
        return;
    }

    // 归一化：把全角空格替换为普通空格
    replaceFullWidthSpaces(name1);
    replaceFullWidthSpaces(name2);

    // 去掉前后空白
    ltrim(name1); rtrim(name1);
    ltrim(name2); rtrim(name2);

    if (name1.empty() || name2.empty()) {
        cout << "未检测到有效姓名！\n";
        return;
    }

    int index1 = findIndex(name1);
    int index2 = findIndex(name2);

    if (index1 == -1 || index2 == -1) {
        cout << "输入的人员不存在！\n";
        return;
    }

    if (index1 == index2) {
        cout << "不能与自己建立关系！\n";
        return;
    }

    // 检查关系是否已存在
    auto& friends1 = adjList[index1];
    for (auto& edge : friends1) {
        if (edge.getTo() == index2) {
            cout << name1 << " 和 " << name2 << " 的关系已存在，亲密度为: "
                << edge.getWeight() << "\n";
            return;
        }
    }

    // 添加双向关系（无向图）
    Edge edge1, edge2;
    edge1.setTo(index2);
    edge1.setWeight(weight);
    edge2.setTo(index1);
    edge2.setWeight(weight);

    adjList[index1].push_back(edge1);
    adjList[index2].push_back(edge2);

    cout << "已建立 " << name1 << " 和 " << name2 << " 的关系，亲密度: " << weight << "\n";
#endif
}

// 删除关系 - 同样需要修改
void SocialNetwork::deleteEdge() {  // 移除参数
#ifdef _WIN32
    // 切换 stdin/stdout 到 UTF-16 模式
    _setmode(_fileno(stdin), _O_U16TEXT);
    _setmode(_fileno(stdout), _O_U16TEXT);

    wstring wname1, wname2;

    wcout << L"请输入第一个人名: ";
    getline(wcin, wname1);
    wcout << L"请输入第二个人名: ";
    getline(wcin, wname2);

    if (wname1.empty() || wname2.empty()) {
        wcout << L"输入不能为空！\n";
        _setmode(_fileno(stdin), _O_TEXT);
        _setmode(_fileno(stdout), _O_TEXT);
        return;
    }

    // 将宽字符串转换为 UTF-8 string
    string name1 = wideToUtf8(wname1);
    string name2 = wideToUtf8(wname2);

    // 归一化：把全角空格替换为普通空格
    replaceFullWidthSpaces(name1);
    replaceFullWidthSpaces(name2);

    // 去掉前后空白
    ltrim(name1); rtrim(name1);
    ltrim(name2); rtrim(name2);

    if (name1.empty() || name2.empty()) {
        wcout << L"未检测到有效姓名！\n";
        _setmode(_fileno(stdin), _O_TEXT);
        _setmode(_fileno(stdout), _O_TEXT);
        return;
    }

    int index1 = findIndex(name1);
    int index2 = findIndex(name2);

    if (index1 == -1 || index2 == -1) {
        wcout << L"输入的人员不存在！\n";
        _setmode(_fileno(stdin), _O_TEXT);
        _setmode(_fileno(stdout), _O_TEXT);
        return;
    }

    bool found = false;

    // 删除name1的邻接表中的关系
    auto& friends1 = adjList[index1];
    for (auto it = friends1.begin(); it != friends1.end();) {
        if (it->getTo() == index2) {
            it = friends1.erase(it);
            found = true;
        }
        else {
            ++it;
        }
    }

    // 删除name2的邻接表中的关系
    auto& friends2 = adjList[index2];
    for (auto it = friends2.begin(); it != friends2.end();) {
        if (it->getTo() == index1) {
            it = friends2.erase(it);
            found = true;
        }
        else {
            ++it;
        }
    }

    if (found) {
        wcout << L"已删除 " << wname1 << L" 和 " << wname2 << L" 的关系\n";
    }
    else {
        wcout << wname1 << L" 和 " << wname2 << L" 之间没有关系\n";
    }
    _setmode(_fileno(stdin), _O_TEXT);
    _setmode(_fileno(stdout), _O_TEXT);
#else
    string name1, name2;

    cout << "请输入第一个人名: ";
    getline(cin, name1);
    cout << "请输入第二个人名: ";
    getline(cin, name2);

    if (name1.empty() || name2.empty()) {
        cout << "输入不能为空！\n";
        return;
    }

    // 归一化：把全角空格替换为普通空格
    replaceFullWidthSpaces(name1);
    replaceFullWidthSpaces(name2);

    // 去掉前后空白
    ltrim(name1); rtrim(name1);
    ltrim(name2); rtrim(name2);

    if (name1.empty() || name2.empty()) {
        cout << "未检测到有效姓名！\n";
        return;
    }

    int index1 = findIndex(name1);
    int index2 = findIndex(name2);

    if (index1 == -1 || index2 == -1) {
        cout << "输入的人员不存在！\n";
        return;
    }

    bool found = false;

    // 删除name1的邻接表中的关系
    auto& friends1 = adjList[index1];
    for (auto it = friends1.begin(); it != friends1.end();) {
        if (it->getTo() == index2) {
            it = friends1.erase(it);
            found = true;
        }
        else {
            ++it;
        }
    }

    // 删除name2的邻接表中的关系
    auto& friends2 = adjList[index2];
    for (auto it = friends2.begin(); it != friends2.end();) {
        if (it->getTo() == index1) {
            it = friends2.erase(it);
            found = true;
        }
        else {
            ++it;
        }
    }

    if (found) {
        cout << "已删除 " << name1 << " 和 " << name2 << " 的关系\n";
    }
    else {
        cout << name1 << " 和 " << name2 << " 之间没有关系\n";
    }
#endif
}



void SocialNetwork::saveToFile() {
#ifdef _WIN32
    _setmode(_fileno(stdin), _O_U16TEXT);
    _setmode(_fileno(stdout), _O_U16TEXT);
    wstring wfilename;
    wcout << L"\n  请输入保存文件名 (例如: network.json): ";
    wcin >> wfilename;
    wcin.ignore(numeric_limits<streamsize>::max(), '\n');
    string filename = wideToUtf8(wfilename);
    _setmode(_fileno(stdin), _O_TEXT);
    _setmode(_fileno(stdout), _O_TEXT);
#else
    string filename;
    cout << "\n  请输入保存文件名 (例如: network.json): ";
    cin >> filename;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
#endif
    if (filename.empty()) {
        cout << "文件名不能为空！\n";
        return;
    }

    // 检查是否以.json（不区分大小写）
    string lowerFilename = filename;
    transform(lowerFilename.begin(), lowerFilename.end(), lowerFilename.begin(), ::tolower);

    if (lowerFilename.size() < 5 ||
        (lowerFilename.substr(lowerFilename.size() - 5) != ".json")) {
        filename += ".json";
    }

    // 创建 JSON 对象
    json data = json::object();

    // 1. 保存人员信息
    json persons = json::array();
    for (const auto& person : vertList) {
        persons.push_back({ {"name", person.getName()} });
    }
    data["persons"] = persons;

    // 2. 保存关系信息
    json edges = json::array();
    // 使用一个集合来避免重复保存双向关系
    set<pair<int, int>> saved_edges;
    for (int i = 0; i < vertList.size(); i++) {
        for (const auto& edge : adjList[i]) {
            int to = edge.getTo();
            // 创建有序对，确保较小的索引在前
            pair<int, int> edge_pair = (i < to) ? make_pair(i, to) : make_pair(to, i);
            // 检查是否已经保存过这条边
            if (saved_edges.find(edge_pair) == saved_edges.end()) {
                json edge_json;
                edge_json["from"] = vertList[i].getName();
                edge_json["to"] = vertList[to].getName();
                edge_json["weight"] = edge.getWeight();
                edges.push_back(edge_json);
                saved_edges.insert(edge_pair);
            }
        }
    }
    data["edges"] = edges;

    // 3. 保存元数据
    data["metadata"]["person_count"] = vertList.size();
    data["metadata"]["edge_count"] = edges.size();
    // 添加时间戳
    time_t now = time(0);
    tm* local_time = localtime(&now);
    char time_str[100];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", local_time);
    data["metadata"]["save_time"] = time_str;
    data["metadata"]["version"] = "1.0";

    // 4. 写入文件
    try {
        // 以二进制模式并带BOM的方式写入UTF-8，确保兼容性
        ofstream file(filename, ios::out | ios::binary); 
        if (!file.is_open()) {
            cout << "无法打开文件 " << filename << " 进行写入！\n";
            return;
        }

        // 写入 UTF-8 BOM (Byte Order Mark)
        const unsigned char utf8_bom[] = { 0xEF, 0xBB, 0xBF };
        file.write(reinterpret_cast<const char*>(utf8_bom), sizeof(utf8_bom));

        // nlohmann::json 库会生成 UTF-8 编码的字符串
        file << data.dump(4); // 缩进4个空格，使JSON更易读

        file.close();
        cout << "数据已成功保存到文件: " << filename << endl;
        cout << "保存了 " << vertList.size() << " 个联系人和 " << edges.size() << " 条关系\n";
        cout << "保存时间: " << time_str << endl;
    }
    catch (const exception& e) {
        cout << "保存文件时发生错误: " << e.what() << endl;
    }
}

void SocialNetwork::loadFromFile() {
#ifdef _WIN32
    _setmode(_fileno(stdin), _O_U16TEXT);
    _setmode(_fileno(stdout), _O_U16TEXT);
    wstring wfilename;
    wcout << L"\n  请输入加载文件名 (例如: network.json): ";
    wcin >> wfilename;
    wcin.ignore(numeric_limits<streamsize>::max(), '\n');
    string filename = wideToUtf8(wfilename);
    _setmode(_fileno(stdin), _O_TEXT);
    _setmode(_fileno(stdout), _O_TEXT);
#else
    string filename;
    cout << "\n  请输入加载文件名 (例如: network.json): ";
    cin >> filename;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
#endif
    if (filename.empty()) {
        cout << "文件名不能为空！\n";
        return;
    }
    // 检查是否以.json（不区分大小写）
    string lowerFilename = filename;
    transform(lowerFilename.begin(), lowerFilename.end(), lowerFilename.begin(), ::tolower);

    if (lowerFilename.size() < 5 ||
        (lowerFilename.substr(lowerFilename.size() - 5) != ".json")) {
        filename += ".json";
    }
    // 清空现有数据
    vertList.clear();
    adjList.clear();
    nameToIndex.clear();

    try {
        // 1. 打开文件并检查
        ifstream file(filename, ios::binary); // 以二进制模式读取
        if (!file.is_open()) {
            cout << "无法打开文件 " << filename << "！\n";
            return;
        }

        // 2. 检查文件是否为空
        file.seekg(0, ios::end);
        if (file.tellg() == 0) {
            cout << "文件为空！\n";
            file.close();
            return;
        }
        file.seekg(0, ios::beg);

        // 3. 解析JSON (nlohmann::json handles BOM and UTF-8 automatically)
        json data;
        file >> data;
        file.close();

        // 4. 验证必需字段
        if (!data.contains("persons") || !data["persons"].is_array()) {
            cout << "错误: JSON 文件中缺少 'persons' 数组！\n";
            return;
        }

        // 5. 加载人员信息
        int loaded_persons = 0;
        for (const auto& person_json : data["persons"]) {
            if (person_json.contains("name") && person_json["name"].is_string()) {
                string person_name = person_json["name"];
                if (!person_name.empty()) {
                    Person p;
                    p.setName(person_name);
                    vertList.push_back(p);
                    adjList.push_back(list<Edge>());
                    nameToIndex[person_name] = vertList.size() - 1;
                    loaded_persons++;
                }
            }
        }
        if (loaded_persons == 0) {
            cout << "警告: 文件中没有找到有效的人员信息！\n";
        }

        // 6. 加载关系信息（如果存在）
        int loaded_edges = 0;
        if (data.contains("edges") && data["edges"].is_array()) {
            for (const auto& edge_json : data["edges"]) {
                if (edge_json.contains("from") && edge_json.contains("to") && edge_json.contains("weight") && edge_json["weight"].is_number()) {
                    string from_name = edge_json["from"];
                    string to_name = edge_json["to"];
                    int weight = edge_json["weight"];

                    if (weight < 1 || weight > 100) {
                        cout << "警告: 关系权重 " << weight << " 不在有效范围(1-100)内，将被忽略\n";
                        continue;
                    }

                    int from_index = findIndex(from_name);
                    int to_index = findIndex(to_name);

                    if (from_index != -1 && to_index != -1 && from_index != to_index) {
                        bool exists = false;
                        for (const auto& edge : adjList[from_index]) {
                            if (edge.getTo() == to_index) {
                                exists = true;
                                break;
                            }
                        }
                        if (!exists) {
                            Edge edge1, edge2;
                            edge1.setTo(to_index);
                            edge1.setWeight(weight);
                            edge2.setTo(from_index);
                            edge2.setWeight(weight);
                            adjList[from_index].push_back(edge1);
                            adjList[to_index].push_back(edge2);
                            loaded_edges++;
                        }
                        else {
                            cout << "警告: 关系 " << from_name << " -> " << to_name << " 已存在，跳过加载。\n";
                        }
                    }
                    else {
                        cout << "警告: 无法加载关系 " << from_name << " -> " << to_name << "，因为至少一人不存在或是同一个人\n";
                    }
                }
            }
        }

        // 7. 显示加载统计信息
        cout << "从文件 " << filename << " 加载成功！\n";
        cout << "加载了 " << loaded_persons << " 个联系人和 " << loaded_edges << " 条关系\n";

        if (data.contains("metadata")) {
            auto& metadata = data["metadata"];
            if (metadata.contains("save_time")) {
                cout << "原保存时间: " << metadata["save_time"] << endl;
            }
            if (metadata.contains("version")) {
                cout << "文件版本: " << metadata["version"] << endl;
            }
        }
    }
    catch (const json::parse_error& e) {
        cout << "JSON 解析错误: " << e.what() << endl;
        cout << "请确保文件是有效的 JSON 格式，且为 UTF-8 编码！\n";
    }
    catch (const exception& e) {
        cout << "加载文件时发生未知错误: " << e.what() << endl;
        cout << "请确保文件存在且格式正确！\n";
    }
}

// 显示所有联系人关系
void SocialNetwork::displayAll() {
    if (vertList.empty()) {
        cout << createSectionHeader("社交网络状态");
        cout << "\n" << ERROR_ICON << " 社交网络为空！暂时还没有任何联系人\n";

        return;
    }

    // 头部
    cout << createSectionHeader("社交关系网络总览");

    int totalFriends = 0;
    int totalIntimacy = 0;

    // 显示每个联系人
    for (int i = 0; i < vertList.size(); i++) {
        string name = vertList[i].getName();
        int friendCount = adjList[i].size();
        totalFriends += friendCount;

        // 选择图标
        string icon;
        if (friendCount >= 10) icon = CROWN_ICON + " ";
        else if (friendCount >= 5) icon = STAR_ICON + " ";
        else if (friendCount >= 2) icon = USER_ICON + " ";
        else if (friendCount == 1) icon = FRIEND_ICON + " ";
        else icon = NEW_ICON + " ";

        cout << "\n" << icon << " " << left << setw(20) << name;
        cout << "好友数: [" << setw(2) << friendCount << "] ";

        // 显示亲密度进度条
        float avgIntimacy = 0;
        if (friendCount > 0) {
            int totalWeight = 0;
            for (auto& edge : adjList[i]) {
                totalWeight += edge.getWeight();
                totalIntimacy += edge.getWeight();
            }
            avgIntimacy = totalWeight / (float)friendCount;
        }

        cout << "亲密度: " << createProgressBar(avgIntimacy, 15);

        cout << "\n" << string(4, ' ') << "好友列表: ";

        // 显示好友
        if (adjList[i].empty()) {
            cout << "暂无好友";
        }
        else {
            int count = 0;
            for (auto& edge : adjList[i]) {
                int to = edge.getTo();
                int weight = edge.getWeight();

                cout << vertList[to].getName() << "(" << weight << ")";
                count++;

                if (count < adjList[i].size()) {
                    cout << ", ";
                    if (count % 4 == 0) {
                        cout << "\n" << string(15, ' ');
                    }
                }
            }
        }

        // 添加分隔线
        if (i < vertList.size() - 1) {
            cout << "\n" << LINE_DASH << "\n";
        }
    }

    // 统计信息
    cout << "\n\n" << LINE_THIN;
    cout << "\n" << STATS_ICON << " 网络统计信息:";
    cout << "\n   总人数: " << vertList.size();
    cout << "\n   总关系数: " << totalFriends / 2;
    cout << "\n   人均好友数: " << fixed << setprecision(1)
        << (vertList.size() > 0 ? totalFriends / (float)vertList.size() : 0);
    cout << "\n   平均亲密度: " << fixed << setprecision(1)
        << (totalFriends > 0 ? totalIntimacy / (float)totalFriends : 0);
    cout << "\n" << LINE_THIN << "\n";
}

// 按亲密程度排序好友
void SocialNetwork::sortFriends() {
#ifdef _WIN32
    _setmode(_fileno(stdin), _O_U16TEXT);
    _setmode(_fileno(stdout), _O_U16TEXT);

    wstring wname;

    wcout << L"\n  请输入要排序的联系人姓名: ";
    getline(wcin, wname);
    bool ascending;
    bool validInput = false;

    while (!validInput) {
        wcout << L"  排序顺序 (0=降序, 1=升序): ";

        // 检查输入是否成功
        if (wcin >> ascending) {
            if (ascending == 0 || ascending == 1) {
                validInput = true;
            }
            else {
                wcout << L"  错误：请输入0或1！\n";
            }
        }
        else {
            wcout << L"  错误：输入无效，请输入0或1！\n";
            wcin.clear();  // 清除错误状态
        }
        wcin.ignore(numeric_limits<streamsize>::max(), '\n');
    }

    string name = wideToUtf8(wname);
    ltrim(name); rtrim(name);
    replaceFullWidthSpaces(name);

    if (name.empty()) {
        wcout << L"未检测到有效姓名！\n";
        _setmode(_fileno(stdin), _O_TEXT);
        _setmode(_fileno(stdout), _O_TEXT);
        return;
    }

    _setmode(_fileno(stdin), _O_TEXT);
    _setmode(_fileno(stdout), _O_TEXT);
#else
    string name;
    bool ascending;
    cout << "\n  请输入要排序的联系人姓名: ";
    getline(cin, name);
    cout << "  排序顺序 (0=降序, 1=升序): ";
    cin >> ascending;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    ltrim(name); rtrim(name);
    replaceFullWidthSpaces(name);
    if (name.empty()) {
        cout << "未检测到有效姓名！\n";
        return;
    }
#endif

    int index = findIndex(name);
    if (index == -1) {
        cout << ERROR_ICON << " 联系人 " << name << " 不存在！\n";
        return;
    }

    vector<FriendInfo> friend_infos;
    for (auto& e : adjList[index]) {
        friend_infos.push_back(FriendInfo(vertList[e.getTo()].getName(), e.getWeight(), e.getTo()));
    }

    if (friend_infos.empty()) {
        cout << INFO_ICON << " " << name << " 暂时没有好友\n";
        return;
    }

    // 排序
    if (ascending) {
        sort(friend_infos.begin(), friend_infos.end(),
            [](const FriendInfo& a, const FriendInfo& b) {
                return a.weight < b.weight;
            });

        cout << createSectionHeader(name + " 的好友列表（按亲密度升序）");
    }
    else {
        sort(friend_infos.begin(), friend_infos.end(),
            [](const FriendInfo& a, const FriendInfo& b) {
                return a.weight > b.weight;
            });

        cout << createSectionHeader(name + " 的好友列表（按亲密度降序）");

        // 显示前三名
        cout << "\n 前三名好友:\n";
        cout << LINE_THIN << "\n";

        for (int i = 0; i < min(3, (int)friend_infos.size()); i++) {
            string rank;
            if (i == 0) rank = TOP1_ICON;
            else if (i == 1) rank = TOP2_ICON;
            else rank = TOP3_ICON;

            cout << " " << rank << " " << left << setw(15) << friend_infos[i].name;
            cout << " -> " << createProgressBar(friend_infos[i].weight, 12);

            string level;
            if (friend_infos[i].weight >= 90) level = " (挚友)";
            else if (friend_infos[i].weight >= 70) level = " (好友)";
            else if (friend_infos[i].weight >= 50) level = " (普通)";
            else level = " (相识)";

            cout << level << "\n";
        }

        if (friend_infos.size() > 3) {
            cout << "\n 其他好友:\n";
            cout << LINE_THIN << "\n";
        }
    }

    // 显示所有好友（如果是升序，显示全部；如果是降序，从第4个开始）
    int start = ascending ? 0 : 3;
    for (int i = start; i < friend_infos.size(); i++) {
        cout << " " << setw(2) << (i + 1) << ". " << left << setw(15) << friend_infos[i].name;
        cout << " -> " << createProgressBar(friend_infos[i].weight, 12) << "\n";
    }

    // 统计信息
    float avgWeight = 0;
    for (auto& info : friend_infos) avgWeight += info.weight;
    avgWeight /= friend_infos.size();

    cout << "\n" << LINE_THIN;
    cout << "\n" << STATS_ICON << " 统计信息:";
    cout << "\n   好友总数: " << friend_infos.size();
    cout << "\n   平均亲密度: " << fixed << setprecision(1) << avgWeight << "%";
    cout << "\n   最高亲密度: " << friend_infos[0].weight << "%";
    cout << "\n   最低亲密度: " << friend_infos.back().weight << "%";
    cout << "\n" << LINE_THIN << "\n";
}

//路径亲密度下界最大值
int SocialNetwork::getBottleneckPath(string startName, string endName) {
    int start = findIndex(startName);
    int end = findIndex(endName);

    if (start == -1 || end == -1) return -1; // Person not found
    if (start == end) return 0; // Same person

    int personCount = vertList.size();
    vector<int> maxBottleNeck(personCount, 0);  // 存储到每个节点的最大瓶颈值
    vector<bool> visited(personCount, false);   // 标记是否已访问
    priority_queue<pair<int, int>> pq;          // 大顶堆，(瓶颈值, 节点索引)

    maxBottleNeck[start] = INT_MAX; // 起点到自身的最小亲密度是"无限大"
    pq.push({ INT_MAX, start });

    while (!pq.empty()) {
        auto current = pq.top();
        int u = current.second;
        pq.pop();

        if (visited[u]) continue; // If already visited, skip.
        visited[u] = true;

        if (u == end) break; // 找到了通往终点节点的最佳路径。

        for (auto& e : adjList[u]) {
            int v = e.getTo();
            int weight = e.getWeight();

            if (!visited[v]) { // 只考虑未访问过的相邻节点
                int newBottleneck = min(maxBottleNeck[u], weight);
                if (newBottleneck > maxBottleNeck[v]) {
                    maxBottleNeck[v] = newBottleneck;
                    pq.push({ newBottleneck, v });
                }
            }
        }
    }

    // 如果我们完成循环而 maxBottleNeck[end] 仍然为 0，则表示不存在路径。
    // 返回 -1 表示“不存在路径”（与“未找到人员”错误保持一致）
    return (maxBottleNeck[end] == 0) ? -1 : maxBottleNeck[end];
}
	
void SocialNetwork::displayTop10() {
    int personCount = vertList.size();
    if (personCount <= 0) {
        cout << ERROR_ICON << " 社交网络为空！\n";
        return;
    }

    vector<pair<int, int>> friendCounts;
    for (int i = 0; i < personCount; i++) {
        friendCounts.push_back({ i, (int)adjList[i].size() });
    }

    sort(friendCounts.begin(), friendCounts.end(),
        [](const pair<int, int>& a, const pair<int, int>& b) {
            return a.second > b.second;
        });

    int displayCount = min(10, personCount);

    cout << createSectionHeader("社交大牛排行榜 TOP" + to_string(displayCount));

    // 创建表格
    cout << "\n" << TABLE_CROSS << string(70, TABLE_HORIZ[0]) << TABLE_CROSS << "\n";
    cout << TABLE_VERT << " 排名 |       姓名        | 好友数 | 社交影响力               " << TABLE_VERT << "\n";
    cout << TABLE_CROSS << string(70, TABLE_HORIZ[0]) << TABLE_CROSS << "\n";

    for (int i = 0; i < displayCount; i++) {
        int idx = friendCounts[i].first;
        string name = vertList[idx].getName();
        int count = friendCounts[i].second;

        // 排名
        string rank;
        if (i == 0) rank = TOP1_ICON;
        else if (i == 1) rank = TOP2_ICON;
        else if (i == 2) rank = TOP3_ICON;
        else rank = " " + to_string(i + 1) + " ";

        cout << TABLE_VERT << " " << left << setw(4) << rank;

        // 姓名
        string nameIcon;
        if (count >= 15) nameIcon = CROWN_ICON;
        else if (count >= 10) nameIcon = STAR_ICON;
        else if (count >= 5) nameIcon = USER_ICON;
        else nameIcon = "   ";

        cout << " | " << nameIcon << " " << setw(15) << name;

        // 好友数
        int maxFriends = friendCounts[0].second;
        float friendRatio = (float)count / maxFriends;
        cout << " | [" << setw(2) << count << "] ";

        // 进度条
        cout << createSmallProgressBar(friendRatio * 100, 10);

        // 好友列表（前2个）
        cout << " | ";
        int shown = 0;
        for (auto& edge : adjList[idx]) {
            if (shown >= 2) break;
            cout << vertList[edge.getTo()].getName();
            if (shown < 1 && adjList[idx].size() > 1) cout << ",";
            shown++;
        }
        if (adjList[idx].size() > 2) {
            cout << "等" << adjList[idx].size() << "人";
        }

        // 对齐
        int remaining = 20 - shown * 8;
        cout << string(remaining, ' ') << TABLE_VERT << "\n";

        // 分隔线
        if (i < displayCount - 1) {
            cout << TABLE_VERT << string(70, '-') << TABLE_VERT << "\n";
        }
    }

    cout << TABLE_CROSS << string(70, TABLE_HORIZ[0]) << TABLE_CROSS << "\n";
}
int SocialNetwork::findIndex(string name) {
	if (nameToIndex.find(name) == nameToIndex.end()) return -1;		// 未找到名字对应的人的ID，返回-1
	return nameToIndex[name];
}



// =============== 辅助函数实现 ===============

string SocialNetwork::createBoxedText(const string& text, int width) {
    string result;
    string border(width, '=');
    result = border + "\n";
    result += centerText(text, width) + "\n";
    result += border + "\n";
    return result;
}

string SocialNetwork::createSectionHeader(const string& title) {
    string result;
    string border(60, '=');
    result = "\n" + border + "\n";

    // 计算居中位置
    int padding = (60 - title.length()) / 2;
    result += string(padding, ' ') + title + "\n";
    result += border + "\n";

    return result;
}

string SocialNetwork::createProgressBar(float percentage, int length) {
    string bar = "[";
    int filled = (int)(percentage * length / 100);

    for (int i = 0; i < length; i++) {
        if (i < filled) {
            if (percentage >= 80) bar += BAR_FULL;
            else if (percentage >= 60) bar += BAR_MID;
            else if (percentage >= 40) bar += BAR_LOW;
            else bar += BAR_LOW;
        }
        else {
            bar += BAR_EMPTY;
        }
    }
    bar += "] " + to_string((int)percentage) + "%";
    return bar;
}

string SocialNetwork::createSmallProgressBar(float percentage, int length) {
    string bar = "[";
    int filled = (int)(percentage * length / 100);

    for (int i = 0; i < length; i++) {
        if (i < filled) bar += BAR_FULL;
        else bar += BAR_EMPTY;
    }
    bar += "]";
    return bar;
}

string SocialNetwork::centerText(const string& text, int width) {
    if (text.length() >= width) return text;

    int leftPadding = (width - text.length()) / 2;
    int rightPadding = width - text.length() - leftPadding;

    return string(leftPadding, ' ') + text + string(rightPadding, ' ');
}

// =============== 美化显示函数实现 ===============

// 美化显示亲密度查询
void SocialNetwork::displayBottleneckBeautiful() {
#ifdef _WIN32
    _setmode(_fileno(stdin), _O_U16TEXT);
    _setmode(_fileno(stdout), _O_U16TEXT);

    wstring wname1, wname2;
    wcout << L"\n  请输入第一个人姓名: ";
    getline(wcin, wname1);
    wcout << L"  请输入第二个人姓名: ";
    getline(wcin, wname2);

    string startName = wideToUtf8(wname1);
    string endName = wideToUtf8(wname2);
    ltrim(startName); rtrim(startName);
    ltrim(endName); rtrim(endName);
    replaceFullWidthSpaces(startName);
    replaceFullWidthSpaces(endName);

    if (startName.empty() || endName.empty()) {
        wcout << L"未检测到有效姓名！\n";
        _setmode(_fileno(stdin), _O_TEXT);
        _setmode(_fileno(stdout), _O_TEXT);
        return;
    }
    _setmode(_fileno(stdin), _O_TEXT);
    _setmode(_fileno(stdout), _O_TEXT);
#else
    string startName, endName;
    cout << "\n  请输入第一个人姓名: ";
    getline(cin, startName);
    cout << "  请输入第二个人姓名: ";
    getline(cin, endName);
    ltrim(startName); rtrim(startName);
    ltrim(endName); rtrim(endName);
    replaceFullWidthSpaces(startName);
    replaceFullWidthSpaces(endName);
    if (startName.empty() || endName.empty()) {
        cout << "未检测到有效姓名！\n";
        return;
    }
#endif

    int start = findIndex(startName);
    int end = findIndex(endName);

    if (start == -1 || end == -1) {
        cout << ERROR_ICON << " 至少一个联系人不存在！\n";
        return;
    }

    if (start == end) {
        cout << INFO_ICON << " 这是同一个人！\n";
        return;
    }

    // 检查是否是直接好友以优化用户体验信息
    for (auto& e : adjList[start]) {
        if (e.getTo() == end) {
            cout << createSectionHeader("直接好友关系检测");
            cout << "\n" << SUCCESS_ICON << " " << startName << " 和 " << endName << " 是直接好友！\n";
            cout << "\n   亲密度: " << createProgressBar(e.getWeight(), 20);
            cout << "\n   关系强度: ";
            if (e.getWeight() >= 80) cout << "强关系 (亲密好友)";
            else if (e.getWeight() >= 60) cout << "中关系 (普通好友)";
            else if (e.getWeight() >= 40) cout << "弱关系 (认识的人)";
            else cout << "微弱关系 (刚认识)";
            cout << "\n" << LINE_THIN << "\n";
            return;
        }
    }

    // 如果不是直接好友，调用 getBottleneckPath 计算间接路径
    int bottleneckValue = getBottleneckPath(startName, endName);

    if (bottleneckValue <= 0) { // Covers -1 for no path
        cout << createSectionHeader("亲密度路径分析");
        cout << "\n" << ERROR_ICON << " " << startName << " 和 " << endName << " 之间没有可达路径！\n";
        cout << "\n   分析结果: 两人之间没有直接或间接的社交联系\n";
        cout << "   建议: 通过共同好友建立联系\n";
        cout << LINE_THIN << "\n";
        return;
    }

    cout << createSectionHeader("亲密度路径分析");
    cout << "\n" << INFO_ICON << " 查询: " << startName << " -> " << endName << "\n";
    cout << LINE_THIN << "\n";
    cout << "   最大亲密度下限: " << createProgressBar(bottleneckValue, 20) << "\n";

    string relationship;
    if (bottleneckValue >= 80) relationship = "强社交关系 (可以通过亲密朋友联系)";
    else if (bottleneckValue >= 60) relationship = "中等社交关系 (可以通过普通朋友联系)";
    else if (bottleneckValue >= 40) relationship = "弱社交关系 (可以通过熟人联系)";
    else relationship = "微弱社交关系 (联系较弱)";

    cout << "   关系强度: " << relationship << "\n";
    cout << "   说明: 这是所有可达路径中的最高亲密度下限\n";
    cout << "         表示两人联系的最紧密程度\n";
    cout << LINE_THIN << "\n";
}

void SocialNetwork::exportToHTML() {
#ifdef _WIN32
    _setmode(_fileno(stdin), _O_U16TEXT);
    _setmode(_fileno(stdout), _O_U16TEXT);
    wstring wfilename;
    wcout << L"\n  请输入HTML文件名 (例如: network.html): ";
    wcin >> wfilename;
    wcin.ignore(numeric_limits<streamsize>::max(), '\n');
    string filename = wideToUtf8(wfilename);
    _setmode(_fileno(stdin), _O_TEXT);
    _setmode(_fileno(stdout), _O_TEXT);
#else
    string filename;
    cout << "\n  请输入HTML文件名 (例如: network.html): ";
    cin >> filename;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
#endif

    // 检查是否以.html或.htm结尾（不区分大小写）
    string lowerFilename = filename;
    transform(lowerFilename.begin(), lowerFilename.end(), lowerFilename.begin(), ::tolower);

    if (lowerFilename.size() < 5 ||
        (lowerFilename.substr(lowerFilename.size() - 5) != ".html" &&
            lowerFilename.substr(lowerFilename.size() - 4) != ".htm")) {
        filename += ".html";
    }

    // 二进制写入+UTF8 BOM头 彻底解决中文乱码
    ofstream file(filename, ios::out | ios::binary);
    if (!file.is_open()) {
        cout << "错误：无法打开文件 " << filename << " 进行写入！" << endl;
        return;
    }
    const unsigned char utf8Bom[3] = { 0xEF, 0xBB, 0xBF };
    file.write((const char*)utf8Bom, sizeof(utf8Bom));

    // 根据节点数量计算动态布局参数
    int nodeCount = vertList.size();
    long long repulsion = (long long)nodeCount * 50 * (1 + (double)nodeCount / 200.0);
    int edgeLength = nodeCount * 3 + 100;

    file << R"(<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>SocialNetwork</title>
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body { font-family: Arial, sans-serif; padding: 20px; background: #f0f2f5; }
        .container { max-width: 1200px; margin: 0 auto; }
        .header { text-align: center; margin-bottom: 20px; padding: 20px; 
                 background: white; border-radius: 10px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); }
        #network { width: 100%; height: 800px; background: white; 
                  border-radius: 10px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); }
        @keyframes spin {
            0% { transform: rotate(0deg); }
            100% { transform: rotate(360deg); }
        }
    </style>
    <script>
        // 多CDN源列表（优先级从高到低）
        const echartsCdnList = [
            'https://unpkg.com/echarts@5.4.3/dist/echarts.min.js',
            'https://cdn.jsdelivr.net/npm/echarts@5.4.3/dist/echarts.min.js',
            'https://cdn.bootcdn.net/ajax/libs/echarts/5.4.3/echarts.min.js'
        ];


        // 递归加载ECharts CDN（带超时机制）
        function loadEcharts(cdnList) {
            if (cdnList.length === 0) {
                alert('所有ECharts CDN加载失败！请手动下载并引入：\nhttps://cdn.jsdelivr.net/npm/echarts@5.4.3/dist/echarts.min.js');
                return false;
            }
    
            const currentCdn = cdnList[0];
            const script = document.createElement('script');
            script.src = currentCdn;
            script.defer = true;

            let loaded = false;
            const timeoutId = setTimeout(() => {
                if (!loaded) {
                    console.warn(`ECharts CDN加载超时（1秒）：${currentCdn}，尝试下一个源`);
                    script.onload = script.onerror = null; // 清除事件处理器
                    document.head.removeChild(script); // 移除失败的script标签
                    loadEcharts(cdnList.slice(1)); // 尝试下一个源
                }
            }, 1000); // 1秒超时

            // CDN加载成功
            script.onload = function() {
                loaded = true;
                clearTimeout(timeoutId);
                console.log(`ECharts加载成功（来源：${currentCdn}）`);
                initECharts(); // 加载成功后初始化图表
            };

            // 当前CDN失败，尝试下一个
            script.onerror = function() {
                loaded = true;
                clearTimeout(timeoutId);
                console.warn(`ECharts CDN加载失败：${currentCdn}，尝试下一个源`);
                loadEcharts(cdnList.slice(1));
            };

            document.head.appendChild(script);
            return true;
        }

        // 原有初始化逻辑适配
        function initECharts() {
            if (typeof echarts === 'undefined') {
                let retryCount = 0;
                const retryTimer = setInterval(() => {
                    if (typeof echarts !== 'undefined' || retryCount >= 5) {
                        clearInterval(retryTimer);
                        if (typeof echarts !== 'undefined') {
                            doInit();
                        } else {
                            alert('ECharts加载超时，请检查网络！');
                        }
                    }
                    retryCount++;
                }, 100);
            } else {
                doInit();
            }
        }

        function doInit() {
        var myChart = echarts.init(document.getElementById('network'));

        var loadingElement = document.getElementById('loading');
        if (loadingElement) {
            loadingElement.style.display = 'none';
        }

        var nodesData = [)";

    // 遍历生成节点数据（保留原有逻辑）
    for (int i = 0; i < vertList.size(); i++) {
        file << (i > 0 ? "," : "") << "\n{"
            << "id: " << i
            << ", name: '" << vertList[i].getName() << "'" // 直接使用UTF-8名字
            << ", symbolSize: " << (adjList[i].size() * 3) + 12
            << "}";
    }

    file << R"(];
    
        var linksData = [)";

    // 遍历生成边数据（保留原有逻辑，绑定亲密度）
    set<pair<int, int>> addedEdges;
    bool firstEdge = true;
    for (int i = 0; i < vertList.size(); i++) {
        for (auto& edge : adjList[i]) {
            int j = edge.getTo();
            int weight = edge.getWeight();
            if (i < j && addedEdges.find({ i, j }) == addedEdges.end()) {
                file << (firstEdge ? "\n" : ",\n") << "{source: " << i << ", target: " << j
                    << ", sourceName: '" << vertList[i].getName() << "'" // 直接使用UTF-8名字
                    << ", targetName: '" << vertList[j].getName() << "'" // 直接使用UTF-8名字
                    << ", weight: " << weight  // 把亲密度存入边的自定义属性
                    << ", lineStyle: {width: " << weight / 20.0 + 0.5 << "}}";
                addedEdges.insert({ i, j });
                firstEdge = false;
            }
        }
    }
    
    // 保留原有tooltip差异化显示逻辑
    file << R"(];

        var option = {
            tooltip: {
                trigger: 'item',
                triggerOn: 'mousemove', 
                formatter: function(params) {
                    if (params.dataType === 'node') {
                        const friendCount = params.data.symbolSize;
                        return `<div style="padding:5px;">
                                  <b style="color:#409EFF;">User Information</b><br/>
                                  Username: ${params.data.name}<br/>
                                  Friends: ${(friendCount - 12) / 3} 
                                </div>`;
                    }
                    else if (params.dataType === 'edge') {
                        const fromName = nodesData[params.data.source].name;
                        const toName = nodesData[params.data.target].name;
                        const intimacy = params.data.weight;
                        return `<div style="padding:5px;">
                                  <b style="color:#E6A23C;">Friendship</b><br/>
                                  ${fromName} <-> ${toName}<br/>
                                  Intimacy Weight: ${intimacy}
                                </div>`;
                    }
                }
            },
            series: [{
                type: 'graph',
                layout: 'force',
                data: nodesData,
                links: linksData,
                roam: true, 
                label: {
                    show: true,
                    position: 'right',
                    fontSize: 12,
                    color: '#333'
                },
                force: {
                    repulsion: )" << repulsion << R"(,
                    gravity: 0.05,
                    edgeLength: )" << edgeLength << R"(,
                    layoutAnimation: false
                },
                lineStyle: {
                    color: 'source',
                    curveness: 0,
                    opacity: 0.7
                },
                emphasis: {
                    focus: 'adjacency',
                    lineStyle: { width: 5 }
                },
                blur: {
                    itemStyle: {
                        opacity: 0.1
                    },
                    lineStyle: {
                        opacity: 0.1
                    }
                }
            }]
        };

        myChart.setOption(option);
        window.addEventListener('resize', function() {
            myChart.resize();
        });
        }

        // 页面加载完成后启动多CDN加载流程
        window.addEventListener('DOMContentLoaded', function() {
            loadEcharts(echartsCdnList);
        });
    </script>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>社交网络关系图</h1>
            <p>People: )" << vertList.size() << R"( | Time: )" << __DATE__ << R"(</p>
        </div>
        <div id="network">
                <div id="loading" style="display: flex; flex-direction: column; justify-content: center; align-items: center; height: 100%;">
                    <div style="font-size: 24px; color: #409EFF; margin-bottom: 20px;">正在加载中...</div>
                    <div style="width: 50px; height: 50px; border: 5px solid #f3f3f3; border-top: 5px solid #409EFF; border-radius: 50%; animation: spin 1s linear infinite;"></div>
                    <div style="margin-top: 20px; color: #666; font-size: 14px;">加载图表数据，请稍候...</div>
            </div>
        </div>
    </div>
</body>
</html>)";

    file.close();
    cout << "HTML文件已生成: " << filename << endl;
    // 自动打开HTML文件
#ifdef _WIN32
    string command = "start \"\" \"" + filename + "\"";
#elif __APPLE__
    string command = "open \"" + filename + "\"";
#else
    string command = "xdg-open \"" + filename + "\"";
#endif
    system(command.c_str());
}

void SocialNetwork::testPerformance() {
    cout << "=== 哈希映射性能测试 ===\n";

    // 测试查找性能
    int testCount = 10000;
    auto start = chrono::high_resolution_clock::now();

    for (int i = 0; i < testCount; i++) {
        // 随机查找测试
        findIndex("测试用户");
    }

    auto end = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::microseconds>(end - start);

    cout << testCount << " 次查找耗时: "
        << duration.count() << " 微秒\n";
    cout << "平均每次查找: "
        << duration.count() / (double)testCount << " 微秒\n";
}
// 在 SocialNetwork.cpp 中的 testPerformance() 函数之后添加

void SocialNetwork::testBottleneckAlgorithm() {
    cout << "=== 瓶颈路径算法性能测试 ===\n";

    // 测试1: 基本功能测试
    cout << "\n1. 基本功能测试:\n";
    cout << LINE_THIN << endl;

    // 创建测试图
    // 图结构:
    // A(0) --5-- B(1) --1-- C(2)
    //   \                /
    //    \--3-- D(3) --2--/

    // 清空现有数据
    vertList.clear();
    adjList.clear();
    nameToIndex.clear();

    // 添加测试人员
    vector<string> testNames;
    testNames.push_back("A");
    testNames.push_back("B");
    testNames.push_back("C");
    testNames.push_back("D");
    testNames.push_back("E");
    testNames.push_back("F");

    for (size_t i = 0; i < testNames.size(); i++) {
        Person p;
        p.setName(testNames[i]);
        vertList.push_back(p);
        adjList.push_back(list<Edge>());
        nameToIndex[testNames[i]] = static_cast<int>(vertList.size()) - 1;
    }

    // 添加测试关系
    // A-B:5, B-C:1, A-D:3, D-C:2
    vector<pair<pair<int, int>, int>> testEdges;
    testEdges.push_back(make_pair(make_pair(0, 1), 5));  // A-B
    testEdges.push_back(make_pair(make_pair(1, 2), 1));  // B-C
    testEdges.push_back(make_pair(make_pair(0, 3), 3));  // A-D
    testEdges.push_back(make_pair(make_pair(3, 2), 2));  // D-C

    for (size_t k = 0; k < testEdges.size(); k++) {
        int i = testEdges[k].first.first;
        int j = testEdges[k].first.second;
        int w = testEdges[k].second;

        Edge e1, e2;
        e1.setTo(j); e1.setWeight(w);
        e2.setTo(i); e2.setWeight(w);
        adjList[i].push_back(e1);
        adjList[j].push_back(e2);
    }

    // 测试用例
    vector<pair<pair<string, string>, pair<int, string>>> testCases;
    testCases.push_back(make_pair(make_pair("A", "C"), make_pair(2, "路径 A-D-C 的瓶颈值 = min(3,2)=2")));
    testCases.push_back(make_pair(make_pair("A", "B"), make_pair(5, "直接连接 A-B")));
    testCases.push_back(make_pair(make_pair("B", "D"), make_pair(3, "路径 B-A-D 的瓶颈值 = min(5,3)=3")));
    testCases.push_back(make_pair(make_pair("A", "E"), make_pair(-1, "E 不可达")));
    testCases.push_back(make_pair(make_pair("A", "A"), make_pair(0, "同一人")));

    int passed = 0;
    for (size_t i = 0; i < testCases.size(); i++) {
        string start = testCases[i].first.first;
        string end = testCases[i].first.second;
        int expected = testCases[i].second.first;
        string description = testCases[i].second.second;

        int result = getBottleneckPath(start, end);
        bool success = (result == expected);

        cout << "测试 " << start << " -> " << end << ": ";
        if (success) {
            cout << SUCCESS_ICON << " 通过 (结果: " << result << ", 预期: " << expected << ")\n";
            passed++;
        }
        else {
            cout << ERROR_ICON << " 失败 (结果: " << result << ", 预期: " << expected << ")\n";
        }
        cout << "   说明: " << description << "\n" << LINE_DASH << "\n";
    }

    cout << "\n基本功能测试: " << passed << "/" << testCases.size() << " 通过\n";

    // 测试2: 性能测试
    cout << "\n2. 性能测试:\n";
    cout << LINE_THIN << endl;

    // 创建大规模测试图
    int N = 1000;  // 节点数
    cout << "创建 " << N << " 个节点的测试图...\n";

    // 清空数据
    vertList.clear();
    adjList.clear();
    nameToIndex.clear();

    // 添加节点
    for (int i = 0; i < N; i++) {
        Person p;
        p.setName("Person_" + to_string(i));
        vertList.push_back(p);
        adjList.push_back(list<Edge>());
        nameToIndex["Person_" + to_string(i)] = i;
    }

    // 添加随机边（每个节点约3-5个连接）
    srand(static_cast<unsigned int>(time(0)));
    int totalEdges = 0;
    for (int i = 0; i < N; i++) {
        int edgeCount = 3 + rand() % 3;
        for (int j = 0; j < edgeCount; j++) {
            int target = rand() % N;
            if (target == i) continue;

            int weight = 10 + rand() % 91;  // 10-100

            // 检查边是否已存在
            bool exists = false;
            for (auto& edge : adjList[i]) {
                if (edge.getTo() == target) {
                    exists = true;
                    break;
                }
            }

            if (!exists) {
                Edge e1, e2;
                e1.setTo(target); e1.setWeight(weight);
                e2.setTo(i); e2.setWeight(weight);
                adjList[i].push_back(e1);
                adjList[target].push_back(e2);
                totalEdges++;
            }
        }
    }

    cout << "创建完成: " << N << " 节点, " << totalEdges << " 条边\n";

    // 性能测试
    int testPairs = 100;
    cout << "测试 " << testPairs << " 对随机节点...\n";

    auto startTime = chrono::high_resolution_clock::now();

    for (int i = 0; i < testPairs; i++) {
        int startIdx = rand() % N;
        int endIdx = rand() % N;

        string startName = "Person_" + to_string(startIdx);
        string endName = "Person_" + to_string(endIdx);

        getBottleneckPath(startName, endName);
    }

    auto endTime = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(endTime - startTime);

    cout << "完成 " << testPairs << " 次查询，耗时: "
        << duration.count() << " 毫秒\n";
    cout << "平均每次查询: "
        << duration.count() / static_cast<double>(testPairs) << " 毫秒\n";

    // 测试3: 算法正确性验证
    cout << "\n3. 算法正确性验证:\n";
    cout << LINE_THIN << endl;

    // 验证几个特定路径
    cout << "验证算法正确性:\n";

    // 创建一个简单链状图验证
    vertList.clear();
    adjList.clear();
    nameToIndex.clear();

    // 创建链: 0--10--1--20--2--30--3
    for (int i = 0; i < 4; i++) {
        Person p;
        p.setName("Node" + to_string(i));
        vertList.push_back(p);
        adjList.push_back(list<Edge>());
        nameToIndex["Node" + to_string(i)] = i;
    }

    // 添加链状连接
    vector<int> weights;
    weights.push_back(10);
    weights.push_back(20);
    weights.push_back(30);

    for (int i = 0; i < 3; i++) {
        Edge e1, e2;
        e1.setTo(i + 1); e1.setWeight(weights[i]);
        e2.setTo(i); e2.setWeight(weights[i]);
        adjList[i].push_back(e1);
        adjList[i + 1].push_back(e2);
    }

    // 验证链状图的瓶颈值
    cout << "链状图测试 (0--10--1--20--2--30--3):\n";
    cout << "  0->1: 预期=10, 实际=" << getBottleneckPath("Node0", "Node1")
        << " " << (getBottleneckPath("Node0", "Node1") == 10 ? SUCCESS_ICON : ERROR_ICON) << "\n";
    cout << "  0->2: 预期=10, 实际=" << getBottleneckPath("Node0", "Node2")
        << " " << (getBottleneckPath("Node0", "Node2") == 10 ? SUCCESS_ICON : ERROR_ICON) << "\n";
    cout << "  0->3: 预期=10, 实际=" << getBottleneckPath("Node0", "Node3")
        << " " << (getBottleneckPath("Node0", "Node3") == 10 ? SUCCESS_ICON : ERROR_ICON) << "\n";
    cout << "  1->3: 预期=20, 实际=" << getBottleneckPath("Node1", "Node3")
        << " " << (getBottleneckPath("Node1", "Node3") == 20 ? SUCCESS_ICON : ERROR_ICON) << "\n";

    cout << "\n=== 测试完成 ===\n";
}