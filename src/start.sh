#	Sabin Raj Tiwari
#	CMSC 621
#	Project 3

PORT1=$1
PORT2=$2
PORT3=$3
PORT4=$4

# Start the backend servers.
./backend $2 &
./backend $3 &
./backend $4 &

# Start the coordinator servers.
sleep 0.5s
./server $1 "localhost" $2 $3 $4

# Wait for the programs to converge.
wait 