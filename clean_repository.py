import subprocess


def has_history(filename):
    """Determine if the file has changes in gitm other than the initial commit. 

    :param filename: the filename to check relative to the project directory. (i.e. Content/ParagonTwinblast/TwinBast.umap) 
    :return: true if the file has had changes committed outside the initial add.
    """
    result = subprocess.run(['git', 'log', '--follow', '--format=oneline', '--', filename], stdout=subprocess.PIPE)

    # Convert output to string
    output = result.stdout.decode('utf-8')

    # Output will contain a line like "ce5fecb0dee67fada27188162390d80c43736b3e Some commit message"
    # for every commit. So a file with a single commit has a single line.

    lines = output.splitlines()
    return 1 != len(lines)

# The following process will be used to clean the repository

# 1. Convert from Git-LFS back to a regular form repository with assets inlined
# 2. Identify "Unused assets" using python script.
#    This will produce a list of assets that can be potentially removed
#    This step should also produce a "Used assets" list.
# 3. For any "Unused asset", check whether it has multiple versions in git history.
#    If it has multiple versions then *maybe* remove from unused list as we may have
#    changed it in some commit. May require manual investigation to determine whether
#    it stays in the list.
# 4. Scan git history for "historic" assets that have been removed. If they were not
#    an earlier version of an asset (i.e. --follow flag does not somehow mark them as
#    a rename of something we are keeping) then add them to "Used assets" list.
# 5. Rewrite git history, removing any assets that are in the Unused list.
# 6. Run the asset renamer script to get all asset rename goodness.
# 7. Run the asset organiser script to get place assets in all the "right" locations.
# 8. Run the asset linter script to make sure I got it all correct.
# 9. Push the repository to github.


#print(has_history('Content/ParagonTwinblast/TwinBast.umap'))
