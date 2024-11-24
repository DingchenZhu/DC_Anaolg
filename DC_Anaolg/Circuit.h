#ifndef _CIRCUIT_H_
#define _CIRCUIT_H_

#include <vector>
#include <map>
#include <string>
#include <Eigen/Dense>

using namespace std;
using namespace Eigen;

class Circuit
{
public:
    // 添加元件：电源、电阻、电容、电感，MOSFET，MOSFET模型
    // 对于电源，我们在这里只进行DC分析，因此传入参数只有节点和数值大小
    void addNode(const string& name);
    void addVoltageSource(const string& name, const string& node1, const string& node2, double voltage);
    void addResistor(const string& name, const string& node1, const string& node2, double resistance);
    void addCapacity(const string& name, const string& node1, const string& node2, double capacitance);
    void addInductor(const string& name, const string& node1, const string& node2, double inductance);
    void addMOSFET(const string& name, const string& ND, const string& NG, const string& NS, const string& modelID, double width, double length);
    void addMOSFETModel(const string& mosID, double VT, double MU, double COX, double LAMBDA, double CJ0);

    // 建立导纳矩阵
    void buildYMatrix();
    // 建立电流向量
    void buildCurrentVector();
    // 电路求解器
    void solve();
    // 计算元件的电流和功耗
    void computeCurrentsAndPowers();

    // 结果输出
    void printNodeVoltages();
    void printCurrentsAndPowers();

private:
    /*我们在这里定义电路的元件结构
    关于结构参数：
    Node：矩阵索引，名称
    VoltageSource：名称，节点，数值
    Resistor: 名称，节点，数值
    Capacity：名称，节点，数值
    Inductor：名称，节点，数值
    MOSFET：名称，端口，ID，尺寸
    model：ID，参数
    */
    struct Node
    {
        int index;
        string name;
    };

    struct VoltageSource
    {
        string name;
        int node1, node2;
        double voltage;
    };

    struct CurrentSource
    {
        string name;
        int node1, node2;
        double current;
    };

    // 为了使电路分析更完整，我们添加受控源的分析
    // VCVS
    struct VoltageControlledVoltageSource
    {
        string name;
        int node1, node2;
        int controlNode1, controlNode2;
        double gain;
    };

    // CCVS
    struct CurrentControlledVoltageSource
    {
        string name;
        int node1, node2;
        string controllingVS;
        double gain;
    };

    // VCCS
    struct VoltageControlledCurrentSource
    {
        string name;
        int node1, node2;
        int controlNode1, controlNode2;
        double gain;
    };

    // CCCS
    struct CurrentControlledCurrentSource
    {
        string name;
        int node1, node2;
        string controllingCS;
        double gain;
    };

    struct Resistor
    {
        string name;
        int node1, node2;
        double resistance;
    };

    struct Capacity
    {
        string name;
        int node1, node2;
        double capacitance;
    };

    struct Inductor
    {
        string name;
        int node1, node2;
        double inductance;
    };

    struct MOSFET
    {
        string name;
        int ND, NG, NS;
        string modelID;
        double width, length;
        bool isPMOS;    // 用来判断是否为PMOS管
    };

    struct MOSFETModel
    {
        string mosID;
        double VT, MU, COX, LAMBDA, CJ0;
    };

    // 存储方式，构建map和vector
    map<string, Node> nodes;
    vector<VoltageSource> voltagesources;
    vector<CurrentSource> currentsources;
    vector<Resistor> resistors;
    vector<Capacity> capacities;
    vector<Inductor> inductors;
    vector<VoltageControlledVoltageSource> vcvsSources;
    vector<CurrentControlledVoltageSource> ccvsSources;
    vector<VoltageControlledCurrentSource> vccsSources;
    vector<CurrentControlledCurrentSource> cccsSources;
    vector<MOSFET> mosfets;
    map<string, MOSFETModel> mosfetmodels;

    // 定义矩阵和向量
    MatrixXd YMatrix;   // 导纳矩阵
    VectorXd CurrentVector;    //电流向量
    VectorXd NodeVoltage;   //节点电压的解


};
#endif