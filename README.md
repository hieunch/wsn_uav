
## Implemented routing algorithms
- GAMBAC (weight balance clustering)
- Strongest energy first 
- Random + f% bounded
- CHP (Convex-hull Based Protocol)

The source codes of all routing algorithms 
are located in `Castalia/Castalia/src/node/communication/routing` directory.

## Components

The system consists of 3 parts: 
* Castalia (forked from [Castalia](https://github.com/boulis/Castalia/)) as a simulator for Wireless Sensor Networks. As described
in the Castalia repo: "Castalia is a simulator for WSNs based on the OMNeT++ platform and can be used by researchers and developers who want to test their distributed algorithms and/or protocols in realistic wireless channel and radio models, with a realistic node behaviour especially relating to access of the radio".
* A web app acts as a tool to facilitate the process of generating sensor networks, 
creating holes, selecting regions of communication, sending simulation requests and displaying results
* Nodejs server act as a bridge from client and the simulator. Server receives simulation requests in the JSON format, transform the request into appropriate .ned Castalia network file, invoke the simulation, parse the result and send back to the client.

In this repo, some features are added to Castalia to support various types of 
logging, drawing and statistics aggregating. Several well-known routing algorithms in wireless sensor network dealing with the presence
of holes in literature are also implemented.

Here is basically how the system works:
![Alt text](images/system_architecture.png?raw=true "System architecture")


## Installing

Prerequisites
- Install [OMNeT++ 4.6](https://www.omnetpp.org/9-articles/software/3724-omnet-4-6-released)
- Install Castalia (this modified Castalia on this repo) by following the instruction on the original [Castalia repo](https://github.com/boulis/Castalia/)
- Install node.js and npm

Castalia
- Located in `Castalia/Castalia` directory
- After adding new routing algorithm or modify source code: `$ ./makemake` then ` make ` to rebuild Castalia (only takes seconds)
- Make sure `posse/Castalia/Castalia/bin` is in your PATH by append this into your .bashrc: `export PATH=[POSSE DIRECTORY PATH]/Castalia/Castalia/bin:$PATH`



Client
- Located in `/client` directory.
- Install dependencies: `$ yarn`
- Start the client: `$ yarn start`

Server
- Located in `/Castalia/Castalia/Simulations/server` directory
- Install dependencies: `$ yarn`
- Make sure we have a directory `logs` and `archives` in server directory: `mkdir logs && mkdir archives`
- Start the server: `$ yarn start`

 

