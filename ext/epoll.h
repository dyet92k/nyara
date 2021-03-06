/* epoll event adapter */

#pragma once

#include <sys/epoll.h>

static struct epoll_event qevents[MAX_E];

static void ADD_E(int fd, VALUE rid) {
  struct epoll_event e;
  e.events = EPOLLIN | EPOLLOUT | EPOLLET;
  e.data.u64 = (uint64_t)rid;

  if (epoll_ctl(q.fd, EPOLL_CTL_ADD, fd, &e))
    rb_sys_fail("epoll_ctl(2) - EPOLL_CTL_ADD");
}

// NOTE either epoll or kqueue removes the event watch from queue when fd closed
static void DEL_E(int fd) {
  struct epoll_event e;
  e.events = EPOLLIN | EPOLLOUT;
  if (epoll_ctl(q.fd, EPOLL_CTL_DEL, fd, &e))
    rb_sys_fail("epoll_ctl(2) - EPOLL_CTL_DEL");
}

static void INIT_E() {
  q.fd = epoll_create(10); // size not important
  if (q.fd == -1) {
    rb_sys_fail("epoll_create(2)");
  }
}

static int SELECT_E(st_table* rids) {
  // heart beat of 0.1 sec, allow ruby signal interrupts to be inserted
  int sz = epoll_wait(q.fd, qevents, MAX_E, 100);
  int accept_sz = 0;

  for (int i = 0; i < sz; i++) {
    if (qevents[i].events & (EPOLLIN | EPOLLOUT)) {
      VALUE rid = (VALUE)qevents[i].data.u64;
      if (rid == sym_accept) {
        accept_sz++;
      } else {
        st_insert(rids, rid, 0);
      }
    }
    // do sth to EPOLLHUP | EPOLLERR | EPOLLRDHUP ?
  }

  return accept_sz;
}
