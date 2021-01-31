CYCLES=2700
LINES=1110
./emu -c $CYCLES test/nestest.nes
diff <(head -$LINES testlog.log) <(head -$LINES test/nestest.log)
