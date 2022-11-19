rmdir /s /q data downloads
del config.yaml
del ac-beetle.bat ac-qtpy_m0.bat ac-feather_m4.bat ac-xiao.bat ac-teensy.bat

arduino-cli config init ^
    --dest-file .\config.yaml

arduino-cli config set directories.data "%cd%\data" ^
    --config-file .\config.yaml

arduino-cli config set directories.downloads "%cd%\downloads" ^
    --config-file .\config.yaml

arduino-cli config set directories.user "%cd%" ^
    --config-file .\config.yaml

::::: Add Board Manager URLs

arduino-cli config add board_manager.additional_urls ^
    https://adafruit.github.io/arduino-board-index/package_adafruit_index.json ^
    --config-file .\config.yaml

arduino-cli config add board_manager.additional_urls ^
    https://files.seeedstudio.com/arduino/package_seeeduino_boards_index.json ^
    --config-file .\config.yaml

:: arduino-cli config add board_manager.additional_urls ^
::     https://www.pjrc.com/teensy/package_teensy_index.json ^
::     --config-file .\config.yaml

::::: Install Cores

:: arduino-cli core install --no-overwrite arduino:avr ^
::     --config-file .\config.yaml
:: echo set FQBN=arduino:avr:leonardo > ac-beetle.bat

arduino-cli core install --no-overwrite adafruit:samd ^
    --config-file .\config.yaml
:: echo set FQBN=adafruit:samd:adafruit_qtpy_m0 > ac-qtpy_m0.bat
echo set FQBN=adafruit:samd:adafruit_feather_m4 > ac-feather_m4.bat

arduino-cli core install --no-overwrite Seeeduino:samd ^
    --config-file .\config.yaml
echo set FQBN=Seeeduino:samd:seeed_XIAO_m0 > ac-xiao.bat

:: arduino-cli core install --no-overwrite teensy:avr ^
::     --config-file .\config.yaml
:: echo set FQBN=teensy:avr:teensy40 > ac-teensy.bat
