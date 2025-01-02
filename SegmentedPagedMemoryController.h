#pragma once
#include"SegmentedPagedMemoryService.h"

class SegmentedPagedMemoryController{

private: 
	SegmentedPagedMemoryService memoryService;

public:
	SegmentedPagedMemoryController(int initSize, int blockSize);	// 236
	void disMenuAction();     //菜单
	void showAllMessageAction();  //显示信息
	void newProcessAction();  //创建新进程
	void returnProcessAction();  //返还进程
	void processMenageAction();  //进程内存申请释放访问
	void changePolyAction();   //为后面扩充做准备，修改算法
};

