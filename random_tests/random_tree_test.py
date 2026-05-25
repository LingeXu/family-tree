#!/usr/bin/env python3
import argparse
import os
import random
import subprocess
import tempfile
from collections import Counter
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
OUT_DIR = ROOT / "random_tests" / "generated"
BIN = Path(tempfile.gettempdir()) / "familytree_random_asan"

HEADER = [
    "\u59d3\u540d",
    "\u51fa\u751f\u65e5\u671f",
    "\u5a5a\u59fb\u72b6\u51b5",
    "\u5730\u5740",
    "\u76ee\u524d\u72b6\u51b5",
    "\u6b7b\u4ea1\u65e5\u671f",
    "\u7236\u4eb2\u59d3\u540d",
]
NONE = "\u65e0"
MARRIAGE = ["\u672a\u5a5a", "\u5df2\u5a5a"]
STATUS = ["\u5065\u5728", "\u5df2\u6545"]
ADDR = [
    "\u5317\u4eac",
    "\u4e0a\u6d77",
    "\u5e7f\u5dde",
    "\u6df1\u5733",
    "\u676d\u5dde",
    "\u5357\u4eac",
]

GIVEN = [
    "\u5b89",
    "\u535a",
    "\u660c",
    "\u8fbe",
    "\u6069",
    "\u98de",
    "\u5149",
    "\u534e",
    "\u9759",
    "\u5eb7",
    "\u660e",
    "\u5b81",
    "\u5e73",
    "\u79cb",
    "\u745e",
    "\u751f",
    "\u6cf0",
    "\u6587",
    "\u65b0",
    "\u8fdc",
]
DUP_NAMES = [
    "\u5f20\u4f1f",
    "\u738b\u82b3",
    "\u674e\u5a1c",
    "\u5218\u6d0b",
    "\u9648\u9759",
    "\u6768\u654f",
]


def run(cmd, data=None):
    env = os.environ.copy()
    env["ASAN_OPTIONS"] = "detect_leaks=0"
    p = subprocess.run(
        cmd,
        input=data,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        cwd=ROOT,
        env=env,
    )
    return p.returncode, p.stdout


def compile_program():
    cmd = [
        "g++",
        "-std=c++17",
        "-Wall",
        "-Wextra",
        "-fsanitize=address",
        "-g",
        "-Iinclude",
        "src/main.cpp",
        "src/FamilyTree.cpp",
        "-o",
        str(BIN),
    ]
    code, out = run(cmd)
    if code != 0:
        raise RuntimeError("compile failed\n" + out)


def cn_name(prefix, idx):
    a = GIVEN[idx % len(GIVEN)]
    b = GIVEN[(idx // len(GIVEN) + idx) % len(GIVEN)]
    return prefix + a + b + "%03d" % idx


def make_row(name, parent_name, year, rng):
    month = rng.randint(1, 12)
    status = rng.choice(STATUS)
    death = NONE
    if status == STATUS[1]:
        death_year = year + rng.randint(55, 88)
        if death_year <= 2025:
            death = "%04d-%02d" % (death_year, month)
        else:
            status = STATUS[0]
    return [
        name,
        "%04d-%02d" % (year, month),
        rng.choice(MARRIAGE),
        rng.choice(ADDR),
        status,
        death,
        parent_name,
    ]


def add_node(nodes, name, parent, year, rng):
    idx = len(nodes)
    if parent is None:
        root = idx
        gen = 1
        parent_name = NONE
    else:
        root = nodes[parent]["root"]
        gen = nodes[parent]["gen"] + 1
        parent_name = nodes[parent]["name"]

    nodes.append(
        {
            "name": name,
            "parent": parent,
            "root": root,
            "gen": gen,
            "year": year,
            "row": make_row(name, parent_name, year, rng),
        }
    )
    return idx


def make_deep_chain(rng):
    nodes = []
    root = add_node(nodes, "\u6df1\u6839\u592a\u7956", None, 1700, rng)
    last = root
    for i in range(1, 90):
        last = add_node(nodes, cn_name("\u6df1\u94fe", i), last, 1700 + i * 3, rng)

    # 这条支线专门检查深层LCA，旧代码50层祖先数组会漏掉它。
    branch_parent = 19
    add_node(nodes, "\u6df1\u652f\u7532", branch_parent, nodes[branch_parent]["year"] + 5, rng)
    add_node(nodes, "\u6df1\u652f\u4e59", branch_parent, nodes[branch_parent]["year"] + 7, rng)
    return nodes


def make_wide_tree(rng):
    nodes = []
    root = add_node(nodes, "\u5bbd\u6811\u59cb\u7956", None, 1910, rng)
    for i in range(1, 181):
        child = add_node(nodes, cn_name("\u5bbd\u679d", i), root, 1930 + (i % 10), rng)
        if i <= 20:
            add_node(nodes, cn_name("\u5bbd\u5b59", i), child, 1960 + (i % 10), rng)
    return nodes


def make_duplicate_tree(rng):
    nodes = []
    root = add_node(nodes, "\u91cd\u540d\u59cb\u7956", None, 1900, rng)
    parents = []
    for i in range(1, 21):
        parents.append(add_node(nodes, cn_name("\u91cd\u7236", i), root, 1930 + (i % 10), rng))

    for i, parent in enumerate(parents):
        for j in range(4):
            name = DUP_NAMES[(i + j) % len(DUP_NAMES)]
            add_node(nodes, name, parent, 1960 + ((i * 4 + j) % 30), rng)
    return nodes


def make_forest(rng):
    nodes = []
    roots = []
    for i in range(4):
        roots.append(add_node(nodes, cn_name("\u6797\u6839", i), None, 1890 + i * 8, rng))

    for i in range(1, 141):
        parent = roots[i % len(roots)] if i <= 12 else rng.randint(0, len(nodes) - 1)
        year = min(nodes[parent]["year"] + rng.randint(18, 35), 2024)
        add_node(nodes, cn_name("\u6797\u4eba", i), parent, year, rng)
    return nodes


def make_large_random(rng):
    nodes = []
    for i in range(3):
        add_node(nodes, cn_name("\u5927\u6811\u6839", i), None, 1880 + i * 12, rng)

    for i in range(1, 421):
        cand = [j for j, n in enumerate(nodes) if n["gen"] < 9]
        parent = rng.choice(cand)
        year = min(nodes[parent]["year"] + rng.randint(18, 34), 2024)
        add_node(nodes, cn_name("\u5927\u6811", i), parent, year, rng)
    return nodes


BUILDERS = [
    ("deep_chain", make_deep_chain),
    ("wide_tree", make_wide_tree),
    ("duplicate_names", make_duplicate_tree),
    ("multi_root_forest", make_forest),
    ("large_random", make_large_random),
]


def write_case(path, nodes, with_header):
    with path.open("w", encoding="utf-8", newline="\n") as f:
        if with_header:
            f.write(",".join(HEADER) + "\n")
        for node in nodes:
            f.write(",".join(node["row"]) + "\n")


def read_saved(path):
    lines = path.read_text(encoding="utf-8").splitlines()
    if not lines or lines[0].split(",") != HEADER:
        raise AssertionError("%s missing header" % path)
    return [line.split(",") for line in lines[1:] if line]


def ancestors(nodes, idx):
    ans = []
    while idx is not None:
        ans.append(idx)
        idx = nodes[idx]["parent"]
    return ans


def is_ancestor(nodes, a, b):
    cur = nodes[b]["parent"]
    while cur is not None:
        if cur == a:
            return True
        cur = nodes[cur]["parent"]
    return False


def find_lca(nodes, a, b):
    aset = set(ancestors(nodes, a))
    for x in ancestors(nodes, b):
        if x in aset:
            return x
    return None


def name_counts(nodes):
    return Counter(n["name"] for n in nodes)


def add_person_input(commands, nodes, idx, counts):
    commands.append(nodes[idx]["name"])
    if counts[nodes[idx]["name"]] > 1:
        commands.append(str(idx + 1))


def add_pair_input(commands, nodes, a, b, counts):
    commands.append(nodes[a]["name"])
    commands.append(nodes[b]["name"])
    if counts[nodes[a]["name"]] > 1:
        commands.append(str(a + 1))
    if counts[nodes[b]["name"]] > 1:
        commands.append(str(b + 1))


def pick_direct(nodes):
    best = None
    best_gap = -1
    for i, n in enumerate(nodes):
        if n["parent"] is None:
            continue
        ans = ancestors(nodes, i)
        gap = len(ans)
        if gap > best_gap:
            best_gap = gap
            best = (ans[-1], i)
    return best


def pick_collateral(nodes):
    best = None
    best_depth = -1
    for i in range(len(nodes)):
        for j in range(i + 1, len(nodes)):
            if nodes[i]["root"] != nodes[j]["root"]:
                continue
            if is_ancestor(nodes, i, j) or is_ancestor(nodes, j, i):
                continue
            lca = find_lca(nodes, i, j)
            depth = nodes[lca]["gen"] if lca is not None else 0
            if depth > best_depth:
                best_depth = depth
                best = (i, j)
    return best


def pick_far_lca(nodes):
    for i in range(len(nodes) - 1, -1, -1):
        for j in range(i):
            if nodes[i]["root"] != nodes[j]["root"]:
                continue
            if is_ancestor(nodes, i, j) or is_ancestor(nodes, j, i):
                continue
            lca = find_lca(nodes, i, j)
            if lca is not None and nodes[i]["gen"] - nodes[lca]["gen"] > 55:
                return i, j
    return None


def pick_no_common(nodes):
    for i in range(len(nodes)):
        for j in range(i + 1, len(nodes)):
            if nodes[i]["root"] != nodes[j]["root"]:
                return i, j
    return None


def pick_duplicate(nodes):
    counts = name_counts(nodes)
    for i, n in enumerate(nodes):
        if counts[n["name"]] > 1:
            return i
    return None


def check_output(text, needle, case_name):
    if needle not in text:
        raise AssertionError("%s missing output: %s" % (case_name, needle))


def run_case(case_id, profile, nodes, data_path):
    save_path = OUT_DIR / ("case_%02d_%s_saved.txt" % (case_id, profile))
    if save_path.exists():
        save_path.unlink()

    counts = name_counts(nodes)
    max_gen = max(n["gen"] for n in nodes)
    check_gens = [min(max_gen, 3)]
    if profile == "wide_tree":
        check_gens = [2]
    if profile == "deep_chain":
        check_gens = [max_gen]

    query = pick_duplicate(nodes)
    if query is None:
        query = len(nodes) - 1

    direct_a, direct_b = pick_direct(nodes)
    lca_pair = pick_far_lca(nodes)
    if lca_pair is None:
        lca_pair = pick_collateral(nodes)
    if lca_pair is None:
        lca_pair = (direct_a, direct_b)
    lca = find_lca(nodes, lca_pair[0], lca_pair[1])

    collateral = pick_collateral(nodes)
    no_common = pick_no_common(nodes)

    commands = ["1", str(data_path.relative_to(ROOT))]
    for gen in check_gens:
        commands += ["3", str(gen)]

    commands.append("4")
    add_person_input(commands, nodes, query, counts)

    commands.append("8")
    add_pair_input(commands, nodes, direct_a, direct_b, counts)

    commands.append("9")
    add_pair_input(commands, nodes, lca_pair[0], lca_pair[1], counts)

    if collateral is not None:
        commands.append("8")
        add_pair_input(commands, nodes, collateral[0], collateral[1], counts)

    if no_common is not None:
        commands.append("10")
        add_pair_input(commands, nodes, no_common[0], no_common[1], counts)

        commands.append("8")
        add_pair_input(commands, nodes, no_common[0], no_common[1], counts)

    commands += ["11", str(save_path.relative_to(ROOT)), "0"]
    data = "\n".join(commands) + "\n"

    code, out = run([str(BIN)], data)
    case_name = "case_%02d_%s" % (case_id, profile)
    if code != 0:
        raise AssertionError("%s exited with %d\n%s" % (case_name, code, out))

    check_output(out, "\u8bfb\u53d6\u5230%d\u6761\u8bb0\u5f55" % len(nodes), case_name)
    for gen in check_gens:
        gen_count = sum(1 for n in nodes if n["gen"] == gen)
        check_output(out, "\u7b2c%d\u4ee3\u5171\u6709%d\u4eba" % (gen, gen_count), case_name)

    if counts[nodes[query]["name"]] > 1:
        check_output(out, "\u53d1\u73b0 %d \u4e2a\"%s\"" % (counts[nodes[query]["name"]], nodes[query]["name"]), case_name)

    if nodes[query]["parent"] is None:
        check_output(out, "\u7236\u4eb2: \u65e0", case_name)
    else:
        pname = nodes[nodes[query]["parent"]]["name"]
        check_output(out, "\u7236\u4eb2: %s(" % pname, case_name)

    check_output(out, "\u662f\u76f4\u7cfb\u4eb2\u5c5e", case_name)
    check_output(out, "\u6700\u8fd1\u5171\u540c\u7956\u5148\u662f%s" % nodes[lca]["name"], case_name)

    if collateral is not None:
        check_output(out, "\u662f\u65c1\u7cfb\u4eb2\u5c5e", case_name)

    if no_common is not None:
        check_output(out, "\u65e0\u5171\u540c\u7956\u5148", case_name)

    saved_rows = read_saved(save_path)
    original = Counter(tuple(n["row"]) for n in nodes)
    saved = Counter(tuple(row) for row in saved_rows)
    if original != saved:
        raise AssertionError("%s saved txt does not match input" % case_name)

    return len(nodes), max_gen, len([n for n in nodes if counts[n["name"]] > 1])


def clean_generated():
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    for path in OUT_DIR.glob("case_*"):
        if path.is_file():
            path.unlink()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--cases", type=int, default=5)
    parser.add_argument("--seed", type=int, default=20260524)
    args = parser.parse_args()

    if args.cases < 5:
        raise SystemExit("--cases must be at least 5")

    rng = random.Random(args.seed)
    clean_generated()
    compile_program()

    print("seed=%d cases=%d" % (args.seed, args.cases))
    for case_id in range(1, args.cases + 1):
        base_profile, builder = BUILDERS[(case_id - 1) % len(BUILDERS)]
        profile = base_profile
        if case_id > len(BUILDERS):
            profile = "%s_%02d" % (base_profile, case_id)
        nodes = builder(rng)
        path = OUT_DIR / ("case_%02d_%s.txt" % (case_id, profile))
        write_case(path, nodes, with_header=(case_id % 2 == 0))
        total, max_gen, dup_count = run_case(case_id, profile, nodes, path)
        print(
            "case_%02d %-16s ok: nodes=%d max_gen=%d duplicate_nodes=%d"
            % (case_id, profile, total, max_gen, dup_count)
        )


if __name__ == "__main__":
    main()
