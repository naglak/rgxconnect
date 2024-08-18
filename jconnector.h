/*
 * Copyright (C) 2023, 2024 Jacek Naglak
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef JCONNECTOR_H
#define JCONNECTOR_H

#include <jack/jack.h>
#include <jack/types.h>
#include <pthread.h>
#include <string>
#include <vector>

using namespace std;

class JackConnector {
private:
  vector<string> key1;
  vector<string> key2;
  jack_client_t *client;
  jack_port_id_t jack_port;
  pthread_t thread_id;
  pthread_mutex_t thread_mutex;
  pthread_mutex_t callback_mutex;
  pthread_cond_t thread_cv;
  pthread_cond_t callback_cv;
  bool callback_guard = false;
  bool thread_guard = false;

public:
  JackConnector(const char *clientname, const char *servername);
  virtual ~JackConnector();

  void add_rule(string, string);
  void launch();

  static void jack_reg_callback(jack_port_id_t port, int registered, void *arg);
  static void jack_shutdown_callback(void *arg);
  static void *run_thread(void *arg);

private:
  void jack_reg_callback_prv(jack_port_id_t, void *);
  void connect(jack_port_id_t);
};

#endif // JCONNECTOR_H
