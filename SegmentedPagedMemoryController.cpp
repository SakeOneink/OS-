#include<iostream>
#include "SegmentedPagedMemoryController.h"
using namespace std;

SegmentedPagedMemoryController::SegmentedPagedMemoryController(int initSize, int blockSize) {
	memoryService.init(initSize, blockSize);
}

void SegmentedPagedMemoryController::disMenuAction() {
	cout << "\n=====��ҳʽ�洢���������=====\n";
	cout << "1.��ʾ��Ϣ(���н��̷��䡢��ҳʹ��������ڴ����)" << endl;
	cout << "2.��������" << endl;
	cout << "3.��������" << endl;
	cout << "4.�����ڴ���������ͷ�(�����¶Ρ��ͷ��϶�)" << endl;
	cout << "5.�л��û���̭�㷨" << endl;
	cout << "0.�˳�" << endl;
	cout << "==============" << endl;
}

void SegmentedPagedMemoryController::showAllMessageAction() {
	memoryService.showAllTable();
}

void SegmentedPagedMemoryController::newProcessAction() {
	string processName;
	int segmentNum;
	cout << "������������name��";
	cin >> processName;
	cout << "������� ���������";
	cin >> segmentNum;

	vector<int> pageNumOfSeg;
	int pageNumTemp;
	for (int i = 0; i < segmentNum; i++) {
		cout << "�������" << i << "���ڵ�ҳ�ĸ���:";
		cin >> pageNumTemp;
		pageNumOfSeg.push_back(pageNumTemp);
	}

	if (memoryService.newProcess(processName, segmentNum, pageNumOfSeg)) {
		cout << "�����½��ɹ�(Ĭ�Ͻ���һ������ε����ڴ�)" << endl;
	}
}
void SegmentedPagedMemoryController::returnProcessAction() {
	string processName;
	cout << "���뷵���Ľ���name��";
	cin >> processName;

	memoryService.returnProcessByProcessName(processName);
}
void SegmentedPagedMemoryController::processMenageAction() {
	string processName;
	cout << "��������Ҫ�����Ľ�������";
	cin >> processName;
	if (!memoryService.showProcessMessageByName(processName)) {
		cout << "����Ľ�����������" << endl;
		return;
	}
	cout << "=====�����ڴ��������ͷ�=====\n";
	cout << "1.�����¶�" << endl;
	cout << "2.�ͷ��϶�" << endl;
	cout << "3.ģ����ʶκ��¶�Ӧҳ��" << endl;
	cout << "==============" << endl;
	int poly, segmentCode, pageCode, pageNum;
	cout << "����ִ�в�����";
	cin >> poly;
	switch (poly) {
	case 1:
		cout << "������¶α�ţ�";
		cin >> segmentCode;
		memoryService.processCallSegment(processName, segmentCode);
		break;
	case 2:
		cout << "�ͷŵ��϶α�ţ�";
		cin >> segmentCode;
		memoryService.processReleaseSegment(processName, segmentCode);
		break;
	case 3:
		cout << "���ʵĶκţ�";
		cin >> segmentCode;
		cout << "���ʵ�ҳ�ţ�";
		cin >> pageCode;
		memoryService.runProcessPage(processName, segmentCode, pageCode);
		break;
	default:
		cout << "null in" << endl;
	}
	memoryService.showProcessMessageByName(processName);
}
void SegmentedPagedMemoryController::changePolyAction() {
	cout << "=====����ѡ��=====\n";
	cout << "1.������δʹ��ҳ���û��㷨(LRU)" << endl;
	cout << "==============" << endl;
	int poly;
	cout << "�������ѡ��";
	cin >> poly;
	switch (poly) {
	case 1:
		memoryService.setPoly(substitutePoly::LRU);
		break;

	default:
		cout << "null in" << endl;
	}
}