import networkx as nx
import pygraphviz as pz

sensitive_func_list = []
untrusted_func_list = []

#put sensitive function names into sensitive_func_list
fo = open("lists.txt", "r")

line = fo.readline()
num = int(line, 10)
idx = 0

while (idx < num):
	idx = idx + 1
	line = fo.readline()
	line = line.rstrip('\n')
	sensitive_func_list.append(line)

#put untrusted function names into untrusted_func_list
line = fo.readline()
num = int(line, 10)
idx = 0

while(idx < num):
	idx = idx + 1
	line = fo.readline()
	line = line.rstrip('\n')
	untrusted_func_list.append(line) 

fo.close()

print(sensitive_func_list)
print(untrusted_func_list)


sensitive_func_id_list = []
untrusted_func_id_list = []
func_id_list = []
func_label_list = []

G = nx.nx_agraph.read_dot('callgraph.dot')

labels = nx.get_node_attributes(G, 'label')
for label in labels.items():
	func_label_list.append(label[1])
	func_id_list.append(label[0])

for sensitive_func in sensitive_func_list:
	func_str = "{" + str(sensitive_func) + "}"
	if func_str in func_label_list:
		idx = func_label_list.index(func_str)
		sensitive_func_id_list.append(func_id_list[idx])


for untrusted_func in untrusted_func_list:
	func_str = "{" + str(untrusted_func) + "}"
	if func_str in func_label_list:
		idx = func_label_list.index(func_str)
		untrusted_func_id_list.append(func_id_list[idx])
print(sensitive_func_id_list)
print(untrusted_func_id_list)

print("------------------\n")

for sensitive_func in sensitive_func_id_list:
	for untrusted_func in untrusted_func_id_list:
		paths = nx.all_simple_paths(G, source=sensitive_func, target=untrusted_func)
		for path in paths:
			path_len = len(path)
			idx = 0
			while(idx < path_len):
				if path[idx] in sensitive_func_id_list and idx + 1 < path_len:
					if not path[idx + 1] in untrusted_func_id_list:
						untrusted_func_id_list.append(path[idx + 1])
				idx = idx + 1


print(sensitive_func_id_list)
print(untrusted_func_id_list)

fo = open("objdump.txt", "r")

content = fo.readlines()
line_num = len(content)
idx = 0

while(idx < line_num):
	for sensitive_func in sensitive_func_list:
		func_str = "<" + str(sensitive_func) + ">:"
		if str(func_str) in str(content[idx]):
			idx = idx + 3
			print(sensitive_func, content[idx])
	idx = idx + 1		

fo.close()
