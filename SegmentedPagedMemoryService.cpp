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

// ��ʼ�� ָ���ڴ��С ���ڴ���С
void SegmentedPagedMemoryService::init(int initSize, int blockSize) {
	thisPoly = substitutePoly::LRU;
	// �ڴ�
	this->totalSize = initSize;//�ڴ��ܿռ�
	memoryTable = new memoryItem[initSize * 1024];
	// �ڴ���
	int blockNum = initSize / blockSize;	//�����
	blockItem blockTemp;   //�ڴ��
	blockTemp.blockSize = blockSize;
	for (int i = 0; i < blockNum; i++) {
		blockTemp.blockCode = i;    //���
		blockTemp.startAddress = i * blockSize;
		blockTable.push_back(blockTemp);  //�ڴ�ֿ��
	}
	// ����ʣ��ռ�
	if (blockSize * blockNum != initSize) {
		blockTemp.blockCode = blockNum;
		blockTemp.blockSize = initSize - blockSize * blockNum;
		blockTemp.startAddress = blockSize * blockNum;
		blockTable.push_back(blockTemp);
		blockNum++;
	}
	this->blockNum = blockNum;
	// ϵͳռ�� ��ҳ��洢λ�÷���
	if (blockNum > 6) {
		for (int i = 0; i < 4; i++) {		// ǰ�Ŀ� sys
			blockTable[i].processCode = "ϵͳռ��";
			blockTable[i].status = USED;
		}
		for (int j = 0; j < 2; j++) {		// ������ table
			blockTable[blockNum - j - 1].processCode = "��ҳ��洢";
			blockTable[blockNum - j - 1].status = USED;
		}
	}

}

// ���ҷ����ҳ����Ϣ����ʼ��ַ(��̬�����ռ���䡢������Ӧ) ���� ����������ڴ���� return ��ʼ��ַ
int SegmentedPagedMemoryService::findTableAreaAddress(int tableSize) {
	int tableStartAdd = blockTable[blockNum - 2].startAddress * 1024;
	for (int i = tableStartAdd; i < totalSize * 1024; i++) {		// �ڴ��в���
		bool flag = true;
		for (int j = 0; j < tableSize; j++) {						// index �� ��tableSize���洢��Ԫ�Ƿ�Ϊ��
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

// ��ȡ��/ҳ�����ж�ʱ ��̭�û������ݵ�ָ��
int SegmentedPagedMemoryService::getEliminationIndicators(pageItem* pagePointer) {
	int eliminationIndicators = 0;		// ��ָ̭��
	switch (thisPoly) {
	
	case substitutePoly::LRU:
		eliminationIndicators = pagePointer->lruNum;
		break;
	}
	return eliminationIndicators;
}
int SegmentedPagedMemoryService::getEliminationIndicators(segmentItem* segPointer) {
	int eliminationIndicators = 0;		// ��ָ̭��
	switch (thisPoly) {
	
	case substitutePoly::LRU:
		eliminationIndicators = segPointer->lruNum;
		break;
	}
	return eliminationIndicators;
}
// pageMapӳ����� ����
void SegmentedPagedMemoryService::addPageMap(pageItem* pagePointer) {
	page2Num[pagePointer] = getEliminationIndicators(pagePointer);
}
// pageMapӳ��ɾ�� ����
void SegmentedPagedMemoryService::deletePageInMap(pageItem* pagePointer) {
	page2Num.erase(pagePointer);
}
// segmentMapӳ�����
void SegmentedPagedMemoryService::addSegmentMap(segmentItem* segPointer) {
	segment2Num[segPointer] = getEliminationIndicators(segPointer);
}
// segmentMapӳ��ɾ�� ����
void SegmentedPagedMemoryService::deleteSegmentInMap(segmentItem* segPointer) {
	segment2Num.erase(segPointer);
}
// ˢ�±�ӳ��
void SegmentedPagedMemoryService::flashMap() {
	for (auto iter = page2Num.begin(); iter != page2Num.end(); iter++) {
		iter->second = getEliminationIndicators(iter->first);
	}
	for (auto iter = segment2Num.begin(); iter != segment2Num.end(); iter++) {
		iter->second = getEliminationIndicators(iter->first);
	}
}


// ������̭�û�������̭ĳһҳ
int  SegmentedPagedMemoryService::eliminatePage() {
	int eliminationIndex = ERROR;		// ҳ��Ӧ���
	pageItem* p = NULL;
	int num = ERROR;
	for (auto iter = page2Num.begin(); iter != page2Num.end(); iter++) {
		if (num == ERROR && iter->first->mFlag) {		// �ҵ���һ�����ڴ��е�ҳ��
			num = iter->second;
			p = iter->first;
		}
		switch (thisPoly) {
		
		case substitutePoly::LRU:
			if (num < iter->second && iter->first->mFlag) {		// ���ڴ����Ҹ���ʱ��δ������
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

// �����ڴ��
int SegmentedPagedMemoryService::allocateBlock(string processName) {
	// �����ڿ��У�ֱ�ӷ���
	for (auto iter = blockTable.begin(); iter != blockTable.end(); iter++) {
		if (iter->status == FREE) {
			iter->processCode = processName;
			iter->status = USED;
			return iter->blockCode;
		}
	}
	// �����ڣ�����̭������̭һ��ҳ�棬���ض�Ӧ���
	int eliminateIndex = eliminatePage();
	if (eliminateIndex != ERROR) {
		blockTable[eliminateIndex].processCode = processName;
	}
	return eliminateIndex;
}

// Ϊ���̷���ռ� ���ض���ʼ��ַ
int SegmentedPagedMemoryService::allocateSpace(string processName, int segmentNum, vector<int> pageNumOfSeg) {

	// �½�����ʵ����
	processItem processTemp;
	processTemp.pName = processName;
	processTemp.segmentStartPointer = new segmentItem[segmentNum];
	processTemp.startSegmentAddress = findTableAreaAddress(segmentNum);
	for (int m = 0; m < segmentNum; m++) {
		// �ڴ����ָ���Ӧ��Ԫs
		memoryTable[processTemp.startSegmentAddress + m].data = &processTemp.segmentStartPointer[m];
	}
	processTemp.segmentNum = segmentNum;
	// �α�����
	segmentItem* segmentTable = processTemp.segmentStartPointer;
	pageItem* pageTable;
	for (int i = 0; i < segmentNum; i++) {
		segmentTable[i].sCode = i;
		segmentTable[i].pageStartPointer = new pageItem[pageNumOfSeg[i]];
		segmentTable[i].startPageAddress = findTableAreaAddress(pageNumOfSeg[i]);
		for (int n = 0; n < pageNumOfSeg[i]; n++) {
			memoryTable[segmentTable[i].startPageAddress + n].data = &segmentTable[i].pageStartPointer[n];   // �ڴ����ָ���Ӧ��Ԫ
		}
		segmentTable[i].pageSize = pageNumOfSeg[i];
		addSegmentMap(&segmentTable[i]);
		// ҳ������
		pageTable = segmentTable[i].pageStartPointer;
		for (int j = 0; j < segmentTable[i].pageSize; j++) {
			pageTable[j].pCode = j;
			addPageMap(&pageTable[j]);
		}
	}
	// �����ѷ�����̱�
	processTable.push_back(processTemp);

	processCallSegment(processName, 0);		// Ĭ�ϰ� 0 �ų���ε����ڴ�
	return processTemp.startSegmentAddress;
}

// ��ָ����ָ���� lruNum++
void SegmentedPagedMemoryService::lruNumCount(segmentItem* pSegItem) {
	for (auto iter = segment2Num.begin(); iter != segment2Num.end(); iter++) {
		if (iter->first == pSegItem) {
			iter->first->lruNum = 0;									// �Է��ʵĶ��� 0
		}
		else if (iter->first->mFlag) {			// ������δ���������ڴ��еĶ� lruNum++
			iter->first->lruNum++;
		}
	}
	flashMap();
}
// ��ָ����ָҳ���� lruNum++
void SegmentedPagedMemoryService::lruNumCount(pageItem* pPageItem) {
	for (auto iter = page2Num.begin(); iter != page2Num.end(); iter++) {
		if (iter->first == pPageItem) {
			iter->first->lruNum = 0;									// �Է��ʵ�ҳ���� 0
		}
		else if (iter->first->mFlag) {			// ������δ���������ڴ��еĶ� lruNum++
			iter->first->lruNum++;
		}
	}
	flashMap();
}													

// ����ҳ input:����ʼ��ַ ���κţ�ҳ��
void SegmentedPagedMemoryService::visitPage(int segTableStartAdd, int segmentCode, int pageCode) {
	segmentItem* pSegItem = (segmentItem*)memoryTable[segTableStartAdd + segmentCode].data;
	pSegItem->useCount++;
	lruNumCount(pSegItem);
	int pageTableStartAdd = pSegItem->startPageAddress;
	pageItem* pPageItem = (pageItem*)memoryTable[pageTableStartAdd + pageCode].data;
	pPageItem->useCount++;												// ��ǰҳ uesCount++
	lruNumCount(pPageItem);												// lruNum
}

// ���ʳ��� �� ĳ�� �е� ĳҳ
void SegmentedPagedMemoryService::runProcessPage(string processName, int segCode, int pageCode) {
	auto iter = find(processTable.begin(), processTable.end(), processName);
	if (iter != processTable.end()) {
		if (segCode >= 0 && segCode < iter->segmentNum) {
			// �����ڴ��еĶα�
			segmentItem* pSegItem = (segmentItem*)memoryTable[iter->startSegmentAddress + segCode].data;		
			if (!pSegItem->mFlag) {		// ���β����ڴ��� ������
				processCallSegment(processName, segCode);
			}
			if (pageCode >= 0 && pageCode < pSegItem->pageSize) {
				// �����ڴ��е�ҳ��
				pageItem* pPageItem = (pageItem*)memoryTable[pSegItem->startPageAddress + pageCode].data;
				if (!pPageItem->mFlag) {			// ��ҳ�治���ڴ� �����/�û���̭
					pPageItem->blockCode = allocateBlock(processName);
					pPageItem->mFlag = true;
				}
				visitPage(iter->startSegmentAddress, segCode, pageCode);
			}
			else {
				cout << "error : Խ���ж�" << endl;
				return;
			}
		}
		else {
			cout << "error : Խ���ж�" << endl;
			return;
		}
	}
	else {
		cout << "err :�����ڽ���" << endl;
		return;
	}
}

						

// ���̵����¶Σ��ڴ����룩
bool SegmentedPagedMemoryService::processCallSegment(string processName, int segmentCode) {
	auto iter = find(processTable.begin(), processTable.end(), processName);
	if (iter != processTable.end()) {
		// ����α�
		segmentItem* segmentTable = iter->segmentStartPointer;
		// �κŶ�Ӧҳ���ڴ����
		if (segmentCode >= 0 && segmentCode < iter->segmentNum) {
			pageItem* pageTable = segmentTable[segmentCode].pageStartPointer;
			// ��ҳ���µ�ÿ��ҳ������ڴ����
			for (int i = 0; i < segmentTable[segmentCode].pageSize; i++) {
				if ((pageTable[i].blockCode = allocateBlock(processName)) != ERROR) {
					pageTable[i].mFlag = true;
					pageTable[i].useCount++;		// ҳ����ʼ�¼
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
			cout << "err : Խ���ж�" << endl;
			return false;
		}
	}
	else {
		cout << "err :���̲�����" << endl;
		return true;
	}
}
// �����ͷ��϶Σ��ڴ��ͷţ�
bool SegmentedPagedMemoryService::processReleaseSegment(string processName, int segmentCode) {
	auto iter = find(processTable.begin(), processTable.end(), processName);
	if (iter != processTable.end()) {
		segmentItem* segmentTable = iter->segmentStartPointer;
		// �ζ�Ӧҳ���ڴ��ͷ�
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
			cout << "err : Խ���ж�" << endl;
			return false;
		}
		flashMap();
		return true;
	}
	else {
		cout << "err :�����ڽ���" << endl;
		return false;
	}
}

// �½����� input���������� ���̶����� ÿ�ζ�Ӧҳ��
bool SegmentedPagedMemoryService::newProcess(string processName, int segmentNum, vector<int> pageNumOfSeg) {
	// �����ڴ���Դ�����
	requestItem reItem = { processName, segmentNum, pageNumOfSeg };
	processRequestTable.push_back(reItem);

	if ((allocateSpace(processName, segmentNum, pageNumOfSeg)) != ERROR) {
		// ���̵õ����䣬����Դ��������Ƴ�
		processRequestTable.erase(find(processRequestTable.begin(), processRequestTable.end(), processName));
		return true;
	}
	return false;
}

// ���ݽ���Name������Ӧ�ڴ�ռ�
void SegmentedPagedMemoryService::returnProcessByProcessName(string processName) {
	auto iter = find(processTable.begin(), processTable.end(), processName);
	if (iter != processTable.end()) {
		// ����������ڴ�ռ�
		for (int i = 0; i < iter->segmentNum; i++) {
			processReleaseSegment(processName, i);
		}
		// ȥ����ַӳ�� ���ٱ�
		segmentItem* segmentTable = iter->segmentStartPointer;
		pageItem* pageTable;
		for (int i = 0; i < iter->segmentNum; i++) {
			pageTable = segmentTable[i].pageStartPointer;
			for (int j = 0; j < segmentTable[i].pageSize; j++) {
				memoryTable[segmentTable[i].startPageAddress + j].data = NULL;		// �ڴ���ҳ��Ԫ�����������
				deletePageInMap(&pageTable[j]);
			}

			memoryTable[iter->startSegmentAddress + i].data = NULL;		// �ڴ��жα�Ԫ�����������
			deleteSegmentInMap(&segmentTable[i]);
		}
		processTable.erase(iter);

		// Ϊ���������δ����Ľ��̳��Է���
		for (vector<requestItem>::iterator iter = processRequestTable.begin(); iter != processRequestTable.end();) {
			if (allocateSpace(iter->pName, iter->segmentNum, iter->pageNumOfSeg) != ERROR) {
				// ���̵õ����䣬����Դ��������Ƴ�
				iter = processRequestTable.erase(iter);
			}
			else {
				iter++;
			}
		}
	}
}				



// ��ʾ���̶�ҳ������Ϣ
bool SegmentedPagedMemoryService::showProcessMessageByName(string  processName) {
	auto iter = find(processTable.begin(), processTable.end(), processName);
	if (iter != processTable.end()) {
		cout << "����" << iter->pName << "�Ķ�ҳ��" << endl;
		// segmentItem* segmentTable = (segmentItem*)memoryTable[iter->startSegmentAddress].data;
		segmentItem* segmentTable = iter->segmentStartPointer;
		cout << "�κ�\t" << "ҳ����ʼ��ַ\t" << "ҳ����\t"  << endl;
		for (int i = 0; i < iter->segmentNum; i++) {
			cout << segmentTable[i].sCode << "\t" << segmentTable[i].startPageAddress << "\t\t" 
				<< segmentTable[i].pageSize << endl;
		}
		// ���ҳ��
		for (int i = 0; i < iter->segmentNum; i++) {
			cout << "�κ�" << i << "��Ӧҳ��" << endl;
			// pageItem* pageTable = (pageItem*)memoryTable[iter->first].data;
			pageItem* pageTable = segmentTable[i].pageStartPointer;
			cout << "ҳ��\t" << "�ڴ�״̬λ\t" << "useCount\t" << "LRUNum\t\t" << "�ڴ���\t" << endl;
			for (int j = 0; j < segmentTable[i].pageSize; j++) {
				cout << pageTable[j].pCode << "\t" << (pageTable[j].mFlag?"�ڴ�":"���") << "\t\t"
					<< pageTable[j].useCount << "\t\t" << pageTable[j].lruNum << "\t\t" << pageTable[j].blockCode << endl;
			}
		}
		return true;
	}
	return false;
}		
// ��ʾ���н�����Ϣ
void SegmentedPagedMemoryService::showAllProcessMessage() {
	for (auto iter = processTable.begin(); iter != processTable.end(); iter++) {
		showProcessMessageByName(iter->pName);
	}
}			

// ��ʾ�ڴ�ֿ����Ϣ
void SegmentedPagedMemoryService::showBlockTable() {
	cout << "�ڴ�ֿ��\n"
		<< "���\t" << "��ʼ��ַ(K)\t" << "������С(K)\t" << "����״̬\t" << "�������Name" << endl;
	for (vector<blockItem>::iterator iter = blockTable.begin(); iter != blockTable.end(); iter++) {
		cout << iter->blockCode << "\t" << iter->startAddress << "\t\t" << iter->blockSize << "\t\t"
			<< ((iter->status == USED) ? "�ѷ���" : "δ����") << "\t\t" << iter->processCode << endl;
	}
}

// ��ʾ������Ϣ
void SegmentedPagedMemoryService::showAllTable() {
	showBlockTable();
	showAllProcessMessage();
}								