#ifndef FAMILY_TREE_H
#define FAMILY_TREE_H

#include <iostream>
#include <fstream>

using namespace std;

// 写在前面: 鉴于尽量避免使用STL容器, 通过底层代码实现存储结构的项目要求, 源程序文件中采用纯指针配合自定义链表实现树结构

// 字符串操作
int StrCompare(const char* a, const char* b);      
bool StrEqual(const char* a, const char* b);       
void StrCopy(char* dest, const char* src, int MaxLen); 

struct ChildNode;
class Person;       //前置声明

// 孩子链表节点 (用单链表存储每个Person的孩子)
struct ChildNode {
    Person* child;           // 指向孩子结点的指针
    ChildNode* next;         // 指向当前孩子结点下一个兄弟结点的指针
    ChildNode(Person* c) : child(c), next(nullptr) {}
};

// Person类 
class Person {
public:
    int id;                  // 编号 (对用户透明)
    char name[50];           // 姓名
    char BirthDate[20];      // 出生日期
    char marriage[20];       // 婚姻状况
    char address[50];        // 地址
    char status[20];         // 目前状况
    char DeathDate[20];      // 死亡日期 (若该Person对象已死亡)

    // 以上信息均采用字符数组表示

    Person* father;          // 指向父亲结点的指针
    ChildNode* ChildHead;    // 孩子链表头结点 (带头结点, ChildHead->next指向第一个孩子)
    int ChildCount;          // 孩子结点的数量

    // 构造函数
    Person(const char* n, const char* b, const char* m, 
           const char* a, const char* s, const char* d);

    // 获取代数
    int GetGeneration() const;

    // 获取所有后代的信息 (通过深度优先搜索实现, 将结果存入数组)
    // 注意: 由于需要计数故采用引用传参传递Count
    void GetAllDescendants(Person** result, int& count, int MaxSize) const;

    // 判断是否拥有后代
    bool HasDescendants() const;

    // 获取孩子链表
    ChildNode* GetChildList() const;

    // 按出生日期插入结点
    void InsertChildSorted(Person* NewChild);

    // 从孩子链表中移除指定孩子
    void RemoveChild(Person* target);

    // 获取兄弟数量
    int GetSiblingCount() const;
};

// FamilyTree类
class FamilyTree {
private:
    Person* root;                      // 根节点
    Person* people[1000];              // 所有成员按id存储, id从1开始
    int PersonCount;                   // 总人数

    void ClearTree(Person* node);      // 释放整棵树
    void DisplayTreeHelper(Person* node, int depth) const;  // 显示整棵树的信息

    // 外层封装: 按姓名收集所有匹配的id (线性扫描)
    void FindIdByName(const char* name, int* id, int& count) const;

    // 交互层: 重名时弹出菜单, 返回用户选择的Person*
    Person* SelectPersonByName(const char* name) const;

    // 寻找两个Person结点的最近公共祖先
    Person* FindLCA(Person* a, Person* b) const;

    // 判断一个Person对象是否是另一个Person对象的祖先
    bool IsAncestor(Person* ancestor, Person* descendant) const;

    // 判断两个Person对象的具体关系
    void GetRelationString(Person* a, Person* b, char* buffer, int BufSize) const;

    // 比较两个Person对象的年龄大小
    static bool BirthEarlier(Person* a, Person* b);

public:
    FamilyTree();
    ~FamilyTree();

    // 系统功能

    // (1) 建立初始家谱树
    void BuildFromFile(const char* FileName);

    // (2) 显示家谱树
    void DisplayTree() const;

    // (3) 通过广度优先搜索显示第n代的人数及姓名
    void ShowGeneration(int n) const;

    // (4) 根据姓名查询并输出本人及其父亲, 孩子, 代数的信息
    void QueryByName(const char* name) const;

    // (5) 给某人添加孩子
    void AddChild(const char* FatherName, const char* name,
                  const char* birth, const char* marriage,
                  const char* address, const char* status,
                  const char* DeathDate);

    // (6) 将某人移出家谱 (遵循后代过继的规则)
    void DeletePerson(const char* name);

    // (7) 修改个人信息
    void ModifyPerson(const char* name, const char* NewBirthDate,
                  const char* NewMarriage, const char* NewAddress,
                  const char* NewStatus, const char* NewDeathDate);  //避免参数名与Person类成员变量完全一致

    // (8) 直系/旁系判断
    void CheckDirectOrCollateral(const char* name1, const char* name2) const;

    // 挑战性问题

    // (1) 输出二人的最近共同祖先
    void FindCommonAncestor(const char* name1, const char* name2) const;

    // (2) 确定二人的具体亲缘关系
    void ShowDetailedRelation(const char* name1, const char* name2) const;

    // 保存到文件
    void SaveToFile(const char* FileName) const;
};

#endif