#include "FamilyTree.h"
#include <iostream>
using namespace std;

// 显示菜单
void ShowMenu() {
    cout << "操作手册:" << endl;
    cout << "1. 从文件加载家谱" << endl;
    cout << "2. 显示家谱树" << endl;
    cout << "3. 显示第n代成员" << endl;
    cout << "4. 查询成员信息" << endl;
    cout << "5. 添加孩子" << endl;
    cout << "6. 删除成员" << endl;
    cout << "7. 修改成员信息" << endl;
    cout << "8. 判断关系(直系/旁系)" << endl;
    cout << "9. 查找最近共同祖先" << endl;
    cout << "10. 显示详细亲缘关系" << endl;
    cout << "11. 保存家谱到文件" << endl;
    cout << "0. 退出" << endl;
    cout << "请选择: ";
}

int main() {
    FamilyTree tree;
    int choice;     // switch语句的case后面只能跟单个字符, 故不采用char
    char filename[100];
    char name[50], name2[50];
    char birth[20], marriage[20], address[50], status[20], death[20];
    int gen;

    cout << "家谱管理系统已启动" << endl;

    while (true) {
        ShowMenu();
        cin >> choice;
        cin.ignore();

        switch (choice) {
            case 1:
                cout << "请输入文件名: ";
                cin.getline(filename, 100);
                tree.BuildFromFile(filename);
                break;

            case 2:
                //tree.DisplayTree();
                break;

            case 3:
                cout << "请输入代数: ";
                cin >> gen;
                cin.ignore();
                //tree.ShowGeneration(gen);
                break;

            case 4:
                cout << "请输入姓名: ";
                cin.getline(name, 50);
                //tree.QueryByName(name);
                break;

            case 5:
                cout << "请输入父亲姓名: ";
                cin.getline(name, 50);
                cout << "请输入孩子姓名: ";
                cin.getline(name2, 50);
                cout << "请输入出生日期: ";
                cin.getline(birth, 20);
                cout << "请输入婚姻状况(未婚/已婚): ";
                cin.getline(marriage, 20);
                cout << "请输入地址: ";
                cin.getline(address, 50);
                cout << "请输入状况(在世/已故): ";
                cin.getline(status, 20);
                if (strcmp(status, "已故") == 0) {
                    cout << "请输入死亡日期: ";
                    cin.getline(death, 20);
                } else {
                    death[0] = '\0';
                }
                //tree.AddChild(name, name2, birth, marriage, address, status, death);
                break;

            case 6:
                cout << "请输入要删除的姓名: ";
                cin.getline(name, 50);
                //tree.DeletePerson(name);
                break;

            case 7:
                cout << "请输入要修改的姓名: ";
                cin.getline(name, 50);
                cout << "请输入新的出生日期(留空则不修改): ";
                cin.getline(birth, 20);
                cout << "请输入新的婚姻状况(留空则不修改): ";
                cin.getline(marriage, 20);
                cout << "请输入新的地址(留空则不修改): ";
                cin.getline(address, 50);
                cout << "请输入新的状况(留空则不修改): ";
                cin.getline(status, 20);
                cout << "请输入新的死亡日期(留空则不修改): ";
                cin.getline(death, 20);
                //tree.ModifyPerson(name, birth, marriage, address, status, death);
                break;

            case 8:
                cout << "请输入第一个人姓名: ";
                cin.getline(name, 50);
                cout << "请输入第二个人姓名: ";
                cin.getline(name2, 50);
                //tree.CheckDirectOrCollateral(name, name2);
                break;

            case 9:
                cout << "请输入第一个人姓名: ";
                cin.getline(name, 50);
                cout << "请输入第二个人姓名: ";
                cin.getline(name2, 50);
                //tree.FindCommonAncestor(name, name2);
                break;

            case 10:
                cout << "请输入第一个人姓名: ";
                cin.getline(name, 50);
                cout << "请输入第二个人姓名: ";
                cin.getline(name2, 50);
                //tree.ShowDetailedRelation(name, name2);
                break;

            case 11:
                cout << "请输入保存文件名: ";
                cin.getline(filename, 100);
                //tree.SaveToFile(filename);
                break;

            case 0:
                cout << "已退出家谱管理系统" << endl;
                return 0;

            default:
                cout << "无效选项, 请重新输入！" << endl;
        }
    }

    return 0;
}