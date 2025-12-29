#include "SocialNetwork.h"

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
	
}
	
void SocialNetwork::displayTop10() {

}
int SocialNetwork::findIndex(string name) {
	if (nameToIndex.find(name) == nameToIndex.end()) return -1;		// 未找到名字对应的人的ID，返回-1
	return nameToIndex[name];
}