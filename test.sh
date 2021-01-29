./emu -c 200 test/nestest.nes
diff <(head -80 testlog.log) <(head -80 test/nestest.log)
