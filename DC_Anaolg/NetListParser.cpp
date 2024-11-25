#include <fstream>
#include <iostream>
#include <regex>
#include <vector>
#include <string>
#include "Circuit.h"
#include <unordered_map>

using namespace std;


// 并查集的实现
class UnionFind {
public:
	UnionFind(int n) {
		parent.resize(n);
		rank.resize(n, 0);
		for (int i = 0; i < n; ++i) {
			parent[i] = i;
		}
	}

	// 查找根节点
	int find(int x) {
		if (parent[x] != x) {
			parent[x] = find(parent[x]); // 路径压缩
		}
		return parent[x];
	}

	// 合并两个节点
	void unionSet(int x, int y) {
		int rootX = find(x);
		int rootY = find(y);

		if (rootX != rootY) {
			// 按秩合并
			if (rank[rootX] > rank[rootY]) {
				parent[rootY] = rootX;
			}
			else if (rank[rootX] < rank[rootY]) {
				parent[rootX] = rootY;
			}
			else {
				parent[rootY] = rootX;
				rank[rootX]++;
			}
		}
	}

private:
	vector<int> parent; // 存储每个节点的父节点
	vector<int> rank;   // 存储树的高度（优化路径压缩时使用）
};

// 定义结点映射函数，用来进行结点的存储和重新编号
vector<pair<string, int>> mapNodes(const vector<string>& nodeNames) {
	// 使用并查集来管理节点
	UnionFind uf(nodeNames.size());

	unordered_map<string, int> nodeNameToIndex;
	int nextIndex = 1; // 从1开始编号，0为地（GND）
	vector<pair<string, int>> result;

	// 首先，把地（GND）节点固定为节点0
	nodeNameToIndex["GND"] = 0;
	result.push_back({ "GND", 0 });

	// 遍历所有节点名称，查找重复并进行合并
	for (int i = 0; i < nodeNames.size(); ++i) {
		const string& nodeName = nodeNames[i];

		// 如果该节点已经存在，查找并合并
		if (nodeNameToIndex.find(nodeName) != nodeNameToIndex.end()) {
			// 查找该节点在并查集中的根节点，并进行合并
			uf.unionSet(i, nodeNameToIndex[nodeName]);
		}
		else {
			// 如果是新节点，分配新的编号
			nodeNameToIndex[nodeName] = i;
		}
	}

	// 根据并查集结果生成节点映射
	for (int i = 0; i < nodeNames.size(); ++i) {
		int root = uf.find(i);
		if (nodeNameToIndex.find(nodeNames[root]) == nodeNameToIndex.end()) {
			result.push_back({ nodeNames[root], nextIndex++ });
			nodeNameToIndex[nodeNames[root]] = nextIndex - 1;
		}
	}

	return result;
}

void NetListParser(const string& filename, Circuit& circuit)
{
	ifstream file(filename);
	if(!file.is_open())
	{
		cerr << "Failed to open the file: " << filename << endl;
		return;
	}

	string line;
	vector<string> nodeNames; //存储提取的结点名称
	while(getline(file, line))
	{
		if(line.empty() || line[0] == '*')
			continue;
		istringstream iss(line);
		vector<string> tokens((istream_iterator<string>(iss)), istream_iterator<string>());
		if(tokens.empty()) continue;

		//不同元件分类
		char type = tokens[0][0];
		switch (type)
		{
		case 'V': // 电压源
			circuit.addVoltageSource(tokens[0], tokens[1], tokens[2], stod(tokens[3]));
			nodeNames.push_back(tokens[1]);
			nodeNames.push_back(tokens[2]);
			break;
		case 'R': // 电阻
			circuit.addResistor(tokens[0], tokens[1], tokens[2], stod(tokens[3]));
			nodeNames.push_back(tokens[1]);
			nodeNames.push_back(tokens[2]);
			break;
		case 'C': // 电容
			circuit.addCapacity(tokens[0], tokens[1], tokens[2], stod(tokens[3]));
			nodeNames.push_back(tokens[1]);
			nodeNames.push_back(tokens[2]);
			break;
		case 'L': // 电感
			circuit.addInductor(tokens[0], tokens[1], tokens[2], stod(tokens[3]));
			nodeNames.push_back(tokens[1]);
			nodeNames.push_back(tokens[2]);
			break;
		case 'M': // MOSFET
			circuit.addMOSFET(tokens[0], tokens[1], tokens[2], tokens[3], tokens[4], stod(tokens[5]), stod(tokens[6]));
			nodeNames.push_back(tokens[1]);
			nodeNames.push_back(tokens[2]);
			nodeNames.push_back(tokens[3]);
			break;
		default:
			cerr << "Unknown component type: " << tokens[0] << endl;
		}
		if (tokens[0] == ".MODEL")
		{
			string modelID = tokens[1];
			double VT = 0, MU = 0, COX = 0, LAMBDA = 0, CJ0 = 0;
			for (size_t i = 2; i < tokens.size(); i += 2) {
				if (tokens[i] == "VT") VT = stod(tokens[i + 1]);
				else if (tokens[i] == "MU") MU = stod(tokens[i + 1]);
				else if (tokens[i] == "COX") COX = stod(tokens[i + 1]);
				else if (tokens[i] == "LAMBDA") LAMBDA = stod(tokens[i + 1]);
				else if (tokens[i] == "CJ0") CJ0 = stod(tokens[i + 1]);
			}
			circuit.addMOSFETModel(modelID, VT, MU, COX, LAMBDA, CJ0);
		}
		else if(tokens[0]== ".PLOTNV")
		{
			circuit.addNode(tokens[1]);
		}
		else if (tokens[0]==".hb")
		{
			cout << "HB analysis with frequency: " << tokens[1] << ", duration: " << tokens[2] << endl;
		}
		else {
			cerr << "Unknown element type: " << tokens[0] << endl;
			}
	}
	vector<pair<string, int>> mappedNodes = mapNodes(nodeNames);
	for (const auto& [name, index] : mappedNodes)
	{
		circuit.addNode(name); 
	}
	file.close();
}
