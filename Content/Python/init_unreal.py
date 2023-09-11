import realityforge.assetlib.validator as validator
import realityforge.scheme.asset_naming_registration as asset_naming_registration

asset_naming_registration.load_rules()
validator.setup()
