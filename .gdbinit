set architecture arm
target remote localhost:3333
mon reset halt
load
break main
continue
tui enable
