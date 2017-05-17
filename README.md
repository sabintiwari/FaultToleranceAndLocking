## Sabin Raj Tiwari | CMSC 621 Project 3

***
#### Make
***
If there are already compiled files in the `src` directory, first run:

```bash
make clean
```

Then, to build the program, run:

```bash
make compile
```

This will create executable files: `client`, `server`, and `backend`

***
#### Run
***
The server and backend can be run using the bash script. After the servers are up and running, the clients can be run.
***
###### Server and Backend (parameters: `server_port`, `backend_port_1`, `backend_port_2`, `backend_port_3`)
All the ports have to be distinct. The client will use the `server_port` to connect. Once the servers are started, the backend processes will write their process ids to the terminal. The pid can be used to kill one of the backend servers to simulate a server crash. The script will start one `server` process that connects to three `backend` processes.
```bash
start.sh 3000 3001 3002 3003
```
###### Client (parameters: `server_address`, `server_port`)
The client must be started using the address and port of the front-end server (i.e. the first port when running the `start.sh` script). Make sure to start the client after the servers are all up and running.
```bash
client localhost 3000
```
###### Backend (parameters: `port`) and Server (parameters: `server_port`, `backend_host`, `backend_port_1`, `backend_port_2`, `backend_port_3`)
If running each server process on separate terminals, make sure to start the backend processes first.
```bash
backend 3001
```
Then start the server with the ports for the three backend processes.
```bash
server 3000 localhost 3001 3002 3003
```
***
#### Design
The overall design of the program is based on the project requirements.
***
###### Two-phase commit and fault tolerance
The two-phase commit gets the vote from all the backend servers before committing or aborting the transaction. If the front-end process notices that a backend server has crashed, it will add the internal id for that backend server to a blacklist so that no connections are attempted to make to that server. Since it is an all or nothing vote, the front-end server will only count the votes from alive backend processes even if there are even number of servers alive.
###### Locking
The locking for the program is implemented in the `server` code. Each of the threads that are handling the client requests will use `pthread_mutex` to make sure that the critical sections that update the data in the servers are properly synchronized so that there are no race conditions and write issues. When the `server` gets a client request, it will spawn a thread. The `server` has a loop that keeps reading inputs from the `client` and quits after the `client` sends "QUIT" or disconnects. This is where the critical section starts. The mutex will lock the condition variable. The `server` will start a vote among the `backend` processes for the and will wait for the transaction to either commit or abort. Then it will release the lock.
###### Inputs for Client
The queries for the clients are validated so any invalid queries will be denied. When performing CREATE and UPDATE, the money have to be in decimal format. So 300 dollars should be entered as 300.00 to validate the input. The account ids have to be entered as integers.