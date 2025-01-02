#include<iostream>
#include "SegmentedPagedMemoryController.h"
using namespace std;

SegmentedPagedMemoryController::SegmentedPagedMemoryController(int initSize, int blockSize) {
	memoryService.init(initSize, blockSize);
}

void SegmentedPagedMemoryController::disMenuAction() {
	cout << "\n=====段页式存储分配与回收=====\n";
	cout << "1.显示信息(所有进程分配、段页使用情况；内存情况)" << endl;
	cout << "2.新增进程" << endl;
	cout << "3.返还进程" << endl;
	cout << "4.进程内存的申请与释放(调入新段、释放老段)" << endl;
	cout << "5.切换置换淘汰算法" << endl;
	cout << "0.退出" << endl;
	cout << "==============" << endl;
}

void SegmentedPagedMemoryController::showAllMessageAction() {
	memoryService.showAllTable();
}

void SegmentedPagedMemoryController::newProcessAction() {
	string processName;
	int segmentNum;
	cout << "输入新增进程name：";
	cin >> processName;
	cout << "输入进程 程序段数：";
	cin >> segmentNum;

	vector<int> pageNumOfSeg;
	int pageNumTemp;
	for (int i = 0; i < segmentNum; i++) {
		cout << "请输入第" << i << "段内的页的个数:";
		cin >> pageNumTemp;
		pageNumOfSeg.push_back(pageNumTemp);
	}

	if (memoryService.newProcess(processName, segmentNum, pageNumOfSeg)) {
		cout << "进程新建成功(默认将第一个程序段调入内存)" << endl;
	}
}
void SegmentedPagedMemoryController::returnProcessAction() {
	string processName;
	cout << "输入返还的进程name：";
	cin >> processName;

	memoryService.returnProcessByProcessName(processName);
}
void SegmentedPagedMemoryController::processMenageAction() {
	string processName;
	cout << "请输入所要操作的进程名：";
	cin >> processName;
	if (!memoryService.showProcessMessageByName(processName)) {
		cout << "输入的进程名不存在" << endl;
		return;
	}
	cout << "=====进程内存申请与释放=====\n";
	cout << "1.调入新段" << endl;
	cout << "2.释放老段" << endl;
	cout << "3.模拟访问段号下对应页号" << endl;
	cout << "==============" << endl;
	int poly, segmentCode, pageCode, pageNum;
	cout << "输入执行操作：";
	cin >> poly;
	switch (poly) {
	case 1:
		cout << "调入的新段编号：";
		cin >> segmentCode;
		memoryService.processCallSegment(processName, segmentCode);
		break;
	case 2:
		cout << "释放的老段编号：";
		cin >> segmentCode;
		memoryService.processReleaseSegment(processName, segmentCode);
		break;
	case 3:
		cout << "访问的段号：";
		cin >> segmentCode;
		cout << "访问的页号：";
		cin >> pageCode;
		memoryService.runProcessPage(processName, segmentCode, pageCode);
		break;
	default:
		cout << "null in" << endl;
	}
	memoryService.showProcessMessageByName(processName);
}
void SegmentedPagedMemoryController::changePolyAction() {
	cout << "=====策略选择=====\n";
	cout << "1.最近最久未使用页面置换算法(LRU)" << endl;
	cout << "==============" << endl;
	int poly;
	cout << "输入策略选择：";
	cin >> poly;
	switch (poly) {
	case 1:
		memoryService.setPoly(substitutePoly::LRU);
		break;

	default:
		cout << "null in" << endl;
	}
}