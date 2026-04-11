#ifndef SERVER_APP_H
#define SERVER_APP_H

#include "sim_frame.h"
#include "sim_host.h"

/* n：接收主机（用于打印节点 id）；in：经整条链路传递后的 SimFrame */
void server_receive(const SimHost *n, const SimFrame *in);

#endif
