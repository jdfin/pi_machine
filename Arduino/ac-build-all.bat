for /d %%s in (2022*) do (
    arduino-cli compile %%s ^
        --config-file ./config.yaml ^
        --fqbn %FQBN%
)
