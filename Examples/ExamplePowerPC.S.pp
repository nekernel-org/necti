# Path: Examples/ExamplePowerPC.S.pp
# Language: PowerPC Assembly
# Build Date: 2024-6-4

bl 0x1000


li r1, 0x500
mr r2, r1
sc
blr
