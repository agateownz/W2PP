import json
from pathlib import Path
from collections import Counter

analysis = json.loads(Path('graphify-out/.graphify_analysis.json').read_text(encoding='utf-8'))
communities = analysis['communities']

# Heuristic label assignment
labels = {}
for cid_str, members in communities.items():
    cid = int(cid_str)
    # Extract meaningful tokens from node IDs
    tokens = []
    for m in members:
        # Split by underscore, remove common prefixes/suffixes
        parts = m.lower().replace('code_', '').replace('tmsrv_', '').replace('dbsrv_', '').replace('msg_', '').replace('exec_', '').split('_')
        tokens.extend(parts)
    
    # Stopwords to ignore
    stopwords = {'cpp', 'h', 'bool', 'long', 'uint', 'lpstr', 'handle', 'hinstance', 'hwnd', 
                 'file', 'class', 'struct', 'code', 'server', 'exec', 'get', 'set', 
                 'process', 'send', 'base', 'basedef', 'item', 'mob', 'user', 'guild', 
                 'account', 'char', 'empty', 'read', 'write', 'init', 'main', 'func', 
                 'cpsock', 'dbsrv', 'tmsrv', 'client', 'patch', 'v7662', 'resource', 
                 'stdafx', 'language', 'plugin', 'package', 'opencode', 'dependencies', 
                 'schema', 'graphify', 'sln', 'vcxproj', 'e', 'wyd', 'test', 'setup', 
                 'w2pp', 'code', 'project', 'solution', 'items', 'visual', 'studio'}
    filtered = [t for t in tokens if t not in stopwords and len(t) > 1]
    
    # Use most common token to form a label
    most_common = Counter(filtered).most_common(2)
    
    if cid == 13:
        labels[cid] = "Project Documentation"
    elif cid == 24:
        labels[cid] = "Build System"
    elif cid == 23:
        labels[cid] = "Client Patch"
    elif cid == 37:
        labels[cid] = "OpenCode Config"
    elif cid == 38:
        labels[cid] = "OpenCode Package"
    elif cid == 41:
        labels[cid] = "Graphify Plugin"
    elif 'readme' in tokens or 'license' in tokens:
        labels[cid] = "Project Documentation"
    elif 'cfiledb' in tokens:
        labels[cid] = "File Database"
    elif 'cranking' in tokens:
        labels[cid] = "Ranking System"
    elif 'ccastlezakum' in tokens:
        labels[cid] = "Castle Zakum"
    elif 'cwartower' in tokens:
        labels[cid] = "War Tower"
    elif 'cnpcgene' in tokens:
        labels[cid] = "NPC Generation"
    elif 'creadfiles' in tokens:
        labels[cid] = "Config Reader"
    elif 'citem' in tokens:
        labels[cid] = "Item System"
    elif 'cmob' in tokens:
        labels[cid] = "Mob System"
    elif 'cuser' in tokens:
        labels[cid] = "User Sessions"
    elif 'cpsock' in tokens:
        labels[cid] = "Socket Layer"
    elif 'processclientmessage' in tokens:
        labels[cid] = "Client Message Handler"
    elif 'processdbmessage' in tokens:
        labels[cid] = "DB Message Handler"
    elif 'sendfunc' in tokens:
        labels[cid] = "Network Broadcasting"
    elif 'getfunc' in tokens:
        labels[cid] = "Utility Queries"
    elif 'basedef' in tokens:
        labels[cid] = "Base Definitions"
    elif 'winmain' in tokens or 'mainwndproc' in tokens or 'initapplication' in tokens:
        labels[cid] = "Main Server Loop"
    elif 'attack' in tokens and 'damage' in tokens:
        labels[cid] = "Combat System"
    elif 'combine' in tokens:
        labels[cid] = "Item Crafting"
    elif 'trade' in tokens:
        labels[cid] = "Trade System"
    elif 'quest' in tokens:
        labels[cid] = "Quest System"
    elif 'guild' in tokens and 'war' in tokens:
        labels[cid] = "Guild War"
    elif 'teleport' in tokens:
        labels[cid] = "Teleport System"
    elif 'chat' in tokens:
        labels[cid] = "Chat System"
    elif 'party' in tokens:
        labels[cid] = "Party System"
    elif 'login' in tokens or 'account' in tokens:
        labels[cid] = "Account System"
    elif 'billing' in tokens:
        labels[cid] = "Billing System"
    elif most_common:
        label_words = [w.capitalize() for w, _ in most_common[:2]]
        labels[cid] = ' '.join(label_words)
    else:
        labels[cid] = f"Cluster {cid}"

# Ensure all communities have a label
for cid in communities:
    cid = int(cid)
    if cid not in labels:
        labels[cid] = f"Cluster {cid}"

# Now regenerate report and save labels
from graphify.build import build_from_json
from graphify.cluster import score_all
from graphify.analyze import god_nodes, surprising_connections, suggest_questions
from graphify.report import generate

extraction = json.loads(Path('graphify-out/.graphify_extract.json').read_text(encoding='utf-8'))
detection  = json.loads(Path('graphify-out/.graphify_detect.json').read_text(encoding='utf-8'))

G = build_from_json(extraction)
communities_int = {int(k): v for k, v in analysis['communities'].items()}
cohesion = {int(k): v for k, v in analysis['cohesion'].items()}
tokens = {'input': extraction.get('input_tokens', 0), 'output': extraction.get('output_tokens', 0)}

questions = suggest_questions(G, communities_int, labels)

report = generate(G, communities_int, cohesion, labels, analysis['gods'], analysis['surprises'], detection, tokens, '.', suggested_questions=questions)
Path('graphify-out/GRAPH_REPORT.md').write_text(report, encoding='utf-8')
Path('graphify-out/.graphify_labels.json').write_text(json.dumps({str(k): v for k, v in labels.items()}), encoding='utf-8')
print('Report updated with community labels')
for cid, label in sorted(labels.items()):
    print(f'  {cid}: {label}')
