#pragma once

#include <string>

using namespace std;

class Person
{
public:
	Person()=default;
	~Person()=default;

	void setName(string name) {
		this->name = name;
	}

	string getName() const {
		return name;
	}

private:
	 string name;
};
