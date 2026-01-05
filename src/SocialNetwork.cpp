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

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#endif
using json = nlohmann::json;

// 在类中添加一个辅助函数
static std::wstring utf8ToWide(const std::string& utf8) {
#ifdef _WIN32
    int wlen = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), (int)utf8.size(), NULL, 0);
    if (wlen <= 0) return L"";
    std::wstring ws(wlen, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), (int)utf8.size(), &ws[0], wlen);
    return ws;
#else
    // 在非Windows平台上，简单转换（假设系统使用UTF-8）
    return std::wstring(utf8.begin(), utf8.end());
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
static void replaceFullWidthSpaces(std::string& s) {
    const std::string fullWidth = "\xE3\x80\x80";
    size_t pos = 0;
    while ((pos = s.find(fullWidth, pos)) != std::string::npos) {
        s.replace(pos, fullWidth.size(), " ");
        pos += 1;
    }
}

static void ltrim(std::string& s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) { return !std::isspace(ch); }));
}
static void rtrim(std::string& s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), s.end());
}

#ifdef _WIN32
// 将 UTF-16 wide string 转成 UTF-8 std::string
static std::string wideToUtf8(const std::wstring& ws) {
    if (ws.empty()) return {};
    int len = WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), (int)ws.size(), NULL, 0, NULL, NULL);
    if (len <= 0) return {};
    std::string out(len, '\0');
    WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), (int)ws.size(), &out[0], len, NULL, NULL);
    return out;
}
#endif

// 调试：打印字节十六进制
static void debug_print_hex(const std::string& s, const char* label = nullptr) {
#ifdef DEBUG
    if (label) std::cout << label;
    std::cout << "DEBUG: raw bytes (len=" << s.size() << "): ";
    std::cout << std::hex << std::setfill('0');
    for (unsigned char c : s) {
        std::cout << std::setw(2) << (int)c << " ";
    }
    std::cout << std::dec << "\n";
#endif
}

void SocialNetwork::addPersons() {
#ifdef _WIN32
    // 切换 stdin/stdout 到 UTF-16 模式，使得 std::wcin/std::wcout 能正确读取/写入控制台（Windows 控制台）
    // 注意：若输入被重定向（例如文件或管道），_setmode 可能不适用。通常对交互式控制台有效。
    _setmode(_fileno(stdin), _O_U16TEXT);
    _setmode(_fileno(stdout), _O_U16TEXT);

    std::wstring winput;
    std::wcout << L"请输入要添加的联系人名称，多个名称用空格分隔：\n";
    std::getline(std::wcin, winput);

    if (winput.empty()) {
        std::wcout << L"输入不能为空！\n";
        // 可将模式恢复为默认（可选）
        _setmode(_fileno(stdin), _O_TEXT);
        _setmode(_fileno(stdout), _O_TEXT);
        return;
    }

    // 将宽字符串转换为 UTF-8 std::string，后续逻辑以 UTF-8 字符串处理
    std::string input = wideToUtf8(winput);

    // 可选：恢复到窄模式（如果你的程序中后续大量使用 std::cout，建议恢复）
    _setmode(_fileno(stdin), _O_TEXT);
    _setmode(_fileno(stdout), _O_TEXT);
#else
    std::string input;
    std::cout << "请输入要添加的联系人名称，多个名称用空格分隔：\n";
    std::getline(std::cin, input);
    if (input.empty()) {
        std::cout << "输入不能为空！\n";
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
        std::cout << "未检测到有效姓名！\n";
        return;
    }

    // 按 ASCII 空白分割
    std::stringstream ss(input);
    std::string name;
    std::vector<std::string> names;
    int addedCount = 0;
    int existCount = 0;

    while (ss >> name) {
        if (name.empty()) continue;
        names.push_back(name);
    }

    if (names.empty()) {
        std::cout << "未检测到有效姓名！\n";
        return;
    }

    std::cout << "\n正在添加以下联系人：\n";
    for (const auto& n : names) {
        std::cout << n << " ";
    }
    std::cout << "\n\n";

    for (const auto& n : names) {
        if (n.empty()) continue;

        if (findIndex(n) != -1) {
            std::cout << "联系人 " << n << " 已存在，跳过添加。\n";
            existCount++;
            continue;
        }

        Person p;
        p.setName(n); // setName 接受 UTF-8 std::string
        vertList.push_back(p);
        adjList.push_back(std::list<Edge>());
        nameToIndex[n] = static_cast<int>(vertList.size()) - 1;
        addedCount++;
        std::cout << "联系人 " << n << " 添加成功！\n";
    }

    std::cout << "\n批量添加完成：\n";
    std::cout << "成功添加 " << addedCount << " 个联系人。\n";
    if (existCount > 0) {
        std::cout << "跳过 " << existCount << " 个已存在的联系人。\n";
    }
}
// 删除联系人
void SocialNetwork::deletePerson() {  // 注意：移除了参数，改为从控制台读取
#ifdef _WIN32
    // 切换 stdin/stdout 到 UTF-16 模式
    _setmode(_fileno(stdin), _O_U16TEXT);
    _setmode(_fileno(stdout), _O_U16TEXT);

    std::wstring winput;
    std::wcout << L"请输入要删除的联系人姓名: ";
    std::getline(std::wcin, winput);

    if (winput.empty()) {
        std::wcout << L"输入不能为空！\n";
        _setmode(_fileno(stdin), _O_TEXT);
        _setmode(_fileno(stdout), _O_TEXT);
        return;
    }

    // 将宽字符串转换为 UTF-8 std::string
    std::string name = wideToUtf8(winput);

    // 归一化：把全角空格替换为普通空格
    replaceFullWidthSpaces(name);

    // 去掉前后空白
    ltrim(name);
    rtrim(name);

    if (name.empty()) {
        std::wcout << L"未检测到有效姓名！\n";
        _setmode(_fileno(stdin), _O_TEXT);
        _setmode(_fileno(stdout), _O_TEXT);
        return;
    }

    int index = findIndex(name);
    if (index == -1) {
        std::wstring wname = utf8ToWide(name);
        std::wcout << L"联系人 " << wname << L" 不存在！\n";
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

    std::wstring wname = utf8ToWide(name);
    std::wcout << L"联系人 " << wname << L" 删除成功！\n";
    _setmode(_fileno(stdin), _O_TEXT);
    _setmode(_fileno(stdout), _O_TEXT);
#else
    std::string name;
    std::cout << "请输入要删除的联系人姓名: ";
    std::getline(std::cin, name);

    if (name.empty()) {
        std::cout << "输入不能为空！\n";
        return;
    }

    // 归一化：把全角空格替换为普通空格
    replaceFullWidthSpaces(name);

    // 去掉前后空白
    ltrim(name);
    rtrim(name);

    if (name.empty()) {
        std::cout << "未检测到有效姓名！\n";
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

    std::wstring wname1, wname2;
    int weight;

    std::wcout << L"请输入第一个人名: ";
    std::getline(std::wcin, wname1);
    std::wcout << L"请输入第二个人名: ";
    std::getline(std::wcin, wname2);
    std::wcout << L"请输入亲密度 (整数): ";
    std::wcin >> weight;
    std::wcin.ignore(); // 清除换行符

    if (wname1.empty() || wname2.empty()) {
        std::wcout << L"输入不能为空！\n";
        _setmode(_fileno(stdin), _O_TEXT);
        _setmode(_fileno(stdout), _O_TEXT);
        return;
    }

    // 将宽字符串转换为 UTF-8 std::string
    std::string name1 = wideToUtf8(wname1);
    std::string name2 = wideToUtf8(wname2);

    // 归一化：把全角空格替换为普通空格
    replaceFullWidthSpaces(name1);
    replaceFullWidthSpaces(name2);

    // 去掉前后空白
    ltrim(name1); rtrim(name1);
    ltrim(name2); rtrim(name2);

    if (name1.empty() || name2.empty()) {
        std::wcout << L"未检测到有效姓名！\n";
        _setmode(_fileno(stdin), _O_TEXT);
        _setmode(_fileno(stdout), _O_TEXT);
        return;
    }

    int index1 = findIndex(name1);
    int index2 = findIndex(name2);

    if (index1 == -1 || index2 == -1) {
        std::wcout << L"输入的人员不存在！\n";
        _setmode(_fileno(stdin), _O_TEXT);
        _setmode(_fileno(stdout), _O_TEXT);
        return;
    }

    if (index1 == index2) {
        std::wcout << L"不能与自己建立关系！\n";
        _setmode(_fileno(stdin), _O_TEXT);
        _setmode(_fileno(stdout), _O_TEXT);
        return;
    }

    // 检查关系是否已存在
    auto& friends1 = adjList[index1];
    for (auto& edge : friends1) {
        if (edge.getTo() == index2) {
            std::wcout << wname1 << L" 和 " << wname2 << L" 的关系已存在，亲密度为: "
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

    std::wcout << L"已建立 " << wname1 << L" 和 " << wname2 << L" 的关系，亲密度: " << weight << L"\n";
    _setmode(_fileno(stdin), _O_TEXT);
    _setmode(_fileno(stdout), _O_TEXT);
#else
    std::string name1, name2;
    int weight;

    std::cout << "请输入第一个人名: ";
    std::getline(std::cin, name1);
    std::cout << "请输入第二个人名: ";
    std::getline(std::cin, name2);
    std::cout << "请输入亲密度 (整数): ";
    std::cin >> weight;
    std::cin.ignore(); // 清除换行符

    if (name1.empty() || name2.empty()) {
        std::cout << "输入不能为空！\n";
        return;
    }

    // 归一化：把全角空格替换为普通空格
    replaceFullWidthSpaces(name1);
    replaceFullWidthSpaces(name2);

    // 去掉前后空白
    ltrim(name1); rtrim(name1);
    ltrim(name2); rtrim(name2);

    if (name1.empty() || name2.empty()) {
        std::cout << "未检测到有效姓名！\n";
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

    std::wstring wname1, wname2;

    std::wcout << L"请输入第一个人名: ";
    std::getline(std::wcin, wname1);
    std::wcout << L"请输入第二个人名: ";
    std::getline(std::wcin, wname2);

    if (wname1.empty() || wname2.empty()) {
        std::wcout << L"输入不能为空！\n";
        _setmode(_fileno(stdin), _O_TEXT);
        _setmode(_fileno(stdout), _O_TEXT);
        return;
    }

    // 将宽字符串转换为 UTF-8 std::string
    std::string name1 = wideToUtf8(wname1);
    std::string name2 = wideToUtf8(wname2);

    // 归一化：把全角空格替换为普通空格
    replaceFullWidthSpaces(name1);
    replaceFullWidthSpaces(name2);

    // 去掉前后空白
    ltrim(name1); rtrim(name1);
    ltrim(name2); rtrim(name2);

    if (name1.empty() || name2.empty()) {
        std::wcout << L"未检测到有效姓名！\n";
        _setmode(_fileno(stdin), _O_TEXT);
        _setmode(_fileno(stdout), _O_TEXT);
        return;
    }

    int index1 = findIndex(name1);
    int index2 = findIndex(name2);

    if (index1 == -1 || index2 == -1) {
        std::wcout << L"输入的人员不存在！\n";
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
        std::wcout << L"已删除 " << wname1 << L" 和 " << wname2 << L" 的关系\n";
    }
    else {
        std::wcout << wname1 << L" 和 " << wname2 << L" 之间没有关系\n";
    }
    _setmode(_fileno(stdin), _O_TEXT);
    _setmode(_fileno(stdout), _O_TEXT);
#else
    std::string name1, name2;

    std::cout << "请输入第一个人名: ";
    std::getline(std::cin, name1);
    std::cout << "请输入第二个人名: ";
    std::getline(std::cin, name2);

    if (name1.empty() || name2.empty()) {
        std::cout << "输入不能为空！\n";
        return;
    }

    // 归一化：把全角空格替换为普通空格
    replaceFullWidthSpaces(name1);
    replaceFullWidthSpaces(name2);

    // 去掉前后空白
    ltrim(name1); rtrim(name1);
    ltrim(name2); rtrim(name2);

    if (name1.empty() || name2.empty()) {
        std::cout << "未检测到有效姓名！\n";
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
    std::wstring wfilename;
    std::wcout << L"\n  请输入保存文件名 (例如: network.json): ";
    std::wcin >> wfilename;
    std::wcin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::string filename = wideToUtf8(wfilename);
    _setmode(_fileno(stdin), _O_TEXT);
    _setmode(_fileno(stdout), _O_TEXT);
#else
    std::string filename;
    std::cout << "\n  请输入保存文件名 (例如: network.json): ";
    std::cin >> filename;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
#endif
    if (filename.empty()) {
        cout << "文件名不能为空！\n";
        return;
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
        std::ofstream file(filename, std::ios::out | std::ios::binary); 
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
    catch (const std::exception& e) {
        cout << "保存文件时发生错误: " << e.what() << endl;
    }
}

void SocialNetwork::loadFromFile() {
#ifdef _WIN32
    _setmode(_fileno(stdin), _O_U16TEXT);
    _setmode(_fileno(stdout), _O_U16TEXT);
    std::wstring wfilename;
    std::wcout << L"\n  请输入加载文件名 (例如: network.json): ";
    std::wcin >> wfilename;
    std::wcin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::string filename = wideToUtf8(wfilename);
    _setmode(_fileno(stdin), _O_TEXT);
    _setmode(_fileno(stdout), _O_TEXT);
#else
    std::string filename;
    std::cout << "\n  请输入加载文件名 (例如: network.json): ";
    std::cin >> filename;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
#endif
    if (filename.empty()) {
        cout << "文件名不能为空！\n";
        return;
    }

    // 清空现有数据
    vertList.clear();
    adjList.clear();
    nameToIndex.clear();

    try {
        // 1. 打开文件并检查
        std::ifstream file(filename, std::ios::binary); // 以二进制模式读取
        if (!file.is_open()) {
            cout << "无法打开文件 " << filename << "！\n";
            return;
        }

        // 2. 检查文件是否为空
        file.seekg(0, std::ios::end);
        if (file.tellg() == 0) {
            cout << "文件为空！\n";
            file.close();
            return;
        }
        file.seekg(0, std::ios::beg);

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
    catch (const std::exception& e) {
        cout << "加载文件时发生未知错误: " << e.what() << endl;
        cout << "请确保文件存在且格式正确！\n";
    }
}

// 显示所有联系人关系
void SocialNetwork::displayAll() {
    displayAllBeautiful();
}

// 按亲密程度排序好友
void SocialNetwork::sortFriends() {
    displaySortFriendsBeautiful();
}
//路径亲密度下界最大值
int SocialNetwork::getBottleneckPath(string startName, string endName) {
    int start = findIndex(startName);
    int end = findIndex(endName);

    if (start == -1 || end == -1) return -1; // Person not found
    if (start == end) return 0; // Same person

    int personCount = vertList.size();
    vector<int> maxBottleNeck(personCount, 0);
    vector<bool> visited(personCount, false); // Keep track of visited nodes
    priority_queue<pair<int, int>> pq;

    maxBottleNeck[start] = INT_MAX; // Initialize start with "infinite" capacity
    pq.push({ INT_MAX, start });

    while (!pq.empty()) {
        auto current = pq.top();
        int u = current.second;
        pq.pop();

        if (visited[u]) continue; // If already visited, skip.
        visited[u] = true;

        if (u == end) break; // Found the best path to the end node.

        for (auto& e : adjList[u]) {
            int v = e.getTo();
            int weight = e.getWeight();

            if (!visited[v]) { // Only consider unvisited neighbors
                int newBottleneck = min(maxBottleNeck[u], weight);
                if (newBottleneck > maxBottleNeck[v]) {
                    maxBottleNeck[v] = newBottleneck;
                    pq.push({ newBottleneck, v });
                }
            }
        }
    }

    // If we finish the loop and maxBottleNeck[end] is still 0, there is no path.
    // Return -1 for "no path" (consistent with "person not found" error)
    return (maxBottleNeck[end] == 0) ? -1 : maxBottleNeck[end];
}
	
void SocialNetwork::displayTop10() {
    displayTop10Beautiful();
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

// 美化显示所有联系人关系
void SocialNetwork::displayAllBeautiful() {
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

// 美化显示排序好友
void SocialNetwork::displaySortFriendsBeautiful() {
#ifdef _WIN32
    _setmode(_fileno(stdin), _O_U16TEXT);
    _setmode(_fileno(stdout), _O_U16TEXT);

    std::wstring wname;
    bool ascending;

    std::wcout << L"\n  请输入要排序的联系人姓名: ";
    std::getline(std::wcin, wname);
    std::wcout << L"  排序顺序 (0=降序, 1=升序): ";
    std::wcin >> ascending;
    std::wcin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // 清除换行符

    std::string name = wideToUtf8(wname);
    ltrim(name); rtrim(name);
    replaceFullWidthSpaces(name);

    if (name.empty()) {
        std::wcout << L"未检测到有效姓名！\n";
        _setmode(_fileno(stdin), _O_TEXT);
        _setmode(_fileno(stdout), _O_TEXT);
        return;
    }

    _setmode(_fileno(stdin), _O_TEXT);
    _setmode(_fileno(stdout), _O_TEXT);
#else
    std::string name;
    bool ascending;
    std::cout << "\n  请输入要排序的联系人姓名: ";
    std::getline(std::cin, name);
    std::cout << "  排序顺序 (0=降序, 1=升序): ";
    std::cin >> ascending;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    ltrim(name); rtrim(name);
    replaceFullWidthSpaces(name);
    if (name.empty()) {
        std::cout << "未检测到有效姓名！\n";
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

// 美化显示社交大牛Top10
void SocialNetwork::displayTop10Beautiful() {
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

    // 统计信息
    //float avgFriends = 0;
    //for (auto& p : friendCounts) avgFriends += p.second;
    //avgFriends /= personCount;

    //cout << "\n" << STATS_ICON << " 网络统计:";
    //cout << "\n   总人数: " << personCount;
    //cout << "\n   人均好友数: " << fixed << setprecision(1) << avgFriends;
    //cout << "\n   社交达人标准: " << (personCount >= 10 ? friendCounts[0].second : 0) << "+ 好友";
    //cout << "\n   前" << displayCount << "名占总好友数: "
    //    << fixed << setprecision(1)
    //    << (friendCounts[0].second * displayCount * 100.0 / (avgFriends * personCount)) << "%";
    //cout << "\n" << LINE_THIN << "\n";
}

// 美化显示亲密度查询
void SocialNetwork::displayBottleneckBeautiful() {
#ifdef _WIN32
    _setmode(_fileno(stdin), _O_U16TEXT);
    _setmode(_fileno(stdout), _O_U16TEXT);

    std::wstring wname1, wname2;
    std::wcout << L"\n  请输入第一个人姓名: ";
    std::getline(std::wcin, wname1);
    std::wcout << L"  请输入第二个人姓名: ";
    std::getline(std::wcin, wname2);

    std::string startName = wideToUtf8(wname1);
    std::string endName = wideToUtf8(wname2);
    ltrim(startName); rtrim(startName);
    ltrim(endName); rtrim(endName);
    replaceFullWidthSpaces(startName);
    replaceFullWidthSpaces(endName);

    if (startName.empty() || endName.empty()) {
        std::wcout << L"未检测到有效姓名！\n";
        _setmode(_fileno(stdin), _O_TEXT);
        _setmode(_fileno(stdout), _O_TEXT);
        return;
    }
    _setmode(_fileno(stdin), _O_TEXT);
    _setmode(_fileno(stdout), _O_TEXT);
#else
    std::string startName, endName;
    std::cout << "\n  请输入第一个人姓名: ";
    std::getline(std::cin, startName);
    std::cout << "  请输入第二个人姓名: ";
    std::getline(std::cin, endName);
    ltrim(startName); rtrim(startName);
    ltrim(endName); rtrim(endName);
    replaceFullWidthSpaces(startName);
    replaceFullWidthSpaces(endName);
    if (startName.empty() || endName.empty()) {
        std::cout << "未检测到有效姓名！\n";
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

void SocialNetwork::displayGraphASCII() {
    if (vertList.empty()) {
        cout << "社交网络为空！\n";
        return;
    }

    cout << createSectionHeader("社交网络图");

    // 简单布局算法 - 将人员放置在一个圆形上
    const int RADIUS = 15;
    const int CENTER_X = 25;
    const int CENTER_Y = 10;
    const int CANVAS_WIDTH = 50;
    const int CANVAS_HEIGHT = 25;

    // 创建画布
    vector<vector<char>> canvas(CANVAS_HEIGHT, vector<char>(CANVAS_WIDTH, ' '));

    // 计算每个人的位置（圆形布局）
    vector<pair<int, int>> positions;
    int n = vertList.size();
    for (int i = 0; i < n; i++) {
        double angle = 2 * 3.14159 * i / n;
        int x = CENTER_X + RADIUS * cos(angle);
        int y = CENTER_Y + RADIUS * sin(angle);
        positions.push_back({ x, y });

        // 在画布上放置人员（用字母表示）
        if (y >= 0 && y < CANVAS_HEIGHT && x >= 0 && x < CANVAS_WIDTH) {
            canvas[y][x] = 'A' + (i % 26); // 用字母代表不同的人
        }
    }

    // 绘制关系线
    for (int i = 0; i < n; i++) {
        for (auto& edge : adjList[i]) {
            int j = edge.getTo();
            if (i < j) { // 避免重复绘制
                int x1 = positions[i].first;
                int y1 = positions[i].second;
                int x2 = positions[j].first;
                int y2 = positions[j].second;

                // 简单的Bresenham直线算法
                int dx = abs(x2 - x1);
                int dy = abs(y2 - y1);
                int sx = (x1 < x2) ? 1 : -1;
                int sy = (y1 < y2) ? 1 : -1;
                int err = dx - dy;

                while (true) {
                    if (y1 >= 0 && y1 < CANVAS_HEIGHT && x1 >= 0 && x1 < CANVAS_WIDTH) {
                        if (canvas[y1][x1] == ' ') {
                            // 根据亲密度使用不同字符
                            int weight = edge.getWeight();
                            if (weight >= 80) canvas[y1][x1] = '=';
                            else if (weight >= 60) canvas[y1][x1] = '-';
                            else if (weight >= 40) canvas[y1][x1] = '.';
                            else canvas[y1][x1] = '·';
                        }
                    }

                    if (x1 == x2 && y1 == y2) break;

                    int e2 = 2 * err;
                    if (e2 > -dy) {
                        err -= dy;
                        x1 += sx;
                    }
                    if (e2 < dx) {
                        err += dx;
                        y1 += sy;
                    }
                }
            }
        }
    }

    // 打印画布
    cout << "\n";
    for (int y = 0; y < CANVAS_HEIGHT; y++) {
        for (int x = 0; x < CANVAS_WIDTH; x++) {
            cout << canvas[y][x];
        }
        cout << "\n";
    }

    // 显示图例
    cout << "\n" << LINE_THIN << "\n";
    cout << "图例说明:\n";
    cout << "  A, B, C... : 联系人（字母代表）\n";
    cout << "  =========  : 高亲密度关系 (80-100)\n";
    cout << "  ---------  : 中亲密度关系 (60-79)\n";
    cout << "  ........   : 低亲密度关系 (40-59)\n";
    cout << "  ·········  : 微弱关系 (1-39)\n";

    // 显示人员对应关系
    cout << "\n人员对应关系:\n";
    for (int i = 0; i < min(n, 26); i++) {
        cout << "  " << char('A' + i) << " : " << vertList[i].getName();
        if ((i + 1) % 3 == 0) cout << "\n";
        else if (i < n - 1) cout << " | ";
    }
    if (n > 26) cout << "\n  (仅显示前26人)";
    cout << "\n" << LINE_THIN << "\n";
}

void SocialNetwork::exportToHTML() {
#ifdef _WIN32
    _setmode(_fileno(stdin), _O_U16TEXT);
    _setmode(_fileno(stdout), _O_U16TEXT);
    std::wstring wfilename;
    std::wcout << L"\n  请输入HTML文件名 (例如: network.html): ";
    std::wcin >> wfilename;
    std::wcin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::string filename = wideToUtf8(wfilename);
    _setmode(_fileno(stdin), _O_TEXT);
    _setmode(_fileno(stdout), _O_TEXT);
#else
    std::string filename;
    std::cout << "\n  请输入HTML文件名 (例如: network.html): ";
    std::cin >> filename;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
#endif

    // 二进制写入+UTF8 BOM头 彻底解决中文乱码
    ofstream file(filename, ios::out | ios::binary);
    if (!file.is_open()) {
        cout << "错误：无法打开文件 " << filename << " 进行写入！" << endl;
        return;
    }
    const unsigned char utf8Bom[3] = { 0xEF, 0xBB, 0xBF };
    file.write((const char*)utf8Bom, sizeof(utf8Bom));

    // HTML头部+样式+ECharts引入
    file << R"(<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>SocialNetwork</title>
    <script src="https://cdn.jsdelivr.net/npm/echarts@5.4.3/dist/echarts.min.js" defer id="echarts-cdn"></script>
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body { font-family: Arial, sans-serif; padding: 20px; background: #f0f2f5; }
        .container { max-width: 1200px; margin: 0 auto; }
        .header { text-align: center; margin-bottom: 20px; padding: 20px; 
                 background: white; border-radius: 10px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); }
        #network { width: 100%; height: 600px; background: white; 
                  border-radius: 10px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); }
    </style>
    <script>
        window.addEventListener('DOMContentLoaded', function() {
            if (typeof echarts === 'undefined') {
                console.warn('CDN版ECharts加载失败，切换到本地版本');
                const localScript = document.createElement('script');
                localScript.src = './js/echarts.min.js'; 
                localScript.onload = function() {
                    console.log('本地版ECharts加载成功');
                };
                localScript.onerror = function() {
                    alert('本地ECharts文件也加载失败，请检查文件路径！');
                };
                document.body.appendChild(localScript);
            }
        });
    </script>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>社交网络关系图</h1>
            <p>People: )" << vertList.size() << R"( | Time: )" << __DATE__ << R"(</p>
        </div>
        <div id="network"></div>
    </div>
    
    <script>
        function initECharts() {
            if (typeof echarts === 'undefined') {
                let retryCount = 0;
                const retryTimer = setInterval(() => {
                    if (typeof echarts !== 'undefined' || retryCount >= 5) {
                        clearInterval(retryTimer);
                        if (typeof echarts !== 'undefined') {
                            doInit();
                        } else {
                            alert('ECharts加载超时，请检查网络或本地文件！');
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
        var nodesData = [)";

    // 遍历生成节点数据
    for (int i = 0; i < vertList.size(); i++) {
        file << (i > 0 ? "," : "") << "\n{"
            << "id: " << i
            << ", name: '" << vertList[i].getName() << "'" // 直接使用UTF-8名字
            << ", symbolSize: " << adjList[i].size() * 5 + 15
            << "}";
    }

    file << R"(];
        
        var linksData = [)";

    // 遍历生成边数据，额外给边绑定【亲密度数值】用于悬浮显示
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
                    << ", weight: " << weight  // 关键：把亲密度存入边的自定义属性，用于悬浮展示
                    << ", lineStyle: {width: " << weight / 10.0 << "}}";
                addedEdges.insert({ i, j });
                firstEdge = false;
            }
        }
    }

    // ========== 核心修改：tooltip 差异化显示 节点/边 ==========
    file << R"(];

        var option = {
            // 提示框核心配置：节点和边 显示完全不同的内容+样式
            tooltip: {
                trigger: 'item',
                triggerOn: 'mousemove', // 鼠标悬浮即触发，无需点击
                formatter: function(params) {
                    // 1. 鼠标悬浮到【节点】上 - 显示用户个人信息
                    if (params.dataType === 'node') {
                        const friendCount = (params.data.symbolSize - 15)/5;
                        return `<div style="padding:5px;">
                                  <b style="color:#409EFF;">User Information</b><br/>
                                  Username: ${params.data.name}<br/>
                                  Friends: ${friendCount} 
                                </div>`;
                    }
                    // 2. 鼠标悬浮到【连线/边】上 - 显示好友关系信息
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
                roam: true, // 拖拽+缩放保留
                label: {
                    show: true,
                    position: 'right',
                    fontSize: 12,
                    color: '#333'
                },
                force: {
                    repulsion: 2500,
                    gravity: 0.1,
                    edgeLength: 250,
                    layoutAnimation: true
                },
                lineStyle: {
                    color: 'source',
                    curveness: 0.2,
                    opacity: 0.7
                },
                // 关键配置：必须打开这个，才能让 边 支持悬浮触发tooltip
                emphasis: {
                    focus: 'adjacency',
                    lineStyle: { width: 8 } // 鼠标悬浮到边上时，连线变粗高亮，超实用！
                }
            }]
        };

        myChart.setOption(option);
        window.addEventListener('resize', function() {
            myChart.resize();
        });
        }
        document.addEventListener('DOMContentLoaded', initECharts);
    </script>
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