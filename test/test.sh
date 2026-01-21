#!/bin/sh

BITRATE=1000000
GDB=arm-none-eabi-gdb

trap cleanup EXIT INT TERM

exit_help() {
    cat<<EOF
Usage: $0 [OPTION] BOARD INTERFACE_DUT INTERFACE_AUX
  -d              check diff between sent/received frames
  -g              GDB remote address [:3333]
  -l              skip program loading
  -r              skip device reset (implies -l)
  -h              display this help and exit
EOF
    exit 0
}

exit_arguments() {
    printf "$@"
    printf "Try '%s -h' for more information.\n" $0
    exit 2
}

set_skip_send() {
    kill -INT $gdb_pid
    tail -f -n 0 $gdb_out | grep -q led_update &
    cat >$gdb_in <<EOF
tbreak led_update
continue
EOF
    wait $! || exit 1
    cat >$gdb_in <<EOF
set skip_send=$1
continue
EOF
}

get_free_buffers() {
    kill -INT $gdb_pid
    tail -f -n 0 $gdb_out | grep -q led_update &
    cat >$gdb_in <<EOF
tbreak led_update
continue
EOF
    wait $! || exit 1
    sleep 1
    tail -f -n 0 $gdb_out | head -1 >$tmpdir/get_free_buffers &
    cat >$gdb_in <<EOF
count_nexts hGS_CAN.list_frame_pool
continue
EOF
    wait $! || exit 1
    free_buffers=`sed 's/$[[:digit:]]* = //' $tmpdir/get_free_buffers`
}

cleanup() {
    kill "$!" 2>/dev/null
    kill "$candump_dut_pid" 2>/dev/null
    kill "$candump_aux_pid" 2>/dev/null
    if [ -n "$gdb_pid" ]
    then
	kill -INT $gdb_pid
	echo "quit" > $gdb_in
    fi
    if $keep_tmpdir
    then
	printf "Keeping temporary directory in %s\n" $tmpdir
    else
	rm -rf $tmpdit
    fi
    trap - EXIT
    exit
}

keep_tmpdir=false

dut_gdb_remote=:3333
dflag=
lflag=
rflag=
while getopts dg:lrh name
do
    case $name in
	d) dflag=1;;
	g) dut_gdb_remote=$OPTARG;;
	l) lflag=1;;
	r) rflag=1; lflag=1;;
	h) exit_help;;
	?) exit_arguments;;
    esac
done
shift $(($OPTIND - 1))
test $# -lt 3 && exit_arguments "Not enough arguements.\n"

binary=$1_test_fw
interface_dut=$2
interface_aux=$3

tmpdir=/tmp/`basename $0`.$PPID
rm -rf $tmpdir
mkdir -p $tmpdir

gdb_in=$tmpdir/gdb_in
gdb_out=$tmpdir/gdb_out
candump_dut=$tmpdir/candump_dut
candump_aux=$tmpdir/candump_aux

mkfifo $gdb_in

if ! command -v $GDB >/dev/null
then
    printf "%s not found\n" $GDB
    exit 2
fi

keep_tmpdir=true

$GDB <>$gdb_in $binary >>$gdb_out 2>&1 &
gdb_pid=$!

cat >$gdb_in <<EOF
define count_nexts
  set \$count = 0
  set \$i = \$arg0.next
  while \$i != &(\$arg0)
    set \$count = \$count + 1
    set \$i = \$i->next
  end
  p \$count
end
target extended-remote $dut_gdb_remote
EOF

test -z "$lflag" && echo "load" >$gdb_in
if [ -z "$rflag" ]
then
    echo "run" >$gdb_in
    sleep 10
else
    echo "continue" >$gdb_in
fi

sudo ip link set $interface_aux down
sudo ip link set $interface_aux type can bitrate $BITRATE
sudo ip link set $interface_aux up

sudo ip link set $interface_dut down
sudo ip link set $interface_dut type can bitrate $BITRATE
sudo ip link set $interface_dut up

get_free_buffers
msgbuf_count_idle=$free_buffers

candump $interface_dut >>$candump_dut &
candump_dut_pid=$!
candump $interface_aux >>$candump_aux &
candump_aux_pid=$!

set_skip_send 1
for i in `seq 1 $msgbuf_count_idle`; do cansend $interface_aux 001#`printf %02x $i`; done 2> $tmpdir/cansend_aux.err
for i in `seq 1 $msgbuf_count_idle`; do cansend $interface_dut 000#`printf %02x $i`; done 2> $tmpdir/cansend_dut.err
set_skip_send 0

sleep 2

get_free_buffers
msgbuf_count_after=$free_buffers

printf "Free buffers idle=%d, after=%d\n" $msgbuf_count_idle $msgbuf_count_after
test $msgbuf_count_idle -eq $msgbuf_count_after || exit 1

awk '{$1=""; print $0}' $candump_dut >> $tmpdir/frames_dut
awk '{$1=""; print $0}' $candump_aux >> $tmpdir/frames_aux
test -z "$dflag" || diff $tmpdir/frames_aux $tmpdir/frames_dut || exit 1

keep_tmpdir=false
