echo "---   compile start    ---"
gcc gen.c -Wall -o gen
if [ "$2" == "1" ]
then
    gcc genmain.c scheduler.c threads.c -Wall -o shin
elif [ "$2" == "2" ]
then
    gcc genmain.c scheduler.c threads2.o -Wall -Dabc -o shin
fi

echo "--- start running(gen) ---"
./gen $1 $2
echo "---  start running(1)  ---"
./cmd.sh shin > ur_out
echo "---  start running(2)  ---"
if [ "$2" == "1" ]
then
    ./cmd.sh remi > out
elif [ "$2" == "2" ]
then
    ./cmd.sh remi2 > out
fi
echo "---  start comparing   ---"
DIFF=$(diff ur_out out)

if [ "$DIFF" != "" ]
then
    echo "Wrong Output"
    echo "$(DIFF)"
fi
