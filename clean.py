import os
import shutil


# clean all folder named as debug, release and x64 under current project.

dirs_to_delete = []

# enumerate current directory
for root, dirs, files in os.walk("."):
    # print("root:", root, "dirs:", dirs, "files:", files, "\n")
    if 'Debug' in dirs:
        dirs_to_delete.append(root + '/Debug')
    if 'Release' in dirs:
        dirs_to_delete.append(root + '/Release')
    if 'x64' in dirs:
        dirs_to_delete.append(root + '/x64')

# print dirs_to_delete
print(dirs_to_delete)

# delete directory in dirs_to_delete
for dir in dirs_to_delete:
    # delete directory
    # convert relative path to absolute path
    absolute_path = os.path.abspath(dir)
    try:
        shutil.rmtree(absolute_path)
    except Exception as e:
        print(f"Failed to delete {absolute_path}. Reason: {e}")
    
