import json, glob
from pathlib import Path
from graphify.semantic_cleanup import load_validated_semantic_fragment, sanitize_semantic_fragment
from graphify.cache import save_semantic_cache

# Step B3 - Merge chunks
chunks = sorted(glob.glob('graphify-out/.graphify_chunk_*.json'))
all_nodes, all_edges, all_hyperedges = [], [], []
total_in, total_out = 0, 0
for c in chunks:
    d, errors = load_validated_semantic_fragment(Path(c))
    if errors:
        print(f'Skipping invalid chunk {c}: ' + '; '.join(errors[:3]))
        continue
    d = sanitize_semantic_fragment(d)
    all_nodes += d.get('nodes', [])
    all_edges += d.get('edges', [])
    all_hyperedges += d.get('hyperedges', [])
    total_in += d.get('input_tokens', 0)
    total_out += d.get('output_tokens', 0)
Path('graphify-out/.graphify_semantic_new.json').write_text(json.dumps({
    'nodes': all_nodes, 'edges': all_edges, 'hyperedges': all_hyperedges,
    'input_tokens': total_in, 'output_tokens': total_out,
}, indent=2))
print(f'Merged {len(chunks)} chunks: {total_in:,} in / {total_out:,} out tokens')

# Save cache
new = json.loads(Path('graphify-out/.graphify_semantic_new.json').read_text())
saved = save_semantic_cache(new.get('nodes', []), new.get('edges', []), new.get('hyperedges', []))
print(f'Cached {saved} files')

# Merge cached + new into semantic.json
cached = json.loads(Path('graphify-out/.graphify_cached.json').read_text()) if Path('graphify-out/.graphify_cached.json').exists() else {'nodes':[],'edges':[],'hyperedges':[]}
all_nodes = cached['nodes'] + new.get('nodes', [])
all_edges = cached['edges'] + new.get('edges', [])
all_hyperedges = cached.get('hyperedges', []) + new.get('hyperedges', [])
seen = set()
deduped = []
for n in all_nodes:
    if n['id'] not in seen:
        seen.add(n['id'])
        deduped.append(n)

merged = {
    'nodes': deduped,
    'edges': all_edges,
    'hyperedges': all_hyperedges,
    'input_tokens': new.get('input_tokens', 0),
    'output_tokens': new.get('output_tokens', 0),
}
merged = sanitize_semantic_fragment(merged)
Path('graphify-out/.graphify_semantic.json').write_text(json.dumps(merged, indent=2))
print(f'Extraction complete - {len(deduped)} nodes, {len(all_edges)} edges ({len(cached["nodes"])} from cache, {len(new.get("nodes",[]))} new)')

# Part C - Merge AST + semantic into final extraction
import sys
ast = json.loads(Path('graphify-out/.graphify_ast.json').read_text())
sem = json.loads(Path('graphify-out/.graphify_semantic.json').read_text())

seen = {n['id'] for n in ast['nodes']}
merged_nodes = list(ast['nodes'])
for n in sem['nodes']:
    if n['id'] not in seen:
        merged_nodes.append(n)
        seen.add(n['id'])

merged_edges = ast['edges'] + sem['edges']
merged_hyperedges = sem.get('hyperedges', [])
merged_final = {
    'nodes': merged_nodes,
    'edges': merged_edges,
    'hyperedges': merged_hyperedges,
    'input_tokens': sem.get('input_tokens', 0),
    'output_tokens': sem.get('output_tokens', 0),
}
merged_final = sanitize_semantic_fragment(merged_final)
Path('graphify-out/.graphify_extract.json').write_text(json.dumps(merged_final, indent=2))
print(f'Merged: {len(merged_nodes)} nodes, {len(merged_edges)} edges ({len(ast["nodes"])} AST + {len(sem["nodes"])} semantic)')
