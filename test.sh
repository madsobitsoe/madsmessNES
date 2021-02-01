CYCLES=9600
LINES=$(wc testlog.log | cut -d' ' -f5)
./emu -c $CYCLES test/nestest.nes
diff -c <(head -$LINES testlog.log) <(head -$LINES test/nestest.log)
