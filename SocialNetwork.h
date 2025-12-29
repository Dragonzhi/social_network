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


class SocialNetwork
{
public:
	SocialNetwork();
	~SocialNetwork();

	// A side
	void addPerson(string name);
	void deletePerson(string name);
	void addEdge(string name1, string name2);
	void deleteEdge(string name1, string name2);
	void saveToFile(string filename);
	void loadFromFile(string filename);
	void displayAll();

	// B side
	void sortFriends(string name, bool ascending);
	int getBottleneckPat(string startName, string endName);		//路径亲密度下界最大值
	void displayTop10();
	int findIndex(string name);
private:
	vector<Person> vertList;
	vector<list<Edge>> adjList;
	map<string, int> nameToIndex;
};

