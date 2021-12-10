#include <log.h>

void logBoundHoleDrop(int pid, const char*reason) {
//     FILE *fp = fopen("TraceBoundHole.tr", "a");
//     fprintf(fp, "D %s %d %s ", SIMTIME_STR(getSimulation()->getSimTime()), pid, reason);
// //  for(auto n : data) {
// //      fprintf(fp, "%d,", n.id());
// //  }
//     fprintf(fp, "\n");
//     fclose(fp);
}

void logBoundHoleRecv(int pid, int myid) {
    // FILE *fp = fopen("TraceBoundHole.tr", "a");
    // fprintf(fp, "R %s %d:%d\n", SIMTIME_STR(getSimulation()->getSimTime()), pid, myid);
    // fclose(fp);
}

void logSend(cMessage *msg, int ID) {
    // FILE *fp = fopen("Trace.tr", "a");
    // fprintf(fp, "S %ld\t%s\t%d\n", msg->getTreeId(), SIMTIME_STR(getSimulation()->getSimTime()), ID);
    // fclose(fp);
}

// void logSendHello(BaseMessage *msg, int senderID) {
//     FILE *fp = fopen("Trace.tr", "a");
//     fprintf(fp, "SEND\tHELLO\t%s\t%s\t%d\n", msg->getPacketID(), SIMTIME_STR(getSimulation()->getSimTime()), senderID);
//     fclose(fp);
// }


// void logDrop(BaseMessage *msg, Node stuckNode, const char* reason) {
//     FILE *fp = fopen("Trace.tr", "a");
//     fprintf(fp, "DROP_%s\tDATA\t%s\t%d\t%f\t%f\t%s\n", reason, msg->getPacketID(), stuckNode.id(), stuckNode.x(), stuckNode.y(), SIMTIME_STR(getSimulation()->getSimTime()));
//     fclose(fp);
// }

// void logSendData(BaseMessage *msg, Node sender, int receiverID) {
//     FILE *fp = fopen("Trace.tr", "a");
//     fprintf(fp, "SEND\tDATA\t%s\t%d-->%d\t%f\t%f\t%s\n", msg->getPacketID(), sender.id(), receiverID, sender.x(), sender.y(), SIMTIME_STR(getSimulation()->getSimTime()));
//     fclose(fp);
// }

// void logRecvHello(BaseMessage *msg, int senderID, int receiverID) {
//     FILE *fp = fopen("Trace.tr", "a");
//     fprintf(fp, "RECV\tHELLO\t%s\t%d-->%d\t%s\n", msg->getPacketID(), senderID, receiverID, SIMTIME_STR(getSimulation()->getSimTime()));
//     fclose(fp);
// }



