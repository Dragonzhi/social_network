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

}
//路径亲密度下界最大值
int SocialNetwork::getBottleneckPath(string startName, string endName) {
	return 0;
}
	
void SocialNetwork::displayTop10() {

}
int SocialNetwork::findIndex(string name) {
	if (nameToIndex.find(name) == nameToIndex.end()) return -1;		// 未找到名字对应的人的ID，返回-1
	return nameToIndex[name];
}