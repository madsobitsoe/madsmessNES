CYCLES=25000

./emu -s 0xc000 -c $CYCLES test/nestest.nes
UNAME=$(uname)
if [ $UNAME = "Linux" ]
then
    LINES=$(wc -l testlog.log | cut -d' ' -f1)
else
    LINES=$(wc testlog.log | cut -d' ' -f5)
fi

echo "Comparing $LINES lines"
diff -c <(head -n "$LINES" testlog.log) <(head -n "$LINES" test/nestest.log)
