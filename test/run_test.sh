echo "Runnig unit tests:"
echo "------------------"

for i in test/*_test
do
    if test -f $i
    then
        if $VALGRIND ./$i 2>> test/process.log
        then
           echo "----------"
        else
           echo "Error log: test/process.log\n"
        fi
    fi
    echo ""
    
    
done

echo ""