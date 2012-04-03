#! /bin/sh
if [ "$1" = "copy" ] ; then
	cp ../../../*.[ch] .
elif [ "$1" = "clean" ] ; then
	for i in `ls ../../../*.[ch]` ; do
		f=`echo $i | sed -e "sx../xxg"`
		rm $f
	done
else
	echo "please specify copy or clean"
	exit 1
fi

