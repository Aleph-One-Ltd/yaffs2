#! /bin/sh
set -x


iterations=100000
n_procs=$(nproc)
instances=$((n_procs / 4))

[ -z $1 ]  || iterations=$1
[ -z $2 ]  || instances=$2

echo "Launching $instances instances each of $iterations iterations"

LAUNCHDIR=`pwd`
RUNDIR=`pwd`/tmp
mkdir $RUNDIR

# Check if RUNDIR is mounted, if not, mount as tmpfs
# because we don't want to hammer the disk.
if [ -z "$(mount | grep $RUNDIR)" ]; then
sudo mount -t tmpfs -osize=15G none $RUNDIR
sudo chmod a+wr $RUNDIR
fi


for i in $(seq $instances)
do
  xterm  -e "$LAUNCHDIR/manage_nor_test.sh  $i $iterations"&
  xterm  -e "$LAUNCHDIR/manage_m18_test.sh  $i $iterations"&
  xterm  -e "$LAUNCHDIR/manage_nand_test.sh $i $iterations"&
done

