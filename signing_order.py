# Script that determines signing order of dylibs in a directory.

#!/usr/bin/env python3
import os
import subprocess
from collections import defaultdict, deque
import sys

if len(sys.argv) != 2:
    print("Usage: signing_order.py /path/to/dylibs")
    sys.exit(1)

folder = sys.argv[1]
dylib_map = {}
dependencies = defaultdict(set)
reverse_dependencies = defaultdict(set)

# Step 1: Find all .dylib files and index by basename
for root, _, files in os.walk(folder):
    for name in files:
        if name.endswith(".dylib"):
            full_path = os.path.join(root, name)
            dylib_map[name] = full_path

# Step 2: Use otool -L to extract dependencies
for name, full_path in dylib_map.items():
    output = subprocess.check_output(["otool", "-L", full_path]).decode()
    for line in output.splitlines()[1:]:
        dep = line.strip().split(" ")[0]
        dep_name = os.path.basename(dep)
        if dep_name in dylib_map and dep_name != name:
            dependencies[name].add(dep_name)
            reverse_dependencies[dep_name].add(name)

# Step 3: Topological sort
in_degree = {name: len(dependencies[name]) for name in dylib_map}
queue = deque([name for name in dylib_map if in_degree[name] == 0])
order = []

while queue:
    lib = queue.popleft()
    order.append(lib)
    for dependent in reverse_dependencies[lib]:
        in_degree[dependent] -= 1
        if in_degree[dependent] == 0:
            queue.append(dependent)

if len(order) != len(dylib_map):
    print("Error: Circular dependency detected.")
    sys.exit(1)

# Step 4: Print full paths in signing order
for name in order:
    print(dylib_map[name])
