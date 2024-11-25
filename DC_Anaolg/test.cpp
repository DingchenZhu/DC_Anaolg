#include <iostream>
#include <fstream>
#include <iomanip>
#include "Circuit.h"

using namespace std;

// 打印解析结果的外部函数
void printParsedCircuit(const Circuit& circuit)
{
    cout << "Parsed Circuit Components:" << endl;

    // 打印节点信息
    cout << "Nodes:" << endl;
    for (const auto& [name, node] : circuit.nodes)
    {
        cout << "  Node Name: " << name << ", Index: " << node.index << endl;
    }

    // 打印电压源信息
    cout << "\nVoltage Sources:" << endl;
    for (const auto& vs : circuit.voltagesources)
    {
        cout << "  Name: " << vs.name
            << ", Node1: " << vs.node1
            << ", Node2: " << vs.node2
            << ", Voltage: " << vs.voltage << " V" << endl;
    }

    // 打印电流源信息
    cout << "\nCurrent Sources:" << endl;
    for (const auto& cs : circuit.currentsources)
    {
        cout << "  Name: " << cs.name
            << ", Node1: " << cs.node1
            << ", Node2: " << cs.node2
            << ", Current: " << cs.current << " A" << endl;
    }

    // 打印电阻信息
    cout << "\nResistors:" << endl;
    for (const auto& res : circuit.resistors)
    {
        cout << "  Name: " << res.name
            << ", Node1: " << res.node1
            << ", Node2: " << res.node2
            << ", Resistance: " << res.resistance << " Ω" << endl;
    }

    // 打印电容信息
    cout << "\nCapacitors:" << endl;
    for (const auto& cap : circuit.capacities)
    {
        cout << "  Name: " << cap.name
            << ", Node1: " << cap.node1
            << ", Node2: " << cap.node2
            << ", Capacitance: " << cap.capacitance << " F" << endl;
    }

    // 打印电感信息
    cout << "\nInductors:" << endl;
    for (const auto& ind : circuit.inductors)
    {
        cout << "  Name: " << ind.name
            << ", Node1: " << ind.node1
            << ", Node2: " << ind.node2
            << ", Inductance: " << ind.inductance << " H" << endl;
    }

    // 打印 MOSFET 信息
    cout << "\nMOSFETs:" << endl;
    for (const auto& mos : circuit.mosfets)
    {
        cout << "  Name: " << mos.name
            << ", ND: " << mos.ND
            << ", NG: " << mos.NG
            << ", NS: " << mos.NS
            << ", ModelID: " << mos.modelID
            << ", Width: " << mos.width << " m"
            << ", Length: " << mos.length << " m"
            << ", Type: " << (mos.isNMOS ? "NMOS" : "PMOS") << endl;
    }

    // 打印 MOSFET 模型信息
    cout << "\nMOSFET Models:" << endl;
    for (const auto& [id, model] : circuit.mosfetmodels)
    {
        cout << "  Model ID: " << model.mosID
            << ", VT: " << model.VT
            << ", MU: " << model.MU
            << ", COX: " << model.COX
            << ", LAMBDA: " << model.LAMBDA
            << ", CJ0: " << model.CJ0 << endl;
    }
}
int main()
{
    // 创建 Circuit 对象
    Circuit circuit;

    // 网表文件路径
    string netlistFile = "D:\\a_大三上\\模拟EDA\\buffer.sp";

    // 调用解析函数
    ifstream file(netlistFile);
    if (!file.is_open())
    {
        cerr << "Error: Unable to open netlist file!" << endl;
        return 1;
    }

    // 调用解析函数（你之前的解析逻辑）
    string line;
    while (getline(file, line))
    {
        if (line.empty() || line[0] == '*')
            continue;

        istringstream iss(line);
        vector<string> tokens((istream_iterator<string>(iss)), istream_iterator<string>());
        if (tokens.empty())
            continue;

        const string& componentType = tokens[0];
        if (componentType[0] == 'V')
            circuit.addVoltageSource(tokens[0], tokens[1], tokens[2], stod(tokens[3]));
        else if (componentType[0] == 'R')
            circuit.addResistor(tokens[0], tokens[1], tokens[2], stod(tokens[3]));
        else if (componentType[0] == 'C')
            circuit.addCapacity(tokens[0], tokens[1], tokens[2], stod(tokens[3]));
        else if (componentType[0] == 'L')
            circuit.addInductor(tokens[0], tokens[1], tokens[2], stod(tokens[3]));
        else if (componentType[0] == 'M')
            circuit.addMOSFET(tokens[0], tokens[1], tokens[2], tokens[3], tokens[4], stod(tokens[5]), stod(tokens[6]));
        else if (componentType == ".MODEL")
            circuit.addMOSFETModel(tokens[1], stod(tokens[3]), stod(tokens[5]), stod(tokens[7]), stod(tokens[9]), stod(tokens[11]));
        else
            cerr << "Unknown component type: " << componentType << endl;
    }

    // 关闭网表文件
    file.close();

    // 调用打印函数
    printParsedCircuit(circuit);

    return 0;
}


