CYCLES=9700
LINES=$(wc testlog.log | cut -d' ' -f5)
./emu -s 0xc000 -c $CYCLES test/nestest.nes
diff -c <(head -$LINES testlog.log) <(head -$LINES test/nestest.log)
