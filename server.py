import json
import os
from http.server import HTTPServer, BaseHTTPRequestHandler
from urllib.parse import parse_qs, urlparse

class FamilyTreeHandler(BaseHTTPRequestHandler):
    
    def do_GET(self):
        parsed = urlparse(self.path)
        path = parsed.path
        
        if path == "/" or path == "/index.html":
            self.serve_html()
        elif path == "/api/tree":
            self.get_tree()
        elif path == "/api/members":
            self.get_members()
        elif path.startswith("/api/query"):
            self.query_member(parsed)
        else:
            self.send_response(404)
            self.end_headers()
    
    def do_POST(self):
        content_length = int(self.headers.get('Content-Length', 0))
        body = self.rfile.read(content_length).decode('utf-8')
        data = parse_qs(body)
        
        if self.path == "/api/add":
            self.add_child(data)
        elif self.path == "/api/delete":
            self.delete_person(data)
        elif self.path == "/api/save":
            self.save_tree()
        else:
            self.send_response(404)
            self.end_headers()
    
    def serve_html(self):
        html_path = "web/index.html"
        if os.path.exists(html_path):
            with open(html_path, 'r', encoding='utf-8') as f:
                content = f.read()
            self.send_response(200)
            self.send_header('Content-Type', 'text/html')
            self.end_headers()
            self.wfile.write(content.encode('utf-8'))
        else:
            # 返回内置简单页面
            simple_html = '''<!DOCTYPE html>
<html>
<head><meta charset="UTF-8"><title>家谱系统</title></head>
<body>
<h1>家谱管理系统</h1>
<p>请创建 web/index.html文件或使用演示数据</p>
<p><a href="/api/tree">查看家谱数据</a></p>
</body>
</html>'''
            self.send_response(200)
            self.send_header('Content-Type', 'text/html')
            self.end_headers()
            self.wfile.write(simple_html.encode('utf-8'))
    
    def get_tree(self):
        """返回家谱树 JSON"""
        if os.path.exists("family_tree.txt"):
            with open("family_tree.txt", 'r', encoding='utf-8') as f:
                lines = f.readlines()
            
            # 构建树形结构
            persons = {}
            children_map = {}
            root_name = None
            
            for line in lines[1:]:  # 跳过表头
                parts = line.strip().split(',')
                if len(parts) < 7:
                    continue
                name = parts[0].strip()
                birth = parts[1].strip()
                status = parts[4].strip()
                father = parts[6].strip()
                
                persons[name] = {
                    "name": name,
                    "birth": birth,
                    "status": status,
                    "children": []
                }
                
                if father == "无":
                    root_name = name
                else:
                    if father not in children_map:
                        children_map[father] = []
                    children_map[father].append(name)
            
            # 构建树
            for father, children in children_map.items():
                if father in persons:
                    persons[father]["children"] = [persons[child] for child in children if child in persons]
            
            tree = persons.get(root_name, {"name": "暂无数据", "children": []})
            
            self.send_response(200)
            self.send_header('Content-Type', 'application/json')
            self.end_headers()
            self.wfile.write(json.dumps(tree).encode('utf-8'))
        else:
            # 返回演示数据
            demo_data = {
                "name": "李甲天",
                "birth": "1900-01",
                "status": "已故",
                "children": [
                    {
                        "name": "李一河",
                        "birth": "1920-05",
                        "status": "已故",
                        "children": [
                            {"name": "李二金", "birth": "1943-04", "status": "健在"},
                            {"name": "李二银", "birth": "1945-07", "status": "健在"}
                        ]
                    },
                    {"name": "李一福", "birth": "1922-08", "status": "已故"},
                    {"name": "李一山", "birth": "1925-11", "status": "健在"}
                ]
            }
            self.send_response(200)
            self.send_header('Content-Type', 'application/json')
            self.end_headers()
            self.wfile.write(json.dumps(demo_data).encode('utf-8'))
    
    def get_members(self):
        """获取所有成员"""
        members = []
        if os.path.exists("family_tree.txt"):
            with open("family_tree.txt", 'r', encoding='utf-8') as f:
                lines = f.readlines()
            for i, line in enumerate(lines[1:], 1):
                parts = line.strip().split(',')
                if len(parts) >= 1:
                    members.append({"id": i, "name": parts[0].strip()})
        
        self.send_response(200)
        self.send_header('Content-Type', 'application/json')
        self.end_headers()
        self.wfile.write(json.dumps(members).encode('utf-8'))
    
    def query_member(self, parsed):
        """查询成员"""
        query = parse_qs(parsed.query)
        name = query.get('name', [''])[0]
        
        result = {"success": False, "count": 0, "results": []}
        
        if name and os.path.exists("family_tree.txt"):
            with open("family_tree.txt", 'r', encoding='utf-8') as f:
                lines = f.readlines()
            for i, line in enumerate(lines[1:], 1):
                parts = line.strip().split(',')
                if len(parts) >= 2 and parts[0].strip() == name:
                    result["success"] = True
                    result["count"] += 1
                    result["results"].append({
                        "id": i,
                        "name": parts[0].strip(),
                        "birth": parts[1].strip() if len(parts) > 1 else ""
                    })
        
        self.send_response(200)
        self.send_header('Content-Type', 'application/json')
        self.end_headers()
        self.wfile.write(json.dumps(result).encode('utf-8'))
    
    def add_child(self, data):
        """添加孩子"""
        father = data.get('father', [''])[0]
        name = data.get('name', [''])[0]
        birth = data.get('birth', [''])[0]
        marriage = data.get('marriage', ['未婚'])[0]
        address = data.get('address', ['未知'])[0]
        status = data.get('status', ['在世'])[0]
        death = data.get('death', [''])[0]
        
        if not name or not birth:
            self.send_response(200)
            self.send_header('Content-Type', 'application/json')
            self.end_headers()
            self.wfile.write(json.dumps({"success": False, "error": "姓名和出生日期不能为空"}).encode('utf-8'))
            return
        
        # 确保文件存在
        if not os.path.exists("family_tree.txt"):
            with open("family_tree.txt", 'w', encoding='utf-8') as f:
                f.write("姓名,出生日期,婚姻状况,地址,目前状况,死亡日期,父亲姓名\n")
        
        # 追加新成员
        with open("family_tree.txt", 'a', encoding='utf-8') as f:
            f.write(f"{name},{birth},{marriage},{address},{status},{death},{father}\n")
        
        self.send_response(200)
        self.send_header('Content-Type', 'application/json')
        self.end_headers()
        self.wfile.write(json.dumps({"success": True}).encode('utf-8'))
    
    def delete_person(self, data):
        """删除成员"""
        name = data.get('name', [''])[0]
        
        if name and os.path.exists("family_tree.txt"):
            with open("family_tree.txt", 'r', encoding='utf-8') as f:
                lines = f.readlines()
            
            new_lines = [lines[0]]  # 保留表头
            deleted = False
            for line in lines[1:]:
                parts = line.strip().split(',')
                if len(parts) >= 1 and parts[0].strip() != name:
                    new_lines.append(line)
                else:
                    deleted = True
            
            if deleted:
                with open("family_tree.txt", 'w', encoding='utf-8') as f:
                    f.writelines(new_lines)
        
        self.send_response(200)
        self.send_header('Content-Type', 'application/json')
        self.end_headers()
        self.wfile.write(json.dumps({"success": True}).encode('utf-8'))
    
    def save_tree(self):
        """保存家谱"""
        self.send_response(200)
        self.send_header('Content-Type', 'application/json')
        self.end_headers()
        self.wfile.write(json.dumps({"success": True}).encode('utf-8'))
    
    def log_message(self, format, *args):
        pass  # 减少日志输出

def main():
    os.makedirs("web", exist_ok=True)
    
    # 确保有数据文件
    if not os.path.exists("family_tree.txt"):
        with open("family_tree.txt", 'w', encoding='utf-8') as f:
            f.write("姓名,出生日期,婚姻状况,地址,目前状况,死亡日期,父亲姓名\n")
            f.write("李甲天,1900-01,已婚,北京,已故,1970-01,无\n")
            f.write("李一河,1920-05,已婚,北京,已故,2005-03,李甲天\n")
            f.write("李一福,1922-08,未婚,北京,已故,2010-06,李甲天\n")
            f.write("李一山,1925-11,已婚,上海,健在,无,李甲天\n")
            f.write("李二金,1943-04,已婚,北京,健在,无,李一河\n")
            f.write("李二银,1945-07,未婚,北京,健在,无,李一河\n")
            f.write("李二铁,1948-12,已婚,北京,健在,无,李一山\n")
            f.write("李三木,1965-03,已婚,北京,健在,无,李二金\n")
            f.write("李三林,1968-08,已婚,北京,健在,无,李二金\n")
            f.write("李三国,1970-11,已婚,北京,健在,无,李二铁\n")
            f.write("李三家,1963-06,未婚,上海,健在,无,李二铁\n")
            f.write("李三建,1966-01,已婚,上海,健在,无,李二铁\n")
            f.write("李四辉,1990-12,未婚,北京,健在,无,李三木\n")
            f.write("李四明,1985-02,已婚,北京,健在,无,李三林\n")
            f.write("李四峰,1986-09,已婚,上海,健在,无,李三国\n")
            f.write("李四岩,1989-03,未婚,上海,健在,无,李三国\n")
            f.write("李五辰,2010-01,未婚,北京,健在,无,李四明\n")
            f.write("李五星,2009-06,未婚,北京,健在,无,李四峰\n")
    
    # 确保有 index.html
    if not os.path.exists("web/index.html"):
        # 下载或复制之前创建的 index.html
        print("请确保 web/index.html 文件存在")
    
    port = 8080
    server = HTTPServer(('localhost', port), FamilyTreeHandler)
    print("=" * 40)
    print("家谱管理系统服务器已启动")
    print(f"访问地址: http://localhost:{port}")
    print("=" * 40)
    print("按 Ctrl+C 停止服务器")
    
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        print("\n服务器已停止")

if __name__ == "__main__":
    main()