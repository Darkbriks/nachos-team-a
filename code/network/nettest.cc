// nettest.cc 
//	Test out message delivery between two "Nachos" machines,
//	using the Post Office to coordinate delivery.
//
//	Two caveats:
//	  1. Two copies of Nachos must be running, with machine ID's 0 and 1:
//		./nachos -m 0 -o 1 &
//		./nachos -m 1 -o 0 &
//
//	  2. You need an implementation of condition variables,
//	     which is *not* provided as part of the baseline threads 
//	     implementation.  The Post Office won't work without
//	     a correct implementation of condition variables.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"

#include "system.h"
#include "network.h"
#include "post.h"
#include "reliablepost.h"
#include "interrupt.h"
#include "thread.h"
#include "machine.h"
#include "reliablepost.h"

// Test out message delivery, by doing the following:
//	1. send a message to the machine with ID "farAddr", at mail box #0
//	2. wait for the other machine's message to arrive (in our mailbox #0)
//	3. send an acknowledgment for the other machine's message
//	4. wait for an acknowledgement from the other machine to our 
//	    original message

void MailTest(int farAddr)
{
    PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;
    const char *data = "Hello there!";
    const char *ack = "Got it!";
    char buffer[MaxMailSize];


     for (int i = 0; i<10; i++){
        // construct packet, mail header for original message
        // To: destination machine, mailbox 0
        // From: our machine, reply to: mailbox 1
        outPktHdr.to = farAddr;		
        outMailHdr.to = 0;
        outMailHdr.from = 1;
        outMailHdr.length = strlen(data) + 1;

        // Send the first message
        postOffice->Send(outPktHdr, outMailHdr, data); 

        // Wait for the first message from the other machine
        postOffice->Receive(0, &inPktHdr, &inMailHdr, buffer);
        printf("Got \"%s\" from %d, box %d\n",buffer,inPktHdr.from,inMailHdr.from);
        fflush(stdout);

        Delay(2);
        // Send acknowledgement to the other machine (using "reply to" mailbox
        // in the message that just arrived
        outPktHdr.to = inPktHdr.from;
        outMailHdr.to = inMailHdr.from;
        outMailHdr.length = strlen(ack) + 1;
        postOffice->Send(outPktHdr, outMailHdr, ack); 

        // Wait for the ack from the other machine to the first message we sent.
        postOffice->Receive(1, &inPktHdr, &inMailHdr, buffer);
        // printf("Got \"%s\" from %d, box %d\n",buffer,inPktHdr.from,inMailHdr.from);
        fflush(stdout);
     }

    // Then we're done!
    interrupt->Halt();
}


//  Test ring topology : token passing between n machines
//  Machine start by sending initial token then each machine forwards it to the next one
//  Test ends when token returns to machine 0

void RingTest(int myAddr, int numMachines) {
    PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;

    const char * token = "TOKEN";
    char buffer[MaxMailSize];

    int nextAddr = (myAddr + 1) % numMachines;

    if (myAddr == 0) { // Machine 0 ( init token) 
        printf("Machine %d : Sending initial token to machine %d.\n", myAddr, nextAddr);
        fflush(stdout);

        outPktHdr.to = nextAddr;		
        outMailHdr.to = 0;
        outMailHdr.from = myAddr;
        outMailHdr.length = strlen(token) + 1;

        postOffice->Send(outPktHdr, outMailHdr, token);

        // Wait for round trip
        postOffice->Receive(0, &inPktHdr, &inMailHdr, buffer);
        printf("Machine %d: Token returned, ring tour completed !\n", myAddr);
        fflush(stdout);

    } else { // Other machines, just hold a moment then forward the token
        postOffice->Receive(0, &inPktHdr, &inMailHdr, buffer);

        printf("Machine %d : Received token from machine %d.\n", myAddr, inPktHdr.from);
        fflush(stdout);

        Delay(5);

        printf("Machine %d: Forwarding token to machine %d\n", myAddr, nextAddr);
        fflush(stdout);


        outPktHdr.to = nextAddr;		
        outMailHdr.to = 0;
        outMailHdr.from = myAddr;
        outMailHdr.length = strlen(buffer) + 1;

        postOffice->Send(outPktHdr, outMailHdr, buffer);
    }
    Delay(5);
    interrupt->Halt();
}


void ReliableMailTest(int farAddr) {
    PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;
    const char *data = "Hello reliable!";
    char buffer[MaxMailSize];

    // Create ReliablePost wrapper around the existing PostOffice
    ReliablePost *reliablePost = new ReliablePost(postOffice);

    printf("ReliableMailTest: Starting event-driven test with machine %d\n", farAddr);
    fflush(stdout);

    // EVENT-DRIVEN TEST - No blocking waits!
    for (int i = 0; i < 20; i++) {
        outPktHdr.to = farAddr;
        outMailHdr.to = MAIL_BOX;
        outMailHdr.from = ACK_BOX;
        outMailHdr.length = strlen(data) + 1;

        // Send message (NON-BLOCKING - returns immediately with seqNum)
        unsigned int seqNum = reliablePost->SendReliable(outPktHdr, outMailHdr, data);

        // EVENT LOOP: Process network events while waiting for ACK
        while (!reliablePost->IsAcked(seqNum)) {
            // Let other threads (including postal worker) run
            currentThread->Yield();

            // Process ACKs and retransmissions
            reliablePost->ProcessEvents();

            // Also check for incoming data messages (non-blocking)
            if (postOffice->HasMessages(MAIL_BOX)) {
                reliablePost->ReceiveReliable(MAIL_BOX, &inPktHdr, &inMailHdr, buffer);
                printf("Got \"%s\" from %d, box %d\n", buffer, inPktHdr.from, inMailHdr.from);
                fflush(stdout);
            }
        }

        printf("[Machine %d] Message %d ACKed!\n", postOffice->GetNetAddr(), i + 1);
        fflush(stdout);
    }

    // Wait for any remaining pending messages
    reliablePost->WaitForPending();

    printf("ReliableMailTest: Test completed!\n");
    fflush(stdout);

    delete reliablePost;
    interrupt->Halt();
}

