CYCLES=9700

./emu -s 0xc000 -c $CYCLES test/nestest.nes
LINES=$(wc testlog.log | cut -d' ' -f5)
diff -c <(head -$LINES testlog.log) <(head -$LINES test/nestest.log)
