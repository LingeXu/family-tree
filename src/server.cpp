#include "FamilyTree.h"
#include "httplib/httplib.h"
#include <sstream>

using namespace std;

// JSON转义
string json_escape(const string& s) {
    string result;
    for (char c : s) {
        if (c == '"') result += "\\\"";
        else if (c == '\\') result += "\\\\";
        else if (c == '\n') result += "\\n";
        else if (c == '\r') result += "\\r";
        else result += c;
    }
    return result;
}

// 递归生成树形JSON
string person_to_json(Person* p) {
    if (!p) return "null";
    
    stringstream ss;
    ss << "{";
    ss << "\"id\":" << p->id << ",";
    ss << "\"name\":\"" << json_escape(p->name) << "\",";
    ss << "\"birth\":\"" << json_escape(p->BirthDate) << "\",";
    ss << "\"marriage\":\"" << json_escape(p->marriage) << "\",";
    ss << "\"address\":\"" << json_escape(p->address) << "\",";
    ss << "\"status\":\"" << json_escape(p->status) << "\",";
    ss << "\"death\":\"" << json_escape(p->DeathDate) << "\",";
    ss << "\"generation\":" << p->GetGeneration() << ",";
    
    if (p->father) {
        ss << "\"father\":\"" << json_escape(p->father->name) << "\",";
    } else {
        ss << "\"father\":\"无\",";
    }
    
    ss << "\"children\":[";
    ChildNode* cur = p->ChildHead->next;
    bool first = true;
    while (cur) {
        if (!first) ss << ",";
        ss << person_to_json(cur->child);
        first = false;
        cur = cur->next;
    }
    ss << "]}";
    
    return ss.str();
}

// 读取文件
string read_file(const string& filename) {
    ifstream file(filename);
    if (!file) return "";
    string content((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
    return content;
}

int main() {
    FamilyTree tree;
    
    // 加载家谱文件
    ifstream test("family_tree.txt");
    if (test) {
        tree.BuildFromFile("family_tree.txt");
    }
    test.close();
    
    httplib::Server svr;
    
    // 首页
    svr.Get("/", [](const httplib::Request& req, httplib::Response& res) {
        string html = read_file("web/index.html");
        if (html.empty()) {
            html = R"(<!DOCTYPE html>
            <html>
            <head><meta charset="UTF-8"><title>家谱系统</title></head>
            <body>
                <h1>家谱管理系统</h1>
                <p>请创建 web/index.html 文件</p>
            </body>
            </html>)";
        }
        res.set_content(html, "text/html");
    });
    
    // API: 获取家谱树
    svr.Get("/api/tree", [&tree](const httplib::Request& req, httplib::Response& res) {
        if (!tree.root) {
            res.set_content("{}", "application/json");
            return;
        }
        string json = person_to_json(tree.root);
        res.set_content(json, "application/json");
    });
    
    // API: 添加孩子
    svr.Post("/api/add", [&tree](const httplib::Request& req, httplib::Response& res) {
        // 简单解析表单数据
        string body = req.body;
        
        // 提取参数
        string father, name, birth;
        
        auto extract = [&](const string& key) -> string {
            size_t pos = body.find(key + "=");
            if (pos == string::npos) return "";
            size_t end = body.find("&", pos);
            if (end == string::npos) end = body.length();
            string value = body.substr(pos + key.length() + 1, end - pos - key.length() - 1);
            return value;
        };
        
        father = extract("father");
        name = extract("name");
        birth = extract("birth");
        
        if (!father.empty() && !name.empty() && !birth.empty()) {
            tree.AddChild(father.c_str(), name.c_str(), birth.c_str(), 
                         "未婚", "未知", "在世", "");
            res.set_content(R"({"success":true})", "application/json");
        } else {
            res.set_content(R"({"success":false,"error":"参数错误"})", "application/json");
        }
    });
    
    // API: 删除成员
    svr.Post("/api/delete", [&tree](const httplib::Request& req, httplib::Response& res) {
        string body = req.body;
        string name;
        
        size_t pos = body.find("name=");
        if (pos != string::npos) {
            size_t end = body.find("&", pos);
            if (end == string::npos) end = body.length();
            name = body.substr(pos + 5, end - pos - 5);
        }
        
        if (!name.empty()) {
            tree.DeletePerson(name.c_str());
            res.set_content(R"({"success":true})", "application/json");
        } else {
            res.set_content(R"({"success":false})", "application/json");
        }
    });
    
    // API: 保存家谱
    svr.Post("/api/save", [&tree](const httplib::Request& req, httplib::Response& res) {
        tree.SaveToFile("family_tree.txt");
        res.set_content(R"({"success":true})", "application/json");
    });
    
    // API: 查询成员
    svr.Get("/api/query", [&tree](const httplib::Request& req, httplib::Response& res) {
        string name = req.get_param_value("name");
        if (name.empty()) {
            res.set_content(R"({"success":false})", "application/json");
            return;
        }
        
        // 查找成员
        int ids[100];
        int count;
        tree.FindIdByName(name.c_str(), ids, count);
        
        if (count == 0) {
            res.set_content(R"({"success":false,"message":"未找到"})", "application/json");
            return;
        }
        
        stringstream ss;
        ss << R"({"success":true,"count":)" << count << R"(,"results":[)";
        for (int i = 0; i < count && i < 10; i++) {
            if (i > 0) ss << ",";
            Person* p = tree.people[ids[i]];
            ss << R"({"name":")" << json_escape(p->name) << R"(","birth":")" 
               << json_escape(p->BirthDate) << R"(","id":)" << p->id << "}";
        }
        ss << "]}";
        
        res.set_content(ss.str(), "application/json");
    });
    
    // API: 获取成员列表
    svr.Get("/api/members", [&tree](const httplib::Request& req, httplib::Response& res) {
        stringstream ss;
        ss << "[";
        for (int i = 1; i <= tree.PersonCount; i++) {
            if (tree.people[i]) {
                if (i > 1) ss << ",";
                ss << R"({"id":)" << tree.people[i]->id 
                   << R"(,"name":")" << json_escape(tree.people[i]->name) << R"("})";
            }
        }
        ss << "]";
        res.set_content(ss.str(), "application/json");
    });
    
    cout << "家谱管理系统服务器已启动" << endl;
    cout << "访问地址: http://localhost:8080" << endl;
    cout << "按Ctrl+C停止服务器" << endl;
    
    svr.listen("localhost", 8080);
    
    return 0;
}