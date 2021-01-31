CYCLES=2200
LINES=940
./emu -c $CYCLES test/nestest.nes
diff <(head -$LINES testlog.log) <(head -$LINES test/nestest.log)
