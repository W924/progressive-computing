#pragma once
#include <vector>

class DecisionTreeNode
{
public:
	int id; // ID，取值从0开始
	int level; // 节点深度，设根节点深度为1

	bool isLeaf; // 标志位，是否为叶节点
	int classification; // 类别，只有当节点为叶结点时，该变量才有意义，取值从0开始，class 0，class 1，……
	std::vector<int> candidate_classes; // 候选类，只有当节点为叶结点时，该变量才有意义

	int divide_attribute; // 划分属性，只有当节点为中间节点(或根节点)时，该变量才起作用，取值从0开始
	double divide_value; // 属性划分值，定义divide_attribute <= divide_value，搜索true_node；否则搜索false_node

	DecisionTreeNode* father_node;
	DecisionTreeNode* true_node, * false_node;

	DecisionTreeNode(int id_val, bool isLeaf_val, int attribute_val, double divide_value_val, int classification_val, std::vector<int> candidate_classes_val);
	void printTreeNode();

	virtual ~DecisionTreeNode();
};
