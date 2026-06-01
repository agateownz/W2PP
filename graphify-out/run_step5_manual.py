import json
from pathlib import Path
from graphify.build import build_from_json
from graphify.cluster import score_all
from graphify.analyze import god_nodes, surprising_connections, suggest_questions
from graphify.report import generate

analysis = json.loads(Path('graphify-out/.graphify_analysis.json').read_text(encoding='utf-8'))

labels = {
    0: "Base Item & Mob Helpers",
    1: "Item Crafting & Trade",
    2: "Data Initialization & I/O",
    3: "DB Server Core",
    4: "Castle & Config Systems",
    5: "Player Commerce & Actions",
    6: "Combat & Battle Logic",
    7: "Networking & Sessions",
    8: "File Database Engine",
    9: "Guild & World Features",
    10: "Broadcast Functions",
    11: "Account & Server Utils",
    12: "Social & Account Systems",
    13: "Project Documentation",
    14: "Main Server Loop",
    15: "Mob AI & Pathing",
    16: "Guild War System",
    17: "World Event Systems",
    18: "Ranking System",
    19: "Player Interactions",
    20: "NPC Generation",
    21: "DB Data Import",
    22: "Damage Calculation",
    23: "Client Patch",
    24: "Build System",
    25: "Ranking Class",
    26: "Billing & Logout",
    27: "NPC Generator Class",
    28: "Socket Class",
    29: "FileDB Class",
    30: "DB User Class",
    31: "Item Class Header",
    32: "Mob Class Header",
    33: "ReadFiles Header",
    34: "TM User Class",
    35: "War Tower Class",
    36: "DB ReadFiles Class",
    37: "OpenCode Config",
    38: "OpenCode Package",
    39: "Castle Zakum Class",
    40: "Item Class Implementation",
    41: "Graphify Plugin",
    42: "Base Definitions Header",
    43: "DB Server Header",
    44: "DB Resources",
    45: "DB Precompiled Header",
    46: "GetFunc Header",
    47: "ProcessDBMessage Header",
    48: "SendFunc Header",
    49: "TM Server Header",
    50: "TM Resources",
}

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
