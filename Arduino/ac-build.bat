
arduino-cli compile %1 ^
    --config-file ./config.yaml ^
    --fqbn %FQBN%

::  --clean
::  --verbose
::  --build-path "%CD%/build"

if "%2" == "" goto :eof

arduino-cli upload %1 -p %2 ^
    --config-file ./config.yaml ^
    --fqbn %FQBN%

::  --verbose
::  --build-path "%CD%/build"
