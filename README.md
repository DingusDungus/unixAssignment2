# Unix assignment 2
#### Noah and Gabriel, undefeatable wifes of great intellectual academic arts

## Server and client
### Server
For the server end of things, being in the `/mathserver` directory, run the command:
```
    make server
```
After compilation is done (and hopefully error free), the server can be run in same folder through:
```
    ./mathserver
```
Add `-h` to the command to recieve further information

### Client
For the client end of things, being in the `/unixAssignment2` directory, run the command:
```
    make client
```
After compilation is done, the client can be run in same directory through:
```
    ./client [server port] [server-ip] (ex ./client 3000 0.0.0.0)
```
## Kmeans and matrix-inversion
Everything is done in directory: `./unixAssignment2`
### Matrix-inversion
For compilation run:
```
    make matinv
```
### Kmeans
For compilation run:
```
    make kmeans
```
#### Done
Now you have successfully compiled everything and have the knowledge to run the software, good luck with your server :)