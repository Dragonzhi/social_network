#include "SocialNetwork.h"
#include <queue>
#include <algorithm>
#include <sstream>
using namespace std;

SocialNetwork::SocialNetwork()
{
    // 构造函数，初始化数据结构
}

SocialNetwork::~SocialNetwork()
{
    // 析构函数，清理资源
}

// 查找人员索引
int SocialNetwork::findIndex(string name) {
    if (nameToIndex.find(name) == nameToIndex.end()) return -1;
    return nameToIndex[name];
}

// 添加联系人
void SocialNetwork::addPerson(string name) {
    if (findIndex(name) != -1) {
        cout << "联系人 " << name << " 已存在！\n";
        return;
    }

    Person p;
    p.setName(name);
    vertList.push_back(p);
    adjList.push_back(list<Edge>()); // 添加空的邻接表
    nameToIndex[name] = vertList.size() - 1;

    cout << "联系人 " << name << " 添加成功！\n";
}

// 删除联系人
void SocialNetwork::deletePerson(string name) {
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
}

// 添加关系
void SocialNetwork::addEdge(string name1, string name2, int weight) {
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
}

// 删除关系
void SocialNetwork::deleteEdge(string name1, string name2) {
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
}

// 显示所有联系人关系
void SocialNetwork::displayAll() {
    if (vertList.empty()) {
        cout << "社交网络为空！\n";
        return;
    }

    cout << "\n======= 所有联系人关系 =======\n";
    for (int i = 0; i < vertList.size(); i++) {
        cout << vertList[i].getName() << " 的好友: ";

        if (adjList[i].empty()) {
            cout << "暂无好友";
        }
        else {
            auto& friends = adjList[i];
            for (auto& edge : friends) {
                int to = edge.getTo();
                cout << vertList[to].getName() << "(" << edge.getWeight() << ") ";
            }
        }
        cout << endl;
    }
    cout << "=============================\n";
}

// 按亲密程度排序好友
void SocialNetwork::sortFriends(string name, bool ascending) {
    int index = findIndex(name);
    if (index == -1) {
        cout << "联系人 " << name << " 不存在！\n";
        return;
    }

    auto& friends = adjList[index];
    if (friends.empty()) {
        cout << name << " 暂无好友\n";
        return;
    }

    // 将好友列表转换为vector以便排序
    vector<pair<int, int>> friendWeights; // pair<朋友索引, 亲密度>
    for (auto& edge : friends) {
        friendWeights.push_back({ edge.getTo(), edge.getWeight() });
    }

    // 排序
    if (ascending) {
        sort(friendWeights.begin(), friendWeights.end(),
            [](const pair<int, int>& a, const pair<int, int>& b) {
                return a.second < b.second;
            });
    }
    else {
        sort(friendWeights.begin(), friendWeights.end(),
            [](const pair<int, int>& a, const pair<int, int>& b) {
                return a.second > b.second;
            });
    }

    // 显示结果
    cout << name << " 的好友按亲密度" << (ascending ? "升序" : "降序") << "排序:\n";
    for (auto& fw : friendWeights) {
        cout << vertList[fw.first].getName() << " - 亲密度: " << fw.second << endl;
    }
}

// 计算瓶颈路径（最大最小值路径）
int SocialNetwork::getBottleneckPath(string startName, string endName) {
    int start = findIndex(startName);
    int end = findIndex(endName);

    if (start == -1 || end == -1) {
        return -1;
    }

    if (start == end) {
        return 0;
    }

    // 使用Dijkstra变种算法，求最大最小值路径
    vector<int> maxMin(vertList.size(), -1); // 存储到每个节点的最大最小值
    vector<bool> visited(vertList.size(), false);

    maxMin[start] = INT_MAX; // 起始节点设置为最大值

    // 使用优先队列，按最大最小值降序排列
    priority_queue<pair<int, int>> pq; // pair<最大最小值, 节点索引>
    pq.push({ INT_MAX, start });

    while (!pq.empty()) {
        auto current = pq.top();
        pq.pop();
        int u = current.second;

        if (visited[u]) continue;
        visited[u] = true;

        // 如果到达目标节点
        if (u == end) {
            return maxMin[u];
        }

        // 遍历所有邻居
        for (auto& edge : adjList[u]) {
            int v = edge.getTo();
            int weight = edge.getWeight();

            // 计算到v的最大最小值
            int newMin = min(maxMin[u], weight);

            if (newMin > maxMin[v]) {
                maxMin[v] = newMin;
                pq.push({ newMin, v });
            }
        }
    }

    return -1; // 没有路径
}

// 显示社交大牛（Top10）
void SocialNetwork::displayTop10() {
    if (vertList.empty()) {
        cout << "社交网络为空！\n";
        return;
    }

    // 收集每个人的好友数量
    vector<pair<string, int>> friendCounts;
    for (int i = 0; i < vertList.size(); i++) {
        string name = vertList[i].getName();
        int count = adjList[i].size();
        friendCounts.push_back({ name, count });
    }

    // 按好友数量降序排序
    sort(friendCounts.begin(), friendCounts.end(),
        [](const pair<string, int>& a, const pair<string, int>& b) {
            return a.second > b.second;
        });

    // 显示Top10
    cout << "\n======= 社交大牛Top10 =======\n";
    int limit = min(10, (int)friendCounts.size());
    for (int i = 0; i < limit; i++) {
        cout << i + 1 << ". " << friendCounts[i].first
            << " - 好友数量: " << friendCounts[i].second << endl;
    }
    cout << "============================\n";
}

// 保存到文件
void SocialNetwork::saveToFile(string filename) {
    ofstream file(filename);
    if (!file.is_open()) {
        cout << "无法打开文件: " << filename << endl;
        return;
    }

    // 保存所有人员
    file << vertList.size() << endl;
    for (int i = 0; i < vertList.size(); i++) {
        file << vertList[i].getName() << endl;
    }

    // 保存所有关系
    int edgeCount = 0;
    for (int i = 0; i < adjList.size(); i++) {
        edgeCount += adjList[i].size();
    }
    edgeCount /= 2; // 因为是无向图

    file << edgeCount << endl;

    // 只保存i < j的关系，避免重复
    for (int i = 0; i < adjList.size(); i++) {
        for (auto& edge : adjList[i]) {
            int j = edge.getTo();
            if (i < j) { // 只保存一次
                file << vertList[i].getName() << " "
                    << vertList[j].getName() << " "
                    << edge.getWeight() << endl;
            }
        }
    }

    file.close();
    cout << "数据已保存到文件: " << filename << endl;
}

// 从文件加载
void SocialNetwork::loadFromFile(string filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cout << "无法打开文件: " << filename << endl;
        return;
    }

    // 清空现有数据
    vertList.clear();
    adjList.clear();
    nameToIndex.clear();

    int personCount;
    file >> personCount;
    file.ignore(); // 忽略换行符

    // 读取所有人员
    for (int i = 0; i < personCount; i++) {
        string name;
        getline(file, name);
        addPerson(name);
    }

    int edgeCount;
    file >> edgeCount;

    // 读取所有关系
    for (int i = 0; i < edgeCount; i++) {
        string name1, name2;
        int weight;
        file >> name1 >> name2 >> weight;
        addEdge(name1, name2, weight);
    }

    file.close();
    cout << "已从文件加载数据: " << filename << endl;
}