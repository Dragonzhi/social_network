#pragma once
class Edge
{
public:
	Edge();
	~Edge();

	void setTo(int to) {
		this->to = to;
	}
	void setWeight(int w) {
		weight = w;
	}

	int getTo() const {
		return to;
	}
	int getWeight() const {
		return weight;
	}

private:
	int to;
	int weight;
};

Edge::Edge()
{
	to = -1;
	weight = 0;
}

Edge::~Edge()
{
}