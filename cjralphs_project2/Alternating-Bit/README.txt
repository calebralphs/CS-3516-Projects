Caleb Ralphs
cjralphs@wpi.edu

To run the alternating-bit:
  1. cd into the project2 folder
  2. type "make" to compile the program (you can type "make clean", if there are extra files)
  3. type "./project2" to run the program
  4. enter the specifications of the probabilities, tracing level, etc.
    Ultimately to test my program, my input was as followed

    Enter the number of messages to simulate: 10
  	Packet loss probability [enter number between 0.0 and 1.0]: .2
  	Packet corruption probability [0.0 for no corruption]: .2
  	Packet out-of-order probability [0.0 for no out-of-order]: .2
  	Average time between messages from sender's layer5 [ > 0.0]: 10
  	Enter Level of tracing desired: 1
  	Do you want actions randomized: (1 = yes, 0 = no)? 1
  	Do you want Bidirectional: (1 = yes, 0 = no)? 0

    The program runs best with:
      Average time between messages from sender's layer5 [ > 0.0]: 10
