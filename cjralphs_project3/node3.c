#include <stdio.h>
#include "project3.h"

extern int TraceLevel;
extern int clocktime;

struct distance_table {
  int costs[MAX_NODES][MAX_NODES];
};
struct distance_table dt3;
struct NeighborCosts   *neighbor3;
struct RoutePacket route_packet;

void printdt3(int MyNodeNumber, struct NeighborCosts *neighbor,
		struct distance_table *dtptr);

/* students to write the following two routines, and maybe some others */

void rtinit3() {
  // printing requested output trace
  printf("rtinit3():     initializing distance table for node 3 at time: %d.\n", clocktime);

  // initializing all of the nodes to not be connected (i.e infite, 9999 connections),
  // excluding the index 3 for obvious reasons
  int to, via;
  for (to = 0; to < MAX_NODES; to++) {
    if (to == 3) {
      continue;
    }
    for (via = 0; via < MAX_NODES; via++) {
      if (via == 3) {
        continue;
      }
      dt3.costs[to][via] = INFINITY;
    }
  }

  // fetching information of the confirguration of the network and the weight between each nodes
  neighbor3 = getNeighborCosts(3);
  // inputing the information on neighboring nodes into the respective distance table
  int node;
  for (node = 0; node < MAX_NODES; node++) {
    if (node == 3) {
      continue;
    }
    // initialize the minimum node costs for the routing packet
    route_packet.mincost[node] = neighbor3->NodeCosts[node];
    // if the node cost between node 3 and this node is less than infinity (9999)
    if (neighbor3->NodeCosts[node] < dt3.costs[node][node]) {
      dt3.costs[node][node] = neighbor3->NodeCosts[node];
    }
    // otherwise we can keep this node cost between them at the initialized at infinity (9999)
  }

  // printing the distance table for node 3
  printdt3(3, neighbor3, &dt3);

  // sending the routing packet with its associated minimum path costs
  int node_num;
  for (node_num = 0; node_num < MAX_NODES; node_num++) {
    if (node_num == 3) {
      continue;
    }
    if (dt3.costs[node_num][node_num] < INFINITY) {
      route_packet.sourceid = 3;
      route_packet.destid = node_num;
      // printing requested output trace
      printf("               sending routing packet from source: %d, to destination: %d, at time: %d.\n", 3, node_num, clocktime);
      toLayer2(route_packet);
    }
  }
}


void rtupdate3( struct RoutePacket *rcvdpkt ) {
  // extracting information from received routing packet
  int source_id = rcvdpkt->sourceid;
  int dest_id = rcvdpkt->destid;
  // printing requested output trace
  printf("rtupdate3():   updating distance table for node 3 at time: %d.\n", clocktime);

  // to keep track if there is a change in the distance table
  int changed = NO;
  // checks to see if the distance table needs to be updated
  int to;
  for (to = 0; to < MAX_NODES; to++) {
    if (to == 3) {
      continue;
    }
    // if the distance to certain node plus distance back to source is less than current distance in table
    if (rcvdpkt->mincost[to] + dt3.costs[source_id][source_id] < dt3.costs[to][source_id]) {
      // change the value in the table to this new sum
      dt3.costs[to][source_id] = rcvdpkt->mincost[to] + dt3.costs[source_id][source_id];
      changed = YES;
    }
  }

  // if there has been a change in the distance table
  if (changed) {
    // printing the distance table for node 3
    printf("               there was a change in the distance table for node 3.\n");
    printdt3(3, neighbor3, &dt3);
    // checking if we need to change the 'mincost' for the route packet
    int to, via;
    for (to = 0; to < MAX_NODES; to++) {
      if (to == 3) {
        continue;
      }
      for (via = 0; via < MAX_NODES; via++) {
        if (via == 3) {
          continue;
        }
        // if the distance in the table is less than the one in the 'mincost' for the route packet
        if (dt3.costs[to][via] < route_packet.mincost[to]) {
          route_packet.mincost[to] = dt3.costs[to][via];
          print_mincost(rcvdpkt);
        }
      }
    }

    // sending updates to all the neighboring nodes
    int node_num;
    for (node_num = 0; node_num < MAX_NODES; node_num++) {
      if (node_num == 3) {
        continue;
      }
      if (dt3.costs[node_num][node_num] < INFINITY) {
        route_packet.sourceid = 3;
        route_packet.destid = node_num;
        // printing requested output trace
        printf("               sending updated routing packet from source: %d, to destination: %d, at time: %d.\n", 3, node_num, clocktime);
        toLayer2(route_packet);
      }
    }
  }
}


/////////////////////////////////////////////////////////////////////
//  printdt
//  This routine is being supplied to you.  It is the same code in
//  each node and is tailored based on the input arguments.
//  Required arguments:
//  MyNodeNumber:  This routine assumes that you know your node
//                 number and supply it when making this call.
//  struct NeighborCosts *neighbor:  A pointer to the structure
//                 that's supplied via a call to getNeighborCosts().
//                 It tells this print routine the configuration
//                 of nodes surrounding the node we're working on.
//  struct distance_table *dtptr: This is the running record of the
//                 current costs as seen by this node.  It is
//                 constantly updated as the node gets new
//                 messages from other nodes.
/////////////////////////////////////////////////////////////////////
void printdt3( int MyNodeNumber, struct NeighborCosts *neighbor,
		struct distance_table *dtptr ) {
    int       i, j;
    int       TotalNodes = neighbor->NodesInNetwork;     // Total nodes in network
    int       NumberOfNeighbors = 0;                     // How many neighbors
    int       Neighbors[MAX_NODES];                      // Who are the neighbors

    // Determine our neighbors
    for ( i = 0; i < TotalNodes; i++ )  {
        if (( neighbor->NodeCosts[i] != INFINITY ) && i != MyNodeNumber )  {
            Neighbors[NumberOfNeighbors] = i;
            NumberOfNeighbors++;
        }
    }
    // Print the header
    printf("                via     \n");
    printf("   D%d |", MyNodeNumber );
    for ( i = 0; i < NumberOfNeighbors; i++ )
        printf("     %d", Neighbors[i]);
    printf("\n");
    printf("  ----|-------------------------------\n");

    // For each node, print the cost by travelling thru each of our neighbors
    for ( i = 0; i < TotalNodes; i++ )   {
        if ( i != MyNodeNumber )  {
            printf("dest %d|", i );
            for ( j = 0; j < NumberOfNeighbors; j++ )  {
                    printf( "  %4d", dtptr->costs[i][Neighbors[j]] );
            }
            printf("\n");
        }
    }
    printf("\n");
}    // End of printdt3
