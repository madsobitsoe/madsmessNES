CYCLES=4200
LINES=1600
./emu -c $CYCLES test/nestest.nes
diff <(head -$LINES testlog.log) <(head -$LINES test/nestest.log)
