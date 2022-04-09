# EECE446_program3

## How to use
You may need to open 3 terminals for p3_registory,p3_peer and peer_v2.

### runn the programs
1.
```
./p3_registory localhost 2010 -d -t
```
localhost is the ip address where you want to run the registory. In this case I put localhost because it runs on jaguar.

2010 is a port number. You can choose any number but it needs to be the same as the number you use for other programs

2.
```
./p3_peer localhost 2010 201111 -d -t
```

The first two arguments are the same as p3_registory. 20111 is a unique id for this peer.

3.
```
./peer localhost 2010 20123
```

All the arguments are the same as p3_peer.

### commands
1 = the terminal for p3_regisotry

2 = the terminal for p3_peer

3 = the terminal for peer

2
```
JOIN
PUBLISH
REGISTER
```

join the registory, publish what it has and register them.

3
```
JOIN
FETCH
aaa.pdf
```
join the registory, FETCH the file 

And you would see the file in the local directory.

