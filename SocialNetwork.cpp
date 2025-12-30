#include "SocialNetwork.h"
#include <queue>
#include <fstream>
#include "json.hpp" 

using json = nlohmann::json;
SocialNetwork::SocialNetwork()
{
    // 构造函数，初始化数据结构
}

SocialNetwork::~SocialNetwork()
{
    // 析构函数，清理资源
}

// 添加联系人
void SocialNetwork::addPerson(string name) {
    if (name == "") {
        cout << "联系人名字不能为空！\n";
        return;
    }
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

void SocialNetwork::saveToFile(string filename) {
    if (filename.empty()) {
        cout << "文件名不能为空！\n";
        return;
    }

    // 创建 JSON 对象
    json data;

    // 1. 保存人员信息
    json persons = json::array();
    for (const auto& person : vertList) {
        persons.push_back({
            {"name", person.getName()}
            });
    }
    data["persons"] = persons;

    // 2. 保存关系信息
    json edges = json::array();
    for (int i = 0; i < vertList.size(); i++) {
        for (const auto& edge : adjList[i]) {
            int to = edge.getTo();
            // 为了避免重复保存双向关系，只保存 i < to 的关系
            if (i < to) {
                json edge_json;
                edge_json["from"] = vertList[i].getName();
                edge_json["to"] = vertList[to].getName();
                edge_json["weight"] = edge.getWeight();
                edges.push_back(edge_json);
            }
        }
    }
    data["edges"] = edges;

    // 3. 保存额外信息
    data["metadata"]["person_count"] = vertList.size();
    data["metadata"]["edge_count"] = edges.size();
    data["metadata"]["save_time"] = "示例时间"; // 可以替换为实际时间

    // 4. 写入文件
    try {
        std::ofstream file(filename);
        if (!file.is_open()) {
            cout << "无法打开文件 " << filename << " 进行写入！\n";
            return;
        }

        file << data.dump(4);  // 缩进4个空格，使JSON更易读
        file.close();

        cout << "数据已成功保存到文件: " << filename << endl;
        cout << "保存了 " << vertList.size() << " 个联系人和 "
            << edges.size() << " 条关系\n";
    }
    catch (const std::exception& e) {
        cout << "保存文件时发生错误: " << e.what() << endl;
    }
}

void SocialNetwork::loadFromFile(string filename) {
    if (filename.empty()) {
        cout << "文件名不能为空！\n";
        return;
    }

    // 清空现有数据
    vertList.clear();
    adjList.clear();
    nameToIndex.clear();

    try {
        // 1. 以二进制模式读取
        std::ifstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            cout << "无法打开文件 " << filename << "！\n";
            return;
        }

        // 2. 读取文件到字符串
        string content((std::istreambuf_iterator<char>(file)),
            std::istreambuf_iterator<char>());
        file.close();

        // 3. 去除 BOM 头
        if (content.size() >= 3 &&
            content[0] == '\xEF' &&
            content[1] == '\xBB' &&
            content[2] == '\xBF') {
            content = content.substr(3);
        }

        // 4. 清理可能的非法字符
        for (size_t i = 0; i < content.size(); i++) {
            unsigned char c = content[i];
            // 检查是否为非法 UTF-8 字节
            if (c == 0xC0 || c == 0xC1 || (c >= 0xF5 && c <= 0xFF)) {
                // 替换为空格或删除
                content[i] = ' ';
            }
        }

        // 5. 解析 JSON
        json data = json::parse(content);

        // 6. 加载数据（简化版）
        // 先添加所有人
        if (data.contains("persons") && data["persons"].is_array()) {
            for (const auto& person_json : data["persons"]) {
                if (person_json.contains("name")) {
                    string name = person_json["name"];
                    Person p;
                    p.setName(name);
                    vertList.push_back(p);
                    adjList.push_back(list<Edge>());
                    nameToIndex[name] = vertList.size() - 1;
                }
            }
        }

        // 再添加所有关系
        if (data.contains("edges") && data["edges"].is_array()) {
            for (const auto& edge_json : data["edges"]) {
                if (edge_json.contains("from") &&
                    edge_json.contains("to") &&
                    edge_json.contains("weight")) {

                    string from = edge_json["from"];
                    string to = edge_json["to"];
                    int weight = edge_json["weight"];

                    int index1 = findIndex(from);
                    int index2 = findIndex(to);

                    if (index1 != -1 && index2 != -1 && index1 != index2) {
                        // 检查关系是否已存在
                        bool exists = false;
                        for (const auto& edge : adjList[index1]) {
                            if (edge.getTo() == index2) {
                                exists = true;
                                break;
                            }
                        }

                        if (!exists) {
                            Edge edge1, edge2;
                            edge1.setTo(index2);
                            edge1.setWeight(weight);
                            edge2.setTo(index1);
                            edge2.setWeight(weight);

                            adjList[index1].push_back(edge1);
                            adjList[index2].push_back(edge2);
                        }
                    }
                }
            }
        }

        cout << "从文件 " << filename << " 加载成功！\n";
        cout << "加载了 " << vertList.size() << " 个联系人和 ";

        int total_edges = 0;
        for (const auto& friends : adjList) {
            total_edges += friends.size();
        }
        cout << total_edges / 2 << " 条关系\n";

    }
    catch (const std::exception& e) {
        cout << "错误: " << e.what() << endl;
        cout << "请确保文件是有效的 UTF-8 编码 JSON 文件！\n";
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
		cout << "联系人 " << name << " 不存在！" << endl;
		return;
	}

	vector<FriendInfo> friend_infos;
	for (auto& e : adjList[index]) {
		friend_infos.push_back(FriendInfo(vertList[e.getTo()].getName(), e.getWeight(), e.getTo()));
	}

	if (friend_infos.empty()) {
		cout << name << "暂时没有好友！" << endl;
		return;
	}

	// 排序
	if (ascending) {
		sort(friend_infos.begin(), friend_infos.end(),
			[](const FriendInfo& a, const FriendInfo& b) {
			return a.weight < b.weight;
		});
		cout << "\n" << name << " 的好友（按亲密度升序）：" << endl;
	}
	else {
		sort(friend_infos.begin(), friend_infos.end(),
			[](const FriendInfo& a, const FriendInfo& b) {
				return a.weight > b.weight;
			});
		cout << "\n" << name << " 的好友（按亲密度降序）：" << endl;
	}

	for (int i = 0; i < friend_infos.size(); i++) {
		cout << i + 1 << ". " << friend_infos[i].name << " (亲密度: " << friend_infos[i].weight << ")" << endl;
	}
}
//路径亲密度下界最大值
int SocialNetwork::getBottleneckPath(string startName, string endName) {
	int start = findIndex(startName);
	int end = findIndex(endName);

	if (start == -1 || end == -1) {
		cout << "至少一个联系人不存在！" << endl;
		return -1;
	}

	if (start == end) {
		cout << "这是同一个人！" << endl;
		return 0;
	}

	// 查询是否是直接好友
	for (auto& e : adjList[start]) {
		if (e.getTo() == end) {
			cout << "他们是直接好友，亲密度为：" << e.getWeight() << endl;
			return e.getWeight();
		}
	}

	int personCount = vertList.size();
	vector<int> distToTree(personCount, -1);
	vector<int> parent(personCount, -1);
	vector<bool> inMST(personCount, false);
	// 最大堆， 存储{权值， 节点ID｝
	priority_queue<pair<int, int>> pq;

	distToTree[start] = INT_MAX;
	pq.push({ 0, start });
	
	while (!pq.empty()) {
		int u = pq.top().second;
		pq.pop();

		if (inMST[u]) continue;
		else inMST[u] = true;

		if (u == end) break;		//  路径已经可达

		for (auto& e : adjList[u]) {
			int v = e.getTo();
			int w = e.getWeight();

			if (!inMST[v] && min(distToTree[u], w)) {
				distToTree[v] = w;
				parent[v] = u;
				pq.push({ distToTree[v], v });
			}
		}
	}

	if (!inMST[end]) {
		return -1;		//两点无法连通
	}
	
	int miniIntimacy = INT_MAX;
	int curr = end;

	while (curr != start) {
		int currentEdgeWeight = distToTree[curr];
		if (currentEdgeWeight < miniIntimacy) {
			miniIntimacy = currentEdgeWeight;
		}
		curr = parent[curr];
	}
	return miniIntimacy;
}
	
void SocialNetwork::displayTop10() {
	int personCount = vertList.size();
	if (personCount <= 0) {
		cout << "社交网络为空！" << endl;
		return;
	}

	// {ID， 好友人数}
	vector<pair<int, int>> friendCounts;
	for (int i = 0; i < personCount; i++) {
		friendCounts.push_back({ i, adjList[i].size() });
	}

	sort(friendCounts.begin(), friendCounts.end(), 
		[](const pair<int, int>& a, const pair<int, int>& b){
		return a.second > b.second;
		});

	int displayCount = min(10, personCount);
	cout << "\n========== 社交大牛Top" << displayCount << " ==========" << endl;
	cout << "排名\t姓名\t\t好友数\t\t直接好友" << endl;
	cout << "---------------------------------------------" << endl;

	for (int i = 0; i < displayCount; i++) {
		int idx = friendCounts[i].first;
		cout << i + 1 << "\t" << vertList[idx].getName();

		// 调整格式
		if (vertList[idx].getName().length() < 8) {
			cout << "\t\t";
		}
		else {
			cout << "\t";
		}

		cout << friendCounts[i].second << "\t\t";

		// 显示前3个好友
		int count = 0;
		for (auto& edge : adjList[idx]) {
			if (count >= 3) break;
			cout << vertList[edge.getTo()].getName();
			if (count < 2 && count < adjList[idx].size() - 1) {
				cout << ", ";
			}
			count++;
		}
		if (adjList[idx].size() > 3) {
			cout << "...";
		}
		cout << endl;
	}
	cout << "==========================================" << endl;
}
int SocialNetwork::findIndex(string name) {
	if (nameToIndex.find(name) == nameToIndex.end()) return -1;		// 未找到名字对应的人的ID，返回-1
	return nameToIndex[name];
}