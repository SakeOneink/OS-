#pragma once
#include"SegmentedPagedMemoryService.h"

class SegmentedPagedMemoryController{

private: 
	SegmentedPagedMemoryService memoryService;

public:
	SegmentedPagedMemoryController(int initSize, int blockSize);	// 236
	void disMenuAction();     //�˵�
	void showAllMessageAction();  //��ʾ��Ϣ
	void newProcessAction();  //�����½���
	void returnProcessAction();  //��������
	void processMenageAction();  //�����ڴ������ͷŷ���
	void changePolyAction();   //Ϊ����������׼�����޸��㷨
};

