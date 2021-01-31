CYCLES=2500
LINES=1070
./emu -c $CYCLES test/nestest.nes
diff <(head -$LINES testlog.log) <(head -$LINES test/nestest.log)
