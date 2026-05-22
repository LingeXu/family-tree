#include "FamilyTree.h"

// 字符串工具实现
int StrCompare(const char* a, const char* b) {
    while (*a && *b && *a == *b) { a++; b++; }
    if (*a < *b) return -1;
    if (*a > *b) return 1;
    return 0;
}

bool StrEqual(const char* a, const char* b) {
    return StrCompare(a, b) == 0;
}

void StrCopy(char* dest, const char* src, int MaxLen) {
    int i = 0;
    while (src[i] && i < MaxLen - 1) {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
}

// Person类成员函数的实现
Person::Person(const char* n, const char* b, const char* m,
               const char* a, const char* s, const char* d)
    : id(0), father(nullptr), ChildHead(nullptr), ChildCount(0) {
    StrCopy(name, n, 50);
    StrCopy(BirthDate, b, 20);
    StrCopy(marriage, m, 20);
    StrCopy(address, a, 50);
    StrCopy(status, s, 20);
    StrCopy(DeathDate, d, 20);

    // 创建头结点
    ChildHead = new ChildNode(nullptr);
}

// 获取代数: 递归向上追溯到根节点
int Person::GetGeneration() const {
    if (father == nullptr) return 1;
    return father->GetGeneration() + 1;
}

// 深度优先搜索获取所有后代
void Person::GetAllDescendants(Person** result, int& count, int MaxSize) const {
    ChildNode* cur = ChildHead->next;
    while (cur != nullptr && count < MaxSize) {
        result[count++] = cur->child;
        cur->child->GetAllDescendants(result, count, MaxSize);
        cur = cur->next;
    }
}

bool Person::HasDescendants() const {
    return ChildHead->next != nullptr;
}

ChildNode* Person::GetChildList() const {
    return ChildHead;
}

// 按出生日期有序插入孩子链表 (不使用STL sort)
void Person::InsertChildSorted(Person* NewChild) {
    NewChild->father = this;
    ChildCount++;

    ChildNode* prev = ChildHead;
    ChildNode* cur = ChildHead->next;

    // 找到第一个出生日期 >= NewChild的位置, 插入
    while (cur != nullptr && StrCompare(cur->child->BirthDate, NewChild->BirthDate) < 0) {
        prev = cur;
        cur = cur->next;
    }

    ChildNode* NewNode = new ChildNode(NewChild);
    NewNode->next = cur;
    prev->next = NewNode;
}

// 从孩子链表中移除指定孩子
void Person::RemoveChild(Person* target) {
    ChildNode* prev = ChildHead;
    ChildNode* cur = ChildHead->next;

    while (cur != nullptr) {
        if (cur->child == target) {
            prev->next = cur->next;
            delete cur;
            ChildCount--;
            return;
        }
        prev = cur;
        cur = cur->next;
    }
}

// 获取兄弟数量 (不包含自己)
int Person::GetSiblingCount() const {
    if (father == nullptr) return 0;
    return father->ChildCount - 1;
}

// FamilyTree类成员函数的实现

FamilyTree::FamilyTree() : root(nullptr), PersonCount(0) {
    for (int i = 0; i < 1000; i++) people[i] = nullptr;
}

FamilyTree::~FamilyTree() {
    for (int i = 1; i <= PersonCount; i++) {
        if (people[i]) ClearTree(people[i]);
    }
}

// 以后序遍历的顺序递归释放整棵树
void FamilyTree::ClearTree(Person* node) {
    if (node == nullptr) return;

    ChildNode* cur = node->ChildHead->next;
    while (cur != nullptr) {
        ClearTree(cur->child);
        cur = cur->next;
    }

    // 释放该节点的孩子链表头结点
    delete node->ChildHead;

    // 释放当前结点
    delete node;
}

bool FamilyTree::BirthEarlier(Person* a, Person* b) {
    return StrCompare(a->BirthDate, b->BirthDate) < 0;
}

// 外层封装: 姓名 -> id (通过线性扫描实现)
void FamilyTree::FindIdByName(const char* name, int* id, int& count) const {
    count = 0;
    for (int i = 1; i <= PersonCount && count < 100; i++) {
        if (people[i] && StrEqual(people[i]->name, name)) {
            id[count++] = i;
        }
    }
}

Person* FamilyTree::SelectPersonByName(const char* name) const {
    int id[100];
    int count;
    FindIdByName(name, id, count);

    if (count == 0) {
        cout << "未找到: " << name << endl;
        return nullptr;
    }
    if (count == 1) return people[id[0]];

    cout << "发现 " << count << " 个\"" << name << "\":" << endl;
    for (int i = 0; i < count; i++) {
        Person* p = people[id[i]];
        cout << "  [id=" << id[i] << "] " << p->name
             << " (" << p->BirthDate << ", " << p->address << ") — 第"
             << p->GetGeneration() << "代" << endl;
    }
    cout << "请输入id: ";
    int Choice;
    cin >> Choice;
    cin.ignore();

    if (Choice >= 1 && Choice <= PersonCount && people[Choice]
        && StrEqual(people[Choice]->name, name)) {
        return people[Choice];
    }
    cout << "错误选择!" << endl;
    return nullptr;
}

// (1) 建立初始家谱树 
void FamilyTree::BuildFromFile(const char* FileName) {
    ifstream in_file(FileName);
    if (!in_file) {
        cout << "无法打开文件: " << FileName << endl;
        return;
    }

    cout << "正在加载文件: " << FileName << endl;
    
    char line[1024];            // 增大缓冲区
    in_file.getline(line, 1024);     // 跳过表头
    
    // 临时存储所有人员信息
    struct TempPerson {
        char name[50];
        char birth[20];
        char marriage[20];
        char address[50];
        char status[20];
        char death[20];
        char father_name[50];
    };
    TempPerson temp_list[1000];
    int temp_count = 0;

    // 读取并存储所有原始数据
    while (in_file.getline(line, 1024) && temp_count < 1000) {
        // 解析CSV
        char name_buf[50] = {0}, birth_buf[20] = {0}, marriage_buf[20] = {0}, 
             address_buf[50] = {0}, status_buf[20] = {0}, death_buf[20] = {0}, father_name_buf[50] = {0};
        
        int pos = 0, field = 0;
        char* fields[7] = {name_buf, birth_buf, marriage_buf, address_buf, status_buf, death_buf, father_name_buf};
        
        for (int i = 0; line[i] != '\0' && field < 7; i++) {
            if (line[i] == ',') {
                fields[field][pos] = '\0';
                field++;
                pos = 0;
            } else {
                fields[field][pos++] = line[i];
            }
        }
        fields[field][pos] = '\0';
        
        // 存储到临时数组
        StrCopy(temp_list[temp_count].name, name_buf, 50);
        StrCopy(temp_list[temp_count].birth, birth_buf, 20);
        StrCopy(temp_list[temp_count].marriage, marriage_buf, 20);
        StrCopy(temp_list[temp_count].address, address_buf, 50);
        StrCopy(temp_list[temp_count].status, status_buf, 20);
        StrCopy(temp_list[temp_count].death, death_buf, 20);
        StrCopy(temp_list[temp_count].father_name, father_name_buf, 50);
        
        // 去除可能存在的\r回车符
        for (int k = 0; temp_list[temp_count].name[k]; k++) {
            if (temp_list[temp_count].name[k] == '\r') {
                temp_list[temp_count].name[k] = '\0';
                break;
            }
        }
        for (int k = 0; temp_list[temp_count].father_name[k]; k++) {
            if (temp_list[temp_count].father_name[k] == '\r') {
                temp_list[temp_count].father_name[k] = '\0';
                break;
            }
        }
        
        temp_count++;
    }
    in_file.close();
    
    cout << "读取到" << temp_count << "条记录" << endl;
    
    // 创建所有 Person 对象
    bool root_printed = false;
    for (int i = 0; i < temp_count; i++) {
        if (PersonCount >= 999) {
            cout << "人数超出上限" << endl;
            break;
        }
        
        Person* p = new Person(temp_list[i].name, temp_list[i].birth, 
                                temp_list[i].marriage, temp_list[i].address,
                                temp_list[i].status, temp_list[i].death);
        p->id = ++PersonCount;
        people[p->id] = p;
        
        // 设置根节点（只输出一次）
        if (StrEqual(temp_list[i].father_name, "无")) {
            root = p;
            if (!root_printed) {
                cout << "根节点:" << p->name << "(id=" << p->id << ")" << endl;
                root_printed = true;
            }
        }
    }

    cout << "创建了" << PersonCount << "个Person对象" << endl;
    
    // 建立父子关系
    cout << "开始建立父子关系" << endl;
    for (int i = 0; i < temp_count; i++) {
        const char* father_name = temp_list[i].father_name;
        
        // 如果是根节点，跳过
        if (strcmp(father_name, "无") == 0) {
            continue;
        }
        
        Person* child = nullptr;
        Person* father = nullptr;
        
        // 查找孩子
        for (int j = 1; j <= PersonCount; j++) {
            if (people[j] && strcmp(people[j]->name, temp_list[i].name) == 0) {
                child = people[j];
                break;
            }
        }
        
        // 查找父亲
        for (int j = 1; j <= PersonCount; j++) {
            if (people[j] && strcmp(people[j]->name, father_name) == 0) {
                father = people[j];
                break;
            }
        }
        
        if (child && father) {
            father->InsertChildSorted(child);
            cout << "成功建立关系: " << father->name << " -> " << child->name << endl;
        }
    }
    
    cout << "家谱加载完成, 共" << PersonCount << "人" << endl;

    if (!root) {
        cout << "未找到根节点!" << endl;
    }
}

// (2) 显示家谱树 
void FamilyTree::DisplayTree() const {
    if (root == nullptr) {
        cout << "家谱为空" << endl;
        return;
    }
    DisplayTreeHelper(root, 0);
}

void FamilyTree::DisplayTreeHelper(Person* node, int depth) const {
    if (node == nullptr) return;

    for (int i = 0; i < depth; i++) cout << "    ";
    cout << "第" << node->GetGeneration() << "代: " << node->name
         << "(" << node->BirthDate << ", " << node->status << ")" << endl;

    ChildNode* cur = node->ChildHead->next;
    while (cur != nullptr) {
        DisplayTreeHelper(cur->child, depth + 1);
        cur = cur->next;
    }
}

// (3) 显示第n代的人数和姓名
void FamilyTree::ShowGeneration(int n) const {
    if (n < 1 || root == nullptr) {
        cout << "无效代数或家谱为空" << endl;
        return;
    }

    // 层序遍历 (用数组模拟队列)
    const int MAX_QUEUE = 1000;
    Person* Queue[MAX_QUEUE];
    int GenQueue[MAX_QUEUE]; 
    int front = 0, rear = 0;

    Queue[rear] = root;
    GenQueue[rear] = 1;
    rear++;

    Person* Result[100];
    int count = 0;

    while (front < rear && count < 100) {
        Person* p = Queue[front];
        int g = GenQueue[front];
        front++;

        if (g == n) Result[count++] = p;

        if (g < n) {
            ChildNode* cur = p->ChildHead->next;
            while (cur != nullptr && rear < MAX_QUEUE) {
                Queue[rear] = cur->child;
                GenQueue[rear] = g + 1;
                rear++;
                cur = cur->next;
            }
        }
    }

    cout << "第" << n << "代共有" << count << "人:" << endl;
    for (int i = 0; i < count; i++) {
        cout << "   " << Result[i]->name << "(" << Result[i]->BirthDate << ")" << endl;
    }
}

// (4) 按姓名查询
void FamilyTree::QueryByName(const char* name) const {
    Person* p = SelectPersonByName(name);
    if (!p) return;

    cout << "个人信息: " << endl;
    cout << "id: " << p->id << endl;
    cout << "姓名: " << p->name << endl;
    cout << "出生年月: " << p->BirthDate << endl;
    cout << "婚姻状况: " << p->marriage << endl;
    cout << "地址: " << p->address << endl;
    cout << "目前状况: " << p->status << endl;
    if (StrEqual(p->status, "已故")) cout << "死亡年月: " << p->DeathDate << endl;
    cout << "代数: 第" << p->GetGeneration() << "代" << endl;

    if (p->father) {
        cout << "父亲: " << p->father->name << "(" << p->father->BirthDate << ")" << endl;
    } else {
        cout << "父亲: 无" << endl;
    }

    cout << "孩子: ";
    if (!p->HasDescendants()) {
        cout << "无" << endl;
    } else {
        ChildNode* cur = p->ChildHead->next;
        while (cur != nullptr) {
            cout << cur->child->name << "(" << cur->child->BirthDate
                 << ", " << cur->child->status << ")" << endl;
            if(cur->next != nullptr)
                cout << "     ";
            cur = cur->next;
        }
    }
}

// (5) 给某人添加孩子
void FamilyTree::AddChild(const char* FatherName, const char* name,
                          const char* birth, const char* marriage,
                          const char* address, const char* status,
                          const char* DeathDate) {
    Person* father = SelectPersonByName(FatherName);
    if (!father) return;

    if (PersonCount >= 999) {
        cout << "人数超出上限" << endl;
        return;
    }

    Person* child = new Person(name, birth, marriage, address, status, DeathDate);
    child->id = ++PersonCount;
    people[child->id] = child;

    father->InsertChildSorted(child);

    cout << "成功添加: " << name << "(id=" << child->id << ")作为" 
         << FatherName << "的孩子" << endl;
}