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

// single-linked list node structs
struct Node {
  struct pkt packet;
  struct Node* next;
};

// head and tail nodes for the linked list of nodes
struct Node* head = NULL;
struct Node* tail = NULL;

// function prototypes
int calc_checksum(struct pkt packet, size_t packet_length);
void queue(struct pkt packet);
void dequeue();
struct pkt head_queue();
void print_queue();

void A_output(struct msg message);
void A_input(struct pkt packet);
void A_timerinterrupt();
void A_init();
void B_output(struct msg message);
void B_input(struct pkt packet);
void B_timerinterrupt();
void B_init();

// modulo number for custom checksum equation
int custom_checksum_num = 255;
// size of time increment for timer (1000 in debugging, 20 regualry)
int time_increment = 1000;
// size of message or packet payload
int pkt_size = 20;

// the next sequence number that will be assigned to next packet
int initial_seq_num;
// expected ACK number of the sender
int send_exp_ACK_num;
// sequence number of the receiver
int recv_seq_num;
// whether or not sender is waiting for an ACK (1 = waiting, 0 = not waiting)
int is_first_send;

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

/****************************************** A_output ***********************************************/
void A_output(struct msg message) {
  // packet to be sent to B-side
  struct pkt pkt_to_send;
  // filling the packet with its relative contents
  pkt_to_send.seqnum = initial_seq_num;
  // should be the same as sequence number to start
  pkt_to_send.acknum = initial_seq_num;

  // copying the message data to the packet to be sent (pkt_to_send)
  int i = 0;
  for (i = 0; i < pkt_size; i++) {
    pkt_to_send.payload[i] = message.data[i];
  }

  // calculating the checksum and assinging it to packet to be sent (pkt_to_send)
  int checksum = calc_checksum(pkt_to_send, pkt_size);
  pkt_to_send.checksum = checksum;
  printf("A: new packet has checksum: %d  \n", pkt_to_send.checksum);

  // add the packet (pkt_to_send) to the tail of the queue
  queue(pkt_to_send);
  printf("A: ");
  print_queue();

  // if A-side is not waiting for an ACK
  if (is_first_send == FALSE) {
    tolayer3(AEntity, head_queue());
    // start the timer now that the packet has been sent to layer 3 A-side
    startTimer(AEntity, time_increment);
    is_first_send = TRUE;
  }

  // changing sequence number 0 -> 1 or 1-> 0
  if (initial_seq_num == 1) {
    initial_seq_num = 0;
  }
  else if (initial_seq_num == 0) {
    initial_seq_num = 1;
  }
  else {
    printf("ERROR: sequence number is %d not following Alt-Bit Protocol.\n", initial_seq_num);
  }
}

/*
 * Just like A_output, but residing on the B side.  USED only when the
 * implementation is bi-directional.
 */
/****************************************** B_output ***********************************************/
void B_output(struct msg message)  {
}

/*
 * A_input(packet), where packet is a structure of type pkt. This routine
 * will be called whenever a packet sent from the B-side (i.e., as a result
 * of a tolayer3() being done by a B-side procedure) arrives at the A-side.
 * packet is the (possibly corrupted) packet sent from the B-side.
 */
/****************************************** A_input *********************************************/
void A_input(struct pkt packet) {
  // printing packet sequence number and sender sequence number
  printf("A: packet ACK number: %d, sender expected ACK number: %d\n", packet.acknum, send_exp_ACK_num);

    // if the ACK number matches the sequence number of the sender (send_exp_ACK_num) and checksums match up
    if (packet.acknum == send_exp_ACK_num) {
      // dequeue the packet and stop the timer
      stopTimer(AEntity);
      dequeue();
      // if the linked list isn't empty
      if (head != NULL) {
        // sends the next packet in the queue because the ACK number matched correctly
        tolayer3(AEntity, head_queue());
        startTimer(AEntity, time_increment);
        printf("A: sending the next packet with checksum: %d\n", head_queue().checksum);
      }
      // changing sender sequence number 0 -> 1 or 1-> 0
      if (send_exp_ACK_num == 1) {
        send_exp_ACK_num = 0;
      }
      else if (send_exp_ACK_num == 0) {
        send_exp_ACK_num = 1;
      }
      else {
        printf("ERROR: sequence number is %d not following Alt-Bit Protocol.\n", send_exp_ACK_num);
      }
    }
    // if the ACK number does not match the sequnce number (i.e packet loss); correct checksum, but worng sequence number
    else {
      if (head != NULL) {
        // recalculate the checksum
        int checksum = calc_checksum(head->packet, pkt_size);
        head->packet.checksum = checksum;
        // sending the next packet again
        A_timerinterrupt();
        printf("A: sending the next packet with checksum: %d\n", head_queue().checksum);
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
  printf("********************\nA: timer interupt\n********************\n");
  printf("A: resending packet with checksum: %d\n", head_queue().checksum);
  // resending packet at the head of the queue
  tolayer3(AEntity, head_queue());
  startTimer(AEntity, time_increment);
}

/* The following routine will be called once (only) before any other    */
/* entity A routines are called. You can use it to do any initialization */
void A_init() {
  // sender sequence number initalized to zero and there is no ACK to be waited upon
  send_exp_ACK_num = 0;
  initial_seq_num = 0;
  is_first_send = FALSE;
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
  printf("B: packet ACK number: %d, receiver sequence number (expected ACK number): %d\n", packet.acknum, recv_seq_num);

  // calculating the checksum for the received packet
  int checksum = calc_checksum(packet, pkt_size);
  printf("B: received packet checksum: %d\n", checksum);

  // if the observed checksum doesn't match the expected checksum
  if (packet.checksum != checksum) {
    printf("\n********************\nB: recieved packet's checksum differs from expected checksum\n********************\n\n");
    // printing out the two checksums if they are different
    printf("***** packet observed checksum: %d\n", packet.checksum);
    printf("***** packet expected checksum: %d\n", checksum);
    // changing ACK number 0 -> 1 or 1 -> 0
    if (packet.acknum == 1) {
      packet.acknum = 0;
    }
    else if (packet.acknum == 0) {
      packet.acknum = 1;
    }
    else {
      printf("ERROR: packet ACK number is %d not following Alt-Bit Protocol.\n", packet.acknum);
    }

    // sending a NACK
    tolayer3(BEntity, packet);
  }
  // if the observed checksum matches the expected checksum
  else {
    // check if the seq number is correct
    if (packet.acknum == recv_seq_num) {
      // convert the array of characters to a message
      struct msg message;
      int i;
      for (i = 0; i < pkt_size; i++) {
        message.data[i] = packet.payload[i];
      }
      // sending the message up to layer 5
      tolayer5(BEntity, message);
      // send the positive ACK back to A-side
      tolayer3(BEntity, packet);

      // changing receiver sequence number 0 -> 1 or 1-> 0
      if (recv_seq_num == 1) {
        recv_seq_num = 0;
      }
      else if (recv_seq_num == 0) {
        recv_seq_num = 1;
      }
      else {
        printf("ERROR: sequence number is %d not following Alt-Bit Protocol.\n", recv_seq_num);
      }
    }
    else {
      // sending a NACK, ACK number is already corrupted
      tolayer3(BEntity, packet);
    }
  }
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
  // initializing the receiver sequence number to zero
  recv_seq_num = 0;
}

// calculates the custome checksum for a given packet
// found the "The 16-bit Fletcher Checksum Algorithm" online and modified it
int calc_checksum(struct pkt packet, size_t packet_length) {
  int sum1 = 0, sum2 = 0;

  int i;
  for(i = 0; i < packet_length; i++){
    sum1 += packet.payload[i] % custom_checksum_num;
    sum2 += sum1 % custom_checksum_num;
  }

  int sum = sum1 + sum2;
  return sum;
}

// queues up a packet at the tail of the queue
void queue(struct pkt packet) {
  // allocated the necessary memory for this packet to be added to the queue
  struct Node* temp = (struct Node*)malloc(sizeof(struct Node));
  temp->packet = packet;
  temp->next = NULL;
  // if the queue is empty to start
  if (head == NULL && tail == NULL) {
    head = temp;
    tail = temp;
    return;
  }
  // make the tail the packet passed in the function
  tail->next = temp;
  tail = temp;
}

void dequeue() {
  struct Node* temp = head;
  // if the queue is empty
  if (head == NULL) {
    printf("Queue: queue is already empty.\n");
    return;
  }
  // if there is only one packet left in the queue
  else if (head == tail) {
    head = NULL;
    tail = NULL;
    printf("Queue: the queue is now empty.\n");
  }
  else {
    // makes the next packet in the queue the head
    head = head->next;
  }
  // free up the memory allocated for that packet in the queue
  free(temp);
}

// returns the packet at the head of the queue as long as the queue is not empty
struct pkt head_queue() {
  if (head != NULL) {
    // returns the packet at the head of the queue
    return head->packet;
  }
  // queue is empty
  else {
    printf("Queue: the queue is empty (no head).\n");
    return;
  }
}

// prints out the checksums of all the packets currently in the queue starting at the head
void print_queue() {
  // starting at head of queue
  struct Node* temp = head;
  printf("Queue: checksums: \n");
  // goes through whole queue until null pointer printing the checksums of each packet
  while(temp != NULL) {
    struct pkt temp_pkt = temp->packet;
    int checksum = temp_pkt.checksum;
    printf("                 %d \n", checksum);
    temp = temp->next;
  }
  printf("\n");
}
