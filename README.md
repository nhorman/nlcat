# nlcat
netlink monitoring and dispatch service

## What is nlcat
In short, nlcat is netcat for netlink sockets.  The utility listens for various
netlnk events and dispatches notification applications in response to them,
passing the netlink data to them on stdin (massaged into a json format for easy
consumption.  This is useful when a platform wishes to do event based monitoring
for things like ip address changes, route additions, etc.
