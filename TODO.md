# TODO

The following are a list of ideas on techniques to learn new skills/tools/techniques in Unreal.

* Import some models and animations from Mixamo and place them into the game
  - Some animations should be applied to the current player character and thus will require retargeting to the current IK_Rig for player.
  - Could also add a key press, that brings a UMG dialog up so that the player can quickly choose an animation to trigger.
  - See https://www.youtube.com/watch?v=_sLnCqBaElI for a good walk through on getting retargeting Mixamo working in Unreal

# Annoyances about UE5

## Curves

I want to define a single "curve" asset that has multiple curves in it and be able to name each curve axis. To do multiple curves at the moment you can use a CurveVector but there has to be 3 components (X, Y, Z) that can not be renamed. Alternatively maybe we want a curve group that allows presentation of multiple curves at the same time? See AItem::PulseCurve and AItem::EquippingPulseCurve as to why this is done.
