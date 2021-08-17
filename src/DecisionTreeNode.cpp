#include "DecisionTreeNode.h"
#include <iostream>

using namespace std;

DecisionTreeNode::DecisionTreeNode(int id_val, bool isLeaf_val, int attribute_val, double divide_value_val, int classification_val, vector<int> candidate_classes_val) {
	id = id_val;
	isLeaf = isLeaf_val;

	divide_attribute = attribute_val;
	divide_value = divide_value_val;
	classification = classification_val;
	candidate_classes = candidate_classes_val;
	
	level = 1; // 创建时深度都默认为1
	father_node = nullptr;
	true_node = nullptr;
	false_node = nullptr;
}

void DecisionTreeNode::printTreeNode() {
	if (isLeaf == false) {
		cout << id << ", node:\tlevel = " << level << ", X[" << divide_attribute << "] <= " << divide_value << ", class = " << classification << ", candidate class = [";
		for (int i = 0; i < candidate_classes.size(); i++) {
			cout << candidate_classes[i] << " ";
		}
		cout << "]" << endl;
	}
	else {
		cout << id << ", leaf:\tlevel = " << level << ", class = " << classification << ", candidate class = [";
		for (int i = 0; i < candidate_classes.size(); i++) {
			cout << candidate_classes[i] << " ";
		}
		cout << "]" << endl;
	}
}

DecisionTreeNode::~DecisionTreeNode() {

}