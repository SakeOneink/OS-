#pragma once
#include<string>
#include<map>
#include<vector>
#include<algorithm>
#include<iostream>
using namespace std;

#define ERROR -1	// �޷�����
#define USED 1		// ����״̬ δ����
#define FREE 0		// ����״̬ δ����

typedef int Status;		// ����״̬

// �û��㷨����
enum class substitutePoly {
	LRU		// ������δʹ��ҳ���û��㷨
};

// �ڴ浥Ԫģ��
typedef struct memoryItem {
	void* data{ NULL };			// �ڴ�����ָ�� ����ָ��洢����
}memoryItem;

// �ڴ��
typedef struct blockItem {
	int blockCode;			// ���
	int startAddress;		// ��ʼ��ַ
	int blockSize;			// ������С
	Status status{ FREE };			// ����״̬ USED/FREE
	string processCode{ "" };		// ������䣬������̵�name

	// ������ֵ blockCode �Ķ�Ӧ��Ԫ
	bool operator == (const int& blockCode) {
		return (this->blockCode == blockCode);
	}
}blockItem;

// ҳ����
typedef struct pageItem {
	int pCode{ -1 };				// ҳ�� ������������
	bool mFlag{ false };			// �ڴ�״̬λ���Ƿ����ڴ��У�
	int useCount{ 0 };				// ʹ�ô���
	int lruNum{ 0 };				// LRU�����ʱ�����Ϊ�����ʣ����� ����ĳҳ������δ�����ʵ�ҳ ++ ��ϵͳ������
	int blockCode{ -1 };			// ��Ӧ���ڴ���
}pageItem;

// �α���
typedef struct segmentItem {
	int sCode{ -1 };					// �κ� ������������
	pageItem* pageStartPointer{ NULL };	
	int startPageAddress{ -1 };			// ҳ����ʼ��ַ
	int pageSize{ 0 };					// ҳ����
	bool mFlag{ false };				// �ڴ�״̬λ �Ƿ����
	bool useFlag{ false };				// ���Ƿ���ʹ��
	int useCount{ 0 };
	int lruNum{ 0 };					
}segmentItem;

// ������
typedef struct processItem {
	string pName{ "" };					// ������
	segmentItem* segmentStartPointer{ NULL };	
	int startSegmentAddress{ -1 };		// �α���ʼ��ַ
	int segmentNum{ 0 };				// ���̵Ķ������α���

	// ������ֵ processCode �Ķ�Ӧ��Ԫ
	bool operator == (const string& processCode) {
		return (this->pName == processCode);
	}
}processItem;
 
// ����������
typedef struct requestItem {
	string pName{ "" };					// ������
	int segmentNum{ 0 };				// ���̵Ķ���
	vector<int> pageNumOfSeg;			// ����ҳ�ĸ���( index �� �κŶ�Ӧ )

	// ������ֵ processCode �Ķ�Ӧ��Ԫ
	bool operator == (const string& processCode) {
		return (this->pName == processCode);
	}
}requestItem;


class SegmentedPagedMemoryService{
private:
	int totalSize;					// ���ڴ�ռ� (K)
	int blockNum;					// �ڴ���� (K)
	memoryItem* memoryTable;					// �ڴ��
	vector<blockItem> blockTable;				// �ڴ�ֿ��
	vector<processItem> processTable;			// ���̱�
	vector<requestItem> processRequestTable;	// �ڴ���Դ�����	

	map<pageItem*, int> page2Num;				// ҳ��ַ �� ��ָ̭���ӳ��
	map<segmentItem*, int> segment2Num;			// �ε�ַ �� ��ָ̭���ӳ��
	substitutePoly thisPoly;								// ѡ�����
private:
	/* ��ҳ���ڴ���� */
	int findTableAreaAddress(int tableSize);						// ���ҿɴ��size��С����ڴ�ռ���ʼ��ַ
	/* ӳ�亯�� */
	int getEliminationIndicators(pageItem* pagePointer);			// ��ȡ��/ҳ�����ж�ʱ ��̭�û������ݵ�ָ��
	int getEliminationIndicators(segmentItem* segPointer);
	void addPageMap(pageItem* pagePointer);							// pageMapӳ����� ����
	void deletePageInMap(pageItem* pagePointer);					// pageMapӳ��ɾ�� ����
	void addSegmentMap(segmentItem* segPointer);					// segmentMapӳ�����
	void deleteSegmentInMap(segmentItem* segPointer);				// segmentMapӳ��ɾ�� ����
	void flashMap();												// ˢ�±�ӳ��
	/* �ڴ���� ��̭ */
	int eliminatePage();														// ������̭�û�������̭һ��ҳ�棬���ض�Ӧ�ڴ���
	int allocateBlock(string processname);										// �����ڴ��
	int allocateSpace(string processName, int segmentNum, vector<int> pageNumOfSeg);		// Ϊ���̷���ռ䣨����Ĭ�ϵ�0��ֱ�ӵ����ڴ棩 ���ض���ʼ��ַ
	/* ����ҳģ�� LRU���� */
	void lruNumCount(pageItem* pPageItem);													// ��ָ����ָҳ���� lruNum++
	void lruNumCount(segmentItem* pSegItem);												// ��ָ����ָ���� lruNum++
	void visitPage(int segTableStartAdd, int segmentCode, int pageCode);					// ����ҳ
public:
	SegmentedPagedMemoryService();
	SegmentedPagedMemoryService(int initSize, int blockSize);				// �����ڴ��С����ռ�
	void init(int initSize, int blockNum);									// ����ͬ�вι���
	void setPoly(substitutePoly newPoly);									// ѡ���ڴ�����㷨����

	bool processCallSegment(string processName, int segmentCode);							// ���̵����¶Σ��ڴ����룩
	bool processReleaseSegment(string processName, int segmentCode);						// �����ͷ��϶Σ��ڴ��ͷţ�
	bool newProcess(string processName, int segmentNum, vector<int> pageNumOfSeg);			// �½�����
	void returnProcessByProcessName(string processName);									// ���ݽ���Name������Ӧ�ڴ�ռ�
	void runProcessPage(string processName, int segCode, int pageCode);						// ���ʳ��� �� ĳ�� �е� ĳҳ
		

	/* ��Ϣ��ʾ */
	bool showProcessMessageByName(string  processName);		// ��ʾ���̶�ҳ������Ϣ
	void showAllProcessMessage();							// ��ʾ���н�����Ϣ
	void showBlockTable();									// ��ʾ�ֿ����Ϣ
	void showAllTable();									// ��ʾ���ű����Ϣ
};

