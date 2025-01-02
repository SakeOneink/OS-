
#include<iostream>
#include"SegmentedPagedMemoryController.h"
using namespace std;

#define OK 1
#define ERROR -1

int main() {
	int choice, flag = OK;
	int initSize, blockSize;
	cout << endl;
	cout << "�������ʼ���ڴ��С�ͷֿ��С(K)��" << endl;
	cin >> initSize >> blockSize;

	SegmentedPagedMemoryController mController(initSize, blockSize);
	while (flag == OK) {
		mController.disMenuAction();  //�˵�
		cout << "��������Ĳ�����";
		cin >> choice;
		switch (choice) {
		case 1:
			mController.showAllMessageAction(); //��ʾ��Ϣ
			break;
		case 2:
			mController.newProcessAction(); //��������
			break;
		case 3:
			mController.returnProcessAction(); //��������
			break;
		case 4:
			mController.processMenageAction(); //�ڴ��������
			break;
		case 5:
			mController.changePolyAction();  //�ı��㷨
			break;
		case 0:
			flag = ERROR;
			break;
		default:
			cout << "\n��������ȷ�Ĳ���ѡ�\n" << endl;
			cin.clear();
		}
	}

	cout << "\n�˳���";
	return 0;
}
