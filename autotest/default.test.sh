#!/bin/bash

TESTDIR=$1
TESTCONF=$2
RESULTDIR=$3

if [ ! -d "$RESULTDIR" ]; then
	mkdir -p $RESULTDIR
fi

CHECKINTERVALL=3

# Testconf "seden", mehrfache Tabulatoren durch einen erstezen, Kommentarzeilen löschen
sed 's/	[	]*/	/g' autotest/$TESTCONF > autotest/$TESTCONF.temp
sed '/^#.*/d' autotest/$TESTCONF.temp > autotest/$TESTCONF.sed
rm autotest/$TESTCONF.temp

TESTS=`cat autotest/$TESTCONF.sed | awk -F '	' '{ print $1 }'`

# ~/.valid.machines file erstellen
autotest/selectmachines.sh > /dev/null 2>/dev/null

for TEST in $TESTS
do
	DURATION=`grep -w -e "^$TEST" autotest/$TESTCONF.sed | awk -F '	' '{ print $2 }'`
	ARGS=`grep -w -e "^$TEST" autotest/$TESTCONF.sed | awk -F '	' '{ print $3 }'`
	TEST_SUCCESS=`grep -w -e "^$TEST" autotest/$TESTCONF.sed | awk -F '	' '{ print $4 }'`
	SUCCESS_SCAN=`grep -w -e "^$TEST" autotest/$TESTCONF.sed | awk -F '	' '{ print $5 }'`
	
	cd $TESTDIR
	make $TEST
	
	START=`date +%s`	# speichere Start und Endzeit in Sekunden seit 01.01.1970 (d.h. per Unix-Timestamp)
	END=$(($START+$DURATION*60))
	
	$OLDPWD/bin/mpirun -machinefile ~/.valid.machines $ARGS $TEST 2>$RESULTDIR/$TEST.stderr > $RESULTDIR/$TEST.stdout &
	
	PID=$!
	while [ `date +%s` -lt $END ]
	do
		if (ps $PID > /dev/null)
		then
			sleep $CHECKINTERVALL
			continue
		else
			break
		fi
	done
	if [ `date +%s` -ge $END ]; then
		echo "maximum duration for test $TEST reached! Killing test $TEST now!" >> $RESULTDIR/$TEST.stdout
	fi
	# Prozess beenden (SIGKILL)
	if (ps $PID > /dev/null)
	then
		for host in `cat ~/.valid.machines`
		do			
			rsh $host pkill -9 -x $TEST
		done
		sleep 2
	fi
	# wenn der Prozess dann IMMERNOCH existiert, Fehlermeldung ausgeben
	if (ps $PID > /dev/null) then
		echo "ERROR: process $PID of test $TEST cannot be killed"
	fi
	
	## "diff" <exact> oder "grep -E" <regexp>
	cd -
	if [ "$SUCCESS_SCAN" = "regexp" ]; then
		LINES=`wc -l $TESTDIR/$TEST_SUCCESS | cut -d ' ' -f 1`
		i=1
		FAILURE=1
		while [ $i -le $LINES ];
		do
			REGEXP=`head -n $i $TESTDIR/$TEST_SUCCESS | tail -n 1`
			head -n $i $RESULTDIR/$TEST.stdout | tail -n 1 | grep -E "$REGEXP" > /dev/null
			if [ $? != 0 ]; then
				#echo "$TEST:failure" >> $RESULTDIR/result
				FAILURE=0
				break
			fi
			i=$(($i+1))
		done
		if [ $FAILURE = 1 ];
		then
			echo "$TEST:success" >> $RESULTDIR/result
		else
			echo "$TEST:failure" >> $RESULTDIR/result
		fi
	elif [ "$SUCCESS_SCAN" = "exact" ]; then
		if (diff $TESTDIR/$TEST_SUCCESS $RESULTDIR/$TEST.stdout > /dev/null)
		then
			echo "$TEST:success" >> $RESULTDIR/result
		else
			echo "$TEST:failure" >> $RESULTDIR/result
		fi
	else
		echo "$ME: unknown scan method \"$SUCCESS_SCAN\"."
	fi
done
