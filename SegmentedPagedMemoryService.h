#pragma once
#include<string>
#include<map>
#include<vector>
#include<algorithm>
#include<iostream>
using namespace std;

#define ERROR -1	// 无法分配
#define USED 1		// 分区状态 未分配
#define FREE 0		// 分区状态 未分配

typedef int Status;		// 分区状态

// 置换算法策略
enum class substitutePoly {
	LRU		// 最近最久未使用页面置换算法
};

// 内存单元模拟
typedef struct memoryItem {
	void* data{ NULL };			// 内存万能指针 用于指向存储内容
}memoryItem;

// 内存块
typedef struct blockItem {
	int blockCode;			// 块号
	int startAddress;		// 起始地址
	int blockSize;			// 分区大小
	Status status{ FREE };			// 分配状态 USED/FREE
	string processCode{ "" };		// 如果分配，分配进程的name

	// 查找数值 blockCode 的对应单元
	bool operator == (const int& blockCode) {
		return (this->blockCode == blockCode);
	}
}blockItem;

// 页表项
typedef struct pageItem {
	int pCode{ -1 };				// 页号 隐含在数组中
	bool mFlag{ false };			// 内存状态位（是否在内存中）
	int useCount{ 0 };				// 使用次数
	int lruNum{ 0 };				// LRU（最近时间最久为被访问）—— 访问某页后其他未被访问的页 ++ （系统开销大）
	int blockCode{ -1 };			// 对应的内存块号
}pageItem;

// 段表项
typedef struct segmentItem {
	int sCode{ -1 };					// 段号 隐含在数组中
	pageItem* pageStartPointer{ NULL };	
	int startPageAddress{ -1 };			// 页表起始地址
	int pageSize{ 0 };					// 页表长度
	bool mFlag{ false };				// 内存状态位 是否分配
	bool useFlag{ false };				// 段是否在使用
	int useCount{ 0 };
	int lruNum{ 0 };					
}segmentItem;

// 进程项
typedef struct processItem {
	string pName{ "" };					// 进程名
	segmentItem* segmentStartPointer{ NULL };	
	int startSegmentAddress{ -1 };		// 段表起始地址
	int segmentNum{ 0 };				// 进程的段数（段表长）

	// 查找数值 processCode 的对应单元
	bool operator == (const string& processCode) {
		return (this->pName == processCode);
	}
}processItem;
 
// 进程请求项
typedef struct requestItem {
	string pName{ "" };					// 进程名
	int segmentNum{ 0 };				// 进程的段数
	vector<int> pageNumOfSeg;			// 段内页的个数( index 与 段号对应 )

	// 查找数值 processCode 的对应单元
	bool operator == (const string& processCode) {
		return (this->pName == processCode);
	}
}requestItem;


class SegmentedPagedMemoryService{
private:
	int totalSize;					// 总内存空间 (K)
	int blockNum;					// 内存块数 (K)
	memoryItem* memoryTable;					// 内存表
	vector<blockItem> blockTable;				// 内存分块表
	vector<processItem> processTable;			// 进程表
	vector<requestItem> processRequestTable;	// 内存资源请求表	

	map<pageItem*, int> page2Num;				// 页地址 和 淘汰指标的映射
	map<segmentItem*, int> segment2Num;			// 段地址 和 淘汰指标的映射
	substitutePoly thisPoly;								// 选择策略
private:
	/* 段页表内存分配 */
	int findTableAreaAddress(int tableSize);						// 查找可存放size大小表的内存空间起始地址
	/* 映射函数 */
	int getEliminationIndicators(pageItem* pagePointer);			// 获取段/页表发生中断时 淘汰置换所依据的指标
	int getEliminationIndicators(segmentItem* segPointer);
	void addPageMap(pageItem* pagePointer);							// pageMap映射添加 单个
	void deletePageInMap(pageItem* pagePointer);					// pageMap映射删除 单个
	void addSegmentMap(segmentItem* segPointer);					// segmentMap映射添加
	void deleteSegmentInMap(segmentItem* segPointer);				// segmentMap映射删除 单个
	void flashMap();												// 刷新表映射
	/* 内存分配 淘汰 */
	int eliminatePage();														// 按照淘汰置换策略淘汰一个页面，返回对应内存块号
	int allocateBlock(string processname);										// 分配内存块
	int allocateSpace(string processName, int segmentNum, vector<int> pageNumOfSeg);		// 为进程分配空间（建表、默认第0段直接调入内存） 返回段起始地址
	/* 访问页模拟 LRU计算 */
	void lruNumCount(pageItem* pPageItem);													// 除指针所指页面外 lruNum++
	void lruNumCount(segmentItem* pSegItem);												// 除指针所指段外 lruNum++
	void visitPage(int segTableStartAdd, int segmentCode, int pageCode);					// 访问页
public:
	SegmentedPagedMemoryService();
	SegmentedPagedMemoryService(int initSize, int blockSize);				// 给定内存大小分配空间
	void init(int initSize, int blockNum);									// 功能同有参构造
	void setPoly(substitutePoly newPoly);									// 选择内存分配算法策略

	bool processCallSegment(string processName, int segmentCode);							// 进程调入新段（内存申请）
	bool processReleaseSegment(string processName, int segmentCode);						// 进程释放老段（内存释放）
	bool newProcess(string processName, int segmentNum, vector<int> pageNumOfSeg);			// 新建进程
	void returnProcessByProcessName(string processName);									// 根据进程Name返还对应内存空间
	void runProcessPage(string processName, int segCode, int pageCode);						// 访问程序 的 某段 中的 某页
		

	/* 信息显示 */
	bool showProcessMessageByName(string  processName);		// 显示进程段页分配信息
	void showAllProcessMessage();							// 显示所有进程信息
	void showBlockTable();									// 显示分块表信息
	void showAllTable();									// 显示三张表的信息
};

