#pragma once
class Edge
{
public:
	Edge()=default;
	~Edge()=default;

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