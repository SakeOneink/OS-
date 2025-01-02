#include "SegmentedPagedMemoryService.h"

SegmentedPagedMemoryService::SegmentedPagedMemoryService() {
	thisPoly = substitutePoly::LRU;
	totalSize = 0;
	blockNum = 0;
	memoryTable = NULL;
}
SegmentedPagedMemoryService::SegmentedPagedMemoryService(int initSize, int blockSize) {
	init(initSize, blockSize);
}

void SegmentedPagedMemoryService::setPoly(substitutePoly newPoly) {
	this->thisPoly = newPoly;
}

// 初始化 指定内存大小 和内存块大小
void SegmentedPagedMemoryService::init(int initSize, int blockSize) {
	thisPoly = substitutePoly::LRU;
	// 内存
	this->totalSize = initSize;//内存总空间
	memoryTable = new memoryItem[initSize * 1024];
	// 内存块表
	int blockNum = initSize / blockSize;	//块个数
	blockItem blockTemp;   //内存块
	blockTemp.blockSize = blockSize;
	for (int i = 0; i < blockNum; i++) {
		blockTemp.blockCode = i;    //编号
		blockTemp.startAddress = i * blockSize;
		blockTable.push_back(blockTemp);  //内存分块表
	}
	// 存在剩余空间
	if (blockSize * blockNum != initSize) {
		blockTemp.blockCode = blockNum;
		blockTemp.blockSize = initSize - blockSize * blockNum;
		blockTemp.startAddress = blockSize * blockNum;
		blockTable.push_back(blockTemp);
		blockNum++;
	}
	this->blockNum = blockNum;
	// 系统占用 段页表存储位置分配
	if (blockNum > 6) {
		for (int i = 0; i < 4; i++) {		// 前四块 sys
			blockTable[i].processCode = "系统占用";
			blockTable[i].status = USED;
		}
		for (int j = 0; j < 2; j++) {		// 后两块 table
			blockTable[blockNum - j - 1].processCode = "段页表存储";
			blockTable[blockNum - j - 1].status = USED;
		}
	}

}

// 查找分配段页表信息的起始地址(动态连续空间分配、最先适应) —— 在最后两个内存块中 return 起始地址
int SegmentedPagedMemoryService::findTableAreaAddress(int tableSize) {
	int tableStartAdd = blockTable[blockNum - 2].startAddress * 1024;
	for (int i = tableStartAdd; i < totalSize * 1024; i++) {		// 内存中查找
		bool flag = true;
		for (int j = 0; j < tableSize; j++) {						// index 下 后tableSize个存储单元是否为空
			if (memoryTable[i + j].data != NULL) {
				flag = false;
				break;
			}
		}
		if (flag) {
			return i;
		}
	}
	return 0;
}

// 获取段/页表发生中断时 淘汰置换所依据的指标
int SegmentedPagedMemoryService::getEliminationIndicators(pageItem* pagePointer) {
	int eliminationIndicators = 0;		// 淘汰指标
	switch (thisPoly) {
	
	case substitutePoly::LRU:
		eliminationIndicators = pagePointer->lruNum;
		break;
	}
	return eliminationIndicators;
}
int SegmentedPagedMemoryService::getEliminationIndicators(segmentItem* segPointer) {
	int eliminationIndicators = 0;		// 淘汰指标
	switch (thisPoly) {
	
	case substitutePoly::LRU:
		eliminationIndicators = segPointer->lruNum;
		break;
	}
	return eliminationIndicators;
}
// pageMap映射添加 单个
void SegmentedPagedMemoryService::addPageMap(pageItem* pagePointer) {
	page2Num[pagePointer] = getEliminationIndicators(pagePointer);
}
// pageMap映射删除 单个
void SegmentedPagedMemoryService::deletePageInMap(pageItem* pagePointer) {
	page2Num.erase(pagePointer);
}
// segmentMap映射添加
void SegmentedPagedMemoryService::addSegmentMap(segmentItem* segPointer) {
	segment2Num[segPointer] = getEliminationIndicators(segPointer);
}
// segmentMap映射删除 单个
void SegmentedPagedMemoryService::deleteSegmentInMap(segmentItem* segPointer) {
	segment2Num.erase(segPointer);
}
// 刷新表映射
void SegmentedPagedMemoryService::flashMap() {
	for (auto iter = page2Num.begin(); iter != page2Num.end(); iter++) {
		iter->second = getEliminationIndicators(iter->first);
	}
	for (auto iter = segment2Num.begin(); iter != segment2Num.end(); iter++) {
		iter->second = getEliminationIndicators(iter->first);
	}
}


// 按照淘汰置换策略淘汰某一页
int  SegmentedPagedMemoryService::eliminatePage() {
	int eliminationIndex = ERROR;		// 页对应块号
	pageItem* p = NULL;
	int num = ERROR;
	for (auto iter = page2Num.begin(); iter != page2Num.end(); iter++) {
		if (num == ERROR && iter->first->mFlag) {		// 找到第一个在内存中的页面
			num = iter->second;
			p = iter->first;
		}
		switch (thisPoly) {
		
		case substitutePoly::LRU:
			if (num < iter->second && iter->first->mFlag) {		// 在内存中且更长时间未被访问
				num = iter->second;
				p = iter->first;
			}
			break;
		
		}
	}
	if (p != NULL) {
		eliminationIndex = p->blockCode;
		p->mFlag = false;
		p->lruNum = 0;
		p->useCount = 0;
		p->blockCode = -1;
	}
	
	return eliminationIndex;
}

// 分配内存块
int SegmentedPagedMemoryService::allocateBlock(string processName) {
	// 若存在空闲，直接分配
	for (auto iter = blockTable.begin(); iter != blockTable.end(); iter++) {
		if (iter->status == FREE) {
			iter->processCode = processName;
			iter->status = USED;
			return iter->blockCode;
		}
	}
	// 不存在，按淘汰策略淘汰一个页面，返回对应块号
	int eliminateIndex = eliminatePage();
	if (eliminateIndex != ERROR) {
		blockTable[eliminateIndex].processCode = processName;
	}
	return eliminateIndex;
}

// 为进程分配空间 返回段起始地址
int SegmentedPagedMemoryService::allocateSpace(string processName, int segmentNum, vector<int> pageNumOfSeg) {

	// 新建进程实例项
	processItem processTemp;
	processTemp.pName = processName;
	processTemp.segmentStartPointer = new segmentItem[segmentNum];
	processTemp.startSegmentAddress = findTableAreaAddress(segmentNum);
	for (int m = 0; m < segmentNum; m++) {
		// 内存表中指向对应表单元s
		memoryTable[processTemp.startSegmentAddress + m].data = &processTemp.segmentStartPointer[m];
	}
	processTemp.segmentNum = segmentNum;
	// 段表载入
	segmentItem* segmentTable = processTemp.segmentStartPointer;
	pageItem* pageTable;
	for (int i = 0; i < segmentNum; i++) {
		segmentTable[i].sCode = i;
		segmentTable[i].pageStartPointer = new pageItem[pageNumOfSeg[i]];
		segmentTable[i].startPageAddress = findTableAreaAddress(pageNumOfSeg[i]);
		for (int n = 0; n < pageNumOfSeg[i]; n++) {
			memoryTable[segmentTable[i].startPageAddress + n].data = &segmentTable[i].pageStartPointer[n];   // 内存表中指向对应表单元
		}
		segmentTable[i].pageSize = pageNumOfSeg[i];
		addSegmentMap(&segmentTable[i]);
		// 页表载入
		pageTable = segmentTable[i].pageStartPointer;
		for (int j = 0; j < segmentTable[i].pageSize; j++) {
			pageTable[j].pCode = j;
			addPageMap(&pageTable[j]);
		}
	}
	// 加入已分配进程表
	processTable.push_back(processTemp);

	processCallSegment(processName, 0);		// 默认把 0 号程序段调入内存
	return processTemp.startSegmentAddress;
}

// 除指针所指段外 lruNum++
void SegmentedPagedMemoryService::lruNumCount(segmentItem* pSegItem) {
	for (auto iter = segment2Num.begin(); iter != segment2Num.end(); iter++) {
		if (iter->first == pSegItem) {
			iter->first->lruNum = 0;									// 对访问的段清 0
		}
		else if (iter->first->mFlag) {			// 对其他未访问且在内存中的段 lruNum++
			iter->first->lruNum++;
		}
	}
	flashMap();
}
// 除指针所指页面外 lruNum++
void SegmentedPagedMemoryService::lruNumCount(pageItem* pPageItem) {
	for (auto iter = page2Num.begin(); iter != page2Num.end(); iter++) {
		if (iter->first == pPageItem) {
			iter->first->lruNum = 0;									// 对访问的页面清 0
		}
		else if (iter->first->mFlag) {			// 对其他未访问且在内存中的段 lruNum++
			iter->first->lruNum++;
		}
	}
	flashMap();
}													

// 访问页 input:段起始地址 ，段号，页号
void SegmentedPagedMemoryService::visitPage(int segTableStartAdd, int segmentCode, int pageCode) {
	segmentItem* pSegItem = (segmentItem*)memoryTable[segTableStartAdd + segmentCode].data;
	pSegItem->useCount++;
	lruNumCount(pSegItem);
	int pageTableStartAdd = pSegItem->startPageAddress;
	pageItem* pPageItem = (pageItem*)memoryTable[pageTableStartAdd + pageCode].data;
	pPageItem->useCount++;												// 当前页 uesCount++
	lruNumCount(pPageItem);												// lruNum
}

// 访问程序 的 某段 中的 某页
void SegmentedPagedMemoryService::runProcessPage(string processName, int segCode, int pageCode) {
	auto iter = find(processTable.begin(), processTable.end(), processName);
	if (iter != processTable.end()) {
		if (segCode >= 0 && segCode < iter->segmentNum) {
			// 访问内存中的段表
			segmentItem* pSegItem = (segmentItem*)memoryTable[iter->startSegmentAddress + segCode].data;		
			if (!pSegItem->mFlag) {		// 若段不在内存中 则调入段
				processCallSegment(processName, segCode);
			}
			if (pageCode >= 0 && pageCode < pSegItem->pageSize) {
				// 访问内存中的页表
				pageItem* pPageItem = (pageItem*)memoryTable[pSegItem->startPageAddress + pageCode].data;
				if (!pPageItem->mFlag) {			// 若页面不在内存 则调入/置换淘汰
					pPageItem->blockCode = allocateBlock(processName);
					pPageItem->mFlag = true;
				}
				visitPage(iter->startSegmentAddress, segCode, pageCode);
			}
			else {
				cout << "error : 越界中断" << endl;
				return;
			}
		}
		else {
			cout << "error : 越界中断" << endl;
			return;
		}
	}
	else {
		cout << "err :不存在进程" << endl;
		return;
	}
}

						

// 进程调入新段（内存申请）
bool SegmentedPagedMemoryService::processCallSegment(string processName, int segmentCode) {
	auto iter = find(processTable.begin(), processTable.end(), processName);
	if (iter != processTable.end()) {
		// 程序段表
		segmentItem* segmentTable = iter->segmentStartPointer;
		// 段号对应页表内存分配
		if (segmentCode >= 0 && segmentCode < iter->segmentNum) {
			pageItem* pageTable = segmentTable[segmentCode].pageStartPointer;
			// 对页表下的每个页面进行内存分配
			for (int i = 0; i < segmentTable[segmentCode].pageSize; i++) {
				if ((pageTable[i].blockCode = allocateBlock(processName)) != ERROR) {
					pageTable[i].mFlag = true;
					pageTable[i].useCount++;		// 页面访问记录
					lruNumCount(&pageTable[i]);			
				}
				else {
					return false;
				}
			}
			segmentTable[segmentCode].mFlag = true;
			segmentTable[segmentCode].useFlag = true;
			segmentTable[segmentCode].useCount++;
			lruNumCount(&segmentTable[segmentCode]);
		}
		else {
			cout << "err : 越界中断" << endl;
			return false;
		}
	}
	else {
		cout << "err :进程不存在" << endl;
		return true;
	}
}
// 进程释放老段（内存释放）
bool SegmentedPagedMemoryService::processReleaseSegment(string processName, int segmentCode) {
	auto iter = find(processTable.begin(), processTable.end(), processName);
	if (iter != processTable.end()) {
		segmentItem* segmentTable = iter->segmentStartPointer;
		// 段对应页表内存释放
		if (segmentCode >= 0 && segmentCode < iter->segmentNum) {
			pageItem* pageTable = segmentTable[segmentCode].pageStartPointer;
			for (int i = 0; i < segmentTable[segmentCode].pageSize; i++) {
				if (pageTable[i].blockCode != -1) {
					blockTable[pageTable[i].blockCode].status = FREE;
					blockTable[pageTable[i].blockCode].processCode = "";
				}
				pageTable[i].blockCode = -1;
				pageTable[i].lruNum = 0;
				pageTable[i].useCount = 0;
				pageTable[i].mFlag = false;
			}
			segmentTable[segmentCode].mFlag = false;
			segmentTable[segmentCode].useFlag = false;
			// segmentTable[segmentCode].useCount++;
		}
		else {
			cout << "err : 越界中断" << endl;
			return false;
		}
		flashMap();
		return true;
	}
	else {
		cout << "err :不存在进程" << endl;
		return false;
	}
}

// 新建进程 input：进程名， 进程段数， 每段对应页数
bool SegmentedPagedMemoryService::newProcess(string processName, int segmentNum, vector<int> pageNumOfSeg) {
	// 加入内存资源请求表
	requestItem reItem = { processName, segmentNum, pageNumOfSeg };
	processRequestTable.push_back(reItem);

	if ((allocateSpace(processName, segmentNum, pageNumOfSeg)) != ERROR) {
		// 进程得到分配，从资源请求表中移除
		processRequestTable.erase(find(processRequestTable.begin(), processRequestTable.end(), processName));
		return true;
	}
	return false;
}

// 根据进程Name返还对应内存空间
void SegmentedPagedMemoryService::returnProcessByProcessName(string processName) {
	auto iter = find(processTable.begin(), processTable.end(), processName);
	if (iter != processTable.end()) {
		// 返还程序段内存空间
		for (int i = 0; i < iter->segmentNum; i++) {
			processReleaseSegment(processName, i);
		}
		// 去除地址映射 销毁表
		segmentItem* segmentTable = iter->segmentStartPointer;
		pageItem* pageTable;
		for (int i = 0; i < iter->segmentNum; i++) {
			pageTable = segmentTable[i].pageStartPointer;
			for (int j = 0; j < segmentTable[i].pageSize; j++) {
				memoryTable[segmentTable[i].startPageAddress + j].data = NULL;		// 内存中页表单元数据依次清空
				deletePageInMap(&pageTable[j]);
			}

			memoryTable[iter->startSegmentAddress + i].data = NULL;		// 内存中段表单元数据依次清空
			deleteSegmentInMap(&segmentTable[i]);
		}
		processTable.erase(iter);

		// 为请求表中尚未分配的进程尝试分配
		for (vector<requestItem>::iterator iter = processRequestTable.begin(); iter != processRequestTable.end();) {
			if (allocateSpace(iter->pName, iter->segmentNum, iter->pageNumOfSeg) != ERROR) {
				// 进程得到分配，从资源请求表中移除
				iter = processRequestTable.erase(iter);
			}
			else {
				iter++;
			}
		}
	}
}				



// 显示进程段页分配信息
bool SegmentedPagedMemoryService::showProcessMessageByName(string  processName) {
	auto iter = find(processTable.begin(), processTable.end(), processName);
	if (iter != processTable.end()) {
		cout << "进程" << iter->pName << "的段页表：" << endl;
		// segmentItem* segmentTable = (segmentItem*)memoryTable[iter->startSegmentAddress].data;
		segmentItem* segmentTable = iter->segmentStartPointer;
		cout << "段号\t" << "页表起始地址\t" << "页表长度\t"  << endl;
		for (int i = 0; i < iter->segmentNum; i++) {
			cout << segmentTable[i].sCode << "\t" << segmentTable[i].startPageAddress << "\t\t" 
				<< segmentTable[i].pageSize << endl;
		}
		// 输出页表
		for (int i = 0; i < iter->segmentNum; i++) {
			cout << "段号" << i << "对应页表：" << endl;
			// pageItem* pageTable = (pageItem*)memoryTable[iter->first].data;
			pageItem* pageTable = segmentTable[i].pageStartPointer;
			cout << "页号\t" << "内存状态位\t" << "useCount\t" << "LRUNum\t\t" << "内存块号\t" << endl;
			for (int j = 0; j < segmentTable[i].pageSize; j++) {
				cout << pageTable[j].pCode << "\t" << (pageTable[j].mFlag?"内存":"外存") << "\t\t"
					<< pageTable[j].useCount << "\t\t" << pageTable[j].lruNum << "\t\t" << pageTable[j].blockCode << endl;
			}
		}
		return true;
	}
	return false;
}		
// 显示所有进程信息
void SegmentedPagedMemoryService::showAllProcessMessage() {
	for (auto iter = processTable.begin(); iter != processTable.end(); iter++) {
		showProcessMessageByName(iter->pName);
	}
}			

// 显示内存分块表信息
void SegmentedPagedMemoryService::showBlockTable() {
	cout << "内存分块表：\n"
		<< "块号\t" << "起始地址(K)\t" << "分区大小(K)\t" << "分配状态\t" << "分配进程Name" << endl;
	for (vector<blockItem>::iterator iter = blockTable.begin(); iter != blockTable.end(); iter++) {
		cout << iter->blockCode << "\t" << iter->startAddress << "\t\t" << iter->blockSize << "\t\t"
			<< ((iter->status == USED) ? "已分配" : "未分配") << "\t\t" << iter->processCode << endl;
	}
}

// 显示所有信息
void SegmentedPagedMemoryService::showAllTable() {
	showBlockTable();
	showAllProcessMessage();
}								