#include <stdio.h>
#include <stdlib.h>
#include "project2.h"

/* ***************************************************************************
 ALTERNATING BIT AND GO-BACK-N NETWORK EMULATOR: VERSION 1.1  J.F.Kurose

   This code should be used for unidirectional or bidirectional
   data transfer protocols from A to B and B to A.
   Network properties:
   - one way network delay averages five time units (longer if there
     are other messages in the channel for GBN), but can be larger
   - packets can be corrupted (either the header or the data portion)
     or lost, according to user-defined probabilities
   - packets may be delivered out of order.

   Compile as gcc -g project2.c student2.c -o p2
**********************************************************************/

// function prototypes
int calc_checksum(struct pkt packet, size_t packet_length);
void A_output(struct msg message);
void A_input(struct pkt packet);
void A_timerinterrupt();
void A_init()
void B_output(struct msg message);
void B_input(struct pkt packet);
void B_timerinterrupt();
void B_init();

// arbitary sequence number assignment
#define ARBITRARY_ACK_NUM 1000;
// modulo number for custom checksum equation
#define CUSTOM_CHECKSUM_NUM 255;
// size of window for sending packets based on sequence number
#define WINDOW_SIZE 8;
// size of time increment for timer (1000 in porject specificaitons)
#define TIME_INCREMENT 1000;
// size of message or packet payload
#define PKT_SIZE 20;

// the next sequence number that will be assigned to next packet
int intital_seq_num;
// the last ACK number that was received
int last_ack_num
// next expected sequence number by B-side
int B_expected_seq_num;
// index for front of packet sending queue window
int window_index;

// storage for packets received at A-side layer 5, ready to be sent
struct pkt A_storage[100];


/********* STUDENTS WRITE THE NEXT SEVEN ROUTINES *********/
/*
 * The routines you will write are detailed below. As noted above,
 * such procedures in real-life would be part of the operating system,
 * and would be called by other procedures in the operating system.
 * All these routines are in layer 4.
 */

/*
 * A_output(message), where message is a structure of type msg, containing
 * data to be sent to the B-side. This routine will be called whenever the
 * upper layer at the sending side (A) has a message to send. It is the job
 * of your protocol to insure that the data in such a message is delivered
 * in-order, and correctly, to the receiving side upper layer.
 */

/****************************** A_output ***********************************/
void A_output(struct msg message) {
  // packet to be sent to B-side
  struct pkt pkt_to_send;
  // filling the packet with its relative contents
  pkt_to_send.seqnum = intital_seq_num;
  pkt_to_send.acknum = ARBITRARY_ACK_NUM;

  // copying the message data to the packet to be sent (pkt_to_send)
  int i = 0;
  for (i = 0; i < PKT_SIZE; i++) {
    pkt_to_send.payload[i] = message.data[i];
  }

  int checksum = calc_checksum(pkt_to_send, PKT_SIZE);
  printf("A: new packet has checksum: %d  \n", checksum);

  // adding the packet to be sent (pkt_to_send) to the storage (A_storage)
  A_storage[intital_seq_num] = pkt_to_send;

  // starting the time if necessary, if the packet is the first packet being sent inside the queue window
  if (intital_seq_num == window_index) {
    // time increment set to 1000 ("a large number")
    startTimer(AEntity, TIME_INCREMENT);
  }

  // sending the packet to B-side if the sequence number is inside the queue window
  if (intital_seq_num < (window_index + WINDOW_SIZE)) {
    tolayer3(AEntity, A_storage[intital_seq_num]);
  }

  // increment the sequence number after the packet is sent
  intital_seq_num++;
}

/*
 * Just like A_output, but residing on the B side.  USED only when the
 * implementation is bi-directional.
 */
/****************************** B_output ***********************************/
void B_output(struct msg message)  {
}

/*
 * A_input(packet), where packet is a structure of type pkt. This routine
 * will be called whenever a packet sent from the B-side (i.e., as a result
 * of a tolayer3() being done by a B-side procedure) arrives at the A-side.
 * packet is the (possibly corrupted) packet sent from the B-side.
 */
/****************************** A_input ***********************************/
void A_input(struct pkt packet) {
  // calculating the checksum
  int checksum = calc_checksum(packet, PKT_SIZE);
  printf("A: received packet has checksum: %d  \n", checksum);
  // checking if the checksum is correct
  if (packet.checksum != checksum) {
    // hanging until there is a timeout
    return;
  }

  // checking ACK number
  if (packet.acknum == 1) {
    // checking to see if there are any more packets in the queue window
    if (packet.acknum + 1 <= window_index) {
      // if the current packet is 1 before the window ends then stop timer
      if (packet.seqnum + 1 == window_index) {
        stoptimer(AEntity);
      }
    }
    // if there are no more pakets in the queue window then slide the window forwards
    else {
      window_index = packet.sequence + 1;
      stoptimer(AEntity);

      // if there are still more packets to send, then restart the timer
      if (window_index < intital_seq_num) {
        starttimer(AEntity, TIME_INCREMENT);
      }
    }
  }
}

/*
 * A_timerinterrupt()  This routine will be called when A's timer expires
 * (thus generating a timer interrupt). You'll probably want to use this
 * routine to control the retransmission of packets. See starttimer()
 * and stoptimer() in the writeup for how the timer is started and stopped.
 */
void A_timerinterrupt() {

}

/* The following routine will be called once (only) before any other    */
/* entity A routines are called. You can use it to do any initialization */
void A_init() {
}


/*
 * Note that with simplex transfer from A-to-B, there is no routine  B_output()
 */

/*
 * B_input(packet),where packet is a structure of type pkt. This routine
 * will be called whenever a packet sent from the A-side (i.e., as a result
 * of a tolayer3() being done by a A-side procedure) arrives at the B-side.
 * packet is the (possibly corrupted) packet sent from the A-side.
 */
void B_input(struct pkt packet) {
}

/*
 * B_timerinterrupt()  This routine will be called when B's timer expires
 * (thus generating a timer interrupt). You'll probably want to use this
 * routine to control the retransmission of packets. See starttimer()
 * and stoptimer() in the writeup for how the timer is started and stopped.
 */
void  B_timerinterrupt() {
}

/*
 * The following routine will be called once (only) before any other
 * entity B routines are called. You can use it to do any initialization
 */
void B_init() {
}

int calc_checksum(struct pkt packet, size_t packet_length) {
  int checksum;

  int i = 0;
  for (i = 0; i < packet_length; i++) {
    checksum += packet.payload[i] % CUSTOM_CHECKSUM_NUM;
  }
  checksum += packet.seqnum;
  return checksum;
}
