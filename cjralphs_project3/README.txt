Caleb Ralphs
cjralphs@wpi.edu

To run the program:
  1. cd into the project3 folder
  2. type "make" to compile the program (you can type "make clean", if there are extra files)
  3. type "./project3" to run the program
  4. enter the specified tracing level (testing with < 2)
    Enter Trace Level: 1

Notes:
I added a 'print_mincost' function to the project3.c file and its prototype to the project3.h file.
This function merely prints the array that is the 'mincost' array that is in the 'RoutePacket' struct.
I wanted to be able to use the function across each Node.N file to reduce code redundency.

I also provided a little extra output trace, so you know what packet is being routed where at what time exactly.
I just did this to reduce ambiguity in the packet routing from node to node. It looks a little odd at the end,
but that is just because we have no sense in implementing a sort of timerinterupt function to know if we have
already checked a certain node. If we did then we could reduce the amount of double-contacts that a node with
multiple neighbors would receive.
