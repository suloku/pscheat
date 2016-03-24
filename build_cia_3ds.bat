set ProjectName=pscheat
echo pscheat
res/arm-none-eabi-strip.exe pscheat.elf
res/bannertool.exe makebanner -i res/banner.png -a res/audio.wav -o res/banner.bnr
res/bannertool.exe makesmdh -s "pscheat" -l "pscheat" -p "suloku" -i icon.png  -o res/icon.icn
res/makerom.exe -f cia -o pscheat.cia -DAPP_ENCRYPTED=false -rsf res/build_cia.rsf -target t -exefslogo -elf pscheat.elf -icon res/icon.icn -banner res/banner.bnr -DAPP_SYSTEM_MODE="64MB" -DAPP_SYSTEM_MODE_EXT="Legacy"
echo Done!