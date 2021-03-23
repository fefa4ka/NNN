echo "Runnig unit tests:"
echo "------------------\n"

for i in test/*_test
do
    if test -f $i
    then
        if $VALGRIND ./$i 2>> test/process.log
        then
           echo "----------"
        else
           echo "[FAILED] ./$i: Read more test/process.log\n"
        fi
    fi
    echo ""
    
    
done

echo ""
