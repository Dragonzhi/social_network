#include "SocialNetwork.h"
#include <queue>

SocialNetwork::SocialNetwork()
{
}

SocialNetwork::~SocialNetwork()
{
}

// A side
void SocialNetwork::addPerson(string name) {

}
void SocialNetwork::deletePerson(string name) {

}
void SocialNetwork::addEdge(string name1, string name2, int weight) {

}
void SocialNetwork::deleteEdge(string name1, string name2) {

}
void SocialNetwork::saveToFile(string filename) {

}
void SocialNetwork::loadFromFile(string filename) {

}
void SocialNetwork::displayAll() {

}

// B side
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

	distToTree[start] = 0;
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

			if (!inMST[v] && w > distToTree[v]) {
				distToTree[v] = w;
				parent[v] = v;
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
}
int SocialNetwork::findIndex(string name) {
	if (nameToIndex.find(name) == nameToIndex.end()) return -1;		// 未找到名字对应的人的ID，返回-1
	return nameToIndex[name];
}