# nlcat
netlink monitoring and dispatch service

## What is nlcat
In short, nlcat is netcat for netlink sockets.  The utility listens for various
netlnk events and dispatches notification applications in response to them,
passing the netlink data to them on stdin (massaged into a json format for easy
consumption.  This is useful when a platform wishes to do event based monitoring
for things like ip address changes, route additions, etc.

## How does it work
For each message received, nlcat will print json output to stdout, delimited by
a newline, making for easy-ish parsing with a shell script, and command line
tools like jq.  Errors are reported on stderr.

## How to I build it
./autogen.sh
./configure
make

## What do I need to build it
Currently, nlcat requires libnl3 for netlink monitoring and libjannson for json
string construction.

## How stable is it
Its not, its a work in progress.  Expect the json scheema to evolve and change
somewhat.  Currently only a few message types are supported, but more are on the
way.
