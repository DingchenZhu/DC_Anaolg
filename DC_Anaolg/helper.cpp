#include "Circuit.h"
#include <Eigen/Dense>
#include <iostream>
#include <map>
#include <vector>

using namespace std;
using namespace Eigen;


// 定义元件添加方法
void Circuit::addNode(const string& name)
{
    if (nodes.find(name) == nodes.end())
    {
        // 将添加到节点放在nodes的末尾：注意添加节点的顺序
        int index = nodes.size();
        nodes[name] = { index, name };
    }
}
void Circuit::addVoltageSource(const string& name, const string& node1, const string& node2, double voltage)
{
    voltagesources.push_back({ name, nodes[node1].index, nodes[node2].index,voltage });
}
void Circuit::addResistor(const string& name, const string& node1, const string& node2, double resistance)
{
    resistors.push_back({ name, nodes[node1].index, nodes[node2].index, resistance });
}
void Circuit::addCapacity(const string& name, const string& node1, const string& node2, double capacitance)
{
    capacities.push_back({ name, nodes[node1].index, nodes[node2].index, capacitance });
}
void Circuit::addInductor(const string& name, const string& node1, const string& node2, double inductance)
{
    inductors.push_back({ name, nodes[node1].index, nodes[node2].index, inductance });
}
void Circuit::addMOSFET(const string& name, const string& ND, const string& NG, const string& NS, const string& modelID, double width, double length)
{
    mosfets.push_back({ name, nodes[ND].index, nodes[NG].index, nodes[NS].index, modelID, width, length });
}
void Circuit::addMOSFETModel(const string& mosID, double VT, double MU, double COX, double LAMBDA, double CJ0)
{
    mosfetmodels[mosID] = { mosID, VT, MU, COX, LAMBDA,CJ0 };
}


// 构建导纳矩阵
void Circuit::buildYMatrix()
{
    int numNodes = nodes.size();
    int numVoltageSources = voltagesources.size();
    YMatrix = MatrixXd::Zero(numNodes + numVoltageSources, numNodes + numVoltageSources);

    // 关于电阻对电路矩阵的贡献
    for (const auto& resistor : resistors)
    {
        int n1 = resistor.node1;
        int n2 = resistor.node2;
        double g = 1.0 / resistor.resistance;

        if (n1 >= 0) YMatrix(n1, n1) += g;
        if (n2 >= 0) YMatrix(n2, n2) += g;

        if (n1 >= 0 && n2 >= 0)
        {
            YMatrix(n1, n2) -= g;
            YMatrix(n2, n1) -= g;
        }
    }

    // 关于MOS器件的影响
    for (const auto& mosfet : mosfets) {
        // 找到对应的 MOSFET 模型参数
        auto it = find_if(mosfetmodels.begin(), mosfetmodels.end(),
            [&](const MOSFETModel& model) { return model.mosID == mosfet.modelID; });
        if (it == mosfetmodels.end()) {
            cerr << "Error: MOSFET model " << mosfet.modelID << " not found!\n";
            continue;
        }
        const MOSFETModel& model = it->second;

        // 提取节点编号
        int ND = mosfet.ND; // 漏极节点
        int NG = mosfet.NG; // 栅极节点
        int NS = mosfet.NS; // 源极节点

        // 计算小信号参数:此处考虑到NMOS和PMOS不同处理方法，添加判断逻辑
        //double VGS = NodeVoltage[NG] - NodeVoltage[NS];
        //double VDS = NodeVoltage[ND] - NodeVoltage[NS];
        double VGS = mosfet.isPMOS
            ? (NodeVoltage[NS] - NodeVoltage[NG]) // PMOS: VGS = V(S) - V(G)
            : (NodeVoltage[NG] - NodeVoltage[NS]); // NMOS: VGS = V(G) - V(S)
        double VDS = mosfet.isPMOS
            ? (NodeVoltage[NS] - NodeVoltage[ND]) // PMOS: VDS = V(S) - V(D)
            : (NodeVoltage[ND] - NodeVoltage[NS]); // NMOS: VDS = V(D) - V(S)

        double g_m = 0.0, g_ds = 0.0;

        if (mosfet.isPMOS ? (VGS < model.VT) : (VGS > model.VT)) {
            if (mosfet.isPMOS ? (VDS < (VGS - model.VT)) : (VDS > (VGS - model.VT))) {
                // 饱和区
                g_m = model.MU * model.COX * (mosfet.width / mosfet.length) * std::abs(VGS - model.VT);
                g_ds = model.LAMBDA * g_m * std::abs(VDS);
            }
            else {
                // 线性区
                g_m = model.MU * model.COX * (mosfet.width / mosfet.length) * std::abs(VDS);
                g_ds = g_m / std::abs(VGS - model.VT);
            }
        }

        // 将 g_m 和 g_ds 添加到导纳矩阵中
        if (ND >= 0 && ND < numNodes) {
            YMatrix(ND, ND) += g_ds;
            if (NS >= 0 && NS < numNodes) YMatrix(ND, NS) -= g_ds;
            if (NG >= 0 && NG < numNodes) YMatrix(ND, NG) += (mosfet.isPMOS ? -g_m : g_m);
        }
        if (NS >= 0 && NS < numNodes) {
            YMatrix(NS, NS) += g_ds;
            if (ND >= 0 && ND < numNodes) YMatrix(NS, ND) -= g_ds;
            if (NG >= 0 && NG < numNodes) YMatrix(NS, NG) -= (mosfet.isPMOS ? -g_m : g_m);
        }
    }

    // 关于独立电压源的影响
    int voltageSourceIndex = numNodes; // 电压源方程从 numNodes 开始
    for (const auto& voltageSource : voltagesources) {
        int n1 = voltageSource.node1;
        int n2 = voltageSource.node2;
        double voltage = voltageSource.voltage;

        // 电压源对矩阵的影响
        if (n1 >= 0) {
            YMatrix(n1, voltageSourceIndex) = 1.0;
            YMatrix(voltageSourceIndex, n1) = 1.0;
        }
        if (n2 >= 0) {
            YMatrix(n2, voltageSourceIndex) = -1.0;
            YMatrix(voltageSourceIndex, n2) = -1.0;
        }

        // 电压源的值会加入到右端电流向量中
        voltageSourceIndex++;
    }
}

void Circuit::buildCurrentVector()
{
    int size = nodes.size();
    CurrentVector = VectorXd::Zero(size + voltagesources.size());

    // 添加独立电流源的贡献
    for (const auto& currentsource : currentsources)
    {
        int n1 = currentsource.node1;
        int n2 = currentsource.node2;
        double value = currentsource.current;

        // 正负极流入电流
        if (n1 >= 0) CurrentVector(n1) += value;
        if (n2 >= 0) CurrentVector(n2) += value;
    }

    // 独立电压源
    int voltageIndexOffset = nodes.size();
    for (size_t i = 0; i < voltagesources.size(); ++i) {
        const auto& voltageSource = voltagesources[i];
        int n1 = voltageSource.node1;
        int n2 = voltageSource.node2;

        // 添加电压源电流的占位变量（通过导纳矩阵约束）
        CurrentVector[voltageIndexOffset + i] = 0; // 由于电流未知，直接占位
    }

    //// 是否需要其他受控源的贡献？
    //for (const auto& vccs : vccsSources) {
    //    int n1 = vccs.node1;           // 输出正极
    //    int n2 = vccs.node2;           // 输出负极
    //    int cn1 = vccs.controlNode1;   // 控制正极
    //    int cn2 = vccs.controlNode2;   // 控制负极
    //    double g = vccs.gain; // 导纳值

    //    double controlVoltage = 0;
    //    if (cn1 >= 0) controlVoltage += NodeVoltage(cn1);
    //    if (cn2 >= 0) controlVoltage -= NodeVoltage(cn2);

    //    double currentContribution = g * controlVoltage;
    //    if (n1 >= 0) CurrentVector(n1) += currentContribution;
    //    if (n2 >= 0) CurrentVector(n2) -= currentContribution;
    //}
    //// 诸如此类，待完善

}