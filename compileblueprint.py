import subprocess

engine = "C:/Program Files/Epic Games/UE_5.1/Engine/Binaries/Win64/UnrealEditor-cmd.exe"
project = "D:/Code/realityforge/Shooter/pelor.uproject"
cmd = [
    engine,
    project,
    "-editortest",
    "-Execcmds=\"Automation SetFilter Stress, Automation list, Automation RunTest Project.Blueprints.Compile Blueprints\"",
    "-testexit=\"Automation Test Queue Empty\""
]

cmd = " ".join(cmd)
# print(cmd)
subprocess.run(cmd)