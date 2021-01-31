CYCLES=3300
LINES=1320
./emu -c $CYCLES test/nestest.nes
diff <(head -$LINES testlog.log) <(head -$LINES test/nestest.log)
