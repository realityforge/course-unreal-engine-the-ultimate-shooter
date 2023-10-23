# TODO

Currently in the UE4 editor there is no way to compile all blueprints in your project. You can compile blueprints individually but there is no in editor button to compile all blueprints. Unreal Engine does offer way to do this via command line however by way of Commandlets.

Commandlets
Commandlets are command line programs that run inside of the Unreal Engine environment. They are most often used to make bulk changes to content, iterate over content to get information about it, or as a unit testing mechanism.

One of those commandlets is CompileAllBlueprints which not surprisingly will compile all blueprints in a project.

CompileAllBlueprints
UE4Editor-Cmd.exe MyGame\MyProject.uproject -run=CompileAllBlueprints

Running this will run through all blueprints in your project and let you know if there are any compilation issues that show up. Very handy!

Having the ability to compile all blueprints acts as a great sanity check on your project and can act as a great pre-commit check on assets before committing to source control.

An excellent extension I am considering to this would be to make a utility widget accessible in editor to trigger the compilation and have the results viewable in an output window.


"C:\Program Files\Epic Games\UE_5.1\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" C:\Projects\Shooter\pelor.uproject -run=CompileAllBlueprints
"C:\Program Files\Epic Games\UE_5.1\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" C:\Projects\Shooter\pelor.uproject -run=CompileAllBlueprints -Run=ResavePackages

----------------------------------------------------------------------------------------------------

So, just going to post my little discovery here... there's a commandlet for diffing arbitrary uassets as text files. No fancy UI like blueprint diffs, but this actually works on any arbitrary uasset (and potentially umaps too).
I have no clue when it was added, but at least as of UE5 you can do this (example uses p4merge to do the actual diff, the commandlet just outputs two text files and then calls into a third party diff tool)
```
UnrealEditor-Cmd.exe C:\Path\To\Your\Project.uproject -NoShaderCompile -run="DiffAssets" %1 %2 DiffCmd="C:\Program Files\Perforce\p4merge.exe {1} {2}"
```
Gives a result like this (this was a diff between two versions of a sequencer asset).
--

----------------------------------------------------------------------------------------------------

* Make sure you use `idea.max.intellisense.filesize = 25000` as custom property in PyCharm so that it can gather intellisense from generated `unreal.py`

* Clean unused assets:
   https://www.youtube.com/watch?v=W0uDbQZjKFU
   https://github.com/mamoniem/UnrealEditorPythonScripts/blob/master/Assets/DeleteUnusedAssets.py

* Add tool for performing diffs on unreal assets via textconv gitconfig setting
  https://git-scm.com/docs/gitattributes#_performing_text_diffs_of_binary_files

The following are a list of ideas on techniques to learn new skills/tools/techniques in Unreal.

* Import some models and animations from Mixamo and place them into the game
  - Some animations should be applied to the current player character and thus will require retargeting to the current IK_Rig for player.
  - Could also add a key press, that brings a UMG dialog up so that the player can quickly choose an animation to trigger.
  - See https://www.youtube.com/watch?v=_sLnCqBaElI for a good walk through on getting retargeting Mixamo working in Unreal

# Annoyances about UE5

## Curves

I want to define a single "curve" asset that has multiple curves in it and be able to name each curve axis. To do multiple curves at the moment you can use a CurveVector but there has to be 3 components (X, Y, Z) that can not be renamed. Alternatively maybe we want a curve group that allows presentation of multiple curves at the same time? See AItem::PulseCurve and AItem::EquippingPulseCurve as to why this is done.

Change all properties controlled by datatables to visible and blueprint read-only

Why are FABRIK NODES not moved Inline? (Then based on slot can decide whether they are active)

Also why separate shooting/reloading slots?

# Clang Tidy

Add check that methods in subclasses of "UBlueprintFunctionLibrary" are static
* Make sure that we do not directly call interface implementations (or _Implementation() default event logic) from within C++ code and instead go through UKismetSystemLibrary 
* Make sure we only allow Cast<>(...) on allow listed classes. Otherwise require that we go through `-bImpl = UKismetSystemLibrary::DoesImplementInterface(Actor, UDoSomeThings::StaticClass())`
