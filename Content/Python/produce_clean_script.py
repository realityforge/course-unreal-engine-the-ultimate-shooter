from os.path import exists


def chunker(seq, size):
    return (seq[pos:pos + size] for pos in range(0, len(seq), size))


files = []
with open(r"C:\Projects\Shooter\tmp\UnusedAssets.txt", "r") as f:
    files.append(f.readlines())

with open(r'C:\Projects\Shooter\Content\Python\maps_assets_to_remove.txt', 'r') as f:
    files.append(f.readlines())

for group in chunker(files, 10):
    args = ['python', r'C:\Projects\Shooter\Content\Python\git-filter-repo.py']
    matched = False
    for f in group:
        if f.startswith('/Game/_Game'):
            continue
        file = f.replace('/Game', 'Content') + '.uasset'
        if exists(file):
            args.append('--path')
            args.append(file)
            matched = True
        file = f.replace('/Game', 'Content') + '.umap'
        if exists(file):
            args.append('--path')
            args.append(file)
            matched = True

    args.append('--invert-paths')
    args.append('--force')

    if matched:
        print(' '.join(args))
