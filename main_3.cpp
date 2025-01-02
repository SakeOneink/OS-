
#include<iostream>
#include"SegmentedPagedMemoryController.h"
using namespace std;

#define OK 1
#define ERROR -1

int main() {
	int choice, flag = OK;
	int initSize, blockSize;
	cout << endl;
	cout << "请输入初始化内存大小和分块大小(K)：" << endl;
	cin >> initSize >> blockSize;

	SegmentedPagedMemoryController mController(initSize, blockSize);
	while (flag == OK) {
		mController.disMenuAction();  //菜单
		cout << "请输入你的操作：";
		cin >> choice;
		switch (choice) {
		case 1:
			mController.showAllMessageAction(); //显示信息
			break;
		case 2:
			mController.newProcessAction(); //新增进程
			break;
		case 3:
			mController.returnProcessAction(); //返还进程
			break;
		case 4:
			mController.processMenageAction(); //内存申请访问
			break;
		case 5:
			mController.changePolyAction();  //改变算法
			break;
		case 0:
			flag = ERROR;
			break;
		default:
			cout << "\n请输入正确的操作选项！\n" << endl;
			cin.clear();
		}
	}

	cout << "\n退出！";
	return 0;
}
