#ifndef SERVER_APP_H
#define SERVER_APP_H

#include "sim_frame.h"
#include "sim_host.h"

void server_receive(const SimHost *n, const SimFrame *in);

#endif
